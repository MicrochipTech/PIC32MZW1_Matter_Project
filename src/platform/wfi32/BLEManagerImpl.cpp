/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2020 Nest Labs, Inc.
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

/**
 *    @file
 *          Provides an implementation of the BLEManager singleton object
 *          for the pic32mzw1 platform.
 */

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <BLEManagerImpl.h>
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <ble/CHIPBleServiceData.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/internal/BLEManager.h>
#include "rnbd.h"
#include "rnbd_example.h"

#define BLE_TASK_NAME "BLE"
#define BLE_TASK_STACK_SIZE 1024
#define BLE_RESP_DATA_QUEUE_SIZE 20

using namespace ::chip;
using namespace ::chip::Ble;

#define BLE_SERVICE_DATA_SIZE 10
#define BT_STACK_HEAP_SIZE (1024 * 6)
typedef void (*pfn_free_buffer_t)(uint8_t *);
static bool heap_allocated = false;
//static SemaphoreHandle_t cmd_mode_bin_sem;
static SemaphoreHandle_t rnbd_read_bin_sem;
static SemaphoreHandle_t rnbd_shw_bin_sem;
static bool send_ind_flag = false;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {
const ChipBleUUID chipUUID_CHIPoBLEChar_RX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x11 } };

const ChipBleUUID ChipUUID_CHIPoBLEChar_TX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x12 } };

char Matter_Service_UUID[] = "0000FFF600001000800000805f9b34fb";
char Matter_Char_TX_UUID[] = "18EE2EF5263D4559959F4F9C429F9D11";
char Matter_Char_RX_UUID[] = "18EE2EF5263D4559959F4F9C429F9D12";
char Matter_Char_ADDITIONAL_DATA_UUID[] = "64630238877245F2B87D748A83218F04";
} // unnamed namespace

BLEManagerImpl BLEManagerImpl::sInstance;
RNBD_ble_service_info_t BLEManagerImpl::ble_service_info;

QueueHandle_t BLEResponseDataQueue;

char *  BLEManagerImpl::RNBD_EVENT_GetToken(char * msg, int item_idx)
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

void BLEManagerImpl::BLEManagerCallback(RNBD_EVENT_t event, int handle, char* msg)
{
    
    switch (event)
    {
        case RNBD_EVT_CONNECT:
            ChipLogProgress(DeviceLayer, "[%s] RNBD_EVT_CONNECT", __func__);
            BLEManagerImpl::sInstance.mNumCons ++;
            BLEManagerImpl::sInstance.mCons[0].ConId = handle;
            BLEManagerImpl::sInstance.mCons[0].connected = true;
            break;
        
        case RNBD_EVT_DISCONNECT:
            ChipLogProgress(DeviceLayer, "[%s] RNBD_EVT_DISCONNECT", __func__);
            BLEManagerImpl::sInstance.mNumCons --;
            BLEManagerImpl::sInstance.mCons[0].ConId = 0;
            BLEManagerImpl::sInstance.mCons[0].connected = false;
            break;

        case RNBD_EVT_WRITE_REQ:
            ChipLogProgress(DeviceLayer, "[%s] RNBD_EVT_WRITE_REQ, msg = %s", __func__, msg);

            char char_handle_str[4];
            uint8_t msg_data[255];
            uint8_t indication_flag;
            int msg_data_len;
            char hexstring[2];   
            int char_handle;         
            char* p;

            strncpy(char_handle_str, &msg[3], 4);
            char_handle = (int) strtol(char_handle_str, NULL, 16);

            p = &msg[8];
            msg_data_len = strlen(p)/2;
            
            /* convert the data from hex string to integer */
            for (int i = 0; i < msg_data_len; i++)
            {
                strncpy(hexstring, p, 2);
                msg_data[i] = (uint8_t)strtol(hexstring, NULL, 16);
                p += 2;
            }
            printf("RNBD_EVT_WRITE_REQ: msg_data_len = %d\r\n", msg_data_len);

            indication_flag = 0;
            for (int i = 0; i < BLEManagerImpl::ble_service_info.num_of_char; i++)
            {
                if (BLEManagerImpl::ble_service_info.characteristics[i].handle == char_handle)
                {
                    if (BLEManagerImpl::ble_service_info.characteristics[i].properties == RNBD_CHAR_PROPERTIES_INDICATE)
                    {
                        indication_flag = 1;
                        if (msg_data[0] > 0)
                            BLEManagerImpl::sInstance.HandleGattServiceNotificationIndicationReq(handle, 1);
                        else
                            BLEManagerImpl::sInstance.HandleGattServiceNotificationIndicationReq(handle, 0);

                    }
                }
            }

            if (!indication_flag)            
                BLEManagerImpl::sInstance.HandleGattServiceWrite(handle, msg_data, msg_data_len);

            break;

        case RNBD_EVT_NOTI_IND_REQ:
            ChipLogProgress(DeviceLayer, "[%s] RNBD_EVT_NOTI_IND_REQ, msg = %s", __func__, msg);

            // do not handle as the enable/ disable of indication is handled at event RNBD_EVT_WRITE_REQ
            
            break;
        default:
            break;

    }
}



void BLEManagerImpl::BLETask(void * pvParameter)
{
    uint8_t resp_data;
    static int ok_counter = 0;

    //xSemaphoreGive(cmd_mode_bin_sem);
    
    while (true)
    {        
        if (pdTRUE  == xSemaphoreTake(rnbd_read_bin_sem, 0))
        {
            resp_data = RNBD_Read();
            //ChipLogProgress(DeviceLayer, "resp_data = 0x%x\r\n", resp_data);
            if (resp_data == 'A' && ok_counter == 0)
                ok_counter ++;
            else if (resp_data == 'O' && ok_counter == 1)
                ok_counter ++;
            else if (resp_data == 'K' && ok_counter == 2)
            {
                xSemaphoreGive(rnbd_shw_bin_sem);
                send_ind_flag = true;
                ok_counter = 0;
                ChipLogProgress(DeviceLayer, "AOK is received\r\n");
            }
            else
            {
                ok_counter = 0;
            }
            xQueueSend(BLEResponseDataQueue, &resp_data, 1);
            
            xSemaphoreGive(rnbd_read_bin_sem);
        }
        
        
        vTaskDelay( 10 ); 
    }
}

CHIP_ERROR BLEManagerImpl::_Init()
{
    CHIP_ERROR err;
    TaskHandle_t sBLETaskHandle;
    bool res = false;
    uint8_t resp_data;
    int i = 0;

    ChipLogProgress(DeviceLayer, "BLEManagerImpl::_Init");
    // Initialize the CHIP BleLayer.
    err = BleLayer::Init(this, this, &DeviceLayer::SystemLayer());
    SuccessOrExit(err);

    //cmd_mode_bin_sem = xSemaphoreCreateBinary();
    rnbd_read_bin_sem = xSemaphoreCreateBinary();
    xSemaphoreGive(rnbd_read_bin_sem);
    
    rnbd_shw_bin_sem = xSemaphoreCreateBinary();
    xSemaphoreGive(rnbd_shw_bin_sem);
    
    BLEResponseDataQueue = xQueueCreate(BLE_RESP_DATA_QUEUE_SIZE, sizeof(uint8_t));
    if (BLEResponseDataQueue == NULL)
    {
        ChipLogProgress(DeviceLayer, "BLEManagerImpl::_Init, Fail to create queue..");
    }

    RNBD_RegisterCallbackFunc(BLEManagerCallback);

    RNBD_Example_Initialized();

    RNBD_SetServiceUUID(Matter_Service_UUID);
    RNBD_SetCharacteristic(Matter_Char_TX_UUID, 0x08, 0xFF);
    RNBD_SetCharacteristic(Matter_Char_RX_UUID, 0x22, 0xFF);
    RNBD_SetCharacteristic(Matter_Char_ADDITIONAL_DATA_UUID, 0x02, 0xFF);

    //xSemaphoreTake(cmd_mode_bin_sem, portMAX_DELAY);

    xSemaphoreTake(rnbd_read_bin_sem, portMAX_DELAY);
    
    RNBD_ListCustomizeService(&ble_service_info);
    xSemaphoreGive(rnbd_read_bin_sem);
    
    ChipLogProgress(DeviceLayer, "ble service info: char uuid = %s", ble_service_info.characteristics[0].uuid);
    ChipLogProgress(DeviceLayer, "ble service info: char uuid = 0x%x", ble_service_info.characteristics[0].handle);
    ChipLogProgress(DeviceLayer, "ble service info: char uuid = 0x%x", ble_service_info.characteristics[0].properties);
    
    xSemaphoreTake(rnbd_read_bin_sem, portMAX_DELAY);
    //res = RNBD_SetDebugLog(7);
    xSemaphoreGive(rnbd_read_bin_sem);
    if (!res)
        ChipLogProgress(DeviceLayer, "Fail to enable RNBD Log");
            
    xSemaphoreTake(rnbd_read_bin_sem, portMAX_DELAY);
    res = RNBD_ClearAdvData(PERMANENT_CNAHGE_MODE);
    xSemaphoreGive(rnbd_read_bin_sem);
    if (!res)
        ChipLogProgress(DeviceLayer, "Fail to clear ble adv data");

    xSemaphoreTake(rnbd_read_bin_sem, portMAX_DELAY);
    res = RNBD_SetAdvData(PERMANENT_CNAHGE_MODE, "01", "06");
    xSemaphoreGive(rnbd_read_bin_sem);

    if (!res)
        ChipLogProgress(DeviceLayer, "Fail to set ble adv data");

    xSemaphoreTake(rnbd_read_bin_sem, portMAX_DELAY);
    res = RNBD_SetAdvData(PERMANENT_CNAHGE_MODE, "16", "F6FF00000FF1FF018000");
    xSemaphoreGive(rnbd_read_bin_sem);
    if (!res)
        ChipLogProgress(DeviceLayer, "Fail to set ble adv data");

    xTaskCreate(BLETask, BLE_TASK_NAME, BLE_TASK_STACK_SIZE, NULL, 1, &sBLETaskHandle);        

exit:
    return err;
}

bool BLEManagerImpl::_IsAdvertisingEnabled(void)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::_IsAdvertisingEnabled");
    return mFlags.Has(Flags::kFlag_AdvertisingEnabled);
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::_SetAdvertisingEnabled, val = %d", val);
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (mFlags.Has(Flags::kFlag_AdvertisingEnabled) != val)
    {
        mFlags.Set(Flags::kFlag_AdvertisingEnabled, val);
        RNBD_EnableAdv(val);
    }


    return err;
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingMode(BLEAdvertisingMode mode)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::_SetAdvertisingMode");
    (void) (mode);

    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR BLEManagerImpl::_GetDeviceName(char * buf, size_t bufSize)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::_GetDeviceName");

    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR BLEManagerImpl::_SetDeviceName(const char * deviceName)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::_SetDeviceName");

    return CHIP_ERROR_NOT_IMPLEMENTED;
}

uint16_t BLEManagerImpl::_NumConnections(void)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::_NumConnections");
    return mNumCons;
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
#ifdef BLE_IMPL_DEBUG    
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::_OnPlatformEvent, event = %d", event->Type);
#endif

    switch (event->Type)
    {
        case DeviceEventType::kCHIPoBLEWriteReceived:

            HandleWriteReceived(event->CHIPoBLEWriteReceived.ConId, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_RX,
                            PacketBufferHandle::Adopt(event->CHIPoBLEWriteReceived.Data));

            break;

        case DeviceEventType::kCHIPoBLESubscribe:
            HandleSubscribeReceived(event->CHIPoBLESubscribe.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX);
            ChipDeviceEvent _event;
            {
                _event.Type = DeviceEventType::kCHIPoBLEConnectionEstablished;
                PlatformMgr().PostEventOrDie(&_event);
            }
            break;

        case DeviceEventType::kCHIPoBLEUnsubscribe:
            HandleUnsubscribeReceived(event->CHIPoBLEUnsubscribe.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX);
            break;

        case DeviceEventType::kCHIPoBLEIndicateConfirm:
            HandleIndicationConfirmation(event->CHIPoBLEIndicateConfirm.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX);
            break;

        case DeviceEventType::kCHIPoBLEConnectionError:
            HandleConnectionError(event->CHIPoBLEConnectionError.ConId, event->CHIPoBLEConnectionError.Reason);
            break;

        case DeviceEventType::kServiceProvisioningChange:
            break;
    }
    

}

bool BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::SubscribeCharacteristic() not supported");
    return false;
}

bool BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::UnsubscribeCharacteristic() not supported");
    return false;
}

bool BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::Closing BLE GATT connection (con %u)", conId);

    /* To Do*/

    return false;
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::GetMTU");
    
    return 131;
}

bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                    PacketBufferHandle data)
{    
    CHIP_ERROR err                  = CHIP_NO_ERROR;
    uint16_t dataLen                = data->DataLength();
    char* data_buf;
    CHIPoBLEConState * conState     = GetConnectionState(conId);

    VerifyOrExit(conState != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);

//#ifdef BLE_IMPL_DEBUG
    ChipLogDetail(DeviceLayer, "Sending indication for CHIPoBLE TX characteristic (con %u, len %u)", conId, dataLen);
//#endif
    data_buf = (char*) pvPortMalloc(dataLen*2 + 1);

    for (int i = 0; i < dataLen; i++)
    {   
        sprintf(&data_buf[i*2], "%02x", data->Start()[i]);
    }

    RNBD_WriteLocalCharacteristic(4100, data_buf, dataLen*2);

    vPortFree(data_buf);
#if 1
    //xSemaphoreTake(rnbd_shw_bin_sem, 0);
    send_ind_flag = false;
    while (1)
    {
        //ChipLogDetail(DeviceLayer, "Loop test");
        if (send_ind_flag == true )
            break;
        //if( xSemaphoreTake( rnbd_shw_bin_sem, ( TickType_t ) 10 ) == pdTRUE )
        //{
        //    break;
        //}
        vTaskDelay( 100 );
    };
#else
    vTaskDelay( 300 );  /* To Do: temperary workaround the missing of the indication ack event issue*/
#endif
    //vTaskDelay( 300 );  /* To Do: temperary workaround the missing of the indication ack event issue*/

    ChipDeviceEvent _event;
    _event.Type                          = DeviceEventType::kCHIPoBLEIndicateConfirm;
    _event.CHIPoBLEIndicateConfirm.ConId = conId;
    if (PlatformMgr().PostEvent(&_event) != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "BLEManagerImpl::_OnPlatformEvent, send event fail..");
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "SendIndication fail, error = %s", ErrorStr(err));
    }

    return err == CHIP_NO_ERROR;
}

bool BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                      PacketBufferHandle data)

{
    ChipLogError(DeviceLayer, "BLEManagerImpl::SendWriteRequest() not supported");
    return false;
}

bool BLEManagerImpl::SendReadRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                     PacketBufferHandle data)
{
    ChipLogError(DeviceLayer, "BLEManagerImpl::SendReadRequest() not supported");
    return false;
}

bool BLEManagerImpl::SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext,
                                      const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogError(DeviceLayer, "BLEManagerImpl::SendReadResponse() not supported");
    return false;
}

void BLEManagerImpl::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId) {}



CHIP_ERROR BLEManagerImpl::HandleGattServiceNotificationIndicationReq(uint16_t conn_id, int enable_flag)
{
    ChipDeviceEvent event;
    CHIP_ERROR result = CHIP_NO_ERROR;

    if (enable_flag)
        event.Type = DeviceEventType::kCHIPoBLESubscribe;
    else
        event.Type = DeviceEventType::kCHIPoBLEUnsubscribe;

    event.CHIPoBLESubscribe.ConId = conn_id;
    if (PlatformMgr().PostEvent(&event) != CHIP_NO_ERROR)
    {
        result = CHIP_ERROR_SENDING_BLOCKED;
    }

    return result;
}
/*
 * If Attribute is for CHIP, pass it through. Otherwise process request for
 * attributes in the GATT DB attribute table.
 */
CHIP_ERROR BLEManagerImpl::HandleGattServiceWrite(uint16_t conn_id, uint8_t * p_data, int data_len)
{
    CHIP_ERROR result = CHIP_NO_ERROR;
    System::PacketBufferHandle buf;

    buf = System::PacketBufferHandle::NewWithData(p_data, data_len, 0, 0);
    if (!buf.IsNull())
    {
#ifdef BLE_DEBUG
        ChipLogDetail(DeviceLayer, "Write received for CHIPoBLE RX characteristic con %04x len %d", conn_id, valLen);
#endif
        // Post an event to the CHIP queue to deliver the data into the CHIP stack.
        {
            ChipDeviceEvent event;
            event.Type                        = DeviceEventType::kCHIPoBLEWriteReceived;
            event.CHIPoBLEWriteReceived.ConId = conn_id;
            event.CHIPoBLEWriteReceived.Data  = std::move(buf).UnsafeRelease();
            CHIP_ERROR status                 = PlatformMgr().PostEvent(&event);
            if (status != CHIP_NO_ERROR)
            {
                result = CHIP_ERROR_SENDING_BLOCKED;
            }
        }
    }
    else
    {
        ChipLogError(DeviceLayer, "BLEManagerImpl: Out of buffers during CHIPoBLE RX");
        result = CHIP_ERROR_NO_MEMORY;
    }

    return result;
}


void BLEManagerImpl::SetAdvertisingData(void)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::SetAdvertisingData()");
    /* To Do */
}


BLEManagerImpl::CHIPoBLEConState * BLEManagerImpl::GetConnectionState(uint16_t conId)
{
    ChipLogError(DeviceLayer, "BLEManagerImpl::GetConnectionState");

    for (uint16_t i = 0; i < kMaxConnections; i++)
    {
        if (mCons[i].ConId == conId)
        {
            return &mCons[i];
        }
    }
    ChipLogError(DeviceLayer, "Failed to find CHIPoBLEConState");

    return NULL;
}


} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif
