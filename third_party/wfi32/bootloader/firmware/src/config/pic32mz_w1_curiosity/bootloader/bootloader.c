/*******************************************************************************
  UART Bootloader Source File

  File Name:
    bootloader.c

  Summary:
    This file contains source code necessary to execute UART bootloader.

  Description:
    This file contains source code necessary to execute UART bootloader.
    It implements bootloader protocol which uses UART peripheral to download
    application firmware into internal flash from HOST-PC.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *******************************************************************************/
// DOM-IGNORE-END

#include "definitions.h"
#include <device.h>

#define FLASH_START             (0x90000000UL)
#define FLASH_LENGTH            (0x100000UL)
#define PAGE_SIZE               (1024UL)
#define ERASE_BLOCK_SIZE        (4096UL)
#define PAGES_IN_ERASE_BLOCK    (ERASE_BLOCK_SIZE / PAGE_SIZE)

#define BOOTLOADER_SIZE         8192

#define APP_START_ADDRESS       (PA_TO_KVA0(0x10000000UL))

#define GUARD_OFFSET            0
#define CMD_OFFSET              2
#define ADDR_OFFSET             0
#define SIZE_OFFSET             1
#define DATA_OFFSET             1
#define CRC_OFFSET              0

#define CMD_SIZE                1
#define GUARD_SIZE              4
#define SIZE_SIZE               4
#define OFFSET_SIZE             4
#define CRC_SIZE                4
#define HEADER_SIZE             (GUARD_SIZE + SIZE_SIZE + CMD_SIZE)
#define DATA_SIZE               ERASE_BLOCK_SIZE

#define WORDS(x)                ((int)((x) / sizeof(uint32_t)))

#define OFFSET_ALIGN_MASK       (~ERASE_BLOCK_SIZE + 1)
#define SIZE_ALIGN_MASK         (~PAGE_SIZE + 1)

#define BTL_GUARD               (0x5048434DUL)

/* Compare Value to achieve a 100Ms Delay */
#define TIMER_COMPARE_VALUE     (CORE_TIMER_FREQUENCY / 10)

enum
{
    BL_CMD_UNLOCK       = 0xa0,
    BL_CMD_DATA         = 0xa1,
    BL_CMD_VERIFY       = 0xa2,
    BL_CMD_RESET        = 0xa3,
};

enum
{
    BL_RESP_OK          = 0x50,
    BL_RESP_ERROR       = 0x51,
    BL_RESP_INVALID     = 0x52,
    BL_RESP_CRC_OK      = 0x53,
    BL_RESP_CRC_FAIL    = 0x54,
};

static uint32_t CACHE_ALIGN input_buffer[WORDS(OFFSET_SIZE + DATA_SIZE)];

static uint32_t CACHE_ALIGN flash_data[WORDS(DATA_SIZE)];

static uint32_t flash_addr          = 0;

static uint32_t unlock_begin        = 0;
static uint32_t unlock_end          = 0;

static uint8_t  input_command       = 0;

static bool     packet_received     = false;
static bool     flash_data_ready    = false;


/* Function to Send the final response for reset command and trigger a reset */
static void trigger_Reset(void)
{
    UART1_WriteByte(BL_RESP_OK);

    while(UART1_TransmitComplete() == false);

    /* Perform system unlock sequence */ 
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;

    RSWRSTSET = _RSWRST_SWRST_MASK;
    (void)RSWRST;
}

/* Function to Generate CRC by reading the firmware programmed into internal flash */
static uint32_t crc_generate(void)
{
    uint32_t   i, j, value;
    uint32_t   crc_tab[256];
    uint32_t   size    = unlock_end - unlock_begin;
    uint32_t   crc     = 0xffffffff;
    uint8_t    data;

    for (i = 0; i < 256; i++)
    {
        value = i;

        for (j = 0; j < 8; j++)
        {
            if (value & 1)
            {
                value = (value >> 1) ^ 0xEDB88320;
            }
            else
            {
                value >>= 1;
            }
        }
        crc_tab[i] = value;
    }

    for (i = 0; i < size; i++)
    {
        data = *(uint8_t *)KVA0_TO_KVA1((unlock_begin + i));

        crc = crc_tab[(crc ^ data) & 0xff] ^ (crc >> 8);
    }
    return crc;
}

/* Function to receive application firmware via UART/USART */
static void input_task(void)
{
    static uint32_t ptr             = 0;
    static uint32_t size            = 0;
    static bool     header_received = false;
    uint8_t         *byte_buf       = (uint8_t *)&input_buffer[0];
    uint8_t         input_data      = 0;

    if (packet_received == true)
    {
        return;
    }

    if (UART1_ReceiverIsReady() == false)
    {
        return;
    }

    input_data = UART1_ReadByte();

    /* Check if 100 ms have elapsed */
    if (CORETIMER_CompareHasExpired())
    {
        header_received = false;
    }

    if (header_received == false)
    {
        byte_buf[ptr++] = input_data;

        if (ptr == HEADER_SIZE)
        {
            if (input_buffer[GUARD_OFFSET] != BTL_GUARD)
            {
                UART1_WriteByte(BL_RESP_ERROR);
            }
            else
            {
                size            = input_buffer[SIZE_OFFSET];
                input_command   = (uint8_t)input_buffer[CMD_OFFSET];
                header_received = true;
            }

            ptr = 0;
        }
    }
    else if (header_received == true)
    {
        if (ptr < size)
        {
            byte_buf[ptr++] = input_data;
        }

        if (ptr == size)
        {
            ptr = 0;
            size = 0;
            packet_received = true;
            header_received = false;
        }
    }

    CORETIMER_Start();
}

/* Function to process the received command */
static void command_task(void)
{
    uint32_t i;

    if (BL_CMD_UNLOCK == input_command)
    {
        uint32_t begin  = (input_buffer[ADDR_OFFSET] & OFFSET_ALIGN_MASK);

        uint32_t end    = begin + (input_buffer[SIZE_OFFSET] & SIZE_ALIGN_MASK);

        if (end > begin && end <= (FLASH_START + FLASH_LENGTH))
        {
            unlock_begin = begin;
            unlock_end = end;
            UART1_WriteByte(BL_RESP_OK);
        }
        else
        {
            unlock_begin = 0;
            unlock_end = 0;
            UART1_WriteByte(BL_RESP_ERROR);
        }
    }
    else if (BL_CMD_DATA == input_command)
    {
        flash_addr = (input_buffer[ADDR_OFFSET] & OFFSET_ALIGN_MASK);

        if (unlock_begin <= flash_addr && flash_addr < unlock_end)
        {
            for (i = 0; i < WORDS(DATA_SIZE); i++)
                flash_data[i] = input_buffer[i + DATA_OFFSET];

            flash_data_ready = true;
        }
        else
        {
            UART1_WriteByte(BL_RESP_ERROR);
        }
    }
    else if (BL_CMD_VERIFY == input_command)
    {
        uint32_t crc        = input_buffer[CRC_OFFSET];
        uint32_t crc_gen    = 0;

        crc_gen = crc_generate();

        if (crc == crc_gen)
            UART1_WriteByte(BL_RESP_CRC_OK);
        else
            UART1_WriteByte(BL_RESP_CRC_FAIL);
    }
    else if (BL_CMD_RESET == input_command)
    {
        trigger_Reset();
    }
    else
    {
        UART1_WriteByte(BL_RESP_INVALID);
    }

    packet_received = false;
}

/* Function to program received application firmware data into internal flash */
static void flash_task(void)
{
    uint32_t addr       = flash_addr;
    uint32_t page       = 0;
    uint32_t write_idx  = 0;


    /* Erase the Current sector */
    NVM_PageErase(addr);

    /* Wait for erase to complete */
    while(NVM_IsBusy() == true);

    for (page = 0; page < PAGES_IN_ERASE_BLOCK; page++)
    {
        NVM_RowWrite(&flash_data[write_idx], addr);

        while(NVM_IsBusy() == true);

        addr += PAGE_SIZE;
        write_idx += WORDS(PAGE_SIZE);
    }

    flash_data_ready = false;

    UART1_WriteByte(BL_RESP_OK);
}

void run_Application(void)
{
    uint32_t msp            = *(uint32_t *)(APP_START_ADDRESS);

    void (*fptr)(void);

    /* Set default to APP_RESET_ADDRESS */
    fptr = (void (*)(void))APP_START_ADDRESS;

    if (msp == 0xffffffff)
    {
        return;
    }

    fptr();
}

bool __WEAK bootloader_Trigger(void)
{
    /* Function can be overriden with custom implementation */
    return false;
}

void bootloader_Tasks(void)
{
    CORETIMER_CompareSet(TIMER_COMPARE_VALUE);

    while (1)
    {
        input_task();

        if (flash_data_ready)
            flash_task();
        else if (packet_received)
            command_task();
    }
}
