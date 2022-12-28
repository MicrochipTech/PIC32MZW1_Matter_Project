/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    ext_flash.h

  Summary:
    External Flash Abstraction Interface definition.
    
  Description:
        External Flash Abstraction Interface definition.
*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2014-2015 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
*******************************************************************************/
// DOM-IGNORE-END

#ifndef __INC_INT_FLASH_H__
#define __INC_INT_FLASH_H__

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
/*  This section lists the other files that are included in this file.
*/

#include "peripheral/nvm/plib_nvm.h"

#define NVM_FLASH_START_ADDRESS    (0x90000000U)
#define NVM_FLASH_ROWSIZE          (1024U)

#ifdef __cplusplus
extern "C" {
#endif


//*************************************************************************
/* Function:
    
    DRV_CLIENT_STATUS INT_Flash_Write
    (
        uint32_t addr,
        uint8_t* buf,
        uint32_t len
    )
    
  Summary:
    Write blocks of data starting from the specified block start address.
  
  Description:
    This function schedules a write operation for writing blocks
    of data into internal flash memory.
  
  Returns:
    true/ false
*/
bool INT_Flash_Write(uint32_t addr, uint8_t* buf, uint32_t len);

//*************************************************************************
/* Function:
    
    bool INT_Flash_Read
    (
        uint32_t addr,
        uint8_t* buf,
        uint32_t len
    )
    
  Summary:
    Read blocks of data starting from the specified block start address.
  
  Description:
    This function schedules a read operation.
  
  Returns:
    true/ false
*/
bool INT_Flash_Read(uint32_t addr, uint8_t* buf, uint32_t len);


#ifdef __cplusplus
}
#endif
#endif //__INC_EXT_FLASH_H__
