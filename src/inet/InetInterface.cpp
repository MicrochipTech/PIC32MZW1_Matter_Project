/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
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
 *      Implementation of network interface abstraction layer.
 *
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include "InetInterface.h"

#include "InetLayer.h"
#include "InetLayerEvents.h"

#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>

#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <lwip/netif.h>
#include <lwip/sys.h>
#include <lwip/tcpip.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif /* HAVE_SYS_SOCKIO_H */
#include <net/if.h>
#include <sys/ioctl.h>
#ifdef __ANDROID__
#include "ifaddrs-android.h"
#else // !defined(__ANDROID__)
#include <ifaddrs.h>
#endif // !defined(__ANDROID__)
#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
#include <net/net_if.h>
#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

#include <stdio.h>
#include <string.h>

namespace chip {
namespace Inet {

#if CHIP_SYSTEM_CONFIG_USE_LWIP

DLL_EXPORT CHIP_ERROR GetInterfaceName(InterfaceId intfId, char * nameBuf, size_t nameBufSize)
{
    if (intfId != INET_NULL_INTERFACEID)
    {
        int status = snprintf(nameBuf, nameBufSize, "%c%c%d", intfId->name[0], intfId->name[1], intfId->num);
        if (status >= static_cast<int>(nameBufSize))
            return CHIP_ERROR_NO_MEMORY;
        return CHIP_NO_ERROR;
    }

    if (nameBufSize < 1)
        return CHIP_ERROR_NO_MEMORY;

    nameBuf[0] = 0;
    return CHIP_NO_ERROR;
}

DLL_EXPORT CHIP_ERROR InterfaceNameToId(const char * intfName, InterfaceId & intfId)
{
    if (strlen(intfName) < 3)
        return INET_ERROR_UNKNOWN_INTERFACE;
    char * parseEnd;
    unsigned long intfNum = strtoul(intfName + 2, &parseEnd, 10);
    if (*parseEnd != 0 || intfNum > UINT8_MAX)
        return INET_ERROR_UNKNOWN_INTERFACE;
    struct netif * intf;
#if LWIP_VERSION_MAJOR >= 2 && LWIP_VERSION_MINOR >= 0 && defined(NETIF_FOREACH)
    NETIF_FOREACH(intf)
#else
    for (intf = netif_list; intf != NULL; intf = intf->next)
#endif
    {
        if (intf->name[0] == intfName[0] && intf->name[1] == intfName[1] && intf->num == (uint8_t) intfNum)
        {
            intfId = intf;
            return CHIP_NO_ERROR;
        }
    }
    intfId = INET_NULL_INTERFACEID;
    return INET_ERROR_UNKNOWN_INTERFACE;
}

bool InterfaceIterator::Next()
{

    // Lock LwIP stack
    LOCK_TCPIP_CORE();

    // Verify the previous netif is still on the list if netifs.  If so,
    // advance to the next nextif.
    struct netif * prevNetif = mCurNetif;
#if LWIP_VERSION_MAJOR >= 2 && LWIP_VERSION_MINOR >= 0 && defined(NETIF_FOREACH)
    NETIF_FOREACH(mCurNetif)
#else
    for (mCurNetif = netif_list; mCurNetif != NULL; mCurNetif = mCurNetif->next)
#endif
    {
        if (mCurNetif == prevNetif)
        {
            mCurNetif = mCurNetif->next;
            break;
        }
    }

    // Unlock LwIP stack
    UNLOCK_TCPIP_CORE();

    return mCurNetif != NULL;
}

CHIP_ERROR InterfaceIterator::GetInterfaceName(char * nameBuf, size_t nameBufSize)
{
    VerifyOrReturnError(HasCurrent(), CHIP_ERROR_INCORRECT_STATE);
    return ::chip::Inet::GetInterfaceName(mCurNetif, nameBuf, nameBufSize);
}

bool InterfaceIterator::IsUp()
{
    return HasCurrent() && netif_is_up(mCurNetif);
}

bool InterfaceIterator::SupportsMulticast()
{
    return HasCurrent() &&
#if LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
        (mCurNetif->flags & (NETIF_FLAG_IGMP | NETIF_FLAG_MLD6 | NETIF_FLAG_BROADCAST)) != 0;
#else
        (mCurNetif->flags & NETIF_FLAG_POINTTOPOINT) == 0;
#endif // LWIP_VERSION_MAJOR > 1 || LWIP_VERSION_MINOR >= 5
}

bool InterfaceIterator::HasBroadcastAddress()
{
    return HasCurrent() && (mCurNetif->flags & NETIF_FLAG_BROADCAST) != 0;
}

bool InterfaceAddressIterator::HasCurrent()
{
    return mIntfIter.HasCurrent() && ((mCurAddrIndex != kBeforeStartIndex) || Next());
}

bool InterfaceAddressIterator::Next()
{
    mCurAddrIndex++;

    while (mIntfIter.HasCurrent())
    {
        struct netif * curIntf = mIntfIter.GetInterfaceId();

        while (mCurAddrIndex < LWIP_IPV6_NUM_ADDRESSES)
        {
            if (ip6_addr_isvalid(netif_ip6_addr_state(curIntf, mCurAddrIndex)))
            {
                return true;
            }
            mCurAddrIndex++;
        }

#if INET_CONFIG_ENABLE_IPV4 && LWIP_IPV4
        if (mCurAddrIndex == LWIP_IPV6_NUM_ADDRESSES)
        {
            if (!ip4_addr_isany(netif_ip4_addr(curIntf)))
            {
                return true;
            }
        }
#endif // INET_CONFIG_ENABLE_IPV4 && LWIP_IPV4

        mIntfIter.Next();
        mCurAddrIndex = 0;
    }

    return false;
}

IPAddress InterfaceAddressIterator::GetAddress()
{
    if (HasCurrent())
    {
        struct netif * curIntf = mIntfIter.GetInterfaceId();

        if (mCurAddrIndex < LWIP_IPV6_NUM_ADDRESSES)
        {
            return IPAddress::FromIPv6(*netif_ip6_addr(curIntf, mCurAddrIndex));
        }
#if INET_CONFIG_ENABLE_IPV4 && LWIP_IPV4
        else
        {
            return IPAddress::FromIPv4(*netif_ip4_addr(curIntf));
        }
#endif // INET_CONFIG_ENABLE_IPV4 && LWIP_IPV4
    }

    return IPAddress::Any;
}

uint8_t InterfaceAddressIterator::GetPrefixLength()
{
    if (HasCurrent())
    {
        if (mCurAddrIndex < LWIP_IPV6_NUM_ADDRESSES)
        {
            return 64;
        }
#if INET_CONFIG_ENABLE_IPV4 && LWIP_IPV4
        else
        {
            struct netif * curIntf = mIntfIter.GetInterfaceId();
            return NetmaskToPrefixLength((const uint8_t *) netif_ip4_netmask(curIntf), 4);
        }
#endif // INET_CONFIG_ENABLE_IPV4 && LWIP_IPV4
    }
    return 0;
}

InterfaceId InterfaceAddressIterator::GetInterfaceId()
{
    if (HasCurrent())
    {
        return mIntfIter.GetInterfaceId();
    }
    return INET_NULL_INTERFACEID;
}

CHIP_ERROR InterfaceAddressIterator::GetInterfaceName(char * nameBuf, size_t nameBufSize)
{
    VerifyOrReturnError(HasCurrent(), CHIP_ERROR_INCORRECT_STATE);
    return mIntfIter.GetInterfaceName(nameBuf, nameBufSize);
}

bool InterfaceAddressIterator::IsUp()
{
    if (HasCurrent())
    {
        return mIntfIter.IsUp();
    }
    return false;
}

bool InterfaceAddressIterator::SupportsMulticast()
{
    if (HasCurrent())
    {
        return mIntfIter.SupportsMulticast();
    }
    return false;
}

bool InterfaceAddressIterator::HasBroadcastAddress()
{
    if (HasCurrent())
    {
        return mIntfIter.HasBroadcastAddress();
    }
    return false;
}

#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

DLL_EXPORT CHIP_ERROR GetInterfaceName(InterfaceId intfId, char * nameBuf, size_t nameBufSize)
{
    if (intfId != INET_NULL_INTERFACEID)
    {
        char intfName[IF_NAMESIZE];
        if (if_indextoname(intfId, intfName) == nullptr)
            return CHIP_ERROR_POSIX(errno);
        if (strlen(intfName) >= nameBufSize)
            return CHIP_ERROR_NO_MEMORY;
        strcpy(nameBuf, intfName);
        return CHIP_NO_ERROR;
    }

    if (nameBufSize < 1)
        return CHIP_ERROR_NO_MEMORY;

    nameBuf[0] = 0;
    return CHIP_NO_ERROR;
}

DLL_EXPORT CHIP_ERROR InterfaceNameToId(const char * intfName, InterfaceId & intfId)
{
    intfId = if_nametoindex(intfName);
    if (intfId == 0)
        return (errno == ENXIO) ? INET_ERROR_UNKNOWN_INTERFACE : CHIP_ERROR_POSIX(errno);
    return CHIP_NO_ERROR;
}

static int sIOCTLSocket = -1;

/**
 * @brief   Returns a global general purpose socket useful for invoking certain network IOCTLs.
 *
 * This function is thread-safe on all platforms.
 */
int GetIOCTLSocket()
{
    if (sIOCTLSocket == -1)
    {
        int s;
#ifdef SOCK_CLOEXEC
        s = socket(AF_INET, SOCK_STREAM, SOCK_CLOEXEC);
        if (s < 0)
#endif
        {
            s = socket(AF_INET, SOCK_STREAM, 0);
            fcntl(s, O_CLOEXEC);
        }

        if (!__sync_bool_compare_and_swap(&sIOCTLSocket, -1, s))
        {
            close(s);
        }
    }
    return sIOCTLSocket;
}

/**
 * @brief   Close the global socket created by \c GetIOCTLSocket.
 *
 * @details
 *   This function is provided for cases were leaving the global IOCTL socket
 *   open would register as a leak.
 *
 *   NB: This function is NOT thread-safe with respect to \c GetIOCTLSocket.
 */
void CloseIOCTLSocket()
{
    if (sIOCTLSocket != -1)
    {
        close(sIOCTLSocket);
        sIOCTLSocket = -1;
    }
}

#if __ANDROID__ && __ANDROID_API__ < 24

static struct if_nameindex * backport_if_nameindex(void);
static void backport_if_freenameindex(struct if_nameindex *);

static void backport_if_freenameindex(struct if_nameindex * inArray)
{
    if (inArray == NULL)
    {
        return;
    }

    for (size_t i = 0; inArray[i].if_index != 0; i++)
    {
        if (inArray[i].if_name != NULL)
        {
            free(inArray[i].if_name);
        }
    }

    free(inArray);
}

static struct if_nameindex * backport_if_nameindex(void)
{
    int err;
    unsigned index;
    size_t intfIter              = 0;
    size_t maxIntfNum            = 0;
    size_t numIntf               = 0;
    size_t numAddrs              = 0;
    struct if_nameindex * retval = NULL;
    struct if_nameindex * tmpval = NULL;
    struct ifaddrs * addrList    = NULL;
    struct ifaddrs * addrIter    = NULL;
    const char * lastIntfName    = "";

    err = getifaddrs(&addrList);
    VerifyOrExit(err >= 0, );

    // coalesce on consecutive interface names
    for (addrIter = addrList; addrIter != NULL; addrIter = addrIter->ifa_next)
    {
        numAddrs++;
        if (strcmp(addrIter->ifa_name, lastIntfName) == 0)
        {
            continue;
        }
        numIntf++;
        lastIntfName = addrIter->ifa_name;
    }

    tmpval = (struct if_nameindex *) malloc((numIntf + 1) * sizeof(struct if_nameindex));
    VerifyOrExit(tmpval != NULL, );
    memset(tmpval, 0, (numIntf + 1) * sizeof(struct if_nameindex));

    lastIntfName = "";
    for (addrIter = addrList; addrIter != NULL; addrIter = addrIter->ifa_next)
    {
        if (strcmp(addrIter->ifa_name, lastIntfName) == 0)
        {
            continue;
        }

        index = if_nametoindex(addrIter->ifa_name);
        if (index != 0)
        {
            tmpval[intfIter].if_index = index;
            tmpval[intfIter].if_name  = strdup(addrIter->ifa_name);
            intfIter++;
        }
        lastIntfName = addrIter->ifa_name;
    }

    // coalesce on interface index
    maxIntfNum = 0;
    for (size_t i = 0; tmpval[i].if_index != 0; i++)
    {
        if (maxIntfNum < tmpval[i].if_index)
        {
            maxIntfNum = tmpval[i].if_index;
        }
    }

    retval = (struct if_nameindex *) malloc((maxIntfNum + 1) * sizeof(struct if_nameindex));
    VerifyOrExit(retval != NULL, );
    memset(retval, 0, (maxIntfNum + 1) * sizeof(struct if_nameindex));

    for (size_t i = 0; tmpval[i].if_index != 0; i++)
    {
        struct if_nameindex * intf = &tmpval[i];
        if (retval[intf->if_index - 1].if_index == 0)
        {
            retval[intf->if_index - 1] = *intf;
        }
        else
        {
            free(intf->if_name);
            intf->if_index = 0;
            intf->if_name  = 0;
        }
    }

    intfIter = 0;

    // coalesce potential gaps between indeces
    for (size_t i = 0; i < maxIntfNum; i++)
    {
        if (retval[i].if_index != 0)
        {
            retval[intfIter] = retval[i];
            intfIter++;
        }
    }

    for (size_t i = intfIter; i < maxIntfNum; i++)
    {
        retval[i].if_index = 0;
        retval[i].if_name  = NULL;
    }

exit:
    if (tmpval != NULL)
    {
        free(tmpval);
    }

    if (addrList != NULL)
    {
        freeifaddrs(addrList);
    }

    return retval;
}

#endif // __ANDROID__ && __ANDROID_API__ < 24

InterfaceIterator::InterfaceIterator()
{
    mIntfArray       = nullptr;
    mCurIntf         = 0;
    mIntfFlags       = 0;
    mIntfFlagsCached = false;
}

InterfaceIterator::~InterfaceIterator()
{
    if (mIntfArray != nullptr)
    {
#if __ANDROID__ && __ANDROID_API__ < 24
        backport_if_freenameindex(mIntfArray);
#else
        if_freenameindex(mIntfArray);
#endif
        mIntfArray = nullptr;
    }
}

bool InterfaceIterator::HasCurrent()
{
    return (mIntfArray != nullptr) ? mIntfArray[mCurIntf].if_index != 0 : Next();
}

bool InterfaceIterator::Next()
{

    if (mIntfArray == nullptr)
    {
#if __ANDROID__ && __ANDROID_API__ < 24
        mIntfArray = backport_if_nameindex();
#else
        mIntfArray = if_nameindex();
#endif
    }
    else if (mIntfArray[mCurIntf].if_index != 0)
    {
        mCurIntf++;
        mIntfFlags       = 0;
        mIntfFlagsCached = false;
    }
    return (mIntfArray != nullptr && mIntfArray[mCurIntf].if_index != 0);
}

InterfaceId InterfaceIterator::GetInterfaceId()
{
    return (HasCurrent()) ? mIntfArray[mCurIntf].if_index : INET_NULL_INTERFACEID;
}

CHIP_ERROR InterfaceIterator::GetInterfaceName(char * nameBuf, size_t nameBufSize)
{
    VerifyOrReturnError(HasCurrent(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(strlen(mIntfArray[mCurIntf].if_name) < nameBufSize, CHIP_ERROR_NO_MEMORY);
    strncpy(nameBuf, mIntfArray[mCurIntf].if_name, nameBufSize);
    return CHIP_NO_ERROR;
}

bool InterfaceIterator::IsUp()
{
    return (GetFlags() & IFF_UP) != 0;
}

bool InterfaceIterator::SupportsMulticast()
{
    return (GetFlags() & IFF_MULTICAST) != 0;
}

bool InterfaceIterator::HasBroadcastAddress()
{
    return (GetFlags() & IFF_BROADCAST) != 0;
}

short InterfaceIterator::GetFlags()
{
    struct ifreq intfData;

    if (!mIntfFlagsCached && HasCurrent())
    {
        strncpy(intfData.ifr_name, mIntfArray[mCurIntf].if_name, IFNAMSIZ);
        intfData.ifr_name[IFNAMSIZ - 1] = '\0';

        int res = ioctl(GetIOCTLSocket(), SIOCGIFFLAGS, &intfData);
        if (res == 0)
        {
            mIntfFlags       = intfData.ifr_flags;
            mIntfFlagsCached = true;
        }
        CloseIOCTLSocket();
    }

    return mIntfFlags;
}

InterfaceAddressIterator::InterfaceAddressIterator()
{
    mAddrsList = nullptr;
    mCurAddr   = nullptr;
}

InterfaceAddressIterator::~InterfaceAddressIterator()
{
    if (mAddrsList != nullptr)
    {
        freeifaddrs(mAddrsList);
        mAddrsList = mCurAddr = nullptr;
    }
}

bool InterfaceAddressIterator::HasCurrent()
{
    return (mAddrsList != nullptr) ? (mCurAddr != nullptr) : Next();
}

bool InterfaceAddressIterator::Next()
{
    while (true)
    {
        if (mAddrsList == nullptr)
        {
            int res = getifaddrs(&mAddrsList);
            if (res < 0)
            {
                return false;
            }
            mCurAddr = mAddrsList;
        }
        else if (mCurAddr != nullptr)
        {
            mCurAddr = mCurAddr->ifa_next;
        }

        if (mCurAddr == nullptr)
        {
            return false;
        }

        if (mCurAddr->ifa_addr != nullptr &&
            (mCurAddr->ifa_addr->sa_family == AF_INET6
#if INET_CONFIG_ENABLE_IPV4
             || mCurAddr->ifa_addr->sa_family == AF_INET
#endif // INET_CONFIG_ENABLE_IPV4
             ))
        {
            return true;
        }
    }
}

IPAddress InterfaceAddressIterator::GetAddress()
{
    if (HasCurrent())
    {
        return IPAddress::FromSockAddr(*mCurAddr->ifa_addr);
    }

    return IPAddress::Any;
}

uint8_t InterfaceAddressIterator::GetPrefixLength()
{
    if (HasCurrent())
    {
        if (mCurAddr->ifa_addr->sa_family == AF_INET6)
        {
#if !__MBED__
            struct sockaddr_in6 & netmask = *reinterpret_cast<struct sockaddr_in6 *>(mCurAddr->ifa_netmask);
            return NetmaskToPrefixLength(netmask.sin6_addr.s6_addr, 16);
#else  // __MBED__
       // netmask is not available through an API for IPv6 interface in Mbed.
       // Default prefix length to 64.
            return 64;
#endif // !__MBED__
        }
        if (mCurAddr->ifa_addr->sa_family == AF_INET)
        {
            struct sockaddr_in & netmask = *reinterpret_cast<struct sockaddr_in *>(mCurAddr->ifa_netmask);
            return NetmaskToPrefixLength(reinterpret_cast<const uint8_t *>(&netmask.sin_addr.s_addr), 4);
        }
    }
    return 0;
}

InterfaceId InterfaceAddressIterator::GetInterfaceId()
{
    if (HasCurrent())
    {
        return if_nametoindex(mCurAddr->ifa_name);
    }
    return INET_NULL_INTERFACEID;
}

CHIP_ERROR InterfaceAddressIterator::GetInterfaceName(char * nameBuf, size_t nameBufSize)
{
    VerifyOrReturnError(HasCurrent(), CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(strlen(mCurAddr->ifa_name) < nameBufSize, CHIP_ERROR_NO_MEMORY);
    strncpy(nameBuf, mCurAddr->ifa_name, nameBufSize);
    return CHIP_NO_ERROR;
}

bool InterfaceAddressIterator::IsUp()
{
    if (HasCurrent())
    {
        return (mCurAddr->ifa_flags & IFF_UP) != 0;
    }
    return false;
}

bool InterfaceAddressIterator::SupportsMulticast()
{
    if (HasCurrent())
    {
        return (mCurAddr->ifa_flags & IFF_MULTICAST) != 0;
    }
    return false;
}

bool InterfaceAddressIterator::HasBroadcastAddress()
{
    if (HasCurrent())
    {
        return (mCurAddr->ifa_flags & IFF_BROADCAST) != 0;
    }
    return false;
}

#endif // CHIP_SYSTEM_CONFIG_USE_SOCKETS && CHIP_SYSTEM_CONFIG_USE_BSD_IFADDRS

#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

DLL_EXPORT CHIP_ERROR GetInterfaceName(InterfaceId intfId, char * nameBuf, size_t nameBufSize)
{
    if (intfId != INET_NULL_INTERFACEID)
    {
        net_if * currentInterface = net_if_get_by_index(intfId);
        if (!currentInterface)
            return CHIP_ERROR_INCORRECT_STATE;
        const char * name = net_if_get_device(currentInterface)->name;
        if (strlen(name) >= nameBufSize)
            return CHIP_ERROR_NO_MEMORY;
        strcpy(nameBuf, name);
        return CHIP_NO_ERROR;
    }

    if (nameBufSize < 1)
        return CHIP_ERROR_NO_MEMORY;

    nameBuf[0] = 0;
    return CHIP_NO_ERROR;
}
DLL_EXPORT CHIP_ERROR InterfaceNameToId(const char * intfName, InterfaceId & intfId)
{
    int currentId = 0;
    net_if * currentInterface;

    while ((currentInterface = net_if_get_by_index(++currentId)) != nullptr)
    {
        if (strcmp(net_if_get_device(currentInterface)->name, intfName) == 0)
        {
            intfId = currentId;
            return CHIP_NO_ERROR;
        }
    }
    return INET_ERROR_UNKNOWN_INTERFACE;
}

InterfaceIterator::InterfaceIterator() : mCurrentInterface(net_if_get_by_index(mCurrentId)) {}

bool InterfaceIterator::HasCurrent(void)
{
    return mCurrentInterface != nullptr;
}

bool InterfaceIterator::Next()
{
    mCurrentInterface = net_if_get_by_index(++mCurrentId);
    return HasCurrent();
}

InterfaceId InterfaceIterator::GetInterfaceId(void)
{
    return HasCurrent() ? mCurrentId : INET_NULL_INTERFACEID;
}

CHIP_ERROR InterfaceIterator::GetInterfaceName(char * nameBuf, size_t nameBufSize)
{
    VerifyOrReturnError(HasCurrent(), CHIP_ERROR_INCORRECT_STATE);
    return ::chip::Inet::GetInterfaceName(mCurrentId, nameBuf, nameBufSize);
}

bool InterfaceIterator::IsUp()
{
    return HasCurrent() && net_if_is_up(mCurrentInterface);
}

bool InterfaceIterator::SupportsMulticast()
{
    return HasCurrent() && NET_IF_MAX_IPV6_MADDR > 0;
}

bool InterfaceIterator::HasBroadcastAddress()
{
    // Zephyr seems to handle broadcast address for IPv4 implicitly
    return HasCurrent() && INET_CONFIG_ENABLE_IPV4;
}

InterfaceAddressIterator::InterfaceAddressIterator() = default;

bool InterfaceAddressIterator::HasCurrent()
{
    return mIntfIter.HasCurrent() && (mCurAddrIndex >= 0 || Next());
}

bool InterfaceAddressIterator::Next()
{
    while (mIntfIter.HasCurrent())
    {
        if (mCurAddrIndex == -1) // first address for the current interface
        {
            const net_if_config * config = net_if_get_config(net_if_get_by_index(mIntfIter.GetInterfaceId()));
            mIpv6                        = config->ip.ipv6;
        }

        while (++mCurAddrIndex < NET_IF_MAX_IPV6_ADDR)
            if (mIpv6->unicast[mCurAddrIndex].is_used)
                return true;

        mCurAddrIndex = -1;
        mIntfIter.Next();
    }

    return false;
}

IPAddress InterfaceAddressIterator::GetAddress()
{
    if (HasCurrent())
    {
        return IPAddress::FromIPv6(mIpv6->unicast[mCurAddrIndex].address.in6_addr);
    }

    return IPAddress::Any;
}

uint8_t InterfaceAddressIterator::GetPrefixLength()
{
    if (HasCurrent())
    {
        net_if * const iface              = net_if_get_by_index(mIntfIter.GetInterfaceId());
        net_if_ipv6_prefix * const prefix = net_if_ipv6_prefix_get(iface, &mIpv6->unicast[mCurAddrIndex].address.in6_addr);
        return prefix ? prefix->len : 128;
    }
    return 0;
}

InterfaceId InterfaceAddressIterator::GetInterfaceId()
{
    if (HasCurrent())
    {
        return mIntfIter.GetInterfaceId();
    }
    return INET_NULL_INTERFACEID;
}

CHIP_ERROR InterfaceAddressIterator::GetInterfaceName(char * nameBuf, size_t nameBufSize)
{
    VerifyOrReturnError(HasCurrent(), CHIP_ERROR_INCORRECT_STATE);
    return mIntfIter.GetInterfaceName(nameBuf, nameBufSize);
}

bool InterfaceAddressIterator::IsUp()
{
    if (HasCurrent())
    {
        return mIntfIter.IsUp();
    }
    return false;
}

bool InterfaceAddressIterator::SupportsMulticast()
{
    if (HasCurrent())
    {
        return mIntfIter.SupportsMulticast();
    }
    return false;
}

bool InterfaceAddressIterator::HasBroadcastAddress()
{
    if (HasCurrent())
    {
        return mIntfIter.HasBroadcastAddress();
    }
    return false;
}

#endif // CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF

void InterfaceAddressIterator::GetAddressWithPrefix(IPPrefix & addrWithPrefix)
{
    if (HasCurrent())
    {
        addrWithPrefix.IPAddr = GetAddress();
        addrWithPrefix.Length = GetPrefixLength();
    }
    else
    {
        addrWithPrefix = IPPrefix::Zero;
    }
}

uint8_t NetmaskToPrefixLength(const uint8_t * netmask, uint16_t netmaskLen)
{
    uint8_t prefixLen = 0;

    for (uint16_t i = 0; i < netmaskLen; i++, prefixLen = static_cast<uint8_t>(prefixLen + 8u))
    {
        uint8_t b = netmask[i];
        if (b != 0xFF)
        {
            if ((b & 0xF0) == 0xF0)
                prefixLen = static_cast<uint8_t>(prefixLen + 4u);
            else
                b = static_cast<uint8_t>(b >> 4);

            if ((b & 0x0C) == 0x0C)
                prefixLen = static_cast<uint8_t>(prefixLen + 2u);
            else
                b = static_cast<uint8_t>(b >> 2);

            if ((b & 0x02) == 0x02)
                prefixLen++;

            break;
        }
    }

    return prefixLen;
}

} // namespace Inet
} // namespace chip
