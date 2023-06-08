/**
 * \file rnbd_example.c
 * \brief This file contains the APIs required to communicate with the RNBD driver library to
 *        create, and open a basic Transparent EUSART demonstration.
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

// #include "rnbd_example.h"
#include "../rnHostLib/rnbd.h"
#include "../rnHostLib/rnbd_interface.h"
#include "definitions.h" // SYS function prototypes
#include <stdbool.h>

// DOM-IGNORE-BEGIN
#ifdef __cplusplus // Provide C++ Compatibility

extern "C" {

#endif
/** MACRO used to configure the application used buffer sizes.
 *  This is used by the application for communication buffers.
 */
#define MAX_BUFFER_SIZE (255 * 2)

/**< Status Buffer instance passed to RNBD drive used for Asynchronous Message Handling (see *asyncBuffer in rnbd.c) */
static char statusBuffer[MAX_BUFFER_SIZE];

/**
 * \ingroup RN_Example
 * \brief Example Implementation of Transparent UART
 *        This can be connected to a Smart BLE 'Terminal'
 *        application for simple data exchange demonstration.
 * * For more details, refer RNBD user guide.
 * \return Connected Status
 * \retval true - Connected.
 * \retval false - Not Connected
 */
static bool RNBD_Example_TransparentUart(void);
/**
 * \ingroup RNBD_Example_Run
 * \brief This 'Runs' the example applications While(1) loop
 *
 * * For more details, refer RNBD user guide.
 * \param none
 * \return void
 */
static void RNBD_Example_Run(void);

bool RNBD_Example_Initialized(void)
{

    bool exampleIsInitialized = false;
    bool isEnterCmdMode       = false;

    RNBD_Init();

    exampleIsInitialized = RNBD_SetAsyncMessageHandler(statusBuffer, (int) sizeof(statusBuffer));

    if (exampleIsInitialized == true)
    {
        isEnterCmdMode = RNBD_EnterCmdMode();
        SYS_CONSOLE_PRINT("isEnterCmdMode = %d\r\n", isEnterCmdMode);

        RNBD_FactoryReset(SF_2); // ToDo: use SF_2 ?

        vTaskDelay(200);

        RNBD_Init();

        isEnterCmdMode = RNBD_EnterCmdMode();
        SYS_CONSOLE_PRINT("isEnterCmdMode = %d\r\n", isEnterCmdMode);

        // SYS_CONSOLE_PRINT("BLEManagerImpl::_OnPlatformEvent, res1 = %d", res);
#if 0
        res = RNBD_ClearAdvData(PERMANENT_CNAHGE_MODE);
        SYS_CONSOLE_PRINT("BLEManagerImpl::_OnPlatformEvent, res2 = %d", res);

        res = RNBD_SetAdvData(PERMANENT_CNAHGE_MODE, "01", "06");
        SYS_CONSOLE_PRINT("BLEManagerImpl::_OnPlatformEvent, res3 = %d", res);
        // res = RNBD_SetAdvData(PERMANENT_CNAHGE_MODE, "16", "FFF6000F00FFF1800100");
        res = RNBD_SetAdvData(PERMANENT_CNAHGE_MODE, "16", "F6FF00000FF1FF018000");
        SYS_CONSOLE_PRINT("BLEManagerImpl::_OnPlatformEvent, res4 = %d", res);
#endif
        // RNBD_Example_Run();
    }
    return (true);
}
static void RNBD_Example_Run(void)
{

    while (true)
    {
        if (true == RNBD_Example_TransparentUart())
        {
            // Connected
        }
        else
        {
            // Not Connected
        }
    }
}
static bool RNBD_Example_TransparentUart(void)
{
    bool isConnected, isOTAComplete;
    isConnected   = RNBD_IsConnected();
    isOTAComplete = RNBD_IsOTAComplete();

    if (true == isConnected && false == isOTAComplete)
    {
        while (RNBD_DataReady())
        {
            UART_CDC_write(RNBD_Read());
        }

        while (UART_CDC_DataReady())
        {
            RNBD.Write(UART_CDC_Read());
        }
    }
    else
    {

        while (RNBD_DataReady())
        {
            UART_CDC_write(RNBD_Read());
        }

        while (UART_CDC_DataReady())
        {
            RNBD.Write(UART_CDC_Read());
        }
    }

    return isConnected;
}

// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif