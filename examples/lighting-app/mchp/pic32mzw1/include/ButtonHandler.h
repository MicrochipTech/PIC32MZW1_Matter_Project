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

#pragma once

#include <stdint.h>
#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support
#include <peripheral/gpio/plib_gpio.h>

#define GPIO_INTERRUPT_PRIORITY (5)

class ButtonHandler
{
public:
    static void Init(void);

private:
    static void GpioInit(void);
    static void light_button_callback(GPIO_PIN pin, uintptr_t context);
    static void func_button_callback(GPIO_PIN pin, uintptr_t context);
    static void TimerCallback(TimerHandle_t xTimer);
};
