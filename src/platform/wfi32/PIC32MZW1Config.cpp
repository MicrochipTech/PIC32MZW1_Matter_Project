/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
 *    Copyright (c) 2019-2020 Google LLC.
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
 *          Utilities for interacting with the the PIC32MZW1 key-value store.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/KeyValueStoreManager.h>

#include <platform/wfi32/PIC32MZW1Config.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#define STORE_MAX_KEY_SIZE    64
namespace chip {
namespace DeviceLayer {
namespace Internal {

// *** CAUTION ***: Changing the names or namespaces of these values will *break* existing devices.

// Namespaces used to store device configuration information.
const char PIC32MZW1Config::kConfigNamespace_ChipFactory[]  = "chip-factory";
const char PIC32MZW1Config::kConfigNamespace_ChipConfig[]   = "chip-config";
const char PIC32MZW1Config::kConfigNamespace_ChipCounters[] = "chip-counters";

// Keys stored in the chip-factory namespace
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_SerialNum             = { kConfigNamespace_ChipFactory, "serial-num" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_MfrDeviceId           = { kConfigNamespace_ChipFactory, "device-id" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_MfrDeviceCert         = { kConfigNamespace_ChipFactory, "device-cert" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_MfrDeviceICACerts     = { kConfigNamespace_ChipFactory, "device-ca-certs" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_MfrDevicePrivateKey   = { kConfigNamespace_ChipFactory, "device-key" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_SoftwareVersion       = { kConfigNamespace_ChipFactory, "software-ver" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_HardwareVersion       = { kConfigNamespace_ChipFactory, "hardware-ver" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_ManufacturingDate     = { kConfigNamespace_ChipFactory, "mfg-date" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_SetupPinCode          = { kConfigNamespace_ChipFactory, "pin-code" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_SetupDiscriminator    = { kConfigNamespace_ChipFactory, "discriminator" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_Spake2pIterationCount = { kConfigNamespace_ChipFactory, "iteration-count" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_Spake2pSalt           = { kConfigNamespace_ChipFactory, "salt" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_Spake2pVerifier       = { kConfigNamespace_ChipFactory, "verifier" };

// Keys stored in the chip-config namespace
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_ServiceConfig      = { kConfigNamespace_ChipConfig, "service-config" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_PairedAccountId    = { kConfigNamespace_ChipConfig, "account-id" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_ServiceId          = { kConfigNamespace_ChipConfig, "service-id" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_LastUsedEpochKeyId = { kConfigNamespace_ChipConfig, "last-ek-id" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_FailSafeArmed      = { kConfigNamespace_ChipConfig, "fail-safe-armed" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_WiFiStationSecType = { kConfigNamespace_ChipConfig, "sta-sec-type" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_RegulatoryLocation = { kConfigNamespace_ChipConfig, "regulatory-location" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_CountryCode        = { kConfigNamespace_ChipConfig, "country-code" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_WiFiSSID           = { kConfigNamespace_ChipConfig, "wifi-ssid" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_WiFiPassword       = { kConfigNamespace_ChipConfig, "wifi-password" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_WiFiSecurity       = { kConfigNamespace_ChipConfig, "wifi-security" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_WiFiMode           = { kConfigNamespace_ChipConfig, "wifimode" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_UniqueId           = { kConfigNamespace_ChipConfig, "unique-id" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_LockUser           = { kConfigNamespace_ChipConfig, "lock-user" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_Credential         = { kConfigNamespace_ChipConfig, "credential" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_LockUserName       = { kConfigNamespace_ChipConfig, "lock-user-name" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_CredentialData     = { kConfigNamespace_ChipConfig, "credential-data" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_UserCredentials    = { kConfigNamespace_ChipConfig, "user-credentials" };
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_WeekDaySchedules   = { kConfigNamespace_ChipConfig, "weekday-schedules" };
;
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_YearDaySchedules = { kConfigNamespace_ChipConfig, "yearday-schedules" };
;
const PIC32MZW1Config::Key PIC32MZW1Config::kConfigKey_HolidaySchedules = { kConfigNamespace_ChipConfig, "holiday-schedules" };
;

// Keys stored in the Chip-counters namespace
const PIC32MZW1Config::Key PIC32MZW1Config::kCounterKey_RebootCount           = { kConfigNamespace_ChipCounters, "reboot-count" };
const PIC32MZW1Config::Key PIC32MZW1Config::kCounterKey_UpTime                = { kConfigNamespace_ChipCounters, "up-time" };
const PIC32MZW1Config::Key PIC32MZW1Config::kCounterKey_TotalOperationalHours = { kConfigNamespace_ChipCounters, "total-hours" };

CHIP_ERROR PIC32MZW1Config::ReadConfigValue(Key key, bool & val)
{
    bool in;
    char key_str[STORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, STORE_MAX_KEY_SIZE);
    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(key_str, static_cast<void *>(&in), sizeof(bool));
    val            = in;
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR PIC32MZW1Config::ReadConfigValue(Key key, uint32_t & val)
{
    uint32_t in;
    char key_str[STORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, STORE_MAX_KEY_SIZE);
    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(key_str, static_cast<void *>(&in), 4);
    val            = in;
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR PIC32MZW1Config::ReadConfigValue(Key key, uint64_t & val)
{
    uint64_t in;
    char key_str[STORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, STORE_MAX_KEY_SIZE);
    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(key_str, static_cast<void *>(&in), 8);
    val            = in;
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR PIC32MZW1Config::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    char key_str[STORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, STORE_MAX_KEY_SIZE);
    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(key_str, buf, bufSize, &outLen);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR PIC32MZW1Config::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    char key_str[STORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, STORE_MAX_KEY_SIZE);
    CHIP_ERROR err = PersistedStorage::KeyValueStoreMgr().Get(key_str, buf, bufSize, &outLen);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR PIC32MZW1Config::WriteConfigValue(Key key, bool val)
{
    char key_str[STORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, STORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Put(key_str, static_cast<void *>(&val), sizeof(bool));
}

CHIP_ERROR PIC32MZW1Config::WriteConfigValue(Key key, uint32_t val)
{
    char key_str[STORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, STORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Put(key_str, static_cast<void *>(&val), 4);
}

CHIP_ERROR PIC32MZW1Config::WriteConfigValue(Key key, uint64_t val)
{
    char key_str[STORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, STORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Put(key_str, static_cast<void *>(&val), 8);
}

CHIP_ERROR PIC32MZW1Config::WriteConfigValueStr(Key key, const char * str)
{
    size_t size                            = strlen(str) + 1;
    char key_str[STORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, STORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Put(key_str, str, size);
}

CHIP_ERROR PIC32MZW1Config::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    char key_str[STORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, STORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Put(key_str, str, strLen);
}
CHIP_ERROR PIC32MZW1Config::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    char key_str[STORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, STORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Put(key_str, static_cast<void *>(&data), dataLen);
}

CHIP_ERROR PIC32MZW1Config::ClearConfigValue(Key key)
{
    char key_str[STORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, STORE_MAX_KEY_SIZE);
    return PersistedStorage::KeyValueStoreMgr().Delete(key_str);
}

bool PIC32MZW1Config::ConfigValueExists(Key key)
{
    char key_str[STORE_MAX_KEY_SIZE] = { 0 };
    key.to_str(key_str, STORE_MAX_KEY_SIZE);
    if (PersistedStorage::KeyValueStoreMgr().Get(key_str, NULL, 0) == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        return false;
    }

    return true;
}

// Clear out keys in config namespace
CHIP_ERROR PIC32MZW1Config::FactoryResetConfig(void)
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    const Key * config_keys[] = { &kConfigKey_ServiceConfig,      &kConfigKey_PairedAccountId, &kConfigKey_ServiceId,
                                  &kConfigKey_LastUsedEpochKeyId, &kConfigKey_FailSafeArmed,   &kConfigKey_WiFiStationSecType,
                                  &kConfigKey_WiFiSSID,           &kConfigKey_WiFiPassword,    &kConfigKey_WiFiSecurity,
                                  &kConfigKey_WiFiMode,           &kConfigKey_SoftwareVersion };

    for (uint32_t i = 0; i < (sizeof(config_keys) / sizeof(config_keys[0])); i++)
    {
        err = ClearConfigValue(*config_keys[i]);
        // Something unexpected happened
        if (err != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND && err != CHIP_NO_ERROR)
        {
            return err;
        }
    }

    // To Do: Erase all key-values including fabric info.
    err = PersistedStorage::KeyValueStoreMgrImpl().Erase();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Erase Key-Value Storage fail..");
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR PIC32MZW1Config::SystemReset(void)
{
    /* Short delay for debug log output before reset. */
    vTaskDelay( 100 );
    
    /* Perform system unlock sequence */ 
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;
    RSWRSTSET  = _RSWRST_SWRST_MASK;
    
    return CHIP_NO_ERROR;
}
void PIC32MZW1Config::RunConfigUnitTest() {}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
