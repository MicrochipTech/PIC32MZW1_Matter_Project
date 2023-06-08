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
 *          General utility methods for the PIC32MZW1 platform.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <PIC32MZW1Utils.h>
#include "network.h"

using namespace ::chip::DeviceLayer::Internal;
using chip::DeviceLayer::Internal::DeviceNetworkInfo;

#define WIFI_DRV_OPEN_WAIT_COUNTER 1000

wifi_config_t PIC32MZW1Utils::mWiFiConfig = {};
wifi_mode_t PIC32MZW1Utils::mWiFiMode = WIFI_MODE_NULL;
DRV_HANDLE PIC32MZW1Utils::wifi_handle = DRV_HANDLE_INVALID;
bool PIC32MZW1Utils::mIsStationConnected = false;

CHIP_ERROR PIC32MZW1Utils::IsAPEnabled(bool & apEnabled)
{
    if (mWiFiMode == WIFI_MODE_AP)
        apEnabled = true;
    else
        apEnabled = false;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PIC32MZW1Utils::IsStationEnabled(bool & staEnabled)
{
    if (mWiFiMode == WIFI_MODE_STA)
        staEnabled = true;
    else
        staEnabled = false;

    return CHIP_NO_ERROR;
}

bool PIC32MZW1Utils::IsStationProvisioned(void)
{
    CHIP_ERROR err;

    char ssid[DeviceLayer::Internal::kMaxWiFiSSIDLength];
    char password[DeviceLayer::Internal::kMaxWiFiKeyLength];
    uint32_t code    = 0;
    size_t offset = 0;
    
    
    err = PIC32MZW1Config::ReadConfigValueStr(PIC32MZW1Config::kConfigKey_WiFiSSID, ssid, sizeof(ssid), offset);
    SuccessOrExit(err);
    err = PIC32MZW1Config::ReadConfigValueStr(PIC32MZW1Config::kConfigKey_WiFiPassword, password, sizeof(password), offset);
    SuccessOrExit(err);
    err = PIC32MZW1Config::ReadConfigValue(PIC32MZW1Config::kConfigKey_WiFiSecurity, code);
    SuccessOrExit(err);

    if (strlen(ssid) != 0)
        return true;

exit:
    return false;
}

CHIP_ERROR PIC32MZW1Utils::IsStationConnected(bool & connected)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    connected      = mIsStationConnected;
    return err;
}

CHIP_ERROR PIC32MZW1Utils::pic32mzw1_wifi_init(void)
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    SYS_STATUS sysStatus;
    DRV_IO_INTENT intent;
    int counter = 0;


    while (DRV_HANDLE_INVALID == wifi_handle && counter < WIFI_DRV_OPEN_WAIT_COUNTER)
    {
        wifi_handle = Network_PIC32MZW_Get_WiFiHandle();
        counter++;
        vTaskDelay(50 / portTICK_PERIOD_MS);
    };


    if (DRV_HANDLE_INVALID == wifi_handle)
        err = CHIP_ERROR_INTERNAL;


#if 0
    while (SYS_STATUS_READY != WDRV_PIC32MZW_Status(sysObj.drvWifiPIC32MZW1))
    {
        vTaskDelay(50 / portTICK_PERIOD_MS);
    };

    wifi_handle = WDRV_PIC32MZW_Open(0, intent);
            
    
#endif
    return err;
}

CHIP_ERROR PIC32MZW1Utils::pic32mzw1_wifi_set_config(char* ssid, char* password, WDRV_PIC32MZW_AUTH_TYPE security )
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_BSSCtxSetSSID(&mWiFiConfig.sta.bssCtx, (uint8_t*) ssid, strlen(ssid))) 
    {
        ChipLogError(DeviceLayer, "SSID set fail");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    
    if(WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_BSSCtxSetChannel(&mWiFiConfig.sta.bssCtx, WDRV_PIC32MZW_CID_ANY))
    {
        ChipLogError(DeviceLayer, "channel set fail");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }


    switch (security )
    {
        case WDRV_PIC32MZW_AUTH_TYPE_OPEN:
        {
            if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetOpen(&mWiFiConfig.sta.authCtx)) 
            {
                ChipLogError(DeviceLayer, "Unable to set Authentication");
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            break;
        }
        case WDRV_PIC32MZW_AUTH_TYPE_WPA2_PERSONAL:
        {
            if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetPersonal(&mWiFiConfig.sta.authCtx, (uint8_t*) password, strlen(password), WDRV_PIC32MZW_AUTH_TYPE_WPA2_PERSONAL)) 
            {
                ChipLogError(DeviceLayer, "Unable to set authentication to WPA2");
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            break;
        }
        case WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_PERSONAL:
        {
            if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetPersonal(&mWiFiConfig.sta.authCtx, (uint8_t*) password, strlen(password), WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_PERSONAL)) 
            {
                ChipLogError(DeviceLayer, "Unable to set authentication to WPAWPA2 MIXED");
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            break;
        }
#ifdef WDRV_PIC32MZW_WPA3_SUPPORT
        case WDRV_PIC32MZW_AUTH_TYPE_WPA3_PERSONAL:
        {
            if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetPersonal(&mWiFiConfig.sta.authCtx, (uint8_t*) password, strlen(password), WDRV_PIC32MZW_AUTH_TYPE_WPA3_PERSONAL)) 
            {
                ChipLogError(DeviceLayer, "Unable to set authentication to WPA3");
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            break;
        }
        case WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_PERSONAL:
        {
            if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetPersonal(&mWiFiConfig.sta.authCtx, (uint8_t*) password, strlen(password), WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_PERSONAL)) 
            {
                ChipLogError(DeviceLayer, "Unable to set authentication to WPA2WPA3 MIXED");
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            break;
        }
        default:
            break;
#endif
    }

    return err;

}

CHIP_ERROR PIC32MZW1Utils::pic32mzw1_wifi_disconnect(void)
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    WDRV_PIC32MZW_STATUS result;
    int wait_ms = 2000;
    
    ChipLogError(DeviceLayer, "pic32mzw1_wifi_disconnect() In");
    
    if (!mIsStationConnected)
        return err;

    result =  WDRV_PIC32MZW_BSSDisconnect(PIC32MZW1Utils::wifi_handle);

    if (result != WDRV_PIC32MZW_STATUS_OK)
    {
        ChipLogError(DeviceLayer, "pic32mzw1_wifi_disconnect() failed: %d", result);
        err = CHIP_ERROR_INTERNAL;
    }
    else
    {
        while ((mIsStationConnected == true) && wait_ms)
        {
            wait_ms -= 50;
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }
        if (wait_ms <= 0)
        {
            ChipLogError(DeviceLayer, "pic32mzw1_wifi_disconnect() failed: Timeout..");
            err = CHIP_ERROR_INTERNAL;
        }

    }

    ChipLogError(DeviceLayer, "pic32mzw1_wifi_disconnect() Out");

    return err;
}

void PIC32MZW1Utils::STANotifyCB(DRV_HANDLE handle, WDRV_PIC32MZW_ASSOC_HANDLE associationHandle, WDRV_PIC32MZW_CONN_STATE currentState)
{
    WDRV_PIC32MZW_MAC_ADDR macAddr;
    WDRV_PIC32MZW_AssocPeerAddressGet(associationHandle, &macAddr);
    
    if (WDRV_PIC32MZW_CONN_STATE_CONNECTED == currentState)
    {
        ChipLogProgress(DeviceLayer, "STANotifyCB:: CONNECTED:: %02X:%02X:%02X:%02X:%02X:%02X", macAddr.addr[0], macAddr.addr[1], macAddr.addr[2], macAddr.addr[3], macAddr.addr[4], macAddr.addr[5]);
        mIsStationConnected = true;
        ChipDeviceEvent event;
        event.Type                           = DeviceEventType::kWiFiConnectivityChange;
        event.WiFiConnectivityChange.Result = ConnectivityChange::kConnectivity_Established;
        PlatformMgr().PostEventOrDie(&event);
    
    }
    else if (WDRV_PIC32MZW_CONN_STATE_DISCONNECTED == currentState)
    {
        ChipLogProgress(DeviceLayer, "STANotifyCB:: DISCONNECTED:: %02X:%02X:%02X:%02X:%02X:%02X", macAddr.addr[0], macAddr.addr[1], macAddr.addr[2], macAddr.addr[3], macAddr.addr[4], macAddr.addr[5]);
        mIsStationConnected = false;

        ChipDeviceEvent event;
        event.Type                           = DeviceEventType::kWiFiConnectivityChange;
        event.WiFiConnectivityChange.Result = ConnectivityChange::kConnectivity_Lost;
        PlatformMgr().PostEventOrDie(&event);

        // To Do: Release DHCP
    }
    else if (WDRV_PIC32MZW_CONN_STATE_FAILED == currentState)
    {
        ChipLogProgress(DeviceLayer, "STANotifyCB:: WDRV_PIC32MZW_CONN_STATE_FAILED");
         
        ChipDeviceEvent event;
        event.Type                           = DeviceEventType::kWiFiConnectivityChange;
        event.WiFiConnectivityChange.Result = ConnectivityChange::kConnectivity_Lost;
        PlatformMgr().PostEventOrDie(&event);
        
    }

}

CHIP_ERROR PIC32MZW1Utils::pic32mzw1_wifi_connect(void)
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    WDRV_PIC32MZW_STATUS result;
    char ssid[DeviceLayer::Internal::kMaxWiFiSSIDLength];
    char password[DeviceLayer::Internal::kMaxWiFiKeyLength];
    uint32_t auth    = 0;
    size_t ssid_offset = 0;
    size_t pw_offset = 0;

    memset(ssid, 0, sizeof(ssid));
    memset(password, 0, sizeof(password));

    err = PIC32MZW1Config::ReadConfigValueStr(PIC32MZW1Config::kConfigKey_WiFiSSID, ssid, sizeof(ssid), ssid_offset);
    SuccessOrExit(err);

    err = PIC32MZW1Config::ReadConfigValueStr(PIC32MZW1Config::kConfigKey_WiFiPassword, password, sizeof(password), pw_offset);
    SuccessOrExit(err);

    err = PIC32MZW1Config::ReadConfigValue(PIC32MZW1Config::kConfigKey_WiFiSecurity, auth);
    SuccessOrExit(err);

    ChipLogProgress(DeviceLayer, "pic32mzw1_wifi_connect(), SSID:%s, Password:%s, Auth:%d, ssid_offset:%d, pw_offset:%d", ssid, password, auth, ssid_offset, pw_offset);
    
    pic32mzw1_wifi_set_config(ssid, password, (WDRV_PIC32MZW_AUTH_TYPE) auth);
    
    result =  WDRV_PIC32MZW_BSSConnect(wifi_handle, &mWiFiConfig.sta.bssCtx, &mWiFiConfig.sta.authCtx, STANotifyCB);
    if (WDRV_PIC32MZW_STATUS_OK != result)
    {
        ChipLogError(DeviceLayer, "PIC32MZW1 WiFi connect fail, result = %d", result);
        err = CHIP_ERROR_INTERNAL;
    }
    else
    {
        err = CHIP_NO_ERROR;
    }
    

    mWiFiMode = WIFI_MODE_STA;
 exit:   
    return err;
}

void PIC32MZW1Utils::APNotifyCB(DRV_HANDLE handle, WDRV_PIC32MZW_ASSOC_HANDLE associationHandle, WDRV_PIC32MZW_CONN_STATE currentState)
{
    WDRV_PIC32MZW_MAC_ADDR macAddr;
    WDRV_PIC32MZW_AssocPeerAddressGet(associationHandle, &macAddr);

    if (WDRV_PIC32MZW_CONN_STATE_CONNECTED == currentState)
    {
        ChipLogProgress(DeviceLayer, "APNotifyCB:: CONNECTED:: %02X:%02X:%02X:%02X:%02X:%02X", macAddr.addr[0], macAddr.addr[1], macAddr.addr[2], macAddr.addr[3], macAddr.addr[4], macAddr.addr[5]);
    }
    else if (WDRV_PIC32MZW_CONN_STATE_DISCONNECTED == currentState)
    {
        ChipLogProgress(DeviceLayer, "APNotifyCB:: DISCONNECTED:: %02X:%02X:%02X:%02X:%02X:%02X", macAddr.addr[0], macAddr.addr[1], macAddr.addr[2], macAddr.addr[3], macAddr.addr[4], macAddr.addr[5]);
        // To Do: Release DHCP
    }
}

CHIP_ERROR PIC32MZW1Utils::pic32mzw1_start_ap(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    WDRV_PIC32MZW_STATUS result;
    wifi_config_t wifiConfig;
    char ssid[DeviceLayer::Internal::kMaxWiFiSSIDLength];

    memset(&wifiConfig.ap, 0, sizeof(wifi_config_ap_t));

    uint16_t vendorId;
    uint16_t productId;
    ReturnErrorOnFailure(GetDeviceInstanceInfoProvider()->GetVendorId(vendorId));
    ReturnErrorOnFailure(GetDeviceInstanceInfoProvider()->GetProductId(productId));

    snprintf(ssid, sizeof(ssid), "%s-%04X-%04X", CHIP_DEVICE_CONFIG_WIFI_AP_SSID_PREFIX,
             vendorId, productId);


    ChipLogProgress(DeviceLayer, "Setting default WiFi AP configuration (SSID: %s)", ssid);

    if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_BSSCtxSetSSID(&wifiConfig.ap.bssCtx, (uint8_t*) ssid, strlen(ssid))) 
    {
        ChipLogError(DeviceLayer, "SSID set fail");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    
    if(WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_BSSCtxSetChannel(&wifiConfig.ap.bssCtx, CHIP_DEVICE_CONFIG_WIFI_AP_CHANNEL))
    {
        ChipLogError(DeviceLayer, "channel set fail");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }


    switch (CHIP_DEVICE_CONFIG_WIFI_AP_SECURITY )
    {
        case WDRV_PIC32MZW_AUTH_TYPE_OPEN:
        {
            if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetOpen(&wifiConfig.ap.authCtx)) 
            {
                ChipLogError(DeviceLayer, "Unable to set Authentication");
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            break;
        }
        case WDRV_PIC32MZW_AUTH_TYPE_WPA2_PERSONAL:
        {
            if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetPersonal(&wifiConfig.ap.authCtx, (uint8_t*) CHIP_DEVICE_CONFIG_WIFI_AP_PASSWORD, strlen(CHIP_DEVICE_CONFIG_WIFI_AP_PASSWORD), WDRV_PIC32MZW_AUTH_TYPE_WPA2_PERSONAL)) 
            {
                ChipLogError(DeviceLayer, "Unable to set authentication to WPA2");
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            break;
        }
        case WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_PERSONAL:
        {
            if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetPersonal(&wifiConfig.ap.authCtx, (uint8_t*) CHIP_DEVICE_CONFIG_WIFI_AP_PASSWORD, strlen(CHIP_DEVICE_CONFIG_WIFI_AP_PASSWORD), WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_PERSONAL)) 
            {
                ChipLogError(DeviceLayer, "Unable to set authentication to WPAWPA2 MIXED");
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            break;
        }
#ifdef WDRV_PIC32MZW_WPA3_SUPPORT
        case WDRV_PIC32MZW_AUTH_TYPE_WPA3_PERSONAL:
        {
            if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetPersonal(&wifiConfig.ap.authCtx, (uint8_t*) CHIP_DEVICE_CONFIG_WIFI_AP_PASSWORD, strlen(CHIP_DEVICE_CONFIG_WIFI_AP_PASSWORD), WDRV_PIC32MZW_AUTH_TYPE_WPA3_PERSONAL)) 
            {
                ChipLogError(DeviceLayer, "Unable to set authentication to WPA3");
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            break;
        }
        case WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_PERSONAL:
        {
            if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_AuthCtxSetPersonal(&wifiConfig.ap.authCtx, (uint8_t*) CHIP_DEVICE_CONFIG_WIFI_AP_PASSWORD, strlen(CHIP_DEVICE_CONFIG_WIFI_AP_PASSWORD), WDRV_PIC32MZW_AUTH_TYPE_WPA2WPA3_PERSONAL)) 
            {
                ChipLogError(DeviceLayer, "Unable to set authentication to WPA2WPA3 MIXED");
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            break;
        }
#endif
    }



    result = WDRV_PIC32MZW_APStart(wifi_handle, &wifiConfig.ap.bssCtx, &wifiConfig.ap.authCtx, &APNotifyCB);

    if (WDRV_PIC32MZW_STATUS_OK == result)
    {
        err = CHIP_NO_ERROR;
        mWiFiMode = WIFI_MODE_AP;
        mIsStationConnected = false;
    }
    else
    {
        err = CHIP_ERROR_NOT_CONNECTED;
    }
    
    return err;
    
}

CHIP_ERROR PIC32MZW1Utils::pic32mzw1_stop_ap(void)
{
    CHIP_ERROR err   = CHIP_NO_ERROR;

    WDRV_PIC32MZW_APStop(wifi_handle);
    mWiFiMode = WIFI_MODE_STA;
    
    return err;
}

CHIP_ERROR PIC32MZW1Utils::pic32mzw1_wifi_get_rssi(int8_t * rssi)
{
    WDRV_PIC32MZW_STATUS result = WDRV_PIC32MZW_STATUS_OK;
    CHIP_ERROR err   = CHIP_NO_ERROR;

    result = WDRV_PIC32MZW_AssocRSSIGet(wifi_handle, rssi, NULL);
    if (result != WDRV_PIC32MZW_STATUS_OK)
    {
        ChipLogError(DeviceLayer, "WDRV_PIC32MZW_AssocRSSIGet failed: %d", (int) result);
        err = CHIP_ERROR_INTERNAL;
    }

    return err;
}

CHIP_ERROR PIC32MZW1Utils::pic32mzw1_wifi_get_mac_addr(uint8_t * mac_addr)
{
    WDRV_PIC32MZW_STATUS result = WDRV_PIC32MZW_STATUS_OK;
    CHIP_ERROR err   = CHIP_NO_ERROR;

    result = WDRV_PIC32MZW_InfoDeviceMACAddressGet(wifi_handle, mac_addr);
    if (result != WDRV_PIC32MZW_STATUS_OK)
    {
        ChipLogError(DeviceLayer, "WDRV_PIC32MZW_AssocRSSIGet failed: %d", (int) result);
        err = CHIP_ERROR_INTERNAL;
    }

    return err;
}
