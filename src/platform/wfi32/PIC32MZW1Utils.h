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

#pragma once

#include "platform/internal/DeviceNetworkInfo.h"
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include "include/wdrv_pic32mzw_client_api.h"

typedef enum
{
    WIFI_MODE_NULL = 0, /**< null mode */
    WIFI_MODE_STA,      /**< WiFi station mode */
    WIFI_MODE_AP,       /**< WiFi soft-AP mode */
    WIFI_MODE_MAX
} wifi_mode_t;

typedef struct
{
    WDRV_PIC32MZW_AUTH_CONTEXT authCtx;
    WDRV_PIC32MZW_BSS_CONTEXT bssCtx;
} wifi_config_sta_t;

typedef struct
{
    WDRV_PIC32MZW_AUTH_CONTEXT authCtx;
    WDRV_PIC32MZW_BSS_CONTEXT bssCtx;
} wifi_config_ap_t;

typedef struct
{
    wifi_config_sta_t sta; /**< configuration of STA */
    wifi_config_ap_t ap;   /**< configuration of AP */
} wifi_config_t;



typedef struct heap_info
{
    size_t HeapMax;
    size_t HeapUsed;
    size_t HeapFree;
} heap_info_t;


namespace chip {
namespace DeviceLayer {
namespace Internal {

class PIC32MZW1Utils
{
public:
    static CHIP_ERROR IsAPEnabled(bool & apEnabled);
    static CHIP_ERROR IsStationEnabled(bool & staEnabled);
    static bool IsStationProvisioned(void);
    static CHIP_ERROR IsStationConnected(bool & connected);

    static CHIP_ERROR pic32mzw1_wifi_disconnect(void);
    static CHIP_ERROR pic32mzw1_wifi_connect(void);
    static CHIP_ERROR pic32mzw1_start_ap(void);
    static CHIP_ERROR pic32mzw1_stop_ap(void);
    static CHIP_ERROR pic32mzw1_wifi_init(void);
    static CHIP_ERROR pic32mzw1_wifi_get_rssi(int8_t * rssi);
    static CHIP_ERROR pic32mzw1_wifi_get_mac_addr(uint8_t * mac_addr);

    static void STANotifyCB(DRV_HANDLE handle, WDRV_PIC32MZW_ASSOC_HANDLE associationHandle, WDRV_PIC32MZW_CONN_STATE currentState);
    static void APNotifyCB(DRV_HANDLE handle, WDRV_PIC32MZW_ASSOC_HANDLE associationHandle, WDRV_PIC32MZW_CONN_STATE currentState);
    static wifi_config_t mWiFiConfig;
    static wifi_mode_t   mWiFiMode;
    static DRV_HANDLE wifi_handle;
    static bool mIsStationConnected;

private:
    static CHIP_ERROR pic32mzw1_wifi_set_config(char* ssid, char* password, WDRV_PIC32MZW_AUTH_TYPE security );
    
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

