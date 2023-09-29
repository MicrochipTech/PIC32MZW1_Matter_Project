/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include "OTAImageProcessorImpl.h"
#include <app/clusters/ota-requestor/OTADownloader.h>
#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include "sst26/ext_flash.h"

#define FLASH_SECTOR_SIZE         4096
#define IMAGE_SIZE         1024*1024
#define BOOT_DESCRIPTOR_SIZE     FLASH_SECTOR_SIZE
#define IMAGE_1_DESCRIPTOR_ADDR     1028*1024
#define IMAGE_1_START_ADDR     1032*1024
#define IMAGE_2_DESCRIPTOR_ADDR     2056*1024
#define IMAGE_2_START_ADDR     2060*1024

#define OTA_QUEUE_LENGTH    4096


using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {

uint8_t OTAImageProcessorImpl::mProgramSlot = 0;
unsigned char OTAImageProcessorImpl::temp_buf[] = {};
int OTAImageProcessorImpl::temp_buf_size = 0;
uint32_t OTAImageProcessorImpl::program_address = IMAGE_1_START_ADDR;
QueueHandle_t OTAImageProcessorImpl::queue_ota_image = NULL;
CRYPT_SHA256_CTX    OTAImageProcessorImpl::sha;
uint8_t OTAImageProcessorImpl::image_digest[] = {};

#ifdef PIC32MZW1_OTA
CHIP_ERROR OTAImageProcessorImpl::PrepareDownload()
{
    ChipLogProgress(SoftwareUpdate, "PrepareDownload");
    DeviceLayer::PlatformMgr().ScheduleWork(HandlePrepareDownload, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Finalize()
{
    ChipLogProgress(SoftwareUpdate, "Finalize");
    DeviceLayer::PlatformMgr().ScheduleWork(HandleFinalize, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Apply()
{
    ChipLogProgress(SoftwareUpdate, "Apply");
    DeviceLayer::PlatformMgr().ScheduleWork(HandleApply, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::Abort()
{
    ChipLogProgress(SoftwareUpdate, "Abort");
    DeviceLayer::PlatformMgr().ScheduleWork(HandleAbort, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::ProcessHeader(ByteSpan & block)
{
    ChipLogProgress(SoftwareUpdate, "block");
    // Only modify the ByteSpan if the OTAImageHeaderParser is currently initialized.
    if (mHeaderParser.IsInitialized())
    {
        OTAImageHeader header;

        // AccumulateAndDecode will cause the OTAImageHeader bytes to be stored
        // in header. We don't do anything with header, however, the other
        // consequence of this call is to advance the data pointer in block. In
        // this or subsequent calls to this API, block will end up pointing at
        // the first byte after OTAImageHeader.
        CHIP_ERROR error = mHeaderParser.AccumulateAndDecode(block, header);

        // If we have not received all the bytes of the OTAImageHeader yet, that is OK.
        // Return CHIP_NO_ERROR and expect that future blocks will contain the rest.
        ReturnErrorCodeIf(error == CHIP_ERROR_BUFFER_TOO_SMALL, CHIP_NO_ERROR);

        // If there is some error other than "too small", return that so future
        // processing will be aborted.
        ReturnErrorOnFailure(error);

        mParams.totalFileBytes = header.mPayloadSize;

        memcpy(image_digest, header.mImageDigest.data(), header.mImageDigest.size());

        // If we are here, then we have received all the OTAImageHeader bytes.
        // Calling Clear() here results in the parser state being set to
        // uninitialized. This means future calls to ProcessHeader will not
        // modify block and those future bytes will be written to the device.
        mHeaderParser.Clear();
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::ProcessBlock(ByteSpan & block)
{
    ChipLogProgress(SoftwareUpdate, "ProcessBlock");

    if ((block.data() == nullptr) || block.empty())
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Store block data for HandleProcessBlock to access
    CHIP_ERROR err = SetBlock(block);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot set block data: %" CHIP_ERROR_FORMAT, err.Format());
    }

    DeviceLayer::PlatformMgr().ScheduleWork(HandleProcessBlock, reinterpret_cast<intptr_t>(this));
    return CHIP_NO_ERROR;
}

bool OTAImageProcessorImpl::IsFirstImageRun()
{
    ChipLogProgress(SoftwareUpdate, "IsFirstImageRun");
    OTARequestorInterface * requestor;
    requestor = GetRequestorInstance();
    ReturnErrorCodeIf(requestor == nullptr, false);

    uint32_t currentVersion;
    ReturnErrorCodeIf(ConfigurationMgr().GetSoftwareVersion(currentVersion) != CHIP_NO_ERROR, false);

    ChipLogProgress(SoftwareUpdate, "%d", currentVersion);
    ChipLogProgress(SoftwareUpdate, "%d", requestor->GetTargetVersion());

    return ((requestor->GetCurrentUpdateState() == OTARequestorInterface::OTAUpdateStateEnum::kApplying) &&
            (requestor->GetTargetVersion() == currentVersion));
}

CHIP_ERROR OTAImageProcessorImpl::ConfirmCurrentImage()
{
    ChipLogProgress(SoftwareUpdate, "ConfirmCurrentImage");
    OTARequestorInterface * requestor = chip::GetRequestorInstance();
    if (requestor == nullptr)
    {
        return CHIP_ERROR_INTERNAL;
    }

    uint32_t currentVersion;
    ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetSoftwareVersion(currentVersion));
    if (currentVersion != requestor->GetTargetVersion())
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    return CHIP_NO_ERROR;
}

void OTAImageProcessorImpl::HandlePrepareDownload(intptr_t context)
{
    ChipLogProgress(SoftwareUpdate, "HandlePrepareDownload");
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "ImageProcessor context is null");
        return;
    }
    else if (imageProcessor->mDownloader == nullptr)
    {
        ChipLogError(SoftwareUpdate, "mDownloader is null");
        return;
    }
    
    image_descriptor image1_desc;

    EXT_Flash_Initialize();
    EXT_Flash_Read(IMAGE_1_DESCRIPTOR_ADDR, (uint8_t*) &image1_desc, sizeof(image_descriptor));

    if (image1_desc.status != IMAGE_DESC_STATUS_IN_USE)    
    {
        ChipLogProgress(SoftwareUpdate, "use slot1 for OTA");
        mProgramSlot = 1;
        program_address = IMAGE_1_START_ADDR;
        EXT_Flash_Erase(IMAGE_1_DESCRIPTOR_ADDR, IMAGE_SIZE + BOOT_DESCRIPTOR_SIZE);
    }
    else
    { 
        ChipLogProgress(SoftwareUpdate, "use slot2 for OTA");
        mProgramSlot = 2;
        program_address = IMAGE_2_START_ADDR;
        EXT_Flash_Erase(IMAGE_2_DESCRIPTOR_ADDR, IMAGE_SIZE + BOOT_DESCRIPTOR_SIZE);
    }

    if (queue_ota_image == NULL)
        queue_ota_image = xQueueCreate( OTA_QUEUE_LENGTH, sizeof( unsigned char ) );

    CRYPT_SHA256_Initialize(&sha);

    // init the OTAImageHeaderParser instance to indicate that we haven't yet
    // parsed the header out of the incoming image.
    imageProcessor->mHeaderParser.Init();

    imageProcessor->mDownloader->OnPreparedForDownload(CHIP_NO_ERROR);
}

void OTAImageProcessorImpl::HandleFinalize(intptr_t context)
{
    ChipLogProgress(SoftwareUpdate, "HandleFinalize");
    uint8_t   hash[SHA256_DIGEST_SIZE];
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    image_descriptor image_desc;

    if (temp_buf_size > 0)
        EXT_Flash_Write(program_address, (uint8_t*) temp_buf, 1024);
      
    CRYPT_SHA256_Finalize(&sha, hash);
        
    if (mProgramSlot == 1)
    {
        memset(&image_desc, 0, sizeof(image_descriptor));

        if ( 0 == memcmp(&hash, image_digest, SHA256_DIGEST_SIZE))
        {
            ChipLogProgress(SoftwareUpdate, "Digest verification is pass");
            image_desc.status = IMAGE_DESC_STATUS_UPGRADABLE;
        }
        else
        {
            ChipLogProgress(SoftwareUpdate, "Digest verification is fail");
            image_desc.status = IMAGE_DESC_STATUS_INVALID;
        }
        
        image_desc.image_size = (uint32_t) imageProcessor->mParams.totalFileBytes;
        EXT_Flash_Write(IMAGE_1_DESCRIPTOR_ADDR, (uint8_t*) &image_desc, sizeof(image_desc));
    }
    else if (mProgramSlot == 2)
    {
        memset(&image_desc, 0, sizeof(image_descriptor));
        if ( 0 == memcmp(&hash, image_digest, SHA256_DIGEST_SIZE))
        {
            ChipLogProgress(SoftwareUpdate, "Digest verification is pass");
            image_desc.status = IMAGE_DESC_STATUS_UPGRADABLE;
        }
        else
        {
            ChipLogProgress(SoftwareUpdate, "Digest verification is fail");
            image_desc.status = IMAGE_DESC_STATUS_INVALID;
        }

        image_desc.image_size = (uint32_t) imageProcessor->mParams.totalFileBytes;
        EXT_Flash_Write(IMAGE_2_DESCRIPTOR_ADDR, (uint8_t*) &image_desc, sizeof(image_desc)); 
    }
    else
    {   
        ChipLogError(SoftwareUpdate, "Failed to Finalize, mProgramSlog = %d", mProgramSlot);
    }
  
    imageProcessor->ReleaseBlock();
}

void OTAImageProcessorImpl::HandleAbort(intptr_t context)
{
    ChipLogProgress(SoftwareUpdate, "HandleAbort");
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        return;
    }

    if (mProgramSlot == 1)    
    {
        EXT_Flash_Erase(IMAGE_1_DESCRIPTOR_ADDR, IMAGE_SIZE + BOOT_DESCRIPTOR_SIZE);
    }
    else if (mProgramSlot == 2)
    { 
        EXT_Flash_Erase(IMAGE_2_DESCRIPTOR_ADDR, IMAGE_SIZE + BOOT_DESCRIPTOR_SIZE);
    }

    imageProcessor->ReleaseBlock();
}

void OTAImageProcessorImpl::HandleProcessBlock(intptr_t context)
{
    ChipLogProgress(SoftwareUpdate, "HandleProcessBlock");
    int i = 0;
    uint8_t queue_is_empty = 0;
    auto * imageProcessor = reinterpret_cast<OTAImageProcessorImpl *>(context);
    if (imageProcessor == nullptr)
    {
        ChipLogError(SoftwareUpdate, "ImageProcessor context is null");
        return;
    }
    else if (imageProcessor->mDownloader == nullptr)
    {
        ChipLogError(SoftwareUpdate, "mDownloader is null");
        return;
    }

    // The call to ProcessHeader will result in the modification of the block ByteSpan data
    // pointer if the OTAImageHeader is present in the image. The result is that only
    // the new application bytes will be written to the device in the flash_area_write calls,
    // as all bytes for the header are skipped.
    ByteSpan block = ByteSpan(imageProcessor->mBlock.data(), imageProcessor->mBlock.size());
   
    CHIP_ERROR error = imageProcessor->ProcessHeader(block);
    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Failed to process OTA image header");
        imageProcessor->mDownloader->EndDownload(error);
        return;
    }
    
    ChipLogProgress(SoftwareUpdate, "downloadedBytes = %d, totalFileBytes = %d", (int)imageProcessor->mParams.downloadedBytes, (int)imageProcessor->mParams.totalFileBytes);

    CRYPT_SHA256_DataAdd(&sha, (uint8_t*)block.data(),block.size());

    for (i = 0; i < block.size(); i++)
    {
        if ( pdTRUE != xQueueSend( queue_ota_image, &block.data()[i], ( TickType_t ) 0 ) )
        {
            ChipLogError(SoftwareUpdate, "Failed to add OTA data to the queue..");
        }
    }

    while (!queue_is_empty)
    {
        while (temp_buf_size < 1024)
        {
            if( xQueueReceive( queue_ota_image, &temp_buf[temp_buf_size], 0 ) )
            {   
                temp_buf_size++;
            }
            else
            {   
                queue_is_empty = 1;
                break;
            }
        }

        if (temp_buf_size == 1024)
        {
            EXT_Flash_Write(program_address, (uint8_t*) temp_buf, 1024);
            
            memset(temp_buf, 0xff, sizeof(temp_buf));
            temp_buf_size = 0;
            program_address += 1024;

        }

    }
    // increment the total downloaded bytes by the potentially modified block ByteSpan size
    imageProcessor->mParams.downloadedBytes += block.size();
    imageProcessor->mDownloader->FetchNextData();
}

void OTAImageProcessorImpl::HandleApply(intptr_t context)
{
    ChipLogProgress(SoftwareUpdate, "Rebooting after 2 seconds...");
    printf("Rebooting after 2 seconds..");

    vTaskDelay( 2000 );
    
    DeviceLayer::Internal::PIC32MZW1Config::SystemReset();

    return;
}

CHIP_ERROR OTAImageProcessorImpl::SetBlock(ByteSpan & block)
{
    ChipLogProgress(SoftwareUpdate, "SetBlock");
    if (!IsSpanUsable(block))
    {
        ReleaseBlock();
        return CHIP_NO_ERROR;
    }
    if (mBlock.size() < block.size())
    {
        if (!mBlock.empty())
        {
            ReleaseBlock();
        }
        uint8_t * mBlock_ptr = static_cast<uint8_t *>(chip::Platform::MemoryAlloc(block.size()));
        if (mBlock_ptr == nullptr)
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        mBlock = MutableByteSpan(mBlock_ptr, block.size());
    }
    CHIP_ERROR err = CopySpanToMutableSpan(block, mBlock);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot copy block data: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTAImageProcessorImpl::ReleaseBlock()
{
    ChipLogProgress(SoftwareUpdate, "ReleaseBlock");
    if (mBlock.data() != nullptr)
    {
        chip::Platform::MemoryFree(mBlock.data());
    }

    mBlock = MutableByteSpan();
    return CHIP_NO_ERROR;
}
#endif 

} // namespace DeviceLayer
} // namespace chip
