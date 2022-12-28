/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/clusters/ota-requestor/OTADownloader.h>
#include <fstream>
#include <lib/core/OTAImageHeader.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/OTAImageProcessor.h>
#include "crypto/crypto.h"

#ifdef PIC32MZW1_OTA

#define IMAGE_DESC_STATUS_INVALID           0
#define IMAGE_DESC_STATUS_IN_USE            1
#define IMAGE_DESC_STATUS_NOT_IN_USE        2
#define IMAGE_DESC_STATUS_UPGRADABLE        3
#define IMAGE_DESC_STATUS_IN_PROGRESS_UPGRADE 4

#define SHA256_DIGEST_SIZE  32

typedef struct
{
    uint8_t type;
    uint8_t status;
    uint32_t ver;
    uint32_t image_size;
} image_descriptor;

namespace chip {
namespace DeviceLayer {

class OTAImageProcessorImpl : public OTAImageProcessorInterface
{
public:
    //////////// OTAImageProcessorInterface Implementation ///////////////
    CHIP_ERROR PrepareDownload() override;
    CHIP_ERROR Finalize() override;
    CHIP_ERROR Apply() override;
    CHIP_ERROR Abort() override;
    CHIP_ERROR ProcessBlock(ByteSpan & block) override;
    bool IsFirstImageRun() override;
    CHIP_ERROR ConfirmCurrentImage() override;

    void SetOTADownloader(OTADownloader * downloader) { mDownloader = downloader; }

    

private:
    //////////// Actual handlers for the OTAImageProcessorInterface ///////////////
    static void HandlePrepareDownload(intptr_t context);
    static void HandleFinalize(intptr_t context);
    static void HandleAbort(intptr_t context);
    static void HandleProcessBlock(intptr_t context);
    static void HandleApply(intptr_t context);

    /**
     * Called to allocate memory for mBlock if necessary and set it to block
     */
    CHIP_ERROR SetBlock(ByteSpan & block);

    /**
     * Called to release allocated memory for mBlock
     */
    CHIP_ERROR ReleaseBlock();

    /**
     * Call to skip over any OTAImageHeader bytes in the downloaded image.
     */
    CHIP_ERROR ProcessHeader(ByteSpan & block);

    MutableByteSpan mBlock;
    OTADownloader * mDownloader;

    static unsigned char temp_buf[1024];
    static int temp_buf_size;
    static uint32_t program_address;
    static QueueHandle_t queue_ota_image;
    static CRYPT_SHA256_CTX sha;
    static uint8_t   image_digest[SHA256_DIGEST_SIZE];

    const struct flash_area * mFlashArea;

    // Parser is used to remove the OTAImageHeader from the incoming image.
    OTAImageHeaderParser mHeaderParser;

    static uint8_t mProgramSlot;
};

} // namespace DeviceLayer
} // namespace chip
#endif
