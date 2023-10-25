/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for bootloader project.

  Description:
    This file contains the "main" function for bootloader project.  The
    "main" function calls the "SYS_Initialize" function to initialize
    all modules in the system.
    It calls "bootloader_start" once system is initialized.
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

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "definitions.h" // SYS function prototypes
#include <stdbool.h>     // Defines true
#include <stddef.h>      // Defines NULL
#include <stdio.h>
#include <stdlib.h> // Defines EXIT_FAILURE
#include "sst26/app_sst26vf.h"
#include "sst26/ext_flash.h"

//#define PRINT_DEBUG_LOG
#define BTL_TRIGGER_PATTERN (0x5048434DUL)

#define FLASH_SECTOR_SIZE 4096
#define KVS_SIZE    0x8000
#define IMAGE_SIZE 1024 * 1024 - KVS_SIZE
#define BOOT_DESCRIPTOR_SIZE FLASH_SECTOR_SIZE
#define INTERNAL_FLASH_SIZE 1024 * 1024
#define IMAGE_1_DESCRIPTOR_ADDR 1028 * 1024
#define IMAGE_1_START_ADDR 1032 * 1024
#define IMAGE_2_DESCRIPTOR_ADDR 2056 * 1024
#define IMAGE_2_START_ADDR 2060 * 1024

#define NVM_START_ADDR 0x90000000

#define IMAGE_DESC_STATUS_INVALID 0
#define IMAGE_DESC_STATUS_IN_USE 1
#define IMAGE_DESC_STATUS_NOT_IN_USE 2
#define IMAGE_DESC_STATUS_UPGRADABLE 3
#define IMAGE_DESC_STATUS_IN_PROGRESS_UPGRADE 4

#define IMAGE_TYPE_FACTORY 0xFE
#define IMAGE_TYPE_OTA 0xFD

#define APP_KVS_STORE_ADDRESS         ((volatile const uint8_t *)0xb00F8000)

#define TEXT_FACTORY_RESET  "Factory Reset ...\r\n"
#define TEXT_INTERNAL_FLASH_ERASE   "Int Flash Erase..\r\n"
#define TEXT_PROGRAM_IMG1   "Prgoram Image1\r\n"
#define TEXT_PROGRAM_IMG2   "Prgoram Image2\r\n"
#define TEXT_JUMP_APPLICATION   "Jump to Appliaction..\r\n"
#define TEXT_BOOTLOADER_START   "Bootloader start..\r\n"


static uint32_t * ramStart = (uint32_t *) BTL_TRIGGER_RAM_START;

bool bootloader_Trigger(void)
{
    // uint32_t i;

    // Cheap delay. This should give at leat 1 ms delay.
    // for (i = 0; i < 2000; i++)
    //{
    //    asm("nop");
    //}

    /* Check for Bootloader Trigger Pattern in first 16 Bytes of RAM to enter
     * Bootloader.
     */
#if 1
    if (BTL_TRIGGER_PATTERN == ramStart[0] && BTL_TRIGGER_PATTERN == ramStart[1] && BTL_TRIGGER_PATTERN == ramStart[2] &&
        BTL_TRIGGER_PATTERN == ramStart[3])
    {
        ramStart[0] = 0;
        return true;
    }
#endif

    /* Check for Switch press to enter Bootloader */
    if (SWITCH_Get() == 0)
    {
        return true;
    }

    return false;
}

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************


typedef struct
{
    uint8_t type;
    uint8_t status;
    uint32_t ver;
    uint32_t image_size;
} image_descriptor;

void hex2ascii(uint8_t input, uint8_t * output)
{
    output[0] = (input & 0xF0) >> 4;
    if (output[0] > 0x9)
        output[0] = output[0] - 0xa + 0x41;
    else
        output[0] = output[0] + 0x30;

    output[1] = input & 0xF;
    if (output[1] > 0x9)
        output[1] = output[1] - 0xa + 0x41;
    else
        output[1] = output[1] + 0x30;
}

bool INT_Flash_Erase(uint32_t addr, uint32_t len)
{
    addr += NVM_START_ADDR;

    while (len)
    {
        if (NVM_PageErase(addr) == false)
        {
            break;
        }
        while (NVM_IsBusy())
            ;

        addr += NVM_FLASH_PAGESIZE;
        len -= NVM_FLASH_PAGESIZE;
    }

    return (len == 0) ? true : false;
}

int main(void)
{
    uint8_t factory_reset = 0;
    image_descriptor image1_desc, image2_desc;
    static uint8_t CACHE_ALIGN buffer[NVM_FLASH_ROWSIZE];
    int image_size;
    uint32_t nvm_prog_address, ext_flash_address;
    char str_buf[50];
    
    /* Initialize all modules */
    SYS_Initialize(NULL);

    EXT_Flash_Initialize();
    
    UART1_Write(TEXT_BOOTLOADER_START, sizeof(TEXT_BOOTLOADER_START));
    
    uint8_t *p = (uint8_t*) APP_KVS_STORE_ADDRESS;
#ifdef PRINT_DEBUG_LOG
    sprintf(str_buf, "KVS bytes: 0x%x, 0x%x, 0x%x, 0x%x, 0x%x , 0x%x\r\n", p[0], p[1], p[2], p[3], p[4], p[5]);
    UART1_Write(str_buf, sizeof(str_buf));
#endif
    if ((p[0] == 0xFF) && (p[0] == 0xFF) && (p[0] == 0xFF) & (p[0] == 0xFF))
        factory_reset = 1;

    if (factory_reset)
    {
        UART1_Write(TEXT_FACTORY_RESET, sizeof(TEXT_FACTORY_RESET));
        EXT_Flash_Erase(0x0, SST26VF_SIZE);

        memset(&image1_desc, 0, sizeof(image1_desc));
        image1_desc.status = IMAGE_DESC_STATUS_IN_USE;
        image1_desc.type = IMAGE_TYPE_FACTORY;
        image1_desc.image_size = INTERNAL_FLASH_SIZE;
        EXT_Flash_Write(IMAGE_1_DESCRIPTOR_ADDR, (uint8_t*) &image1_desc, sizeof(image_descriptor));

        p = (uint8_t*) APP_START_ADDRESS;
        EXT_Flash_Write(IMAGE_1_START_ADDR, p, INTERNAL_FLASH_SIZE);
              
    }
    
    /* read spi image1 and 2 descriptor */
    EXT_Flash_Read(IMAGE_1_DESCRIPTOR_ADDR, (uint8_t *) &image1_desc, sizeof(image1_desc));
    EXT_Flash_Read(IMAGE_2_DESCRIPTOR_ADDR, (uint8_t *) &image2_desc, sizeof(image2_desc));

    memset(str_buf, 0 , sizeof(str_buf));
    sprintf(str_buf, "image1 status: 0x%x\r\n", image1_desc.status);
    UART1_Write(str_buf, sizeof(str_buf));
    sprintf(str_buf, "image2 status: 0x%x\r\n", image2_desc.status);
    UART1_Write(str_buf, sizeof(str_buf));

    if (image1_desc.status == IMAGE_DESC_STATUS_IN_PROGRESS_UPGRADE)
    {
        if (image2_desc.status != IMAGE_DESC_STATUS_INVALID)
        {
            image_size = image2_desc.image_size;
            nvm_prog_address = NVM_START_ADDR;
            ext_flash_address = IMAGE_2_START_ADDR;
            UART1_Write(TEXT_INTERNAL_FLASH_ERASE, sizeof(TEXT_INTERNAL_FLASH_ERASE));
            INT_Flash_Erase( 0, IMAGE_SIZE);

            while (image_size > 0)
            {
                EXT_Flash_Read(ext_flash_address, (uint8_t*) &buffer, NVM_FLASH_ROWSIZE);
                UART1_Write(TEXT_PROGRAM_IMG2, sizeof(TEXT_PROGRAM_IMG2));
                NVM_RowWrite((uint32_t*)buffer, nvm_prog_address);
                while (NVM_IsBusy());

                nvm_prog_address += NVM_FLASH_ROWSIZE;
                ext_flash_address += NVM_FLASH_ROWSIZE;
                image_size -= NVM_FLASH_ROWSIZE;
            }

            EXT_Flash_Erase(IMAGE_2_DESCRIPTOR_ADDR, BOOT_DESCRIPTOR_SIZE);
            image2_desc.status = IMAGE_DESC_STATUS_IN_USE;
            EXT_Flash_Write(IMAGE_2_DESCRIPTOR_ADDR, (uint8_t*) &image2_desc, sizeof(image_descriptor));

            EXT_Flash_Erase(IMAGE_1_DESCRIPTOR_ADDR, BOOT_DESCRIPTOR_SIZE);
            image1_desc.status = IMAGE_DESC_STATUS_INVALID;
            EXT_Flash_Write(IMAGE_1_DESCRIPTOR_ADDR, (uint8_t*) &image1_desc, sizeof(image_descriptor));
        }
    }
    else if (image2_desc.status == IMAGE_DESC_STATUS_IN_PROGRESS_UPGRADE)
    {
        if (image1_desc.status != IMAGE_DESC_STATUS_INVALID)
        {
            image_size = image1_desc.image_size;
            nvm_prog_address = NVM_START_ADDR;
            ext_flash_address = IMAGE_1_START_ADDR;
            UART1_Write(TEXT_INTERNAL_FLASH_ERASE, sizeof(TEXT_INTERNAL_FLASH_ERASE));
            INT_Flash_Erase( 0, IMAGE_SIZE);

            while (image_size > 0)
            {
                EXT_Flash_Read(ext_flash_address, (uint8_t*) &buffer, NVM_FLASH_ROWSIZE);
                UART1_Write(TEXT_PROGRAM_IMG1, sizeof(TEXT_PROGRAM_IMG1));
                NVM_RowWrite((uint32_t*)buffer, nvm_prog_address);
                while (NVM_IsBusy());

                nvm_prog_address += NVM_FLASH_ROWSIZE;
                ext_flash_address += NVM_FLASH_ROWSIZE;
                image_size -= NVM_FLASH_ROWSIZE;
            }

            EXT_Flash_Erase(IMAGE_1_DESCRIPTOR_ADDR, BOOT_DESCRIPTOR_SIZE);
            image1_desc.status = IMAGE_DESC_STATUS_IN_USE;
            EXT_Flash_Write(IMAGE_1_DESCRIPTOR_ADDR, (uint8_t*) &image1_desc, sizeof(image_descriptor));

            EXT_Flash_Erase(IMAGE_2_DESCRIPTOR_ADDR, BOOT_DESCRIPTOR_SIZE);
            image2_desc.status = IMAGE_DESC_STATUS_INVALID;
            EXT_Flash_Write(IMAGE_2_DESCRIPTOR_ADDR, (uint8_t*) &image2_desc, sizeof(image_descriptor));
        }

    }
    else if (image1_desc.status == IMAGE_DESC_STATUS_UPGRADABLE)
    {
        UART1_Write(TEXT_PROGRAM_IMG1, sizeof(TEXT_PROGRAM_IMG1));

        /* change image2 boot descriptor status to not in use */
        if (image2_desc.status == IMAGE_DESC_STATUS_IN_USE)
        {
            EXT_Flash_Erase(IMAGE_2_DESCRIPTOR_ADDR, BOOT_DESCRIPTOR_SIZE);
            image2_desc.status = IMAGE_DESC_STATUS_NOT_IN_USE;
            EXT_Flash_Write(IMAGE_2_DESCRIPTOR_ADDR, (uint8_t*) &image2_desc, sizeof(image_descriptor));
        }
        /* change image1 boot descriptor status to in progress upgrade */
        EXT_Flash_Erase(IMAGE_1_DESCRIPTOR_ADDR, BOOT_DESCRIPTOR_SIZE);
        image1_desc.status = IMAGE_DESC_STATUS_IN_PROGRESS_UPGRADE;
        EXT_Flash_Write(IMAGE_1_DESCRIPTOR_ADDR, (uint8_t*) &image1_desc, sizeof(image_descriptor));

        image_size = image1_desc.image_size;
        nvm_prog_address = NVM_START_ADDR;
        ext_flash_address = IMAGE_1_START_ADDR;
        
        UART1_Write(TEXT_INTERNAL_FLASH_ERASE, sizeof(TEXT_INTERNAL_FLASH_ERASE));
        INT_Flash_Erase( 0, IMAGE_SIZE);

        while (image_size > 0)
        {
            EXT_Flash_Read(ext_flash_address, (uint8_t*) &buffer, NVM_FLASH_ROWSIZE);
            
#ifdef PRINT_DEBUG_LOG
            sprintf(str_buf, "UpgradeImage %d, 0x%x, 0x%x\r\n", image_size, buffer[0], buffer[1]);
            UART1_Write(str_buf, sizeof(str_buf));
#endif

            NVM_RowWrite((uint32_t*)buffer, nvm_prog_address);
            while (NVM_IsBusy());

            nvm_prog_address += NVM_FLASH_ROWSIZE;
            ext_flash_address += NVM_FLASH_ROWSIZE;
            image_size -= NVM_FLASH_ROWSIZE;
        }

        EXT_Flash_Erase(IMAGE_1_DESCRIPTOR_ADDR, BOOT_DESCRIPTOR_SIZE);
        image1_desc.status = IMAGE_DESC_STATUS_IN_USE;
        EXT_Flash_Write(IMAGE_1_DESCRIPTOR_ADDR, (uint8_t*) &image1_desc, sizeof(image_descriptor));
    }
    else if (image2_desc.status == IMAGE_DESC_STATUS_UPGRADABLE)
    {
        UART1_Write(TEXT_PROGRAM_IMG2, sizeof(TEXT_PROGRAM_IMG2));

        /* change image1 boot descriptor status to not in use */
        if (image1_desc.status == IMAGE_DESC_STATUS_IN_USE)
        {
            EXT_Flash_Erase(IMAGE_1_DESCRIPTOR_ADDR, BOOT_DESCRIPTOR_SIZE);
            image1_desc.status = IMAGE_DESC_STATUS_NOT_IN_USE;
            EXT_Flash_Write(IMAGE_1_DESCRIPTOR_ADDR, (uint8_t*) &image1_desc, sizeof(image_descriptor));
        }
        /* change image1 boot descriptor status to in progress upgrade */
        EXT_Flash_Erase(IMAGE_2_DESCRIPTOR_ADDR, BOOT_DESCRIPTOR_SIZE);
        image2_desc.status = IMAGE_DESC_STATUS_IN_PROGRESS_UPGRADE;
        EXT_Flash_Write(IMAGE_2_DESCRIPTOR_ADDR, (uint8_t*) &image2_desc, sizeof(image_descriptor));


        image_size = image2_desc.image_size;
        nvm_prog_address = NVM_START_ADDR;
        ext_flash_address = IMAGE_2_START_ADDR;
        
        UART1_Write(TEXT_INTERNAL_FLASH_ERASE, sizeof(TEXT_INTERNAL_FLASH_ERASE));
        INT_Flash_Erase( 0, IMAGE_SIZE);

        while (image_size > 0)
        {
            EXT_Flash_Read(ext_flash_address, (uint8_t*) &buffer, NVM_FLASH_ROWSIZE);
#ifdef PRINT_DEBUG_LOG
            sprintf(str_buf, "UpgradeImage %d, 0x%x, 0x%x\r\n", image_size, buffer[0], buffer[1]);
            UART1_Write(str_buf, sizeof(str_buf));
#endif
            NVM_RowWrite((uint32_t*)buffer, nvm_prog_address);
            while (NVM_IsBusy());

            nvm_prog_address += NVM_FLASH_ROWSIZE;
            ext_flash_address += NVM_FLASH_ROWSIZE;
            image_size -= NVM_FLASH_ROWSIZE;
        }

        UART1_Write(str_buf, sizeof(str_buf));
        EXT_Flash_Erase(IMAGE_2_DESCRIPTOR_ADDR, BOOT_DESCRIPTOR_SIZE);
        image2_desc.status = IMAGE_DESC_STATUS_IN_USE;
        EXT_Flash_Write(IMAGE_2_DESCRIPTOR_ADDR, (uint8_t*) &image2_desc, sizeof(image_descriptor));
    }
    
    UART1_Write(TEXT_JUMP_APPLICATION, sizeof(TEXT_JUMP_APPLICATION));
    
    PRISS = 0x0;
    run_Application(APP_JUMP_ADDRESS);


    //bootloader_UART_Tasks();


    /* Execution should not come here during normal operation */
    return (EXIT_FAILURE);
}

/*******************************************************************************
 End of File
*/
