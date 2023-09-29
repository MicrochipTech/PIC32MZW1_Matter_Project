/**
 * \brief Replace mbedTLS ECDH Functions with hardware acceleration &
 * hardware key security.
 *
 * \copyright (c) 2015-2020 Microchip Technology Inc. and its subsidiaries.
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip software
 * and any derivatives exclusively with Microchip products. It is your
 * responsibility to comply with third party license terms applicable to your
 * use of third party software (including open source software) that may
 * accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
 * PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT,
 * SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE
 * OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF
 * MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
 * FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL
 * LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED
 * THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR
 * THIS SOFTWARE.
 */

/* mbedTLS boilerplate includes */

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_ECDH_C)

#include "mbedtls/ecdh.h"
#include "mbedtls/platform_util.h"

/* Cryptoauthlib Includes */
#include "cryptoauthlib.h"
#include "atca_basic.h"
#include "atca_mbedtls_wrap.h"
#include <string.h>

uint8_t atca_io_protection_key[32] = {
    0x37, 0x80, 0xe6, 0x3d, 0x49, 0x68, 0xad, 0xe5,
    0xd8, 0x22, 0xc0, 0x13, 0xfc, 0xc3, 0x23, 0x84,
    0x5d, 0x1b, 0x56, 0x9f, 0xe7, 0x05, 0xb6, 0x00,
    0x06, 0xfe, 0xec, 0x14, 0x5a, 0x0d, 0xb1, 0xe3
};

int atca_mbedtls_ecdh_ioprot_cb(uint8_t secret[32])
{
    memcpy(secret, atca_io_protection_key, 32);
    return 0;
}

#ifdef MBEDTLS_ECDH_GEN_PUBLIC_ALT
/** Generate ECDH keypair */
int mbedtls_ecdh_gen_public(mbedtls_ecp_group *grp, mbedtls_mpi *d, mbedtls_ecp_point *Q,
                            int (*f_rng)(void *, unsigned char *, size_t),
                            void *p_rng)
{
    printf("[%s] In\r\n",__func__);
    SYS_CONSOLE_PRINT("Debug: atca: mbedtls_ecdh_gen_public in\r\n");
    int ret = 0;
    uint8_t public_key[ATCA_PUB_KEY_SIZE];
    uint8_t temp = 1;
    uint16_t slotid = atca_mbedtls_ecdh_slot_cb();

    if (!grp || !d || !Q)
    {
        ret = MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
    }

    if (grp->id != MBEDTLS_ECP_DP_SECP256R1)
    {
        ret = MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
    }

    if (!ret)
    {
        ret = mbedtls_mpi_lset(d, slotid);
    }

    if (!ret)
    {
        ret = atcab_genkey(slotid, public_key);
    }

    if (!ret)
    {
        ret = mbedtls_mpi_read_binary(&(Q->X), public_key, ATCA_PUB_KEY_SIZE / 2);
    }

    if (!ret)
    {
        ret = mbedtls_mpi_read_binary(&(Q->Y), &public_key[ATCA_PUB_KEY_SIZE / 2], ATCA_PUB_KEY_SIZE / 2);
    }

    if (!ret)
    {
        ret = mbedtls_mpi_read_binary(&(Q->Z), &temp, 1);
    }

    return ret;
}
#endif /* MBEDTLS_ECDH_GEN_PUBLIC_ALT */

#ifdef MBEDTLS_ECDH_COMPUTE_SHARED_ALT
/*
 * Compute shared secret (SEC1 3.3.1)
 */
int mbedtls_ecdh_compute_shared(mbedtls_ecp_group *grp, mbedtls_mpi *z,
                                const mbedtls_ecp_point *Q, const mbedtls_mpi *d,
                                int (*f_rng)(void *, unsigned char *, size_t),
                                void *p_rng)
{
    printf("[%s] In\r\n",__func__);
    printf("Debug: atca: mbedtls_ecdh_compute_shared in\r\n");

    int ret = 0;
    uint8_t public_key[ATCA_PUB_KEY_SIZE];
    uint8_t shared_key[ATCA_KEY_SIZE];
    uint16_t slotid;
    uint8_t secret[ATCA_KEY_SIZE];
    printf("Debug: atca: mbedtls_ecdh_compute_shared log1\r\n");
    if (!grp || !z || !Q || !d)
    {
        ret = MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
    }
    printf("Debug: atca: mbedtls_ecdh_compute_shared log2\r\n");
    if (grp->id != MBEDTLS_ECP_DP_SECP256R1)
    {
        ret = MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
    }
    printf("Debug: atca: mbedtls_ecdh_compute_shared log3\r\n");
    if (!ret)
    {
        ret = mbedtls_mpi_write_binary(&(Q->X), public_key, ATCA_PUB_KEY_SIZE / 2);
    }
    printf("Debug: atca: mbedtls_ecdh_compute_shared log4\r\n");
    if (!ret)
    {
        ret = mbedtls_mpi_write_binary(&(Q->Y), &public_key[ATCA_PUB_KEY_SIZE / 2], ATCA_PUB_KEY_SIZE / 2);
    }
    printf("Debug: atca: mbedtls_ecdh_compute_shared log5\r\n");
    if (!ret)
    {
        //slotid = *(uint16_t*)d->p;
        atca_mbedtls_eckey_t key_info;
        ret = mbedtls_mpi_write_binary(d, (unsigned char *) &key_info, sizeof(atca_mbedtls_eckey_t));
        slotid = key_info.handle;
        printf("Debug: atca: mbedtls_ecdh_compute_shared log51, p = %d, n = %d, s = %d\r\n", *(uint16_t*)d->p, d->n, d->s);
        printf("Debug: atca: mbedtls_ecdh_compute_shared log51, slotid = %d\r\n", slotid);
        if (ATECC608 == atcab_get_device_type())
        {
            printf("Debug: atca: mbedtls_ecdh_compute_shared log6\r\n");
            ret = atca_mbedtls_ecdh_ioprot_cb(secret);
            if (!ret)
            {   
                printf("Debug: atca: mbedtls_ecdh_compute_shared log61, slotid = %d\r\n", slotid);
                if (slotid > 15)
                {
                    ret = atcab_ecdh_tempkey_ioenc(public_key, shared_key, secret);
                }
                else
                {
                    ret = atcab_ecdh_ioenc(slotid, public_key, shared_key, secret);
                }
            }
            printf("Debug: atca: mbedtls_ecdh_compute_shared log7\r\n");
            mbedtls_platform_zeroize(secret, ATCA_KEY_SIZE);
        }
        else
        {
            printf("Debug: atca: mbedtls_ecdh_compute_shared log8\r\n");
            ret = atcab_ecdh(slotid, public_key, shared_key);
        }
    }

    if (!ret)
    {   
        printf("Debug: atca: mbedtls_ecdh_compute_shared log90\r\n");
        ret = mbedtls_mpi_read_binary(z, shared_key, ATCA_KEY_SIZE);
    }
    printf("Debug: atca: mbedtls_ecdh_compute_shared log9, ret=%d\r\n",ret);
    return ret;
}
#endif /* MBEDTLS_ECDH_COMPUTE_SHARED_ALT */

#endif /* MBEDTLS_ECDH_C */
