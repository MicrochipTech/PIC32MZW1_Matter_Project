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

#ifndef __INC_EXT_FLASH_H__
#define __INC_EXT_FLASH_H__

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
/*  This section lists the other files that are included in this file.
*/

//#include "driver/driver_common.h"
//#include "configuration.h"

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
/* Function:
    EXT_Flash_Initialize(void);
    
  Summary:
    Initializes external flash driver instance.
    
  Description:
    This function initializes the external flash driver instance. 
*/
bool EXT_Flash_Initialize(void);


//*************************************************************************
/* Function:
    
    DRV_CLIENT_STATUS EXT_Flash_Open(void)
    
  Summary:
    Open EXT_Flash interface.
  
  Description:
    Open EXT_Flash interface.
  
  Returns:
    DRV_CLIENT_STATUS_READY
    DRV_CLIENT_STATUS_ERROR
*/
bool EXT_Flash_Open(void);


//*************************************************************************
/* Function:
    
    DRV_CLIENT_STATUS EXT_Flash_Close(void)
    
  Summary:
    Close EXT_Flash interface.
  
  Description:
    Close EXT_Flash interface.
  
*/
void EXT_Flash_Close(void);



//*************************************************************************
/* Function:
    
    DRV_CLIENT_STATUS EXT_Flash_Write
    (
        uint32_t addr,
        uint8_t* buf,
        uint32_t len
    )
    
  Summary:
    Write blocks of data starting from the specified block start address.
  
  Description:
    This function schedules a non-blocking write operation for writing blocks
    of data into flash memory.
  
  Returns:
    DRV_CLIENT_STATUS_READY - Indicates that the driver completes the request.
    DRV_CLIENT_STATUS_BUSY - Indicates that the driver is start processing request.
    DRV_CLIENT_STATUS_ERROR - Indicates that the driver failed the request.
*/
bool EXT_Flash_Write(uint32_t addr, uint8_t* buf, uint32_t len);

//*************************************************************************
/* Function:
    
    DRV_CLIENT_STATUS EXT_Flash_Read
    (
        uint32_t addr,
        uint8_t* buf,
        uint32_t len
    )
    
  Summary:
    Read blocks of data starting from the specified block start address.
  
  Description:
    This function schedules a non-blocking read operation.
  
  Returns:
    DRV_CLIENT_STATUS_READY - Indicates that the driver completes the request.
    DRV_CLIENT_STATUS_BUSY - Indicates that the driver is start processing request.
    DRV_CLIENT_STATUS_ERROR - Indicates that the driver failed the request.
*/
bool EXT_Flash_Read(uint32_t addr, uint8_t* buf, uint32_t len);


//*************************************************************************
/* Function:
    
    DRV_CLIENT_STATUS EXT_Flash_Erase
    (
        uint32_t addr,
        uint32_t len
    )
    
  Summary:
    Erase blocks of data starting from the specified block start address.
  
  Description:
    This function schedules a non-blocking erase operation.
  
  Returns:
    DRV_CLIENT_STATUS_READY - Indicates that the driver completes the request.
    DRV_CLIENT_STATUS_BUSY - Indicates that the driver is start processing request.
    DRV_CLIENT_STATUS_ERROR - Indicates that the driver failed the request.
*/
bool EXT_Flash_Erase(uint32_t addr, uint32_t len);


//*************************************************************************
/* Function:
    DRV_CLIENT_STATUS EXT_Flash_ClientStatus(void)
    
  Summary:
    Gets the current status of the External Flash Driver module.
  
  Description:
    This function provides the current status of the External Flash Driver module.
  
  Returns:
    SYS_STATUS_READY - Indicates that the driver is ready and accept requests
                       for new operations
    SYS_STATUS_BUSY - Indicates that the driver is still processing request.
    DRV_CLIENT_STATUS_ERROR - Indicates that the driver failed the request.
*/
//DRV_CLIENT_STATUS EXT_Flash_ClientStatus(void);
bool EXT_Flash_Busy();


//*************************************************************************
/* Function:
    uint32_t EXT_Flash_Capacity(void)
    
  Summary:
    Get the flash device capacity.
  
  Description:
    Get the flash device capacity.
  
  Returns:
    
*/
uint32_t EXT_Flash_Capacity(void);

#ifdef __cplusplus
}
#endif
#endif //__INC_EXT_FLASH_H__
