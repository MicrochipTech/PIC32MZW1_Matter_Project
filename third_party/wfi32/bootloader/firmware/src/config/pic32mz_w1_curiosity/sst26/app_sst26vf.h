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

#ifndef __APP_SST26VF__
#define __APP_SST26VF__

#include "definitions.h"                // SYS function prototypes
#include <string.h>

void APP_SST26_Reset(void);
void APP_SST26_WriteEnable(void);
void APP_SST26_WriteDisable(void);
void APP_SST26_SectorErase(uint32_t address);
void APP_SST26_ChipErase(void);
void APP_SST26_PageProgram(uint32_t address, uint8_t* pPageData);
void APP_SST26_MemoryRead(uint32_t address, uint8_t* pReadBuffer, uint32_t nBytes, bool isHighSpeedRead);
uint8_t APP_SST26_StatusRead(void);
uint8_t APP_SST26_ConfigRegisterRead(void);
void APP_SST26_JEDEC_ID_Read(uint8_t* manufacturerID, uint16_t* deviceID);
void APP_SST26_GlobalWriteProtectionUnlock(void);
void APP_SST26_MinPowerOnDelay(void);
void APP_SST26_SPIEventHandler(uintptr_t context );
void APP_SST26_Initialize(void);

#define SST26VF_SIZE          (32*1024*1024)
#define SST26VF_PAGE_SIZE     (256)
#define SST26VF_SECTOR_SIZE   (4096)

#endif //__APP_SST26VF__