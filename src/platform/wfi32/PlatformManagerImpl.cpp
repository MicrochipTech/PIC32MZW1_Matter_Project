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
 *          Provides an implementation of the PlatformManager object
 *          for the PSoC6 platform.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <crypto/CHIPCryptoPAL.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.ipp>


// To Do:
extern "C" int mbedtls_hardware_poll(void * data, unsigned char * output, size_t len, size_t * olen)
{
    (void) data;
    char test[] = "123456"; 
    
    memcpy(output,test, sizeof(test));
    *olen = len;
    
    return 0;
}

namespace chip {
namespace DeviceLayer {

namespace Internal {
extern CHIP_ERROR InitLwIPCoreLock(void);
}

PlatformManagerImpl PlatformManagerImpl::sInstance;

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    CHIP_ERROR err;
    ChipLogError(DeviceLayer, "log1\r\n");
    
    // Make sure the LwIP core lock has been initialized
    err = Internal::InitLwIPCoreLock();
    //ChipLogError(DeviceLayer, "log2,err = %d\r\n", err);
    SuccessOrExit(err);
    ChipLogError(DeviceLayer, "log3\r\n");
    	
    mStartTime = System::SystemClock().GetMonotonicTimestamp();
    //ChipLogError(DeviceLayer, "log4, mStartTime = %d\r\n", mStartTime);
    //return err;
    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    err = Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_InitChipStack();
    //ChipLogError(DeviceLayer, "log4,err = %d\r\n", err);
    SuccessOrExit(err);
    ChipLogError(DeviceLayer, "log6\r\n");
exit:
    return err;
}

CHIP_ERROR PlatformManagerImpl::InitLwIPCoreLock(void)
{
    return Internal::InitLwIPCoreLock();
}

void PlatformManagerImpl::_Shutdown()
{
    uint64_t upTime = 0;

    if (GetDiagnosticDataProvider().GetUpTime(upTime) == CHIP_NO_ERROR)
    {
        uint32_t totalOperationalHours = 0;

        if (ConfigurationMgr().GetTotalOperationalHours(totalOperationalHours) == CHIP_NO_ERROR)
        {
            ConfigurationMgr().StoreTotalOperationalHours(totalOperationalHours + static_cast<uint32_t>(upTime / 3600));
        }
        else
        {
            ChipLogError(DeviceLayer, "Failed to get total operational hours of the Node");
        }
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to get current uptime since the Node’s last reboot");
    }

    Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_Shutdown();
}

} // namespace DeviceLayer
} // namespace chip
