/**
 * \file rnbd_interface.h
 * \brief This file provides and interface between the RNBD and the hardware.
 */
/*
    (c) 2019 Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip software and any
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party
    license terms applicable to your use of third party software (including open source software) that
    may accompany Microchip software.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS
    FOR A PARTICULAR PURPOSE.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS
    SOFTWARE.
*/
#ifndef RNBD_INTERFACE_H
#define RNBD_INTERFACE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * \ingroup RNBD_INTERFACE
 * Enum of the RNBD System Configuration Modes
 */
typedef enum
{
    TEST_MODE        = 0x00,
    APPLICATION_MODE = 0x01
} RNBD_SYSTEM_MODES_t;

typedef enum
{
    RNBD_EVT_CONNECT,
    RNBD_EVT_DISCONNECT,
    RNBD_EVT_WRITE_REQ,
    RNBD_EVT_NOTI_IND_REQ,
    RNBD_EVT_NOTI_IND_VALUE,
    RNBD_EVT_STREAM_OPEN,
} RNBD_EVENT_t;

typedef void (*RNBD_EventCallBackFunc)(RNBD_EVENT_t event, int handle, char * message);

/**
 * \ingroup RNBD_INTERFACE
 * Struct of RNBD Interface Function Pointer Prototypes
 */
typedef struct
{
    // RNBD UART interface control
    void (*Write)(uint8_t txData);
    uint8_t (*Read)(void);
    bool (*TransmitDone)(void);
    bool (*DataReady)(void);
    // RNBD RX_IND pin control
    void (*IndicateRx)(bool value);
    // RNBD Reset pin control
    void (*ResetModule)(bool value);
    // RNBD Mode pin set
    void (*SetSystemMode)(RNBD_SYSTEM_MODES_t mode);
    // Delay API
    void (*DelayMs)(uint32_t delayCount);
    // Status Message Handler
    void (*AsyncHandler)(char * message);
    // Added Function for callback to application layer
    // RNBD_EventCallBackFunc * EventCallBackFunc;
    // void (*EventCallBackFunc)(RNBD_EVENT_t event, int handle, char * message);
} iRNBD_FunctionPtrs_t;

extern const iRNBD_FunctionPtrs_t RNBD;

/**
 * \ingroup RNBD_INTERFACE
 * \brief Checks Connected State of RNBD
 * \return Connected Status
 * \retval true - Connected.
 * \retval false - Not Connected
 */
bool RNBD_IsConnected(void);
uint8_t UART_CDC_Read(void);
void UART_CDC_write(uint8_t buffer);
size_t UART_CDC_DataReady(void);
uint8_t UART_BLE_Read(void);
void UART_BLE_write(uint8_t buffer);
size_t UART_BLE_DataReady(void);
bool UART_BLE_TransmitDone(void);
bool RNBD_IsOTAComplete(void);

#endif /* RNBD_INTERFACE_H */