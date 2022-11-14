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

#include <stdio.h>
#include "definitions.h"

// ---- Lighting Example App Config ----

#define APP_TASK_NAME "APP"

#define APP_LIGHT_BUTTON_IDX 0
#define APP_FUNCTION_BUTTON_IDX 1

#define APP_LIGHT_BUTTON 1
#define APP_FUNCTION_BUTTON 2
#define APP_BUTTON_DEBOUNCE_PERIOD_MS 50

#define APP_BUTTON_PRESSED 0
#define APP_BUTTON_RELEASED 1

#define SYSTEM_STATE_LED 1
#define LIGHT_LED GPIO_PIN_RK13

// Time it takes in ms for the simulated actuator to move from one
// state to another.
#define ACTUATOR_MOVEMENT_PERIOS_MS 1

// ---- Thread Polling Config ----
#define THREAD_ACTIVE_POLLING_INTERVAL_MS 100
#define THREAD_INACTIVE_POLLING_INTERVAL_MS 1000

// PIC32 Logging
#ifdef __cplusplus
extern "C" {
#endif

void appError(int err);
void pic32mzw1_Log(const char * aFormat, ...);
void pic32mzw1_Log_Init(void);

#define PIC32_LOG(...) pic32mzw1_Log(__VA_ARGS__)
#define PIC32_LOG_DBG(level,fmt,...) SYS_DEBUG_PRINT(level, "[PIC32MZW1] " fmt ,##__VA_ARGS__)
#define PIC32_LOG_INIT pic32mzw1_Log_Init()

#ifdef __cplusplus
}

#include <lib/core/CHIPError.h>
void appError(CHIP_ERROR error);
#endif
