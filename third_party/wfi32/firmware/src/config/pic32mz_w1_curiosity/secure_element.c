/*******************************************************************************
 * Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries.
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

#include "secure_element.h"
#include "definitions.h"
#include "atca_basic.h"
#include "atca_status.h"

void Secure_Element_Init(void)
{
    extern ATCAIfaceCfg atecc608_0_init_data;
    ATCA_STATUS atcaStat;
    uint8_t sernum[9];
    atcaStat = atcab_init(&atecc608_0_init_data);
    if (ATCA_SUCCESS == atcaStat)
    {
        atcaStat = atcab_read_serial_number(sernum);
    }
    SYS_CONSOLE_PRINT("sernum = 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\r\n", sernum[0], sernum[1], sernum[2], sernum[3],
                      sernum[4], sernum[5], sernum[6], sernum[7], sernum[8]);

    /* Check the config zone lock status */
    ATCA_STATUS lock_status;
    bool lock = false;
    if (ATCA_SUCCESS != (lock_status = atcab_is_locked(ATCA_ZONE_CONFIG, &lock)))
    {
        SYS_CONSOLE_PRINT("Unable to get config lock status: %x\r\n", lock_status);
    }

}