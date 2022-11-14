/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "ButtonHandler.h"
#include "AppConfig.h"
#include "AppTask.h"


namespace {
constexpr int kButtonCount = 2;

TimerHandle_t buttonTimers[kButtonCount]; // FreeRTOS timers used for debouncing
// buttons. Array to hold handles to
// the created timers.

} // namespace

void ButtonHandler::Init(void)
{
    GpioInit();
    // Create FreeRTOS sw timers for debouncing buttons.
    for (uint8_t i = 0; i < kButtonCount; i++)
    {
        buttonTimers[i] = xTimerCreate("BtnTmr",                      // Just a text name, not used by the RTOS kernel
                                       APP_BUTTON_DEBOUNCE_PERIOD_MS, // timer period
                                       false,                         // no timer reload (==one-shot)
                                       (void *) (int) i,              // init timer id = button index
                                       TimerCallback                  // timer callback handler (all buttons use
                                                                      // the same timer cn function)
        );
    }
}
typedef  void (*GPIO_PIN_CALLBACK) ( GPIO_PIN pin, uintptr_t context);
void ButtonHandler::GpioInit(void)
{
    bool result;
    result = GPIO_PinInterruptCallbackRegister( GPIO_PIN_RB8, light_button_callback, NULL);
    if (!result)
        PIC32_LOG("Light button interrupt set fail..");
    GPIO_PinIntEnable(GPIO_PIN_RB8, GPIO_INTERRUPT_ON_MISMATCH);
    
    result = GPIO_PinInterruptCallbackRegister( GPIO_PIN_RA10, func_button_callback, NULL);
    if (!result)
        PIC32_LOG("Function button interrupt set fail..");
    GPIO_PinIntEnable(GPIO_PIN_RA10, GPIO_INTERRUPT_ON_MISMATCH);
    
}
void ButtonHandler::light_button_callback(GPIO_PIN pin, uintptr_t context)
{
    portBASE_TYPE taskWoken = pdFALSE;
    xTimerStartFromISR(buttonTimers[APP_LIGHT_BUTTON_IDX], &taskWoken);
}

void ButtonHandler::func_button_callback(GPIO_PIN pin, uintptr_t context)
{
    portBASE_TYPE taskWoken = pdFALSE;
    xTimerStartFromISR(buttonTimers[APP_FUNCTION_BUTTON_IDX], &taskWoken);
}

void ButtonHandler::TimerCallback(TimerHandle_t xTimer)
{
    // Get the button index of the expired timer and call button event helper.
    uint32_t timerId;
    uint8_t buttonevent = 0;
    timerId             = (uint32_t) pvTimerGetTimerID(xTimer);
    if (timerId)
    {
        buttonevent = SWITCH1_Get();
    }
    else
    {
        buttonevent = SWITCH2_Get();
    }
    GetAppTask().ButtonEventHandler(timerId, (buttonevent) ? APP_BUTTON_RELEASED : APP_BUTTON_PRESSED);
}
