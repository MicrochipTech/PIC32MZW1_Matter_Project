/*******************************************************************************
* Copyright (C) 2021 Microchip Technology Inc. and its subsidiaries.
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
#include "int_flash.h"

typedef struct 
{
    uint32_t buf[NVM_FLASH_ROWSIZE/sizeof(uint32_t)];
}INT_FLASH_DATA;

static INT_FLASH_DATA  __attribute__((coherent, aligned(32))) int_flash;

void nvm_callback_func(uintptr_t context);

volatile bool bNvmTaskCompleted = false;

// implementation
void nvm_callback_func(uintptr_t context)
{
    bNvmTaskCompleted=true;
}


bool INT_Flash_Read(uint32_t addr, uint8_t* buf, uint32_t len)
{
    addr += NVM_FLASH_START_ADDRESS;
        
    return NVM_Read((uint32_t*)buf, len, addr);
}

//---------------------------------------------------------------------------
bool INT_Flash_Write(uint32_t addr, uint8_t* buf, uint32_t len)
{

    addr += NVM_FLASH_START_ADDRESS;
    NVM_CallbackRegister(nvm_callback_func, (uintptr_t)NULL);
    
    while (len)
    {
        memcpy(int_flash.buf, buf, NVM_FLASH_ROWSIZE);
        bNvmTaskCompleted = false;
        if (NVM_RowWrite(int_flash.buf, addr) == false)
        {
            break;
        }
        while(bNvmTaskCompleted == false);
             
        buf  += NVM_FLASH_ROWSIZE;
        addr += NVM_FLASH_ROWSIZE;
        len  -= NVM_FLASH_ROWSIZE;
        
    }
    
    return (len == 0)? true: false;

}
