/*******************************************************************************
  System Configuration Header

  File Name:
    configuration.h

  Summary:
    Build-time configuration header for the system defined by this project.

  Description:
    An MPLAB Project may have multiple configurations.  This file defines the
    build-time options for a single configuration.

  Remarks:
    This configuration header must not define any prototypes or data
    definitions (or include any files that do).  It only provides macro
    definitions for build-time configuration options

*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
 * Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
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
 * PARTICULAR PURPOSE.
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
 * ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *******************************************************************************/
// DOM-IGNORE-END

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
/*  This section Includes other configuration headers necessary to completely
    define this configuration.
*/

#include "user.h"
#include "device.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: System Configuration
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// Section: System Service Configuration
// *****************************************************************************
// *****************************************************************************

#define SYS_DEBUG_ENABLE
#define SYS_DEBUG_GLOBAL_ERROR_LEVEL SYS_ERROR_DEBUG
#define SYS_DEBUG_BUFFER_DMA_READY
#define SYS_DEBUG_USE_CONSOLE

/* TIME System Service Configuration Options */
#define SYS_TIME_INDEX_0 (0)
#define SYS_TIME_MAX_TIMERS (5)
#define SYS_TIME_HW_COUNTER_WIDTH (32)
#define SYS_TIME_HW_COUNTER_PERIOD (4294967295U)
#define SYS_TIME_HW_COUNTER_HALF_PERIOD (SYS_TIME_HW_COUNTER_PERIOD >> 1)
#define SYS_TIME_CPU_CLOCK_FREQUENCY (200000000)
#define SYS_TIME_COMPARE_UPDATE_EXECUTION_CYCLES (620)

#define SYS_CONSOLE_DEVICE_MAX_INSTANCES 1
#define SYS_CONSOLE_UART_MAX_INSTANCES 1
#define SYS_CONSOLE_USB_CDC_MAX_INSTANCES 0
#define SYS_CONSOLE_PRINT_BUFFER_SIZE 512

#define SYS_CONSOLE_INDEX_0 0

// *****************************************************************************
// *****************************************************************************
// Section: Driver Configuration
// *****************************************************************************
// *****************************************************************************
/*** WiFi PIC32MZW1 Driver Configuration ***/
#define WDRV_PIC32MZW1_DEVICE_USE_SYS_DEBUG
#define WDRV_PIC32MZW_WPA3_SUPPORT
#define WDRV_PIC32MZW_BA414E_SUPPORT
#define WDRV_PIC32MZW_ALARM_PERIOD_1MS 390
#define WDRV_PIC32MZW_ALARM_PERIOD_MAX 168

// *****************************************************************************
// *****************************************************************************
// Section: Middleware & Other Library Configuration
// *****************************************************************************
// *****************************************************************************

/* MPLAB Harmony BA414E Driver Definitions*/
#define DRV_BA414E_NUM_CLIENTS 5

/* Net Pres RTOS Configurations*/
#define DRV_BA414E_RTOS_STACK_SIZE 1024
#define DRV_BA414E_RTOS_TASK_PRIORITY 1

/*** wolfCrypt Library Configuration ***/
#define MICROCHIP_PIC32
#define MICROCHIP_MPLAB_HARMONY
#define MICROCHIP_MPLAB_HARMONY_3
#define HAVE_MCAPI
#define SIZEOF_LONG_LONG 8
#define WOLFSSL_USER_IO
#define NO_WRITEV
#define NO_FILESYSTEM
#define USE_FAST_MATH
#define NO_PWDBASED
#define HAVE_MCAPI
#define WOLF_CRYPTO_CB // provide call-back support
#define WOLFCRYPT_ONLY
#define WOLFSSL_MICROCHIP_PIC32MZ
// ---------- CRYPTO HARDWARE MANIFEST START ----------
#define WOLFSSL_HAVE_MCHP_HW_CRYPTO_ECC_HW_BA414E
#define WOLFSSL_HAVE_MCHP_BA414E_CRYPTO
// ---------- CRYPTO HARDWARE MANIFEST END ----------
// ---------- FUNCTIONAL CONFIGURATION START ----------
#define WOLFSSL_AES_SMALL_TABLES
#define NO_MD4
#define WOLFSSL_SHA224
#define NO_PIC32MZ_HASH
#define WOLFSSL_AES_128
#define WOLFSSL_AES_192
#define WOLFSSL_AES_256
#define WOLFSSL_AES_DIRECT
#define HAVE_AES_DECRYPT
#define HAVE_AES_ECB
#define HAVE_AES_CBC
#define WOLFSSL_AES_COUNTER
#define WOLFSSL_AES_OFB
#define HAVE_AESGCM
#define HAVE_AESCCM
#define NO_RC4
#define NO_HC128
#define NO_RABBIT
#define HAVE_ECC
#define NO_DH
#define NO_DSA
#define FP_MAX_BITS 4096
#define USE_CERT_BUFFERS_2048
#define NO_DEV_RANDOM
#define HAVE_HASHDRBG
#define WC_NO_HARDEN
#define SINGLE_THREADED
#define NO_SIG_WRAPPER
#define NO_ERROR_STRINGS
#define NO_WOLFSSL_MEMORY
// ---------- FUNCTIONAL CONFIGURATION END ----------

/* Network Configuration Index 0 */
#define TCPIP_NETWORK_DEFAULT_INTERFACE_NAME_IDX0 "PIC32MZW1"
#define TCPIP_IF_PIC32MZW1

#define TCPIP_NETWORK_DEFAULT_HOST_NAME_IDX0 "MCHPBOARD_W"
#define TCPIP_NETWORK_DEFAULT_MAC_ADDR_IDX0 0

#define TCPIP_NETWORK_DEFAULT_IP_ADDRESS_IDX0 "0.0.0.0"
#define TCPIP_NETWORK_DEFAULT_IP_MASK_IDX0 "255.255.255.0"
#define TCPIP_NETWORK_DEFAULT_GATEWAY_IDX0 "192.168.1.1"
#define TCPIP_NETWORK_DEFAULT_DNS_IDX0 "192.168.1.1"
#define TCPIP_NETWORK_DEFAULT_SECOND_DNS_IDX0 "0.0.0.0"
#define TCPIP_NETWORK_DEFAULT_POWER_MODE_IDX0 "full"
#define TCPIP_NETWORK_DEFAULT_INTERFACE_FLAGS_IDX0                                                                                 \
    TCPIP_NETWORK_CONFIG_DHCP_CLIENT_ON | TCPIP_NETWORK_CONFIG_DNS_CLIENT_ON | TCPIP_NETWORK_CONFIG_IP_STATIC

#define TCPIP_NETWORK_DEFAULT_MAC_DRIVER_IDX0 WDRV_PIC32MZW1_MACObject

/*** TCPIP Heap Configuration ***/
#define TCPIP_STACK_USE_EXTERNAL_HEAP

#define TCPIP_STACK_MALLOC_FUNC malloc

#define TCPIP_STACK_CALLOC_FUNC calloc

#define TCPIP_STACK_FREE_FUNC free

#define TCPIP_STACK_HEAP_USE_FLAGS TCPIP_STACK_HEAP_FLAG_ALLOC_UNCACHED

#define TCPIP_STACK_HEAP_USAGE_CONFIG TCPIP_STACK_HEAP_USE_DEFAULT

#define TCPIP_STACK_SUPPORTED_HEAPS 1

// *****************************************************************************
// *****************************************************************************
// Section: TCPIP Stack Configuration
// *****************************************************************************
// *****************************************************************************

#define TCPIP_STACK_TICK_RATE 5
#define TCPIP_STACK_SECURE_PORT_ENTRIES 10

#define TCPIP_STACK_ALIAS_INTERFACE_SUPPORT false

#define TCPIP_PACKET_LOG_ENABLE 0

/* TCP/IP stack event notification */
#define TCPIP_STACK_USE_EVENT_NOTIFICATION
#define TCPIP_STACK_USER_NOTIFICATION true
#define TCPIP_STACK_DOWN_OPERATION true
#define TCPIP_STACK_IF_UP_DOWN_OPERATION true
#define TCPIP_STACK_MAC_DOWN_OPERATION true
#define TCPIP_STACK_INTERFACE_CHANGE_SIGNALING false
#define TCPIP_STACK_CONFIGURATION_SAVE_RESTORE true
#define TCPIP_STACK_EXTERN_PACKET_PROCESS true

/* TCP/IP RTOS Configurations*/
#define TCPIP_RTOS_STACK_SIZE 1024
#define TCPIP_RTOS_PRIORITY 1

// *****************************************************************************
// *****************************************************************************
// Section: Application Configuration
// *****************************************************************************
// *****************************************************************************

// DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
// DOM-IGNORE-END

#endif // CONFIGURATION_H
/*******************************************************************************
 End of File
*/
