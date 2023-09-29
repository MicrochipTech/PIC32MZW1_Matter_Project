/*******************************************************************************
 * Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries.
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

/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "network.h"
#include "configuration.h"
#include "definitions.h"
#include "system/debug/sys_debug.h"
#include "tcpip/tcpip.h"
#include "tcpip/tcpip_manager.h"
#include "user.h"
#include "wdrv_pic32mzw_client_api.h"

#include "tcpip/src/tcpip_private.h"

#include "conf_tinyservices.h"

#include <lwip/tcp.h>
#include <lwip/tcpip.h>

// #define PACKET_DEBUG
#define HEAP_DEBUG

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

NETWORK_DATA networkData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
 */

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

#define TCPIP_THIS_MODULE_ID TCPIP_MODULE_NONE
// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

#define APP_WIFI_PROV_WIFI_CONFIG_ID "apply"
#define APP_WIFI_PROV_DONE_ID "finish"

int g_softAP    = 0;
int g_init_flag = 0;

/** The netif for the pic32mzw1 in station mode. */
struct netif pic32mzw1_netif_sta;
struct netif pic32mzw1_netif_ap;

static SemaphoreHandle_t packet_recv_mutex;

DRV_HANDLE wdrvHandle = DRV_HANDLE_INVALID;

bool pktHandler(TCPIP_NET_HANDLE hNet, struct _tag_TCPIP_MAC_PACKET * rxPkt, uint16_t frameType, const void * hParam);

static TCPIP_EVENT_HANDLE TCPIP_event_handle;

static void APP_TcpipStack_EventHandler(TCPIP_NET_HANDLE hNet, TCPIP_EVENT event, const void * fParam)
{
    const char * netName = TCPIP_STACK_NetNameGet(hNet);
    SYS_CONSOLE_PRINT("APP: %s  event = %d\r\n", netName, event);
    if (event & TCPIP_EV_CONN_ESTABLISHED)
    {
        if (TCPIP_DHCP_IsEnabled(hNet) == false)
        {
            TCPIP_DHCP_Enable(hNet);
        }
    }
    else if (event & TCPIP_EV_CONN_LOST)
    {
        TCPIP_DHCP_Disable(hNet);
    }
    else
    {
        SYS_CONSOLE_PRINT("APP: %s Unknown event = %d\r\n", netName, event);
    }
}

void APP_TcpipDhcp_EventHandler(TCPIP_NET_HANDLE hNet, TCPIP_DHCP_EVENT_TYPE evType, const void * param)
{
    switch (evType)
    {
    case DHCP_EVENT_BOUND: {
        TCPIP_DHCP_INFO DhcpInfo;
        if (TCPIP_DHCP_InfoGet(hNet, &DhcpInfo))
        {
            SYS_CONSOLE_PRINT("APP: IP address = %d.%d.%d.%d \r\n", DhcpInfo.dhcpAddress.v[0], DhcpInfo.dhcpAddress.v[1],
                              DhcpInfo.dhcpAddress.v[2], DhcpInfo.dhcpAddress.v[3]);
        }
        break;
    }
    case DHCP_EVENT_CONN_ESTABLISHED: {
        SYS_CONSOLE_MESSAGE("APP: Connection to the DHCP server established\r\n");
        break;
    }
    case DHCP_EVENT_CONN_LOST: {
        SYS_CONSOLE_MESSAGE("APP: Connection to the DHCP server lost\r\n");
        break;
    }
    default:
        break;
    }
}

bool pktHandler(TCPIP_NET_HANDLE hNet, struct _tag_TCPIP_MAC_PACKET * rxPkt, uint16_t frameType, const void * hParam)
{
    struct pbuf * rx_pkt;
    // printf("In 1\r\n");
    xSemaphoreTake(packet_recv_mutex, portMAX_DELAY);
#ifdef PACKET_DEBUG
    SYS_CONSOLE_PRINT("APP: pktHandler In, size = %d\r\n", rxPkt->pDSeg->segLen);
    SYS_CONSOLE_PRINT("data: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\r\n", rxPkt->pDSeg->segLoad[0], rxPkt->pDSeg->segLoad[1],
                      rxPkt->pDSeg->segLoad[2], rxPkt->pDSeg->segLoad[3], rxPkt->pDSeg->segLoad[4], rxPkt->pDSeg->segLoad[5],
                      rxPkt->pDSeg->segLoad[6], rxPkt->pDSeg->segLoad[7]);
#endif
    //rx_pkt = pbuf_alloc(PBUF_RAW, rxPkt->pDSeg->segLen + 14, PBUF_RAM);
    rx_pkt = pbuf_alloc(PBUF_RAW, rxPkt->pDSeg->segLen + 14, PBUF_POOL);

    if (rx_pkt == NULL)
    {
        printf("rx_pkt = NULL\r\n");
        return false;
    }
    memcpy(rx_pkt->payload, rxPkt->pMacLayer, rxPkt->pDSeg->segLen + 14);

    if (!g_softAP)
    {
        pic32mzw1_netif_sta.input(rx_pkt, &pic32mzw1_netif_sta);
    }
    else
    {
        pic32mzw1_netif_ap.input(rx_pkt, &pic32mzw1_netif_ap);
    }
    TCPIP_PKT_PacketAcknowledge(rxPkt, TCPIP_MAC_PKT_ACK_RX_OK);
    // printf("In 2\r\n");
    xSemaphoreGive(packet_recv_mutex);

    return true;
}
TCPIP_NET_HANDLE netH;

/** Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'
/** Maximum transfer unit. */
#define NET_MTU 1500

bool MAC_Packet_Acknowledge(TCPIP_MAC_PACKET * pPkt, const void * param)
{
    TCPIP_PKT_PacketFree(pPkt);
    return false;
}

static err_t PIC32MZW1_Netif_Tx(struct netif * netif, struct pbuf * p)
{
    // printf("Out 1\r\n");
    char * payload = (char *) p->payload;

#ifdef PACKET_DEBUG
    SYS_CONSOLE_PRINT("lwip: PIC32MZW1_Netif_Tx\r\n");
    SYS_CONSOLE_PRINT("payload size = %d, 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\r\n", p->len, payload[0], payload[1], payload[2],
                      payload[3], payload[4], payload[5], payload[6], payload[7]);
#endif

    TCPIP_NET_IF * pNetIf = (TCPIP_NET_IF *) netH;
    TCPIP_MAC_PACKET * pTxPkt;

    pTxPkt = TCPIP_PKT_PacketAlloc(sizeof(TCPIP_MAC_PACKET), p->len, 0);
    if (pTxPkt != 0)
    {
        pTxPkt->ackFunc = MAC_Packet_Acknowledge;
        memcpy(pTxPkt->pDSeg->segLoad, p->payload, p->len);
        pTxPkt->pDSeg->segLen = p->len;
        pTxPkt->next          = 0;
    }
    _TCPIPStackPacketTx(pNetIf, pTxPkt);
    // printf("Out 2\r\n");
    return ERR_OK;
}
static void PIC32MZW1_Netif_Low_Level_Init(struct netif * netif)
{
    static uint8_t mac_addr[6];

    if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_InfoDeviceMACAddressGet(wdrvHandle, mac_addr))
    {
        SYS_CONSOLE_PRINT("WDRV_PIC32MZW_InfoDeviceMACAddressGet fail...");
    }

    if (netif->num == 0)
    {
        SYS_CONSOLE_PRINT("XXX netif = %d\r\n", netif->num);
        netif->hwaddr_len = sizeof(mac_addr);
        memcpy(netif->hwaddr, mac_addr, sizeof(mac_addr));
    }
    else
    {
        SYS_CONSOLE_PRINT("??? netif = %d\r\n", netif->num);
        mac_addr[5]       = 0x11;
        netif->hwaddr_len = sizeof(mac_addr);
        memcpy(netif->hwaddr, mac_addr, sizeof(mac_addr));
    }
    netif->mtu = NET_MTU;
    netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_MLD6
#if LWIP_IGMP
        | NETIF_FLAG_IGMP
#endif // LWIP_IGMP
        ;
}

static uint8_t pic32mzw1_netif_ipv6_mac_mcast[6] = { 0x33, 0x33, 0x00, 0x00, 0x00, 0x00 };

static err_t PIC32MZW1_Netif_IPV6_MAC_Filter(struct netif * netif, ip6_addr_t * group, u8_t action)
{

    memcpy(&pic32mzw1_netif_ipv6_mac_mcast, &group->addr[3], 4);

    SYS_CONSOLE_PRINT("action = %d, mac_filter = %x:%x:%x:%x:%x:%x: \r\n", action, pic32mzw1_netif_ipv6_mac_mcast[0],
                      pic32mzw1_netif_ipv6_mac_mcast[1], pic32mzw1_netif_ipv6_mac_mcast[2], pic32mzw1_netif_ipv6_mac_mcast[3],
                      pic32mzw1_netif_ipv6_mac_mcast[4], pic32mzw1_netif_ipv6_mac_mcast[5]);

    return 0;
}

err_t PIC32MZW1_Netif_Init(struct netif * netif)
{
#if LWIP_NETIF_HOSTNAME
    netif->hostname = "PIC32MZW1";
#endif /* LWIP_NETIF_HOSTNAME */
#if LWIP_SNMP
    NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, NET_LINK_SPEED);
#endif /* LWIP_SNMP */
    netif->state      = NULL;
    netif->name[0]    = IFNAME0;
    netif->name[1]    = IFNAME1;
    netif->output     = etharp_output;
    netif->linkoutput = PIC32MZW1_Netif_Tx;
#if LWIP_IPV6
    netif->output_ip6 = ethip6_output;
#endif
#if LWIP_IGMP
    // netif->igmp_mac_filter = wilc_netif_ipv4_mac_filter;
#endif // #if LWIP_IGMP
#if LWIP_IPV6_MLD
    netif->mld_mac_filter = PIC32MZW1_Netif_IPV6_MAC_Filter;
#endif // #if LWIP_IPV6_MLD
    PIC32MZW1_Netif_Low_Level_Init(netif);
    return ERR_OK;
}

DHCP_STATUS_CALLBACK dhcp_status_cb_fn;
NETWORK_PROVISIONING_CALLBACK network_provisioning_cb_fn;

static void Status_Callback(struct netif * netif)
{
    SYS_CONSOLE_PRINT("lwip: Status_Callback\r\n");

    if (netif == &pic32mzw1_netif_sta)
        dhcp_status_cb_fn(netif);
}

/* Parse Wi-Fi configuration file */
/* Format is APP_WIFI_PROV_WIFI_CONFIG_ID,<SSID>,<AUTH>,<PASSPHRASE>*/
static int8_t Parse_Wifi_Config(char * payload)
{
    char * p;
    char * key;
    int8_t ret = 0;
    char ssid[WDRV_PIC32MZW_MAX_SSID_LEN];
    char passphrase[WDRV_PIC32MZW_PSK_LEN];
    uint8_t auth;

    p = strtok(payload, ",");
    if (p != NULL && !strncmp(p, APP_WIFI_PROV_WIFI_CONFIG_ID, strlen(APP_WIFI_PROV_WIFI_CONFIG_ID)))
    {
        p = strtok(NULL, ",");
        if (p)
            strcpy(ssid, p);

        p = strtok(NULL, ",");
        if (p)
        {
            auth = atoi(p);
            if (OPEN < auth && auth < WIFI_AUTH_MAX)
            {
                p = strtok(NULL, ",");
                if (p)
                    strcpy(passphrase, p);
                else
                    ret = -1;
            }
            else if (auth == OPEN)
                strcpy(passphrase, "");
            else
                ret = -1;
        }
        else
            ret = -1;

        SYS_CONSOLE_PRINT("SSID:%s - PASSPHRASE:%s - AUTH:%d\r\n", ssid, passphrase, auth);

        if (ret == 0)
        {
            switch (auth)
            {
            case WPAWPA2MIXED:
                network_provisioning_cb_fn(ssid, passphrase, WDRV_PIC32MZW_AUTH_TYPE_WPAWPA2_PERSONAL);
                break;

            case WEP:
                network_provisioning_cb_fn(ssid, passphrase, WDRV_PIC32MZW_AUTH_TYPE_WEP);
                break;

            case OPEN:
                network_provisioning_cb_fn(ssid, passphrase, WDRV_PIC32MZW_AUTH_TYPE_OPEN);
                break;
            default:
                SYS_CONSOLE_PRINT("Not support, auth = %d\r\n", auth);
                ret = -1;
                break;
            }
        }
    }
    return ret;
}

static err_t Provisioning_Receive_Fn(void * arg, struct tcp_pcb * pcb, struct pbuf * p, err_t err)
{
    struct http_state * hs = (struct http_state *) arg;
    char payload[100];

    SYS_CONSOLE_PRINT("http_recv: pcb=%p pbuf=%p err=%s\n", (void *) pcb, (void *) p, lwip_strerr(err));
    if (p != NULL)
    {
        Parse_Wifi_Config(p->payload);
        /* Inform TCP that we have taken the data. */
        tcp_recved(pcb, p->tot_len);
        pbuf_free(p);
    }
    else
    {
        pbuf_free(p);
        tcp_close(pcb);
    }

    return ERR_OK;
}

static err_t Provisioning_Server_Accept(void * arg, struct tcp_pcb * pcb, err_t err)
{
    tcp_recv(pcb, Provisioning_Receive_Fn);

    tcp_accepted(pcb);

    return ERR_OK;
}

static void Provisioning_Server_Start()
{
    struct tcp_pcb * pcb;

    {
        pcb = tcp_new();

        tcp_bind(pcb, IP_ADDR_ANY, 80);
        pcb = tcp_listen(pcb);
        tcp_accept(pcb, Provisioning_Server_Accept);
    }
}

void Network_PIC32MZW_NETWORK_Provision_Callback_Register(NETWORK_PROVISIONING_CALLBACK cb)
{
    network_provisioning_cb_fn = cb;
}

void Network_PIC32MZW_Set_WiFi_Mode(int softAP)
{
    g_softAP    = softAP;
    g_init_flag = 1;
}

struct netif * Network_PIC32MZW_Get_Interface(void)
{
    return &pic32mzw1_netif_sta;
}
int Network_PIC32MZW_StartDHCP(DHCP_STATUS_CALLBACK cb)
{
    dhcp_status_cb_fn = cb;
    netif_set_up(&pic32mzw1_netif_sta);
    netif_set_link_up(&pic32mzw1_netif_sta);
    err_t err = dhcp_start(&pic32mzw1_netif_sta);

    return err;
}
int Network_PIC32MZW_StopDHCP(void)
{
    netif_set_down(&pic32mzw1_netif_sta);
    // netif_set_link_down(&pic32mzw1_netif_sta);
    dhcp_stop(&pic32mzw1_netif_sta);

    return 0;
}
int Network_PIC32MZW_StartDHCPServer(void)
{

    netif_set_link_up(&pic32mzw1_netif_ap);
    netif_set_up(&pic32mzw1_netif_ap);
    lwip_tiny_dhcpserver_start();

    Provisioning_Server_Start();
    return 0;
}
DRV_HANDLE Network_PIC32MZW_Get_WiFiHandle(void)
{
    return wdrvHandle;
}
/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */
void Network_Tasks(void)
{
    SYS_STATUS tcpipStat;
    bool status;

    int i, nNets;
    static init_flag = 0;
    static struct pbuf * rx;
    TCPIP_MODULE_SIGNAL_HANDLE signalH;
    const void * netHandlerParams;

#ifdef HEAP_DEBUG
    static int counter = 0;
#endif

    switch (networkData.state)
    {
    case NETWORK_STATE_WIFI_INIT: {

        if (SYS_STATUS_READY == WDRV_PIC32MZW_Status(sysObj.drvWifiPIC32MZW1))
        {
            wdrvHandle = WDRV_PIC32MZW_Open(0, 0);

            if (DRV_HANDLE_INVALID != wdrvHandle)
            {
                networkData.state = NETWORK_STATE_INIT;
            }
        }
        break;
    }
    /* Application's initial state. */
    case NETWORK_STATE_INIT: {
        // if (init_flag == 0)
        if (g_init_flag)
        {
            if (!g_softAP)
            {
                struct ip_addr ip_addr;
                memset(&ip_addr, 0, sizeof(struct ip_addr));
                netif_add(&pic32mzw1_netif_sta, &ip_addr, &ip_addr, &ip_addr, 0, PIC32MZW1_Netif_Init, tcpip_input);

                netif_create_ip6_linklocal_address(&pic32mzw1_netif_sta, 1);
                pic32mzw1_netif_sta.ip6_autoconfig_enabled = 1;
                netif_set_status_callback(&pic32mzw1_netif_sta, Status_Callback);
                netif_set_default(&pic32mzw1_netif_sta);
            }
            else
            {
                /* Add AP/WD interface. */
                ip4_addr_t mask       = SN_MASK_IP;
                ip4_addr_t gateway    = GW_ADDR_IP;
                ip4_addr_t ip_addr_ap = AP_ADDR_IP;

                netif_add(&pic32mzw1_netif_ap, &ip_addr_ap, &mask, &gateway, 0, PIC32MZW1_Netif_Init, tcpip_input);
                netif_set_default(&pic32mzw1_netif_ap);
            }
            packet_recv_mutex = xSemaphoreCreateMutex();

            // init_flag = 1;
            SYS_CONSOLE_PRINT("lwip: add interface\r\n");

            networkData.state = NETWORK_STATE_TCPIP_WAIT_FOR_TCPIP_INIT;
        }

#if 0            
            SYS_STATUS sysStatus;
            
            sysStatus = WDRV_PIC32MZW_Status(sysObj.drvWifiPIC32MZW1);
                    //SYS_CONSOLE_PRINT("APP_STATE_INIT, sysStatus = %d\r\n", sysStatus);
            if (SYS_STATUS_READY == sysStatus)
            {
                networkData.state = NETWORK_STATE_TCPIP_WAIT_FOR_TCPIP_INIT;
            }
            else if (SYS_STATUS_READY_EXTENDED == sysStatus)
            {
                printf("APP_STATE_INIT\r\n");
                if (WDRV_PIC32MZW_SYS_STATUS_RF_CONF_MISSING == WDRV_PIC32MZW_StatusExt(sysObj.drvWifiPIC32MZW1))
                {
                    /* Continue to initialisation state to allow application to set reg domain from command */
                    networkData.state = NETWORK_STATE_TCPIP_WAIT_FOR_TCPIP_INIT;
                }
            }
#endif
        break;
    }

    case NETWORK_STATE_TCPIP_WAIT_FOR_TCPIP_INIT: {
        tcpipStat = TCPIP_STACK_Status(sysObj.tcpip);

        if (tcpipStat < 0)
        {
            SYS_CONSOLE_MESSAGE("APP: TCP/IP stack initialization failed!\r\n");
            // networkData.state = APP_TCPIP_ERROR;
        }
        else if (SYS_STATUS_READY == tcpipStat)
        {
            nNets = TCPIP_STACK_NumberOfNetworksGet();
            for (i = 0; i < nNets; i++)
            {
                netH               = TCPIP_STACK_IndexToNet(i);
                TCPIP_event_handle = TCPIP_STACK_HandlerRegister(netH, TCPIP_EV_CONN_ALL, APP_TcpipStack_EventHandler, NULL);
                // TCPIP_DHCP_HandlerRegister(netH, APP_TcpipDhcp_EventHandler, NULL);
            }

            TCPIP_STACK_PacketHandlerRegister(netH, pktHandler, netHandlerParams);
            /// signalH = TCPIP_MODULE_SignalFunctionRegister( TCPIP_MODULE_MANAGER, appSignalFunc);
            //_TCPIPStackSignalHandlerRegister(TCPIP_MODULE_MANAGER,appSignalHandler,100);
            // networkData.state = APP_WIFI_CONFIG;
            networkData.state = NETWORK_STATE_IDLE;
        }
        break;
    }

    case NETWORK_STATE_IDLE: {
#ifdef HEAP_DEBUG
        counter++;
        if (counter > 20)
        {
            SYS_CONSOLE_PRINT("APP: free heap = %d, minimum = %d\r\n", xPortGetFreeHeapSize(), xPortGetMinimumEverFreeHeapSize());
            counter = 0;
        }
#endif
        break;
    }

    case NETWORK_STATE_ERROR: {
        SYS_CONSOLE_MESSAGE("APP: APP_WIFI_ERROR\r\n");
        networkData.state = NETWORK_STATE_IDLE;
        break;
    }

    default:
        break;
    }
}

/*******************************************************************************
 End of File
 */
