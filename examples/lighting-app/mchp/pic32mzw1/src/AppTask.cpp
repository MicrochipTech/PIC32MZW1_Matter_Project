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

#include "AppTask.h"
#include "AppConfig.h"
#include "AppEvent.h"
#include "ButtonHandler.h"
#include "LEDWidget.h"
#include "qrcodegen.h"
#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/server/Dnssd.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <assert.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/wfi32/NetworkCommissioningDriver.h>
#include <DeviceInfoProviderImpl.h>
#include <platform/wfi32/PIC32MZW1Config.h>

/* OTA related includes */
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorDriver.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <platform/wfi32/OTAImageProcessorImpl.h>


using chip::BDXDownloader;
using chip::CharSpan;
using chip::DefaultOTARequestor;
using chip::FabricIndex;
using chip::GetRequestorInstance;
using chip::NodeId;
using chip::System::Layer;

using namespace ::chip;
using namespace chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::System;

#endif
#define FACTORY_RESET_TRIGGER_TIMEOUT 5000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#define APP_TASK_STACK_SIZE (1024)//(4096)
#define APP_TASK_PRIORITY 2
#define APP_EVENT_QUEUE_SIZE 10



namespace {
TimerHandle_t sFunctionTimer; // FreeRTOS app sw timer.

TaskHandle_t sAppTaskHandle;
QueueHandle_t sAppEventQueue;

LEDWidget sStatusLED;
LEDWidget sLightLED;

bool sIsWiFiStationProvisioned = false;
bool sIsWiFiStationEnabled     = false;
bool sIsWiFiStationConnected   = false;
bool sIsWiFiAPActive = false;


//uint8_t sAppEventQueueBuffer[APP_EVENT_QUEUE_SIZE * sizeof(AppEvent)];
//StaticQueue_t sAppEventQueueStruct;

//StackType_t appStack[APP_TASK_STACK_SIZE / sizeof(StackType_t)];
//StaticTask_t appTaskStruct;

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
DefaultOTARequestor gRequestorCore;
DefaultOTARequestorStorage gRequestorStorage;
DefaultOTARequestorDriver gRequestorUser;
BDXDownloader gDownloader;
OTAImageProcessorImpl gImageProcessor;
#endif

} // namespace

using namespace ::chip;
using namespace chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

AppTask AppTask::sAppTask;
static chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

namespace {
app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */, &(NetworkCommissioning::PIC32WiFiDriver::GetInstance()));
} // namespace

void NetWorkCommissioningInstInit()
{
    sWiFiNetworkCommissioningInstance.Init();
}

static void InitServer(intptr_t context)
{
    PIC32_LOG("InitServer Enter..\r\n");
    
    //gExampleDeviceInfoProvider.SetStorageDelegate(&Server::GetInstance().GetPersistentStorage());
    ///chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);
    
    // Init ZCL Data Model
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();

    chip::Server::GetInstance().Init(initParams);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR    
    GetAppTask().InitOTARequestor();
#endif
}

CHIP_ERROR AppTask::StartAppTask()
{
    sAppEventQueue = xQueueCreate(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent));
    if (sAppEventQueue == NULL)
    {
        PIC32_LOG("Failed to allocate app event queue");
        appError(APP_ERROR_EVENT_QUEUE_FAILED);
    }

    // Start App task.
    xTaskCreate(AppTaskMain, APP_TASK_NAME, APP_TASK_STACK_SIZE, NULL, 1, &sAppTaskHandle);
    printf("StartAppTask Exit\r\n");
    return (sAppTaskHandle == nullptr) ? APP_ERROR_CREATE_TASK_FAILED : CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Register the callback to init the MDNS server when connectivity is available
    PlatformMgr().AddEventHandler(
        [](const ChipDeviceEvent * event, intptr_t arg) {
            // Restart the server whenever an ip address is renewed
            if (event->Type == DeviceEventType::kInternetConnectivityChange)
            {
                if (event->InternetConnectivityChange.IPv4 == kConnectivity_Established ||
                    event->InternetConnectivityChange.IPv6 == kConnectivity_Established)
                {
                    chip::app::DnssdServer::Instance().StartServer();
                }
            }
        },
        0);


    chip::DeviceLayer::PlatformMgr().ScheduleWork(InitServer, reinterpret_cast<intptr_t>(nullptr));

    // Initialise WSTK buttons PB0 and PB1 (including debounce).
    ButtonHandler::Init();

    // Create FreeRTOS sw timer for Function Selection.
    sFunctionTimer = xTimerCreate("FnTmr",          // Just a text name, not used by the RTOS kernel
                                  1,                // == default timer period (mS)
                                  false,            // no timer reload (==one-shot)
                                  (void *) this,    // init timer id = app task obj context
                                  TimerEventHandler // timer callback handler
    );
    if (sFunctionTimer == NULL)
    {
        appError(APP_ERROR_CREATE_TIMER_FAILED);
    }

    NetWorkCommissioningInstInit();
    
    PIC32_LOG("Current Firmware Version: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);
    err = LightMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        PIC32_LOG("LightMgr().Init() failed");
        appError(err);
    }

    LightMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    // Initialize LEDs
    sStatusLED.Init(SYSTEM_STATE_LED);
    sLightLED.Init(LIGHT_LED);
    sLightLED.Set(LightMgr().IsLightOn());


    ConfigurationMgr().LogDeviceConfig();
    // Print setup info
    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kOnNetwork));


    PIC32_LOG("AppTask::Init() Exit\r\n");

    return err;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;
    
    PIC32_LOG("AppTaskMain Enter..\r\n");
    
    CHIP_ERROR err = sAppTask.Init();
    
    if (err != CHIP_NO_ERROR)
    {
        appError(err);
    }

    while (true)
    {
        //BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, portMAX_DELAY);
        BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, 10);
        if (eventReceived == pdTRUE)
        {
            PIC32_LOG("AppTaskMain eventReceived..\r\n");
            sAppTask.DispatchEvent(&event);
        }
        
        // Collect connectivity and configuration state from the CHIP stack. Because
        // the CHIP event loop is being run in a separate task, the stack must be
        // locked while these values are queried.  However we use a non-blocking
        // lock request (TryLockCHIPStack()) to avoid blocking other UI activities
        // when the CHIP task is busy (e.g. with a long crypto operation).
        if (PlatformMgr().TryLockChipStack())
        {
            sIsWiFiStationEnabled     = ConnectivityMgr().IsWiFiStationEnabled();
            sIsWiFiStationConnected   = ConnectivityMgr().IsWiFiStationConnected();
            sIsWiFiStationProvisioned = ConnectivityMgr().IsWiFiStationProvisioned();
            sIsWiFiAPActive   = ConnectivityMgr().IsWiFiAPActive();
            PlatformMgr().UnlockChipStack();
        }

        if (sIsWiFiStationEnabled && sIsWiFiStationProvisioned && sIsWiFiStationConnected) // ToDo: check sIsWiFiStationConnected also
        {
            sStatusLED.Set(true);
        }
        else if (sIsWiFiAPActive)
        {
            sStatusLED.Blink(200, 200);
        }
        else
        {
            sStatusLED.Blink(500, 500);
        }
        
        sStatusLED.Animate();
        sLightLED.Animate();

    }

}

void AppTask::LightActionEventHandler(AppEvent * event)
{
    bool initiated = false;
    LightingManager::Action_t action;
    int32_t actor  = 0;
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (event->Type == AppEvent::kEventType_Light)
    {
        action = static_cast<LightingManager::Action_t>(event->LightEvent.Action);
        actor  = event->LightEvent.Actor;
    }
    else if (event->Type == AppEvent::kEventType_Button)
    {
        if (event->ButtonEvent.Action == APP_BUTTON_PRESSED)
        {
            if (LightMgr().IsLightOn())
            {
                action = LightingManager::OFF_ACTION;
            }
            else
            {
                action = LightingManager::ON_ACTION;
            }
            actor = AppEvent::kEventType_Button;
        }
        else
        {
            err = APP_ERROR_UNHANDLED_EVENT;
        }
            
    }
    else
    {
        err = APP_ERROR_UNHANDLED_EVENT;
    }

    if (err == CHIP_NO_ERROR)
    {
        initiated = LightMgr().InitiateAction(actor, action);

        if (!initiated)
        {
            PIC32_LOG("Action is already in progress or active.");
        }
    }
}

void AppTask::ButtonEventHandler(uint8_t btnIdx, uint8_t btnAction)
{
    if (btnIdx != APP_LIGHT_BUTTON_IDX && btnIdx != APP_FUNCTION_BUTTON_IDX)
    {
        return;
    }

    AppEvent button_event              = {};
    button_event.Type                  = AppEvent::kEventType_Button;
    button_event.ButtonEvent.ButtonIdx = btnIdx;
    button_event.ButtonEvent.Action    = btnAction;

    if (btnIdx == APP_LIGHT_BUTTON_IDX)
    {
        button_event.Handler = LightActionEventHandler;
        sAppTask.PostEvent(&button_event);
    }
    else if (btnIdx == APP_FUNCTION_BUTTON_IDX)
    {
        button_event.Handler = FunctionHandler;
        sAppTask.PostEvent(&button_event);
    }
}

void AppTask::TimerEventHandler(TimerHandle_t timer)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = (void *) timer;
    event.Handler            = FunctionTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::FunctionTimerEventHandler(AppEvent * event)
{

    if (event->Type != AppEvent::kEventType_Timer)
    {
        return;
    }

    // If we reached here, the button was held past FACTORY_RESET_TRIGGER_TIMEOUT,
    // initiate factory reset
    if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == Function::kFactoryReset)
    {
        PIC32_LOG("Factory Reset Triggered");

        // Actually trigger Factory Reset
        sAppTask.mFunction = Function::kNoneSelected;
        chip::Server::GetInstance().ScheduleFactoryReset();
        
    }
}

void AppTask::FunctionHandler(AppEvent * event)
{
    // To trigger software update: press the APP_FUNCTION_BUTTON button briefly (<
    // FACTORY_RESET_TRIGGER_TIMEOUT) To initiate factory reset: press the
    // APP_FUNCTION_BUTTON for FACTORY_RESET_TRIGGER_TIMEOUT +
    // FACTORY_RESET_CANCEL_WINDOW_TIMEOUT All LEDs start blinking after
    // FACTORY_RESET_TRIGGER_TIMEOUT to signal factory reset has been initiated.
    // To cancel factory reset: release the APP_FUNCTION_BUTTON once all LEDs
    // start blinking within the FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    if (event->ButtonEvent.Action == APP_BUTTON_PRESSED)
    {
        if (!sAppTask.mFunctionTimerActive && sAppTask.mFunction == Function::kNoneSelected)
        {
            PIC32_LOG("Factory Reset will start in %d ms", FACTORY_RESET_TRIGGER_TIMEOUT);
            sAppTask.StartTimer(FACTORY_RESET_TRIGGER_TIMEOUT);
            sAppTask.mFunction = Function::kFactoryReset;
        }
    }
    else
    {
        // If the button was released before factory reset got initiated, start Thread Network
        if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == Function::kFactoryReset)
        {
            sAppTask.CancelTimer();
            sAppTask.mFunction = Function::kNoneSelected;
            PIC32_LOG("Factory Reset has been Canceled");
            
            CHIP_ERROR err;
            char wifimode[] = "softAP";
            err = DeviceLayer::Internal::PIC32MZW1Config::WriteConfigValueStr(DeviceLayer::Internal::PIC32MZW1Config::kConfigKey_WiFiMode, wifimode, strlen(wifimode));
            if (err != CHIP_NO_ERROR)
                PIC32_LOG("Update key 'wifimode' fail..");
            
            PIC32_LOG("System reset..");
            DeviceLayer::Internal::PIC32MZW1Config::SystemReset();

        }
        else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == Function::kFactoryReset)
        {
            // Set Light status LED back to show state of light.
            //sLightLED.Set(LightMgr().IsLightOn());

            sAppTask.CancelTimer();

            // Change the function to none selected since factory reset has been
            // canceled.
            sAppTask.mFunction = Function::kNoneSelected;

            PIC32_LOG("Button is release");
        }
        else{
            sAppTask.CancelTimer();
            sAppTask.mFunction = Function::kNoneSelected;
        }
    }
}

void AppTask::CancelTimer()
{
    if (xTimerStop(sFunctionTimer, 0) == pdFAIL)
    {
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }

    mFunctionTimerActive = false;
}

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    if (xTimerIsTimerActive(sFunctionTimer))
    {
        PIC32_LOG("app timer already started!");
        CancelTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(sFunctionTimer, aTimeoutInMs / portTICK_PERIOD_MS, 100) != pdPASS)
    {
        appError(APP_ERROR_START_TIMER_FAILED);
    }

    mFunctionTimerActive = true;
}

void AppTask::ActionInitiated(LightingManager::Action_t action, int32_t actor)
{
    // Action initiated, update the light led
    if (action == LightingManager::ON_ACTION)
    {
        PIC32_LOG("Turning light ON");
        sLightLED.Set(true);
    }
    else if (action == LightingManager::OFF_ACTION)
    {
        PIC32_LOG("Turning light OFF");
        sLightLED.Set(false);
    }

    if (actor == AppEvent::kEventType_Button)
    {
        sAppTask.mSyncClusterToButtonAction = true;
    }
}

void AppTask::ActionCompleted(LightingManager::Action_t action)
{
    // action has been completed bon the light
    if (action == LightingManager::ON_ACTION)
    {
        PIC32_LOG("Light ON");
    }
    else if (action == LightingManager::OFF_ACTION)
    {
        PIC32_LOG("Light OFF");
    }

    if (sAppTask.mSyncClusterToButtonAction)
    {
        chip::DeviceLayer::PlatformMgr().ScheduleWork(UpdateClusterState, reinterpret_cast<intptr_t>(nullptr));
        sAppTask.mSyncClusterToButtonAction = false;
    }
}

void AppTask::PostLightActionRequest(int32_t actor, LightingManager::Action_t action)
{
    AppEvent event;
    event.Type              = AppEvent::kEventType_Light;
    event.LightEvent.Actor  = actor;
    event.LightEvent.Action = action;
    event.Handler           = LightActionEventHandler;
    PostEvent(&event);
}

void AppTask::PostEvent(const AppEvent * event)
{
    if (sAppEventQueue != NULL)
    {
        BaseType_t status;
        //if (xPortIsInsideInterrupt())
        if (0)
        {
            BaseType_t higherPrioTaskWoken = pdFALSE;
            status                         = xQueueSendFromISR(sAppEventQueue, event, &higherPrioTaskWoken);

#ifdef portYIELD_FROM_ISR
            portYIELD_FROM_ISR(higherPrioTaskWoken);
#elif portEND_SWITCHING_ISR // portYIELD_FROM_ISR or portEND_SWITCHING_ISR
            portEND_SWITCHING_ISR(higherPrioTaskWoken);
#else                       // portYIELD_FROM_ISR or portEND_SWITCHING_ISR
//#error "Must have portYIELD_FROM_ISR or portEND_SWITCHING_ISR"
#endif // portYIELD_FROM_ISR or portEND_SWITCHING_ISR
        }
        else
        {
            status = xQueueSend(sAppEventQueue, event, 1);
        }

        if (!status)
            PIC32_LOG("Failed to post event to app task event queue");
    }
    else
    {
        PIC32_LOG("Event Queue is NULL should never happen");
    }
}

void AppTask::DispatchEvent(AppEvent * event)
{
    if (event->Handler)
    {
        event->Handler(event);
    }
    else
    {
        PIC32_LOG("Event received with no handler. Dropping event.");
    }
}

void AppTask::UpdateClusterState(intptr_t context)
{
    uint8_t newValue = LightMgr().IsLightOn();

    // write the new on/off value
    EmberAfStatus status =
        emberAfWriteAttribute(1, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, (uint8_t *) &newValue, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        PIC32_LOG("ERR: updating on/off %x", status);
    }

}
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
void AppTask::InitOTARequestor()
{
    PIC32_LOG("InitOTARequestor In" );
    SetRequestorInstance(&gRequestorCore);
    ConfigurationMgr().StoreSoftwareVersion(CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION);
    gRequestorStorage.Init(chip::Server::GetInstance().GetPersistentStorage());
    gRequestorCore.Init(chip::Server::GetInstance(), gRequestorStorage, gRequestorUser, gDownloader);
    gImageProcessor.SetOTADownloader(&gDownloader);
    gDownloader.SetImageProcessorDelegate(&gImageProcessor);

    gRequestorUser.Init(&gRequestorCore, &gImageProcessor);

    PIC32_LOG("Current Software Version: %u", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION);
    PIC32_LOG("Current Software Version String: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);
}
#endif
