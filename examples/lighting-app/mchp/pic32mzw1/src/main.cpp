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
#define MAIN_TASK_STACK_SIZE (4096)
#define MAIN_TASK_PRIORITY 2

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
extern "C" void vApplicationIdleHook(void)
{
    // FreeRTOS Idle callback
}

extern "C" void vApplicationDaemonTaskStartupHook()
{
    PIC32_LOG("vApplicationDaemonTaskStartupHook\r\n");
    // Init Chip memory management before the stack
    chip::Platform::MemoryInit();

    /* Create the Main task. */
    xTaskCreate(main_task, "Main task", MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, NULL);
}

extern "C" void vApplicationStackOverflowHook( TaskHandle_t pxTask, signed char *pcTaskName )
{
   ( void ) pcTaskName;
   ( void ) pxTask;

   /* Run time task stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook  function is
   called if a task stack overflow is detected.  Note the system/interrupt
   stack is not checked. */
   taskDISABLE_INTERRUPTS();
   for( ;; );
}



static void main_task(void * pvParameters)
{
    
    PIC32_LOG("main_task\r\n");

#if 0
    CHIP_ERROR ret = chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init();

    if (ret != CHIP_NO_ERROR)
    {
        PIC32_LOG("PersistedStorage::KeyValueStoreMgrImpl().Init() failed");
        appError(ret);
    }
#endif
    CHIP_ERROR ret; 
    ret = PlatformMgr().InitChipStack();
    if (ret != CHIP_NO_ERROR)
    {
        PIC32_LOG("PlatformMgr().InitChipStack() failed");
        //appError(ret);
    }
#if 0
    ret = chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName("PIC32MZW1_LIGHT");
    if (ret != CHIP_NO_ERROR)
    {
        PIC32_LOG("ConnectivityMgr().SetBLEDeviceName() failed");
        appError(ret);
    }
#endif
    PIC32_LOG("Starting Platform Manager Event Loop");
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
    

    printf("test log31\r\n");
    //PIC32_LOG("main_task exit\r\n");
    printf("test log4\r\n");
    
    //while (1)
    //{
        //PIC32_LOG("main_task exit\r\n");
        //vTaskDelay(50/portTICK_PERIOD_MS);
    //}
    vTaskDelete(NULL);
}

// ================================================================================
// Main Code
// ================================================================================
int main(void)
{
    SYS_Initialize( NULL );
	
	//SYS_CONSOLE_MESSAGE("\n\rReceived Characters:");
    
    PIC32_LOG("==================================================\r\n");
    PIC32_LOG("chip-pic32mzw1-lighting-example starting Version %d\r\n", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION);
    PIC32_LOG("==================================================\r\n");
    
    printf("test log1\r\n");
    // Init Chip memory management before the stack
    chip::Platform::MemoryInit();
    
    //printf("test log2\r\n");
    xTaskCreate(main_task, "Main task", MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, NULL);
    printf("test log3\r\n");
    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();
    
    //PIC32_LOG("Test is finish\r\n");
    printf("Test is finish\r\n");
    while (1);

#if 0

    init_wfi32Platform();
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    // Clear watchdog timer (started by bootloader) so that it doesn't trigger a reset
    cyhal_wdt_t wdt_obj;
    cyhal_wdt_init(&wdt_obj, cyhal_wdt_get_max_timeout_ms());
    cyhal_wdt_free(&wdt_obj);
#endif
#ifdef HEAP_MONITORING
    MemMonitoring::startHeapMonitoring();
#endif

    PIC32_LOG("==================================================\r\n");
    PIC32_LOG("chip-pic32mzw1-lighting-example starting Version %d\r\n", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION);
    PIC32_LOG("==================================================\r\n");

    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();

    chip::Platform::MemoryShutdown();
    PlatformMgr().StopEventLoopTask();
    PlatformMgr().Shutdown();

    // Should never get here.
    PIC32_LOG("vTaskStartScheduler() failed");
#endif
}
