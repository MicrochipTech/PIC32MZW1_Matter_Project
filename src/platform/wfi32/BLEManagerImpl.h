/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

#pragma once

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE


#include <BLEManager.h>
#include "rnbd.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

enum wiced_bt_gatt_status_t
{
    WICED_BT_GATT_SUCCESS,
    WICED_BT_GATT_ILLEGAL_PARAMETER,

};

enum ble_event
{
    BTM_ENABLED_EVT,
    BTM_CONNECT_EVT

};


/**
 * Concrete implementation of the NetworkProvisioningServer singleton object for the  PSoC 6 platform.
 */
class BLEManagerImpl final : public BLEManager,
                             private Ble::BleLayer,
                             private Ble::BlePlatformDelegate,
                             private Ble::BleApplicationDelegate
{
    // Allow the BLEManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend BLEManager;

    // ===== Members that implement the BLEManager internal interface.

    CHIP_ERROR _Init(void);
    void _Shutdown() {}
    bool _IsAdvertisingEnabled(void);
    CHIP_ERROR _SetAdvertisingEnabled(bool val);
    bool _IsFastAdvertisingEnabled(void);
    bool _IsAdvertising(void);
    CHIP_ERROR _SetAdvertisingMode(BLEAdvertisingMode mode);
    CHIP_ERROR _GetDeviceName(char * buf, size_t bufSize);
    CHIP_ERROR _SetDeviceName(const char * deviceName);
    uint16_t _NumConnections(void);
    void _OnPlatformEvent(const ChipDeviceEvent * event);
    ::chip::Ble::BleLayer * _GetBleLayer(void);

    // ===== Members that implement virtual methods on BlePlatformDelegate.

    bool SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId,
                                 const Ble::ChipBleUUID * charId) override;
    bool UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId,
                                   const Ble::ChipBleUUID * charId) override;
    bool CloseConnection(BLE_CONNECTION_OBJECT conId) override;
    uint16_t GetMTU(BLE_CONNECTION_OBJECT conId) const override;
    bool SendIndication(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                        System::PacketBufferHandle data) override;
    bool SendWriteRequest(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                          System::PacketBufferHandle data) override;
    bool SendReadRequest(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                         System::PacketBufferHandle data) override;
    bool SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext, const Ble::ChipBleUUID * svcId,
                          const Ble::ChipBleUUID * charId) override;

    // ===== Members that implement virtual methods on BleApplicationDelegate.

    void NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId) override;

    // ===== Members for internal use by the following friends.

    friend BLEManager & BLEMgr(void);
    friend BLEManagerImpl & BLEMgrImpl(void);

public:
    static BLEManagerImpl sInstance;
    static void BLETask(void * pvParameter);
    static void TestFunc(void * pvParameter);
    static RNBD_ble_service_info_t ble_service_info;

    // ===== Private members reserved for use by this class only.
    enum class Flags : uint16_t
    {
        kFlag_AsyncInitCompleted     = 0x0001, /**< One-time asynchronous initialization actions have been performed. */
        kFlag_AdvertisingEnabled     = 0x0002, /**< The application has enabled CHIPoBLE advertising. */
        kFlag_FastAdvertisingEnabled = 0x0004, /**< The application has enabled fast advertising. */
        kFlag_Advertising            = 0x0008, /**< The system is currently CHIPoBLE advertising. */
        kFlag_AdvertisingRefreshNeeded =
            0x0010, /**< The advertising state/configuration has changed, but the SoftDevice has yet to be updated. */
        kFlag_DeviceNameSet    = 0x0020,
        kFlag_StackInitialized = 0x0040,
    };

    enum
    {
        kMaxConnections = BLE_LAYER_NUM_BLE_ENDPOINTS,
        kMaxDeviceNameLength = 16
    };

    struct CHIPoBLEConState
    {
        // System::PacketBuffer * PendingIndBuf;
        uint16_t ConId;
        uint16_t Mtu;
        bool connected;
    };

    CHIPoBLEConState mCons[kMaxConnections];
    uint16_t mNumCons;
    CHIPoBLEServiceMode mServiceMode;
    BitFlags<Flags> mFlags;
    char mDeviceName[kMaxDeviceNameLength + 1];
    
    void SetAdvertisingData(void);

    CHIP_ERROR HandleGattServiceWrite(uint16_t conn_id, uint8_t * p_data, int data_len);
    CHIP_ERROR HandleGattServiceNotificationIndicationReq(uint16_t conn_id, int enable_flag);

    static void BLEManagerCallback(RNBD_EVENT_t event, int handle, char* msg);
    static char *  RNBD_EVENT_GetToken(char * msg, int item_idx);

    CHIPoBLEConState * GetConnectionState(uint16_t conId);
};

/**
 * Returns a reference to the public interface of the BLEManager singleton object.
 *
 * Internal components should use this to access features of the BLEManager object
 * that are common to all platforms.
 */
inline BLEManager & BLEMgr(void)
{
    return BLEManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the BLEManager singleton object.
 *
 * Internal components can use this to gain access to features of the BLEManager
 * that are specific to the PSoC 6 platform.
 */
inline BLEManagerImpl & BLEMgrImpl(void)
{
    return BLEManagerImpl::sInstance;
}

inline Ble::BleLayer * BLEManagerImpl::_GetBleLayer()
{
    return this;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
