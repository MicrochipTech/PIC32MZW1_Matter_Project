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
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/ConnectivityManager.h>
#include <platform/DeviceInstanceInfoProvider.h>

#include <platform/internal/GenericConnectivityManagerImpl_UDP.ipp>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <platform/internal/GenericConnectivityManagerImpl_TCP.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.ipp>
#endif
#include <platform/internal/GenericConnectivityManagerImpl_WiFi.ipp>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/wfi32/PIC32MZW1Utils.h>
#include <platform/wfi32/PIC32MZW1Config.h>
#include <platform/internal/BLEManager.h>

#include "network.h"
#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#include <lwip/nd6.h>
#include <lwip/netif.h>

#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/autoip.h"
#include "lwip/inet.h"
#include <lwip/ip_addr.h>

#include "lwip/opt.h"
#include <platform/wfi32/NetworkCommissioningDriver.h>
#include <type_traits>

#if !CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
#error "WiFi Station support must be enabled when building for PSoC6"
#endif

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::TLV;

namespace chip {
namespace DeviceLayer {

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

ConnectivityManager::WiFiStationMode ConnectivityManagerImpl::_GetWiFiStationMode(void)
{

    return mWiFiStationMode;
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiStationMode(WiFiStationMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    VerifyOrExit(val != kWiFiStationMode_NotSupported, err = CHIP_ERROR_INVALID_ARGUMENT);
    if (val != kWiFiStationMode_ApplicationControlled)
    {
        ChipLogProgress(DeviceLayer, "WiFi station mode change: %s -> %s", WiFiStationModeToStr(mWiFiStationMode),
                        WiFiStationModeToStr(val));
        mWiFiStationMode = val;
        /* Schedule work for disabled case causes station mode not getting enabled */
        if (mWiFiStationMode != kWiFiStationMode_Disabled)
        {
            DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
        }
        else
        {
            /* Call Drive Station directly to disable directly instead of scheduling */
            DriveStationState();
        }
    }

exit:
    return err;
}

bool ConnectivityManagerImpl::_IsWiFiStationEnabled(void)
{
    return (mWiFiStationMode == kWiFiStationMode_Enabled);
}

bool ConnectivityManagerImpl::_IsWiFiStationProvisioned(void)
{
    return mIsProvisioned;
}

void ConnectivityManagerImpl::_ClearWiFiStationProvision(void)
{
    if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled)
    {
        wifi_config_t stationConfig;

        memset(&stationConfig, 0, sizeof(stationConfig));
        ChipLogProgress(DeviceLayer, "To Do: clear WiFi STA setting");
        //Internal::P6Utils::p6_wifi_set_config(WIFI_IF_STA, &stationConfig);

        DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
        DeviceLayer::SystemLayer().ScheduleWork(DriveAPState, NULL);
    }
}

CHIP_ERROR ConnectivityManagerImpl::_SetWiFiAPMode(WiFiAPMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(val != kWiFiAPMode_NotSupported, err = CHIP_ERROR_INVALID_ARGUMENT);

    if (mWiFiAPMode != val)
    {
        ChipLogProgress(DeviceLayer, "WiFi AP mode change: %s -> %s", WiFiAPModeToStr(mWiFiAPMode), WiFiAPModeToStr(val));
    }
    mWiFiAPMode = val;
    DeviceLayer::SystemLayer().ScheduleWork(DriveAPState, NULL);
exit:
    return err;
}

void ConnectivityManagerImpl::_DemandStartWiFiAP(void)
{
    if (mWiFiAPMode == kWiFiAPMode_OnDemand || mWiFiAPMode == kWiFiAPMode_OnDemand_NoStationProvision)
    {
        mLastAPDemandTime = System::SystemClock().GetMonotonicTimestamp();
        DeviceLayer::SystemLayer().ScheduleWork(DriveAPState, NULL);
    }
}

void ConnectivityManagerImpl::_StopOnDemandWiFiAP(void)
{
    if (mWiFiAPMode == kWiFiAPMode_OnDemand || mWiFiAPMode == kWiFiAPMode_OnDemand_NoStationProvision)
    {
        mLastAPDemandTime = System::Clock::kZero;
        DeviceLayer::SystemLayer().ScheduleWork(DriveAPState, NULL);
    }
}

void ConnectivityManagerImpl::_MaintainOnDemandWiFiAP(void)
{
    if (mWiFiAPMode == kWiFiAPMode_OnDemand || mWiFiAPMode == kWiFiAPMode_OnDemand_NoStationProvision)
    {
        if (mWiFiAPState == kWiFiAPState_Activating || mWiFiAPState == kWiFiAPState_Active)
        {
            mLastAPDemandTime = System::SystemClock().GetMonotonicTimestamp();
        }
    }
}

void ConnectivityManagerImpl::_SetWiFiAPIdleTimeout(System::Clock::Timeout val)
{
    mWiFiAPIdleTimeout = val;
    DeviceLayer::SystemLayer().ScheduleWork(DriveAPState, NULL);
}

CHIP_ERROR ConnectivityManagerImpl::_GetAndLogWiFiStatsCounters(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int8_t rssi;
    uint8_t mac_addr[6];

    err = Internal::PIC32MZW1Utils::pic32mzw1_wifi_get_rssi(&rssi);
    SuccessOrExit(err);
    err = Internal::PIC32MZW1Utils::pic32mzw1_wifi_get_mac_addr(mac_addr);
    SuccessOrExit(err);


    ChipLogProgress(DeviceLayer,
                    "WiFi-Telemetry\n"
                    "BSSID: %02x:%02x:%02x:%02x:%02x:%02x\n"
                    "RSSI: %d\n"
                    "Channel: %d\n",
                    mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5],
                    rssi, ap_channel);
exit:
    return err;
}


void ConnectivityManagerImpl::dhcp_status_cb(struct netif *netif)
{
    ChipLogProgress(DeviceLayer, "dhcp_status_cb() In...");

    if (netif_is_up(netif)){

        if (!ip4_addr_isany(netif_ip4_addr(netif)) && !ip4_addr_isany(netif_ip4_gw(netif)))
        {
            //ChipLogProgress(DeviceLayer, "ip addr: %s\r\n", ip4addr_ntoa(netif_ip4_addr(netif))); 
            ConnectivityManagerImpl().OnIPAddressAvailable();
            return;
        }

#if 0
        for (uint8_t i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++)
        {
            if (ip6_addr_islinklocal(netif_ip6_addr(netif, i)) &&
                ip6_addr_isvalid(netif_ip6_addr_state(netif, i)))
            {

                //ChipLogProgress(DeviceLayer,"IPv6 Address %i Assigned : %s", i, ip6addr_ntoa(netif_ip6_addr(netif, i)));
                ConnectivityManagerImpl().OnIPAddressAvailable();
                return;
            }
        }
#endif
       
    }
    
    return;
}


// ==================== ConnectivityManager Platform Internal Methods ====================
CHIP_ERROR ConnectivityManagerImpl::_Init()
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    mLastStationConnectFailTime   = System::Clock::kZero;
    mLastAPDemandTime             = System::Clock::kZero;
    mWiFiStationMode              = kWiFiStationMode_Disabled;
    mWiFiStationState             = kWiFiStationState_NotConnected;
    mWiFiAPMode                   = kWiFiAPMode_Disabled;
    mWiFiAPState                  = kWiFiAPState_NotActive;
    mWiFiStationReconnectInterval = System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_WIFI_STATION_RECONNECT_INTERVAL);
    mWiFiAPIdleTimeout            = System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_WIFI_AP_IDLE_TIMEOUT);
    mFlags.SetRaw(0);
    ap_channel                    = 0;

    // WiFi-Station mode is enabled by default
    
    ChipLogProgress(DeviceLayer, "ConnectivityManagerImpl() log1");

    err = Internal::PIC32MZW1Utils::pic32mzw1_wifi_init();
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    PlatformMgr().AddEventHandler(
        [](const ChipDeviceEvent * event, intptr_t arg) {
            // Restart the server whenever an ip address is renewed
            if (event->Type == DeviceEventType::kWiFiConnectivityChange)
            {
                
                ChipLogProgress(DeviceLayer,"ConnectivityManager receive  kWiFiConnectivityChange..\r\n");
                //struct netif * nf;
                Network_PIC32MZW_StartDHCP(&dhcp_status_cb);
                ChipLogProgress(DeviceLayer,"ConnectivityManager receive  log2\r\n");
                //dhcp_start(nf);
            }
        },
        0);

    // If there is no persistent station provision...
    if (!IsWiFiStationProvisioned())
    {
        ChipLogProgress(DeviceLayer, "ConnectivityManagerImpl() log2");



        if (CHIP_DEVICE_CONFIG_DEFAULT_STA_SSID[0] != 0)
        {
            ChipLogProgress(DeviceLayer, "ConnectivityManagerImpl() log3");
            err = Internal::PIC32MZW1Config::WriteConfigValueStr(Internal::PIC32MZW1Config::kConfigKey_WiFiSSID, CHIP_DEVICE_CONFIG_DEFAULT_STA_SSID, strlen(CHIP_DEVICE_CONFIG_DEFAULT_STA_SSID));
            SuccessOrExit(err);
            err = Internal::PIC32MZW1Config::WriteConfigValueStr(Internal::PIC32MZW1Config::kConfigKey_WiFiPassword, CHIP_DEVICE_CONFIG_DEFAULT_STA_PASSWORD, strlen(CHIP_DEVICE_CONFIG_DEFAULT_STA_PASSWORD));
            SuccessOrExit(err);
            //err = Internal::PIC32MZW1Config::WriteConfigValue(Internal::PIC32MZW1Config::kConfigKey_WiFiSecurity, CHIP_DEVICE_CONFIG_DEFAULT_STA_SECURITY);
            err = Internal::PIC32MZW1Config::WriteConfigValue(Internal::PIC32MZW1Config::kConfigKey_WiFiSecurity, (uint32_t) CHIP_DEVICE_CONFIG_DEFAULT_STA_SECURITY);
            SuccessOrExit(err);

            mIsProvisioned = true;
            // Enable WiFi station mode.
            ReturnErrorOnFailure(SetWiFiStationMode(kWiFiStationMode_Enabled));
            
        }
        else
        {
            ChipLogProgress(DeviceLayer, "ConnectivityManagerImpl() log4");
            ReturnErrorOnFailure(SetWiFiStationMode(kWiFiStationMode_Disabled));
        }
    }
    else
    {
        ChipLogProgress(DeviceLayer, "ConnectivityManagerImpl() log5");
        // Enable WiFi station mode.
        ReturnErrorOnFailure(SetWiFiStationMode(kWiFiStationMode_Enabled));
    }
    ChipLogProgress(DeviceLayer, "DriverStationState()");
    // Queue work items to bootstrap the station state machines once the Chip event loop is running.
    err = DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
    SuccessOrExit(err);

exit:
    return err;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event) {}

void ConnectivityManagerImpl::_OnWiFiScanDone()
{
    DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
}

void ConnectivityManagerImpl::_OnWiFiStationProvisionChange()
{
    DeviceLayer::SystemLayer().ScheduleWork(DriveStationState, NULL);
}

void ConnectivityManagerImpl::DriveStationState(::chip::System::Layer * aLayer, void * aAppState)
{
    sInstance.DriveStationState();
}

void ConnectivityManagerImpl::DriveAPState(::chip::System::Layer * aLayer, void * aAppState)
{
    sInstance.DriveAPState();
}

void ConnectivityManagerImpl::ChangeWiFiStationState(WiFiStationState newState)
{
    if (mWiFiStationState != newState)
    {
        ChipLogProgress(DeviceLayer, "WiFi station state change: %s -> %s", WiFiStationStateToStr(mWiFiStationState),
                        WiFiStationStateToStr(newState));
        mWiFiStationState = newState;
        SystemLayer().ScheduleLambda([]() { NetworkCommissioning::PIC32WiFiDriver::GetInstance().OnNetworkStatusChange(); });
    }
}

void ConnectivityManagerImpl::ChangeWiFiAPState(WiFiAPState newState)
{
    if (mWiFiAPState != newState)
    {
        ChipLogProgress(DeviceLayer, "WiFi AP state change: %s -> %s", WiFiAPStateToStr(mWiFiAPState), WiFiAPStateToStr(newState));
        mWiFiAPState = newState;
    }
}


CHIP_ERROR ConnectivityManagerImpl::ConfigureWiFiAP()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = Internal::PIC32MZW1Utils::pic32mzw1_start_ap();

    return err;
}

void ConnectivityManagerImpl::DriveAPState()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    WiFiAPState targetState;
    bool APModeEnabled;

    // Determine if AP mode is currently enabled in the P6 WiFi layer.
    err = Internal::PIC32MZW1Utils::IsAPEnabled(APModeEnabled);
    SuccessOrExit(err);

    // Adjust the Connectivity Manager's AP state to match the state in the WiFi layer.
    if (APModeEnabled && (mWiFiAPState == kWiFiAPState_NotActive || mWiFiAPState == kWiFiAPState_Deactivating))
    {
        ChangeWiFiAPState(kWiFiAPState_Activating);
    }
    if (!APModeEnabled && (mWiFiAPState == kWiFiAPState_Active || mWiFiAPState == kWiFiAPState_Activating))
    {
        ChangeWiFiAPState(kWiFiAPState_Deactivating);
    }
    // If the AP interface is not under application control...
    if (mWiFiAPMode != kWiFiAPMode_ApplicationControlled)
    {
        // Determine the target (desired) state for AP interface...
        // The target state is 'NotActive' if the application has expressly disabled the AP interface.
        if (mWiFiAPMode == kWiFiAPMode_Disabled)
        {
            targetState = kWiFiAPState_NotActive;
        }

        // The target state is 'Active' if the application has expressly enabled the AP interface.
        else if (mWiFiAPMode == kWiFiAPMode_Enabled)
        {
            targetState = kWiFiAPState_Active;
        }

        // The target state is 'Active' if the AP mode is 'On demand, when no station is available'
        // and the station interface is not provisioned or the application has disabled the station
        // interface.
        else if (mWiFiAPMode == kWiFiAPMode_OnDemand_NoStationProvision &&
                 (!IsWiFiStationProvisioned() || GetWiFiStationMode() == kWiFiStationMode_Disabled))
        {
            targetState = kWiFiAPState_Active;
        }

        // The target state is 'Active' if the AP mode is one of the 'On demand' modes and there
        // has been demand for the AP within the idle timeout period.
        else if (mWiFiAPMode == kWiFiAPMode_OnDemand || mWiFiAPMode == kWiFiAPMode_OnDemand_NoStationProvision)
        {
            System::Clock::Timestamp now = System::SystemClock().GetMonotonicTimestamp();

            if (mLastAPDemandTime != System::Clock::kZero && now < (mLastAPDemandTime + mWiFiAPIdleTimeout))
            {
                targetState = kWiFiAPState_Active;

                // Compute the amount of idle time before the AP should be deactivated and
                // arm a timer to fire at that time.
                System::Clock::Timeout apTimeout = (mLastAPDemandTime + mWiFiAPIdleTimeout) - now;
                err                              = DeviceLayer::SystemLayer().StartTimer(apTimeout, DriveAPState, nullptr);
                SuccessOrExit(err);
                ChipLogProgress(DeviceLayer, "Next WiFi AP timeout in %" PRIu32 " ms",
                                System::Clock::Milliseconds32(apTimeout).count());
            }
            else
            {
                targetState = kWiFiAPState_NotActive;
            }
        }

        // Otherwise the target state is 'NotActive'.
        else
        {
            targetState = kWiFiAPState_NotActive;
        }

        // If the current AP state does not match the target state...
        if (mWiFiAPState != targetState)
        {
            // If the target state is 'Active' and the current state is NOT 'Activating', enable
            // and configure the AP interface, and then enter the 'Activating' state.  Eventually
            // a SYSTEM_EVENT_AP_START event will be received from the P6 WiFi layer which will
            // cause the state to transition to 'Active'.
            if (targetState == kWiFiAPState_Active)
            {
                if (mWiFiAPState != kWiFiAPState_Active)
                {
                    err = Internal::PIC32MZW1Utils::pic32mzw1_start_ap();
                    SuccessOrExit(err);
                    ChangeWiFiAPState(kWiFiAPState_Active);
                }
            }

            // Otherwise, if the target state is 'NotActive' and the current state is not 'Deactivating',
            // disable the AP interface and enter the 'Deactivating' state.  Later a SYSTEM_EVENT_AP_STOP
            // event will move the AP state to 'NotActive'.
            else
            {
                if (mWiFiAPState != kWiFiAPState_Deactivating)
                {
                    err = Internal::PIC32MZW1Utils::pic32mzw1_stop_ap();
                    SuccessOrExit(err);
                    ChangeWiFiAPState(kWiFiAPState_Deactivating);
                }
            }
        }
    }

exit:
    if (err != CHIP_NO_ERROR && mWiFiAPMode != kWiFiAPMode_ApplicationControlled)
    {
        SetWiFiAPMode(kWiFiAPMode_Disabled);
        Internal::PIC32MZW1Utils::pic32mzw1_stop_ap();
    }
}

void ConnectivityManagerImpl::DriveStationState()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    bool stationConnected;
    WDRV_PIC32MZW_STATUS result;
    ChipLogProgress(DeviceLayer, "DriverStationState() In");
    // If the station interface is currently connected ...
    if (mWiFiStationState == kWiFiStationState_Connected || mWiFiStationState == kWiFiStationState_Connecting) 
    {
        ChipLogProgress(DeviceLayer, "DriverStationState() log1");
        
        // If the WiFi station interface is no longer enabled, or no longer provisioned,
        // disconnect the station from the AP, unless the WiFi station mode is currently
        // under application control.
        if (mWiFiStationMode != kWiFiStationMode_ApplicationControlled &&
            (mWiFiStationMode != kWiFiStationMode_Enabled || !IsWiFiStationProvisioned()))
        {
            ChipLogProgress(DeviceLayer, "Disconnecting WiFi station interface");
        
            err =  Internal::PIC32MZW1Utils::pic32mzw1_wifi_disconnect();

            if (err == CHIP_NO_ERROR)
            {
                System::Clock::Timestamp now = System::SystemClock().GetMonotonicTimestamp();
                mLastStationConnectFailTime = now;

            }
            else
            {
                ChipLogError(DeviceLayer, "p6_wifi_disconnect() failed: %s", chip::ErrorStr(err));
            }

            
            SuccessOrExit(result);

            ChangeWiFiStationState(kWiFiStationState_Disconnecting);
        }
    }
    // Otherwise the station interface is NOT connected to an AP, so...
    else
    {
        ChipLogProgress(DeviceLayer, "DriverStationState() log2");
        System::Clock::Timestamp now = System::SystemClock().GetMonotonicTimestamp();

        // Advance the station state to NotConnected if it was previously Connected or Disconnecting,
        // or if a previous initiated connect attempt failed.
        if (mWiFiStationState == kWiFiStationState_Disconnecting ||
            mWiFiStationState == kWiFiStationState_Connecting_Failed)
        {
            ChipLogProgress(DeviceLayer, "DriverStationState() log3");
            WiFiStationState prevState = mWiFiStationState;
            ChangeWiFiStationState(kWiFiStationState_NotConnected);
            if (prevState != kWiFiStationState_Connecting_Failed)
            {
                ChipLogProgress(DeviceLayer, "WiFi station interface disconnected");
                mLastStationConnectFailTime = System::Clock::kZero;
            }
            else
            {
                mLastStationConnectFailTime = now;
            }
        }
        // If the WiFi station interface is now enabled and provisioned (and by implication,
        // not presently under application control), AND the system is not in the process of
        // scanning, then...
        if (mWiFiStationMode == kWiFiStationMode_Enabled && IsWiFiStationProvisioned())
        {
            ChipLogProgress(DeviceLayer, "DriverStationState() log4");
            // Initiate a connection to the AP if we haven't done so before, or if enough
            // time has passed since the last attempt.
            if (mLastStationConnectFailTime == System::Clock::kZero ||
                now >= mLastStationConnectFailTime + mWiFiStationReconnectInterval)
            {
                ChangeWiFiStationState(kWiFiStationState_Connecting);
                ChipLogProgress(DeviceLayer, "Attempting to connect WiFi station interface");
                err = Internal::PIC32MZW1Utils::pic32mzw1_wifi_connect();
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(DeviceLayer, "pic32mzw1_wifi_connect() failed: %s", chip::ErrorStr(err));
                }
                SuccessOrExit(err);
            }

            // Otherwise arrange another connection attempt at a suitable point in the future.
            else
            {
                System::Clock::Timeout timeToNextConnect = (mLastStationConnectFailTime + mWiFiStationReconnectInterval) - now;
                ChipLogProgress(DeviceLayer, "Next WiFi station reconnect in %" PRIu32 " ms ",
                                System::Clock::Milliseconds32(timeToNextConnect).count());

                err = DeviceLayer::SystemLayer().StartTimer(timeToNextConnect, DriveStationState, NULL);
                SuccessOrExit(err);
            }
        }
    }

exit:

    ChipLogProgress(DeviceLayer, "Done driving station state, nothing else to do...");
    // Kick-off any pending network scan that might have been deferred due to the activity
    // of the WiFi station.
}

void ConnectivityManagerImpl::UpdateInternetConnectivityState(void)
{
    bool haveIPv4Conn            = false;
    bool haveIPv6Conn            = false;
    const bool hadIPv4Conn       = mFlags.Has(ConnectivityFlags::kHaveIPv4InternetConnectivity);
    const bool hadIPv6Conn       = mFlags.Has(ConnectivityFlags::kHaveIPv6InternetConnectivity);
    struct netif * net_interface = NULL;
    IPAddress addr;
    bool stationConnected;
    Internal::PIC32MZW1Utils::IsStationConnected(stationConnected);

    ChipLogProgress(DeviceLayer, "UpdateInternetConnectivityState");
    // If the WiFi station is currently in the connected state...
    if ((mWiFiStationState == kWiFiStationState_Connected) || stationConnected)
    {
        net_interface = Network_PIC32MZW_Get_Interface();
        if (net_interface != NULL && netif_is_up(net_interface) && netif_is_link_up(net_interface))
        {
            if (!ip4_addr_isany(netif_ip4_addr(net_interface)) && !ip4_addr_isany(netif_ip4_gw(net_interface)))
            {
                haveIPv4Conn = true;
                ChipDeviceEvent event;
                event.Type                           = DeviceEventType::kInterfaceIpAddressChanged;
                event.InterfaceIpAddressChanged.Type = InterfaceIpChangeType::kIpV4_Assigned;
                PlatformMgr().PostEventOrDie(&event);
                ChipLogProgress(DeviceLayer, "IPv4 Address Assigned : %s", ip4addr_ntoa(netif_ip4_addr(net_interface)));
            }

            for (uint8_t i = 0; i < LWIP_IPV6_NUM_ADDRESSES; i++)
            {
                if (ip6_addr_islinklocal(netif_ip6_addr(net_interface, i)) &&
                    ip6_addr_isvalid(netif_ip6_addr_state(net_interface, i)))
                {
                    haveIPv6Conn = true;
                    ChipDeviceEvent event;
                    event.Type                           = DeviceEventType::kInterfaceIpAddressChanged;
                    event.InterfaceIpAddressChanged.Type = InterfaceIpChangeType::kIpV6_Assigned;
                    PlatformMgr().PostEventOrDie(&event);
                    ChipLogProgress(DeviceLayer, "IPv6 Address Assigned : %s", ip6addr_ntoa(netif_ip6_addr(net_interface, i)));
                }
            }
        }
    }
    // If the internet connectivity state has changed...
    if (haveIPv4Conn != hadIPv4Conn || haveIPv6Conn != hadIPv6Conn)
    {
        // Update the current state.
        mFlags.Set(ConnectivityFlags::kHaveIPv4InternetConnectivity, haveIPv4Conn)
            .Set(ConnectivityFlags::kHaveIPv6InternetConnectivity, haveIPv6Conn);

        // Alert other components of the state change.
        ChipDeviceEvent event;
        event.Type                                 = DeviceEventType::kInternetConnectivityChange;
        event.InternetConnectivityChange.IPv4      = GetConnectivityChange(hadIPv4Conn, haveIPv4Conn);
        event.InternetConnectivityChange.IPv6      = GetConnectivityChange(hadIPv6Conn, haveIPv6Conn);
        event.InternetConnectivityChange.ipAddress = addr;
        PlatformMgr().PostEventOrDie(&event);

        if (haveIPv4Conn != hadIPv4Conn)
        {
            ChipLogProgress(DeviceLayer, "%s Internet connectivity %s", "IPv4", (haveIPv4Conn) ? "ESTABLISHED" : "LOST");
        }

        if (haveIPv6Conn != hadIPv6Conn)
        {
            ChipLogProgress(DeviceLayer, "%s Internet connectivity %s", "IPv6", (haveIPv6Conn) ? "ESTABLISHED" : "LOST");
        }
    }
}

CHIP_ERROR ConnectivityManagerImpl::OnIPAddressAvailable(void)
{
    ChipLogProgress(DeviceLayer, "IP address available on WiFi station interface");
    UpdateInternetConnectivityState();
    return CHIP_NO_ERROR;
}


CHIP_ERROR ConnectivityManagerImpl::ping_thread()
{

    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace DeviceLayer
} // namespace chip
