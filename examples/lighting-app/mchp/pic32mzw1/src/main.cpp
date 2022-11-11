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

#include <stdio.h>
#include <stdlib.h>
#include "definitions.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include <FreeRTOS.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>

#include <AppTask.h>

#include "AppConfig.h"
//#include "cyhal_wdt.h"
#include "init_wfi32Platform.h"
#include <app/server/Server.h>

#ifdef HEAP_MONITORING
#include "MemMonitoring.h"
#endif
#define MAIN_TASK_STACK_SIZE (1024)
#define MAIN_TASK_PRIORITY 2

//#include <strings.h>
using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;

volatile int apperror_cnt;
static void main_task(void * pvParameters);

// ================================================================================
// App Error
//=================================================================================
void appError(int err)
{
    PIC32_LOG("!!!!!!!!!!!! App Critical Error: %d !!!!!!!!!!!", err);
    portDISABLE_INTERRUPTS();
    while (1)
        ;
}

void appError(CHIP_ERROR error)
{
    appError(static_cast<int>(error.AsInteger()));
}

// ================================================================================
// FreeRTOS Callbacks
// ================================================================================
extern "C" void vApplicationDaemonTaskStartupHook()
{
    PIC32_LOG("vApplicationDaemonTaskStartupHook\r\n");
    // Init Chip memory management before the stack
    chip::Platform::MemoryInit();

    /* Create the Main task. */
    xTaskCreate(main_task, "Main task", MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, NULL);
}

static void main_task(void * pvParameters)
{
    
    PIC32_LOG("main_task enter..\r\n");

#if 1
    CHIP_ERROR ret = chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init();

    if (ret != CHIP_NO_ERROR)
    {
        appError(ret);
    }
#endif

    ret = PlatformMgr().InitChipStack();
    if (ret != CHIP_NO_ERROR)
    {
        appError(ret);
    }

    ret = PlatformMgr().StartEventLoopTask();
    if (ret != CHIP_NO_ERROR)
    {
        PIC32_LOG("PlatformMgr().StartEventLoopTask() failed");
        appError(ret);
    }
    ret = GetAppTask().StartAppTask();
    if (ret != CHIP_NO_ERROR)
    {
        PIC32_LOG("GetAppTask().Init() failed");
        appError(ret);
    }
    /* Delete task */
    

    vTaskDelete(NULL);
}

// ================================================================================
// Main Code
// ================================================================================
int main(void)
{
    SYS_Initialize( NULL );
	
    SYS_Tasks();

    PIC32_LOG_INIT;


    PIC32_LOG("==================================================\r\n");
    PIC32_LOG("chip-pic32mzw1-lighting-example starting Version %d\r\n", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION);
    PIC32_LOG("==================================================\r\n");
    
    
    // Init Chip memory management before the stack
    chip::Platform::MemoryInit();
    
    //printf("test log2\r\n");
    xTaskCreate(main_task, "Main task", MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, NULL);
    
    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();
    
    //chip::Platform::MemoryShutdown();
    //PlatformMgr().StopEventLoopTask();
    //PlatformMgr().Shutdown();


#ifdef HEAP_MONITORING
    MemMonitoring::startHeapMonitoring();
#endif


}
