/**
 * \file rnbd_interface.c
 * \brief This file provides and interface between the RNBD and the hardware.
 */
/*
    (c) 2023 Microchip Technology Inc. and its subsidiaries.

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
#include "rnbd_interface.h"
#include "definitions.h"
#include <stdio.h>
#include <string.h>

static bool connected = false, OTAComplete = false; //**< RNBD connection state */
static uint32_t delay_ms_cycles = CPU_CLOCK_FREQUENCY / 1000;
static uint8_t readbuffer[1];
static size_t dummyread = 0;
static uint8_t cdcreadbuffer[1];

/**
 * \ingroup RNBD_INTERFACE
 * \brief Initiate Hardware Reset of RNBD
 *
 * This API is used to issue a hardware Reset to the RNBD.
 *
 *
 * \param value true - Reset State false - Normal State
 * \return Nothing
 */
static void RNBD_Reset(bool value);

/**
 * \ingroup RNBD_INTERFACE
 * \brief Notify RNBD to expect incoming data
 *
 * This API notifies the RNBD Device to expect incoming Data.
 *
 * \param value true - RX Incoming false - No Data Incoming
 * \return Nothing
 *
 */
static void RNBD_IndicateRx(bool value);

/**
 * \ingroup RNBD_INTERFACE
 * \brief Set the RNBD System Configuration Mode
 *
 * This API sets the System Configuration Mode to Either Normal application Mode
 * or Test Mode/Flash Update/EEPROM configuration
 *
 * \param mode APPLICATION_MODE - Normal Application Mode TEST_MODE - Test Mode/Flash Update/EEPROM Configuration
 * \return Nothing
 */
static void RNBD_SetSystemMode(RNBD_SYSTEM_MODES_t mode);

/**
 * \ingroup RNBD_INTERFACE
 * \brief Handle RNBD Status Message
 *
 * This API is used to handle incoming status messages.
 * It prints all status messages, If DISCONNECT or STREAM_OPEN is received it manages
 * the state of bool connected.
 *
 * \param message Passed status message
 * \return Nothing
 */
static void RNBD_MessageHandler(char * message);

/**
 * \ingroup RNBD_INTERFACE
 * \brief Write API transmitting to RNBD module
 *
 * This API is used to send data bytes to the RNBD module
 *
 * HINT: This API is in place to give compile time memory allocation.
 *       Functionality exist locally within file.
 *       Use of IN LINE to prevent additional stack depth requirement.
 *       APIs can be injected in place if suitable to save (1) stack depth level
 *
 * \param txData - data byte to send
 * \return Nothing
 */
static inline void RNBD_Write(uint8_t txData);

/**
 * \ingroup RNBD_INTERFACE
 * \brief Read API to capture data bytes from RNBD module
 *
 * This API is used to receive data bytes to the RNBD module
 *
 * HINT: This API is in place to give compile time memory allocation.
 *       Functionality exist locally within file.
 *       Use of IN LINE to prevent additional stack depth requirement.
 *       APIs can be injected in place if suitable to save (1) stack depth level
 *
 * \param N/A
 * \return uint8_t readDataByte - Byte captured from RNBD module
 */
static inline uint8_t RNBD_read(void);

/**
 * \ingroup RNBD_INTERFACE
 * \brief Returns if Write to RNBD module was completed
 *
 * This API is used to receive status of communication with RNBD module
 *
 * HINT: This API is in place to give compile time memory allocation.
 *       Functionality exist locally within file.
 *       Use of IN LINE to prevent additional stack depth requirement.
 *       APIs can be injected in place if suitable to save (1) stack depth level
 *
 * \param N/A
 * \return bool status - RNBD is ready for another data byte
 */
static inline bool RNBD_is_tx_done(void);

/**
 * \ingroup RNBD_INTERFACE
 * \brief Returns if Read from RNBD module is ready
 *
 * This API is used to receive status of communication with RNBD module
 *
 * HINT: This API is in place to give compile time memory allocation.
 *       Functionality exist locally within file.
 *       Use of IN LINE to prevent additional stack depth requirement.
 *       APIs can be injected in place if suitable to save (1) stack depth level
 *
 * \param N/A
 * \return bool status - RNBD is ready to provide another byte
 */
static inline bool RNBD_is_rx_ready(void);

/**
 * \ingroup RNBD_INTERFACE
 * \brief Returns if Read from RNBD module is ready
 *
 * This API is used to receive status of communication with RNBD module
 *
 * HINT: This API is in place to give compile time memory allocation.
 *       Functionality exist locally within file.
 *       Use of IN LINE to prevent additional stack depth requirement.
 *       APIs can be injected in place if suitable to save (1) stack depth level
 *
 * \param N/A
 * \return bool status - RNBD is ready to provide another byte
 */
static inline void RNBD_Delay(uint32_t delayCount);
/*****************************************************
 *   *OPTIONAL* APPLICATION MESSAGE FORMATTING API(s)
 ******************************************************/
/**
 * \ingroup RNBD_MESSAGE_TYPE
 * Enum of the MESSAGE TYPES supported in Driver Example(s)
 */
typedef enum
{
    DISCONNECT_MSG  = 0,
    STREAM_OPEN_MSG = 1,
    GENERAL_MSG     = 2,
} RNBD_MESSAGE_TYPE;
/**
 * \def GENERAL_PRINT_SIZE_LIMIT
 * This macro provide a definition used to process
 */
#define GENERAL_PRINT_SIZE_LIMIT (100)
/**
 * \ingroup RNBD_MESSAGE
 * \brief Prints the START Message "<<< " for UART_CDC
 *
 * This API prints *Optional Application* Messages
 *
 * \param N/A
 * \return N/A
 */
static inline void RNBD_PrintMessageStart(void);
/**
 * \ingroup RNBD_MESSAGE
 * \brief Prints the END Message ">>>\r\n" for UART_CDC
 *
 * This API prints *Optional Application* Messages
 *
 * \param N/A
 * \return N/A
 */
static inline void RNBD_PrintMessageEnd(void);
/**
 * \ingroup RNBD_MESSAGE
 * \brief Prints the Indicator [ or ] to UART_CDC
 *        [ - Disconnected | ] - Connected
 *
 * This API prints *Optional Application* Messages
 *
 * \param N/A
 * \return N/A
 */
static inline void RNBD_PrintIndicatorCharacters(RNBD_MESSAGE_TYPE msgType);
/**
 * \ingroup RNBD_MESSAGE
 *
 * This API prints *Optional Application* Messages
 *
 * \param N/A
 * \return N/A
 */
static inline void RNBD_PrintMessage(char * passedMessage);
static int conn_id;
/*****************************************************
 *   Driver Instance Declaration(s) API(s)
 ******************************************************/

const iRNBD_FunctionPtrs_t RNBD = { .Write         = RNBD_Write,
                                    .Read          = RNBD_read,
                                    .TransmitDone  = RNBD_is_tx_done,
                                    .DataReady     = RNBD_is_rx_ready,
                                    .IndicateRx    = RNBD_IndicateRx,
                                    .ResetModule   = RNBD_Reset,
                                    .SetSystemMode = RNBD_SetSystemMode,
                                    .DelayMs       = RNBD_Delay,
                                    .AsyncHandler  = RNBD_MessageHandler };

RNBD_EventCallBackFunc EventCallBackFunc;

/*****************************************************
 *   Driver Public API
 ******************************************************/

bool RNBD_IsConnected(void)
{
    return connected;
}

bool RNBD_IsOTAComplete(void)
{
    return OTAComplete;
}

/*****************************************************
 *   Driver Implementation Private API(s)
 ******************************************************/

static inline void RNBD_Write(uint8_t txData)
{
    UART_BLE_write(txData);
}

static inline uint8_t RNBD_read(void)
{
    return UART_BLE_Read();
}

static inline bool RNBD_is_tx_done(void)
{
    return (UART_BLE_TransmitDone());
}

static inline bool RNBD_is_rx_ready(void)
{
    return UART_BLE_DataReady();
}

uint8_t UART_CDC_Read(void)
{
    return 0;
}
void UART_CDC_write(uint8_t buffer)
{

}
size_t UART_CDC_DataReady(void)
{
    return 0;
}
uint8_t UART_BLE_Read(void)
{
    dummyread = 0;
    while (dummyread == 0)
    {
        dummyread = UART2_Read(readbuffer, 1);
    }
    return *(uint8_t *) readbuffer;
}

void UART_BLE_write(uint8_t buffer)
{
    dummyread = UART2_Write(&buffer, 1);
}

size_t UART_BLE_DataReady(void)
{
    return UART2_ReadCountGet();
}

bool UART_BLE_TransmitDone(void)
{
    return UART2_WriteCountGet() ? false : true;
}

static inline void RNBD_Delay(uint32_t delayCount)
{
    if (delayCount > 0U)
    {
        delayCount *= delay_ms_cycles;
        while (delayCount--)
        {
            asm("nop");
        }
    }
}

static void RNBD_Reset(bool value)
{
    if (true == value)
    {
        BT_RST_Clear();
    }
    else
    {
        BT_RST_Set();
    }
}

static void RNBD_IndicateRx(bool value)
{
    // Implement desired support code for RX Indicator
}

static void RNBD_SetSystemMode(RNBD_SYSTEM_MODES_t mode)
{
    // Implement desired support code for system Mode
}

/*****************************************************
 *   *Optional* Message Formatting Private API(s)
 ******************************************************/
static inline void RNBD_PrintMessageStart(void)
{
    UART_CDC_write((uint8_t) '<');
    UART_CDC_write((uint8_t) '<');
    UART_CDC_write((uint8_t) '<');
    UART_CDC_write((uint8_t) ' ');
}
static inline void RNBD_PrintMessageEnd(void)
{
    UART_CDC_write((uint8_t) ' ');
    UART_CDC_write((uint8_t) '>');
    UART_CDC_write((uint8_t) '>');
    UART_CDC_write((uint8_t) '>');
    UART_CDC_write((uint8_t) ' ');
    UART_CDC_write((uint8_t) '\r');
    UART_CDC_write((uint8_t) '\n');
}
static inline void RNBD_PrintIndicatorCharacters(RNBD_MESSAGE_TYPE msgType)
{
    if (DISCONNECT_MSG == msgType)
    {
        UART_CDC_write((uint8_t) '[');
    }
    else if (STREAM_OPEN_MSG == msgType)
    {
        UART_CDC_write((uint8_t) ']');
    }
    else
    {
    }
}
static inline void RNBD_PrintMessage(char * passedMessage)
{
    char printCharacter[GENERAL_PRINT_SIZE_LIMIT];
    strcpy(printCharacter, passedMessage);
    for (uint8_t messageIndex = 0; messageIndex < strlen(passedMessage); messageIndex++)
    {
        UART_CDC_write((uint8_t) printCharacter[messageIndex]);
    }
}

static char * RNBD_GetToken(char * msg, int item_idx)
{
    char * token;
    int i = 0;

    token = strtok(msg, ",");
    if (token != NULL)
    {
        for (i = 0; i < (item_idx - 1); i++)
        {
            token = strtok(NULL, ",");
            if (token == NULL)
                break;
        }
    }

    return token;
}

static void RNBD_MessageHandler(char * message)
{

    RNBD_MESSAGE_TYPE messageType = GENERAL_MSG;

    if (strstr(message, "DISCONNECT") != NULL)
    {
        messageType = DISCONNECT_MSG;
        connected   = false;
        OTAComplete = false;
        SYS_CONSOLE_PRINT("BLE DISCONNECT\r\n");

        RNBD_EVENT_t event = RNBD_EVT_DISCONNECT;
        int handle;

        conn_id = 0;

        EventCallBackFunc(event, handle, NULL);
    }
    else if (strstr(message, "CONNECT") != NULL)
    {
        SYS_CONSOLE_PRINT("BLE CONNECT\r\n");
        int handle = 0;
        char * conn_handle_str;
        RNBD_EVENT_t event = RNBD_EVT_CONNECT;

        conn_handle_str = RNBD_GetToken(message, 4);
        if (conn_handle_str != NULL)
        {
            handle  = atoi(conn_handle_str);
            conn_id = handle;
        }
        EventCallBackFunc(event, handle, message);
    }
    else if (strstr(message, "WV,") != NULL)
    {
        int handle = 0;
        // char * conn_handle_str;
        char conn_handle_str[4];

        // strcpy(temp_buf, message);
        RNBD_EVENT_t event = RNBD_EVT_WRITE_REQ;

        // conn_handle_str = RNBD_GetToken(temp_buf, 2);
        strncpy(conn_handle_str, &message[3], 4);
        if (conn_handle_str != NULL)
            handle = atoi(conn_handle_str);

        EventCallBackFunc(event, conn_id, message);
    }
    else if (strstr(message, "WC,") != NULL)
    {
        int handle = 0;
        char conn_handle_str[4];

        RNBD_EVENT_t event = RNBD_EVT_NOTI_IND_REQ;

        strncpy(conn_handle_str, &message[3], 4);
        if (conn_handle_str != NULL)
            handle = atoi(conn_handle_str);

        EventCallBackFunc(event, conn_id, message);
    }
    else if (strstr(message, "STREAM_OPEN") != NULL)
    {
        messageType = STREAM_OPEN_MSG;
        connected   = true;
        SYS_CONSOLE_PRINT("BLE STREAM OPEN\r\n");
    }
    else if (strstr(message, "OTA_REQ") != NULL)
    {
        OTAComplete = true;
        RNBD.Write('\r');
        RNBD.Write('\n');

        RNBD.Write('O');
        RNBD.Write('T');
        RNBD.Write('A');
        RNBD.Write('A');
        RNBD.Write(',');
        RNBD.Write('0');
        RNBD.Write('1');
        RNBD.Write('\r');
        RNBD.Write('\n');
    }
    else
    {
        SYS_CONSOLE_PRINT("RNBD_MessageHandler: %s\r\n", message);
        messageType = GENERAL_MSG;
    }

}
