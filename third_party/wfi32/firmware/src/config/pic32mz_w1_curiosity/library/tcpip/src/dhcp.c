/*******************************************************************************
  Dynamic Host Configuration Protocol (DHCP) Client

  Summary:
    Module for Microchip TCP/IP Stack
    
  Description:
    - Provides automatic IP address, subnet mask, gateway address, 
      DNS server address, and other configuration parameters on DHCP 
      enabled networks.
    - Reference: RFC 2131, 2132
*******************************************************************************/

/*****************************************************************************
 Copyright (C) 2012-2018 Microchip Technology Inc. and its subsidiaries.

Microchip Technology Inc. and its subsidiaries.

Subject to your compliance with these terms, you may use Microchip software 
and any derivatives exclusively with Microchip products. It is your 
responsibility to comply with third party license terms applicable to your 
use of third party software (including open source software) that may 
accompany Microchip software.

THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A PARTICULAR 
PURPOSE.

IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE 
FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN 
ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY, 
THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*****************************************************************************/
#include <ctype.h>
#include "tcpip/src/tcpip_private.h"

// Dummy functions when DHCP Client module is disabled
bool TCPIP_DHCP_IsEnabled(TCPIP_NET_HANDLE hNet)
{
    return false;
}

bool TCPIP_DHCP_IsActive(TCPIP_NET_HANDLE hNet)
{
    return false;
}

bool TCPIP_DHCP_Disable(TCPIP_NET_HANDLE hNet)
{
    return false;
}

bool TCPIP_DHCP_Enable(TCPIP_NET_HANDLE hNet)
{
    return false;
}

bool TCPIP_DHCP_Renew(TCPIP_NET_HANDLE hNet)
{
    return false;
}


bool TCPIP_DHCP_Request(TCPIP_NET_HANDLE hNet, IPV4_ADDR reqAddress)
{
    return false;
}

bool TCPIP_DHCP_InfoGet(TCPIP_NET_HANDLE hNet, TCPIP_DHCP_INFO* pDhcpInfo)
{
    return false;
}




