/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <credentials/examples/ExampleDACs.h>
#include <credentials/examples/ExamplePAI.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include "FactoryDataProvider.h"
#include "CHIPCryptoPALEcc608.h"
#include "atca_basic.h"
#include "atca_status.h"
#include "atcacert/atcacert_def.h"
#include "atcacert/atcacert_client.h"
#include "AttestationPAI.h"

namespace chip {
namespace DeviceLayer {

const uint8_t g_cert_template_2_device[] = {
    0x30, 0x82, 0x01, 0xe3, 0x30, 0x82, 0x01, 0x89, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x08, 0x39, 0xdd, 0x1f, 0x36, 0xf8, 0x63,
    0x2e, 0xa2, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x30, 0x3d, 0x31, 0x25, 0x30, 0x23, 0x06,
    0x03, 0x55, 0x04, 0x03, 0x0c, 0x1c, 0x4d, 0x61, 0x74, 0x74, 0x65, 0x72, 0x20, 0x44, 0x65, 0x76, 0x20, 0x50, 0x41, 0x49, 0x20,
    0x30, 0x78, 0x46, 0x46, 0x46, 0x31, 0x20, 0x6e, 0x6f, 0x20, 0x50, 0x49, 0x44, 0x31, 0x14, 0x30, 0x12, 0x06, 0x0a, 0x2b, 0x06,
    0x01, 0x04, 0x01, 0x82, 0xa2, 0x7c, 0x02, 0x01, 0x0c, 0x04, 0x46, 0x46, 0x46, 0x31, 0x30, 0x1e, 0x17, 0x0d, 0x32, 0x30, 0x31,
    0x30, 0x31, 0x35, 0x31, 0x34, 0x30, 0x30, 0x30, 0x30, 0x5a, 0x17, 0x0d, 0x34, 0x30, 0x31, 0x30, 0x31, 0x35, 0x31, 0x34, 0x30,
    0x30, 0x30, 0x30, 0x5a, 0x30, 0x50, 0x31, 0x22, 0x30, 0x20, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x19, 0x4d, 0x61, 0x74, 0x74,
    0x65, 0x72, 0x20, 0x44, 0x65, 0x76, 0x65, 0x6c, 0x6f, 0x70, 0x6d, 0x65, 0x6e, 0x74, 0x20, 0x44, 0x41, 0x43, 0x20, 0x30, 0x31,
    0x31, 0x14, 0x30, 0x12, 0x06, 0x0a, 0x2b, 0x06, 0x01, 0x04, 0x01, 0x82, 0xa2, 0x7c, 0x02, 0x01, 0x0c, 0x04, 0x46, 0x46, 0x46,
    0x31, 0x31, 0x14, 0x30, 0x12, 0x06, 0x0a, 0x2b, 0x06, 0x01, 0x04, 0x01, 0x82, 0xa2, 0x7c, 0x02, 0x02, 0x0c, 0x04, 0x38, 0x30,
    0x30, 0x31, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce,
    0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x16, 0x77, 0x63, 0x86, 0x1f, 0xe4, 0xe7, 0xfb, 0x2c, 0x1f, 0x10, 0x01, 0x30,
    0xc1, 0x6f, 0x12, 0xaa, 0xad, 0xda, 0xd2, 0xb7, 0x84, 0x43, 0xea, 0x73, 0xe5, 0xdc, 0x3b, 0xde, 0xbb, 0x86, 0x73, 0x61, 0x8e,
    0x24, 0x5c, 0x84, 0x78, 0x85, 0xc5, 0xbe, 0x98, 0xd1, 0x77, 0xd3, 0xeb, 0xb9, 0xed, 0x6a, 0xf1, 0x3a, 0x58, 0x7d, 0x52, 0x31,
    0x49, 0xc2, 0xf0, 0xef, 0x0d, 0x72, 0x8d, 0x7c, 0x64, 0xa3, 0x60, 0x30, 0x5e, 0x30, 0x0c, 0x06, 0x03, 0x55, 0x1d, 0x13, 0x01,
    0x01, 0xff, 0x04, 0x02, 0x30, 0x00, 0x30, 0x0e, 0x06, 0x03, 0x55, 0x1d, 0x0f, 0x01, 0x01, 0xff, 0x04, 0x04, 0x03, 0x02, 0x07,
    0x80, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e, 0x04, 0x16, 0x04, 0x14, 0xe3, 0x8f, 0x96, 0xed, 0x97, 0x2e, 0x3f, 0x57, 0x02,
    0x22, 0xc1, 0x2a, 0x90, 0x1d, 0xf1, 0xa3, 0xa7, 0x96, 0xbb, 0x93, 0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30,
    0x16, 0x80, 0x14, 0x63, 0x54, 0x0e, 0x47, 0xf6, 0x4b, 0x1c, 0x38, 0xd1, 0x38, 0x84, 0xa4, 0x62, 0xd1, 0x6c, 0x19, 0x5d, 0x8f,
    0xfb, 0x3c, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x03, 0x48, 0x00, 0x30, 0x45, 0x02, 0x20,
    0x02, 0xd5, 0x44, 0xc3, 0xf7, 0x23, 0xb1, 0x14, 0x0d, 0xf3, 0xc5, 0xc3, 0x43, 0x28, 0xaa, 0x6a, 0x97, 0x0e, 0xb2, 0xb3, 0x8a,
    0x08, 0x84, 0xc1, 0x2d, 0x31, 0x92, 0x46, 0x98, 0x39, 0x8f, 0x2c, 0x02, 0x21, 0x00, 0xd6, 0xbc, 0x59, 0x04, 0xa8, 0xf0, 0xa2,
    0xa0, 0xbe, 0xbc, 0x35, 0x33, 0xee, 0x85, 0xf0, 0x04, 0x9f, 0xae, 0x88, 0xcc, 0xaf, 0xa9, 0xee, 0xee, 0x19, 0x2b, 0x3a, 0x39,
    0xfb, 0x43, 0xc6, 0x36
};

const atcacert_def_t g_cert_def_2_device = {
    .type                = CERTTYPE_X509,
    .template_id         = 2,
    .chain_id            = 0,
    .private_key_slot    = 0,
    .sn_source           = SNSRC_STORED, // SNSRC_PUB_KEY_HASH,
    .cert_sn_dev_loc     = { .zone = DEVZONE_DATA, .slot = 8, .is_genkey = 0, .offset = 0, .count = 8 },
    .issue_date_format   = DATEFMT_RFC5280_UTC,
    .expire_date_format  = DATEFMT_RFC5280_UTC,
    .tbs_cert_loc        = { .offset = 4, .count = 394 },
    .expire_years        = 20,
    .public_key_dev_loc  = { .zone = DEVZONE_DATA, .slot = 0, .is_genkey = 1, .offset = 0, .count = 64 },
    .comp_cert_dev_loc   = { .zone = DEVZONE_DATA, .slot = 10, .is_genkey = 0, .offset = 0, .count = 72 },
    .std_cert_elements   = { { // STDCERT_PUBLIC_KEY
                               .offset = 239,
                               .count  = 64 },
                             { // STDCERT_SIGNATURE
                               .offset = 413,
                               .count  = 74 },
                             { // STDCERT_ISSUE_DATE
                               .offset = 102,
                               .count  = 13 },
                             { // STDCERT_EXPIRE_DATE
                               .offset = 117,
                               .count  = 13 },
                             { // STDCERT_SIGNER_ID
                               .offset = 94,
                               .count  = 4 },
                             { // STDCERT_CERT_SN
                               .offset = 15,
                               .count  = 8 },
                             { // STDCERT_AUTH_KEY_ID
                               .offset = 381,
                               .count  = 20 },
                             { // STDCERT_SUBJ_KEY_ID
                               .offset = 348,
                               .count  = 20 } },
    .cert_elements       = NULL,
    .cert_elements_count = 0,
    .cert_template       = g_cert_template_2_device,
    .cert_template_size  = sizeof(g_cert_template_2_device),
    .ca_cert_def         = NULL, //&g_cert_def_1_signer,
};

uint8_t device_der_qa[550];


CHIP_ERROR LoadKeypairFromRaw(Crypto::P256Keypair & keypair)
{
    Crypto::ECC608_Set_Slot(0); /* always use slot 0 for key attestation */
    keypair.Initialize(Crypto::ECPKeyTarget::ECDSA);

    return CHIP_NO_ERROR;
}


CHIP_ERROR FactoryDataProvider::GetDeviceAttestationCert(MutableByteSpan & out_dac_buffer)
{
    size_t tmp_size    = 550;
    int status;

    ChipLogProgress(DeviceLayer, "Reading Device Certificate from ECC chip");

    if (ATCA_SUCCESS != (status = atcacert_read_cert(&g_cert_def_2_device, NULL, device_der_qa, &tmp_size)))
    {
        ChipLogProgress(DeviceLayer, "Failed to read device certificate: %d\r\n", status);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "Success to read device certificate: %d\r\n", status);
    }

    /* Compare the device certificate */

    printf("Device Certificate, size = %d\r\n", tmp_size);
    {
        for (int i = 0; i < tmp_size; i++)
        { 
            printf("%02X ", device_der_qa[i]);
        }
    }

    return CopySpanToMutableSpan(ByteSpan(device_der_qa), out_dac_buffer);
}

CHIP_ERROR FactoryDataProvider::GetProductAttestationIntermediateCert(MutableByteSpan & out_pai_buffer)
{
    return CopySpanToMutableSpan(ByteSpan(AttestationCerts::kPaiCert), out_pai_buffer);                                          
}

CHIP_ERROR FactoryDataProvider::GetCertificationDeclaration(MutableByteSpan & out_cd_buffer)
{
    //-> format_version = 1
    //-> vendor_id = 0xFFF1
    //-> product_id_array = [ 0x8000, 0x8001, 0x8002, 0x8003, 0x8004, 0x8005, 0x8006, 0x8007, 0x8008, 0x8009, 0x800A, 0x800B,
    // 0x800C, 0x800D, 0x800E, 0x800F, 0x8010, 0x8011, 0x8012, 0x8013, 0x8014, 0x8015, 0x8016, 0x8017, 0x8018, 0x8019, 0x801A,
    // 0x801B, 0x801C, 0x801D, 0x801E, 0x801F, 0x8020, 0x8021, 0x8022, 0x8023, 0x8024, 0x8025, 0x8026, 0x8027, 0x8028, 0x8029,
    // 0x802A, 0x802B, 0x802C, 0x802D, 0x802E, 0x802F, 0x8030, 0x8031, 0x8032, 0x8033, 0x8034, 0x8035, 0x8036, 0x8037, 0x8038,
    // 0x8039, 0x803A, 0x803B, 0x803C, 0x803D, 0x803E, 0x803F, 0x8040, 0x8041, 0x8042, 0x8043, 0x8044, 0x8045, 0x8046, 0x8047,
    // 0x8048, 0x8049, 0x804A, 0x804B, 0x804C, 0x804D, 0x804E, 0x804F, 0x8050, 0x8051, 0x8052, 0x8053, 0x8054, 0x8055, 0x8056,
    // 0x8057, 0x8058, 0x8059, 0x805A, 0x805B, 0x805C, 0x805D, 0x805E, 0x805F, 0x8060, 0x8061, 0x8062, 0x8063 ]
    //-> device_type_id = 0x0016
    //-> certificate_id = "ZIG20142ZB330003-24"
    //-> security_level = 0
    //-> security_information = 0
    //-> version_number = 0x2694
    //-> certification_type = 0
    //-> dac_origin_vendor_id is not present
    //-> dac_origin_product_id is not present
    const uint8_t kCdForAllExamples[541] = {
        0x30, 0x82, 0x02, 0x19, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x02, 0xa0, 0x82, 0x02, 0x0a, 0x30,
        0x82, 0x02, 0x06, 0x02, 0x01, 0x03, 0x31, 0x0d, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02,
        0x01, 0x30, 0x82, 0x01, 0x71, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x07, 0x01, 0xa0, 0x82, 0x01, 0x62,
        0x04, 0x82, 0x01, 0x5e, 0x15, 0x24, 0x00, 0x01, 0x25, 0x01, 0xf1, 0xff, 0x36, 0x02, 0x05, 0x00, 0x80, 0x05, 0x01, 0x80,
        0x05, 0x02, 0x80, 0x05, 0x03, 0x80, 0x05, 0x04, 0x80, 0x05, 0x05, 0x80, 0x05, 0x06, 0x80, 0x05, 0x07, 0x80, 0x05, 0x08,
        0x80, 0x05, 0x09, 0x80, 0x05, 0x0a, 0x80, 0x05, 0x0b, 0x80, 0x05, 0x0c, 0x80, 0x05, 0x0d, 0x80, 0x05, 0x0e, 0x80, 0x05,
        0x0f, 0x80, 0x05, 0x10, 0x80, 0x05, 0x11, 0x80, 0x05, 0x12, 0x80, 0x05, 0x13, 0x80, 0x05, 0x14, 0x80, 0x05, 0x15, 0x80,
        0x05, 0x16, 0x80, 0x05, 0x17, 0x80, 0x05, 0x18, 0x80, 0x05, 0x19, 0x80, 0x05, 0x1a, 0x80, 0x05, 0x1b, 0x80, 0x05, 0x1c,
        0x80, 0x05, 0x1d, 0x80, 0x05, 0x1e, 0x80, 0x05, 0x1f, 0x80, 0x05, 0x20, 0x80, 0x05, 0x21, 0x80, 0x05, 0x22, 0x80, 0x05,
        0x23, 0x80, 0x05, 0x24, 0x80, 0x05, 0x25, 0x80, 0x05, 0x26, 0x80, 0x05, 0x27, 0x80, 0x05, 0x28, 0x80, 0x05, 0x29, 0x80,
        0x05, 0x2a, 0x80, 0x05, 0x2b, 0x80, 0x05, 0x2c, 0x80, 0x05, 0x2d, 0x80, 0x05, 0x2e, 0x80, 0x05, 0x2f, 0x80, 0x05, 0x30,
        0x80, 0x05, 0x31, 0x80, 0x05, 0x32, 0x80, 0x05, 0x33, 0x80, 0x05, 0x34, 0x80, 0x05, 0x35, 0x80, 0x05, 0x36, 0x80, 0x05,
        0x37, 0x80, 0x05, 0x38, 0x80, 0x05, 0x39, 0x80, 0x05, 0x3a, 0x80, 0x05, 0x3b, 0x80, 0x05, 0x3c, 0x80, 0x05, 0x3d, 0x80,
        0x05, 0x3e, 0x80, 0x05, 0x3f, 0x80, 0x05, 0x40, 0x80, 0x05, 0x41, 0x80, 0x05, 0x42, 0x80, 0x05, 0x43, 0x80, 0x05, 0x44,
        0x80, 0x05, 0x45, 0x80, 0x05, 0x46, 0x80, 0x05, 0x47, 0x80, 0x05, 0x48, 0x80, 0x05, 0x49, 0x80, 0x05, 0x4a, 0x80, 0x05,
        0x4b, 0x80, 0x05, 0x4c, 0x80, 0x05, 0x4d, 0x80, 0x05, 0x4e, 0x80, 0x05, 0x4f, 0x80, 0x05, 0x50, 0x80, 0x05, 0x51, 0x80,
        0x05, 0x52, 0x80, 0x05, 0x53, 0x80, 0x05, 0x54, 0x80, 0x05, 0x55, 0x80, 0x05, 0x56, 0x80, 0x05, 0x57, 0x80, 0x05, 0x58,
        0x80, 0x05, 0x59, 0x80, 0x05, 0x5a, 0x80, 0x05, 0x5b, 0x80, 0x05, 0x5c, 0x80, 0x05, 0x5d, 0x80, 0x05, 0x5e, 0x80, 0x05,
        0x5f, 0x80, 0x05, 0x60, 0x80, 0x05, 0x61, 0x80, 0x05, 0x62, 0x80, 0x05, 0x63, 0x80, 0x18, 0x24, 0x03, 0x16, 0x2c, 0x04,
        0x13, 0x5a, 0x49, 0x47, 0x32, 0x30, 0x31, 0x34, 0x32, 0x5a, 0x42, 0x33, 0x33, 0x30, 0x30, 0x30, 0x33, 0x2d, 0x32, 0x34,
        0x24, 0x05, 0x00, 0x24, 0x06, 0x00, 0x25, 0x07, 0x94, 0x26, 0x24, 0x08, 0x00, 0x18, 0x31, 0x7d, 0x30, 0x7b, 0x02, 0x01,
        0x03, 0x80, 0x14, 0x62, 0xfa, 0x82, 0x33, 0x59, 0xac, 0xfa, 0xa9, 0x96, 0x3e, 0x1c, 0xfa, 0x14, 0x0a, 0xdd, 0xf5, 0x04,
        0xf3, 0x71, 0x60, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x30, 0x0a, 0x06, 0x08,
        0x2a, 0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x04, 0x47, 0x30, 0x45, 0x02, 0x20, 0x24, 0xe5, 0xd1, 0xf4, 0x7a, 0x7d,
        0x7b, 0x0d, 0x20, 0x6a, 0x26, 0xef, 0x69, 0x9b, 0x7c, 0x97, 0x57, 0xb7, 0x2d, 0x46, 0x90, 0x89, 0xde, 0x31, 0x92, 0xe6,
        0x78, 0xc7, 0x45, 0xe7, 0xf6, 0x0c, 0x02, 0x21, 0x00, 0xf8, 0xaa, 0x2f, 0xa7, 0x11, 0xfc, 0xb7, 0x9b, 0x97, 0xe3, 0x97,
        0xce, 0xda, 0x66, 0x7b, 0xae, 0x46, 0x4e, 0x2b, 0xd3, 0xff, 0xdf, 0xc3, 0xcc, 0xed, 0x7a, 0xa8, 0xca, 0x5f, 0x4c, 0x1a,
        0x7c,
    };

    return CopySpanToMutableSpan(ByteSpan{ kCdForAllExamples }, out_cd_buffer);
}

CHIP_ERROR FactoryDataProvider::GetFirmwareInformation(MutableByteSpan & out_firmware_info_buffer)
{
    // TODO: We need a real example FirmwareInformation to be populated.
    out_firmware_info_buffer.reduce_size(0);

    return CHIP_NO_ERROR;
}

CHIP_ERROR FactoryDataProvider::SignWithDeviceAttestationKey(const ByteSpan & message_to_sign,
                                                            MutableByteSpan & out_signature_buffer)
{
    Crypto::P256ECDSASignature signature;
    //Crypto::P256Keypair keypair;

    ChipLogProgress(DeviceLayer, "SignWithDeviceAttestationKey In");
    VerifyOrReturnError(IsSpanUsable(out_signature_buffer), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(IsSpanUsable(message_to_sign), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_signature_buffer.size() >= signature.Capacity(), CHIP_ERROR_BUFFER_TOO_SMALL);

    if (!dac_key_is_loaded)
    {
        ReturnErrorOnFailure(LoadKeypairFromRaw(mdac_key_pair));
        dac_key_is_loaded = 1;
    }
    ReturnErrorOnFailure(mdac_key_pair.ECDSA_sign_msg(message_to_sign.data(), message_to_sign.size(), signature));

    return CopySpanToMutableSpan(ByteSpan{ signature.ConstBytes(), signature.Length() }, out_signature_buffer);
}





} // namespace chip
} // namespace DeviceLayer
