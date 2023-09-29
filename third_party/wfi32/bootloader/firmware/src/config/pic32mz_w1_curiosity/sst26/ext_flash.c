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

#include "app_sst26vf.h"
#include "definitions.h"

#include <string.h>
typedef struct
{
    uint32_t dummy;
} EXT_FLASH_DATA;
EXT_FLASH_DATA __attribute__((coherent, aligned(32))) ext_flash;

#define PAGE_PROGRAM_SIZE 256
//---------------------------------------------------------------------------
bool EXT_Flash_Initialize(void)
{
    // configPRINTF( ( "EXT_Flash_Initialize: In\r\n" ) );
    uint8_t manufacturerID;
    uint16_t deviceID;
    //UART1_Write("hello21", 7);
    APP_SST26_Initialize();
    //UART1_Write("hello22", 7);
    APP_SST26_MinPowerOnDelay();
    //UART1_Write("hello23", 7);
    APP_SST26_Reset();
    //UART1_Write("hello24", 7);
    APP_SST26_GlobalWriteProtectionUnlock();
    //UART1_Write("hello25", 7);
    APP_SST26_JEDEC_ID_Read(&manufacturerID, &deviceID);
    
    if (manufacturerID != 0xBF || deviceID != 0x2642)
    {
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
bool EXT_Flash_Open(void)
{
    return true;
}
//---------------------------------------------------------------------------
void EXT_Flash_Close(void) {}
//---------------------------------------------------------------------------
void EXT_Flash_Deinitialize(void) {}
//---------------------------------------------------------------------------
bool EXT_Flash_Write(uint32_t addr, uint8_t * buf, uint32_t len)
{
    uint8_t status;
    uint8_t temp_buf[SST26VF_PAGE_SIZE];

    while (len)
    {
        memset(temp_buf, 0xff, SST26VF_PAGE_SIZE);
        if (len < SST26VF_PAGE_SIZE)
            memcpy(temp_buf, buf, len);
        else
            memcpy(temp_buf, buf, SST26VF_PAGE_SIZE);

        APP_SST26_PageProgram(addr, temp_buf);

        do
        {
            status = APP_SST26_StatusRead();
        } while (status & 0x80);

        addr += SST26VF_PAGE_SIZE;
        buf += SST26VF_PAGE_SIZE;

        if (len < SST26VF_PAGE_SIZE)
            len = 0;
        else
            len -= SST26VF_PAGE_SIZE;
    }

    return (len == 0) ? true : false;

    return true;
}
//---------------------------------------------------------------------------
bool EXT_Flash_Read(uint32_t addr, uint8_t * buf, uint32_t len)
{
    APP_SST26_MemoryRead(addr, buf, len, true);
    return true;
}
//---------------------------------------------------------------------------
bool EXT_Flash_Erase(uint32_t addr, uint32_t len)
{
    uint8_t status;

    if (addr == 0 && len == SST26VF_SIZE)
    {
        APP_SST26_ChipErase();

        do
        {
            status = APP_SST26_StatusRead();
        } while (status & 0x80);
    }
    else
    {
        if (len % SST26VF_SECTOR_SIZE > 0)
        {
            // configPRINTF( ( "[%s] the len is not in sector size, erase fail ..\r\n", __func__ ) );
            return false;
        }

        while (len)
        {
            APP_SST26_SectorErase(addr);
            do
            {
                status = APP_SST26_StatusRead();
            } while (status & 0x80);

            addr += SST26VF_SECTOR_SIZE;
            len -= SST26VF_SECTOR_SIZE;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
bool EXT_Flash_Busy(void)
{
    return false;
}
//---------------------------------------------------------------------------
uint32_t EXT_Flash_Capacity(void)
{
    return SST26VF_SIZE;
}
//---------------------------------------------------------------------------
