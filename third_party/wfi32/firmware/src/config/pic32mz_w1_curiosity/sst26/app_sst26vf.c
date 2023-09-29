/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

/*******************************************************************************
 * Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "definitions.h" // SYS function prototypes
#include <string.h>

#define SPI1_USE_IRQ

/* Application's state machine enum */
typedef enum
{
    APP_SST26_STATE_INITIALIZE,
    APP_SST26_STATE_WAIT_MIN_POWER_UP_TIME,
    APP_SST26_STATE_RESET,
    APP_SST26_STATE_GLOBAL_BLK_PROTECTION_UNLOCK,
    APP_SST26_STATE_JEDEC_ID_READ,
    APP_SST26_STATE_SECTOR_ERASE,
    APP_SST26_STATE_READ_STATUS,
    APP_SST26_STATE_PAGE_PROGRAM,
    APP_SST26_STATE_MEMORY_READ,
    APP_SST26_STATE_VERIFY,
    APP_SST26_STATE_XFER_SUCCESSFUL,
    APP_SST26_STATE_XFER_ERROR,
    APP_SST26_STATE_IDLE,
} APP_SST26_STATES;

/* SST26 Flash Commands */
#define APP_SST26_CMD_ENABLE_RESET 0x66
#define APP_SST26_CMD_MEMORY_RESET 0x99
#define APP_SST26_CMD_STATUS_REG_READ 0x05
#define APP_SST26_CMD_CONFIG_REG_READ 0x35
#define APP_SST26_CMD_MEMORY_READ 0x03
#define APP_SST26_CMD_MEMORY_HIGH_SPEED_READ 0x0B
#define APP_SST26_CMD_ENABLE_WRITE 0x06
#define APP_SST26_CMD_DISABLE_WRITE 0x04
#define APP_SST26_CMD_4KB_SECTOR_ERASE 0x20
#define APP_SST26_CMD_BLOCK_ERASE 0xD8
#define APP_SST26_CMD_CHIP_ERASE 0xC7
#define APP_SST26_CMD_PAGE_PROGRAM 0x02
#define APP_SST26_CMD_JEDEC_ID_READ 0x9F
#define APP_SST26_CMD_GLOBAL_BLOCK_PROTECTION_UNLOCK 0x98

#define APP_SST26_STATUS_BIT_RES_0 (0x01 << 0)
#define APP_SST26_STATUS_BIT_WEL (0x01 << 1)
#define APP_SST26_STATUS_BIT_WSE (0x01 << 2)
#define APP_SST26_STATUS_BIT_WSP (0x01 << 3)
#define APP_SST26_STATUS_BIT_WPLD (0x01 << 4)
#define APP_SST26_STATUS_BIT_SEC (0x01 << 5)
#define APP_SST26_STATUS_BIT_RES_6 (0x01 << 6)
#define APP_SST26_STATUS_BIT_BUSY (0x01 << 7)

#define APP_SST26_PAGE_PROGRAM_SIZE_BYTES 256
#if 0
/*** Macros for SPI1_CS pin ***/
#define SPI1_CS_Set() (LATASET = (1 << 1))
#define SPI1_CS_Clear() (LATACLR = (1 << 1))
#define SPI1_CS_Toggle() (LATAINV = (1 << 1))
#define SPI1_CS_OutputEnable() (TRISACLR = (1 << 1))
#define SPI1_CS_InputEnable() (TRISASET = (1 << 1))
#define SPI1_CS_Get() ((PORTA >> 1) & 0x1)
#define SPI1_CS_PIN GPIO_PIN_RA1
#endif
#define APP_SST26_CS_ENABLE() SPI1_CS_Clear()
#define APP_SST26_CS_DISABLE() SPI1_CS_Set()

typedef struct
{
    APP_SST26_STATES state;
    APP_SST26_STATES nextState;
    uint8_t transmitBuffer[APP_SST26_PAGE_PROGRAM_SIZE_BYTES + 5];
    uint8_t manufacturerID;
    uint16_t deviceID;
    uint8_t isCSDeAssert;
    volatile bool isTransferDone;
} APP_SST26_DATA;

APP_SST26_DATA appSST26Data;

void APP_SST26_Reset(void)
{
    appSST26Data.isTransferDone = false;

    appSST26Data.transmitBuffer[0] = APP_SST26_CMD_ENABLE_RESET;
    APP_SST26_CS_ENABLE();
    appSST26Data.isCSDeAssert = true;
    SPI1_Write(appSST26Data.transmitBuffer, 1);
#ifdef SPI1_USE_IRQ
    while (appSST26Data.isTransferDone == false)
        ;
#endif
    appSST26Data.isTransferDone = false;

    appSST26Data.transmitBuffer[0] = APP_SST26_CMD_MEMORY_RESET;

    APP_SST26_CS_ENABLE();
    appSST26Data.isCSDeAssert = true;
    SPI1_Write(appSST26Data.transmitBuffer, 1);
#ifdef SPI1_USE_IRQ
    while (appSST26Data.isTransferDone == false)
        ;
#endif
}

void APP_SST26_WriteEnable(void)
{
    appSST26Data.isTransferDone = false;

    appSST26Data.transmitBuffer[0] = APP_SST26_CMD_ENABLE_WRITE;

    APP_SST26_CS_ENABLE();
    appSST26Data.isCSDeAssert = true;
    SPI1_Write(appSST26Data.transmitBuffer, 1);
#ifdef SPI1_USE_IRQ
    while (appSST26Data.isTransferDone == false)
        ;
#endif
}
#if 0
void APP_SST26_WriteDisable(void)
{
    appSST26Data.isTransferDone = false;    
    
    appSST26Data.transmitBuffer[0] = APP_SST26_CMD_DISABLE_WRITE;
    
    APP_SST26_CS_ENABLE();
    appSST26Data.isCSDeAssert = true;
    SPI1_Write(appSST26Data.transmitBuffer, 1);
#ifdef SPI1_USE_IRQ
    while (appSST26Data.isTransferDone == false);
#endif
}
#endif
void APP_SST26_SectorErase(uint32_t address)
{
    APP_SST26_WriteEnable();

    appSST26Data.isTransferDone = false;

    /* The address bits from A11:A0 are don't care and must be Vih or Vil */
    address = address & 0xFFFFF000;

    appSST26Data.transmitBuffer[0] = APP_SST26_CMD_4KB_SECTOR_ERASE;
    appSST26Data.transmitBuffer[1] = (address >> 16);
    appSST26Data.transmitBuffer[2] = (address >> 8);
    appSST26Data.transmitBuffer[3] = address;

    APP_SST26_CS_ENABLE();
    appSST26Data.isCSDeAssert = true;
    SPI1_Write(appSST26Data.transmitBuffer, 4);
#ifdef SPI1_USE_IRQ
    while (appSST26Data.isTransferDone == false)
        ;
#endif
}
//
// void APP_SST26_BlockErase(uint32_t address)
//{
//    APP_SST26_WriteEnable();
//
//    appSST26Data.isTransferDone = false;
//
//    /* The address bits from A11:A0 are don't care and must be Vih or Vil */
//    address = address & 0xFFFFF000;
//
//    appSST26Data.transmitBuffer[0] = APP_SST26_CMD_BLOCK_ERASE;
//    appSST26Data.transmitBuffer[1] = (address >> 16);
//    appSST26Data.transmitBuffer[2] = (address >> 8);
//    appSST26Data.transmitBuffer[3] = address;
//
//    APP_SST26_CS_ENABLE();
//    appSST26Data.isCSDeAssert = true;
//    SPI1_Write(appSST26Data.transmitBuffer, 4);
//
//    while (appSST26Data.isTransferDone == false);
//}

void APP_SST26_ChipErase(void)
{
    APP_SST26_WriteEnable();

    appSST26Data.isTransferDone = false;

    appSST26Data.transmitBuffer[0] = APP_SST26_CMD_CHIP_ERASE;

    APP_SST26_CS_ENABLE();
    appSST26Data.isCSDeAssert = true;
    SPI1_Write(appSST26Data.transmitBuffer, 1);
#ifdef SPI1_USE_IRQ
    while (appSST26Data.isTransferDone == false)
        ;
#endif
}

void APP_SST26_PageProgram(uint32_t address, uint8_t * pPageData)
{
    uint32_t i;

    APP_SST26_WriteEnable();

    appSST26Data.isTransferDone = false;

    appSST26Data.transmitBuffer[0] = APP_SST26_CMD_PAGE_PROGRAM;
    appSST26Data.transmitBuffer[1] = (address >> 16);
    appSST26Data.transmitBuffer[2] = (address >> 8);
    appSST26Data.transmitBuffer[3] = address;

    for (i = 0; i < APP_SST26_PAGE_PROGRAM_SIZE_BYTES; i++)
    {
        appSST26Data.transmitBuffer[4 + i] = pPageData[i];
    }

    APP_SST26_CS_ENABLE();
    appSST26Data.isCSDeAssert = true;
    SPI1_Write(appSST26Data.transmitBuffer, (4 + APP_SST26_PAGE_PROGRAM_SIZE_BYTES));
#ifdef SPI1_USE_IRQ
    while (appSST26Data.isTransferDone == false)
        ;
#endif
}

void APP_SST26_MemoryRead(uint32_t address, uint8_t * pReadBuffer, uint32_t nBytes, bool isHighSpeedRead)
{
    uint8_t nTxBytes;

    appSST26Data.isTransferDone = false;

    appSST26Data.transmitBuffer[1] = (address >> 16);
    appSST26Data.transmitBuffer[2] = (address >> 8);
    appSST26Data.transmitBuffer[3] = address;

    if (isHighSpeedRead == true)
    {
        appSST26Data.transmitBuffer[0] = APP_SST26_CMD_MEMORY_HIGH_SPEED_READ;
        /* For high speed read, perform a dummy write */
        appSST26Data.transmitBuffer[4] = 0xFF;
        nTxBytes                       = 5;
    }
    else
    {
        appSST26Data.transmitBuffer[0] = APP_SST26_CMD_MEMORY_READ;
        nTxBytes                       = 4;
    }

    APP_SST26_CS_ENABLE();
    appSST26Data.isCSDeAssert = false;
    SPI1_Write(appSST26Data.transmitBuffer, nTxBytes);
#ifdef SPI1_USE_IRQ
    while (appSST26Data.isTransferDone == false)
        ;
#endif
    appSST26Data.isTransferDone = false;
    appSST26Data.isCSDeAssert   = true;
    SPI1_Read(pReadBuffer, nBytes);
#ifdef SPI1_USE_IRQ
    while (appSST26Data.isTransferDone == false)
        ;
#endif
}

uint8_t APP_SST26_StatusRead(void)
{
    uint8_t status;
    appSST26Data.isTransferDone = false;

    appSST26Data.transmitBuffer[0] = APP_SST26_CMD_STATUS_REG_READ;

    APP_SST26_CS_ENABLE();
    appSST26Data.isCSDeAssert = true;
    SPI1_WriteRead(appSST26Data.transmitBuffer, 1, appSST26Data.transmitBuffer, (1 + 1));
#ifdef SPI1_USE_IRQ
    while (appSST26Data.isTransferDone == false)
        ;
#endif
    status = appSST26Data.transmitBuffer[1];

    return status;
}
#if 0
uint8_t APP_SST26_ConfigRegisterRead(void)
{
    uint8_t config_reg;
    appSST26Data.isTransferDone = false;    
    
    appSST26Data.transmitBuffer[0] = APP_SST26_CMD_CONFIG_REG_READ;
    
    APP_SST26_CS_ENABLE();  
    appSST26Data.isCSDeAssert = true;    
    SPI1_WriteRead(appSST26Data.transmitBuffer, 1, appSST26Data.transmitBuffer, (1+1));    
        
    while (appSST26Data.isTransferDone == false);  
    
    config_reg = appSST26Data.transmitBuffer[1];
    
    return config_reg;
}
#endif
void APP_SST26_JEDEC_ID_Read(uint8_t * manufacturerID, uint16_t * deviceID)
{
    appSST26Data.isTransferDone = false;

    appSST26Data.transmitBuffer[0] = APP_SST26_CMD_JEDEC_ID_READ;

    APP_SST26_CS_ENABLE();
    appSST26Data.isCSDeAssert = true;
    SPI1_WriteRead(appSST26Data.transmitBuffer, 1, appSST26Data.transmitBuffer, (1 + 3));
#ifdef SPI1_USE_IRQ
    while (appSST26Data.isTransferDone == false)
        ;
#endif
    *manufacturerID = appSST26Data.transmitBuffer[1];
    *deviceID       = (appSST26Data.transmitBuffer[2] << 8UL) | appSST26Data.transmitBuffer[3];
}

#if 1
void APP_SST26_GlobalWriteProtectionUnlock(void)
{
    APP_SST26_WriteEnable();

    appSST26Data.isTransferDone    = false;
    appSST26Data.transmitBuffer[0] = APP_SST26_CMD_GLOBAL_BLOCK_PROTECTION_UNLOCK;

    APP_SST26_CS_ENABLE();
    appSST26Data.isCSDeAssert = true;
    SPI1_Write(appSST26Data.transmitBuffer, 1);
#ifdef SPI1_USE_IRQ
    while (appSST26Data.isTransferDone == false)
        ;
#endif
}
#endif
void APP_SST26_MinPowerOnDelay(void)
{
    uint32_t i;

    /* Cheap delay.
     * Based on the CPU frequency, ensure the delay is at-least 100 microseconds.
     */
    for (i = 0; i < 100000; i++)
    {
        __asm__("NOP");
    }
}

#ifdef SPI1_USE_IRQ
/* This function will be called by SPI PLIB when transfer is completed */
void APP_SST26_SPIEventHandler(uintptr_t context)
{
    uint8_t * isCSDeAssert = (uint8_t *) context;

    if (*isCSDeAssert == true)
    {
        /* De-assert the chip select */
        APP_SST26_CS_DISABLE();
    }

    appSST26Data.isTransferDone = true;
}
#endif

void APP_SST26_Initialize(void)
{
    APP_SST26_CS_DISABLE();
    appSST26Data.state = APP_SST26_STATE_INITIALIZE;

#ifdef SPI1_USE_IRQ
    SPI1_CallbackRegister(APP_SST26_SPIEventHandler, (uintptr_t) &appSST26Data.isCSDeAssert);
#endif
}
