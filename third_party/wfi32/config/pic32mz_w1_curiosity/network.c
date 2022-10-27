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
#include "user.h"
#include "tcpip/tcpip.h"
#include "tcpip/tcpip_manager.h"
#include "wdrv_pic32mzw_client_api.h"

#include "tcpip/src/tcpip_private.h"




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


#define TCPIP_THIS_MODULE_ID    TCPIP_MODULE_NONE
// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/** The netif for the pic32mzw1 in station mode. */
struct netif pic32mzw1_netif_sta;

DRV_HANDLE wdrvHandle = DRV_HANDLE_INVALID;

bool pktHandler(TCPIP_NET_HANDLE hNet, struct _tag_TCPIP_MAC_PACKET* rxPkt, uint16_t frameType, const void* hParam);


static TCPIP_EVENT_HANDLE TCPIP_event_handle;





static void APP_TcpipStack_EventHandler(TCPIP_NET_HANDLE hNet, TCPIP_EVENT event, const void *fParam)
{
    const char *netName = TCPIP_STACK_NetNameGet(hNet);
    SYS_CONSOLE_PRINT("APP: %s  event = %d\r\n", netName, event);
    if (event & TCPIP_EV_CONN_ESTABLISHED) 
    {
        if (TCPIP_DHCP_IsEnabled(hNet) == false)
        {
            TCPIP_DHCP_Enable(hNet);
        }
    }
    else if(event & TCPIP_EV_CONN_LOST)
    {
        TCPIP_DHCP_Disable(hNet);
    }
    else
    {
        SYS_CONSOLE_PRINT("APP: %s Unknown event = %d\r\n", netName, event);
    }
}

void APP_TcpipDhcp_EventHandler(TCPIP_NET_HANDLE hNet, TCPIP_DHCP_EVENT_TYPE evType, const void* param)
{
    switch(evType)
    {
        case DHCP_EVENT_BOUND:
        {
            TCPIP_DHCP_INFO DhcpInfo;
            if(TCPIP_DHCP_InfoGet(hNet, &DhcpInfo))
            {
                SYS_CONSOLE_PRINT("APP: IP address = %d.%d.%d.%d \r\n", 
                        DhcpInfo.dhcpAddress.v[0], DhcpInfo.dhcpAddress.v[1], DhcpInfo.dhcpAddress.v[2], DhcpInfo.dhcpAddress.v[3]);
            }
            break;
        }
        case DHCP_EVENT_CONN_ESTABLISHED:
        {
            SYS_CONSOLE_MESSAGE("APP: Connection to the DHCP server established\r\n");
            break;
        }
        case DHCP_EVENT_CONN_LOST:
        {
            SYS_CONSOLE_MESSAGE("APP: Connection to the DHCP server lost\r\n");
            break;
        }
        default:
            break;
    }
}

bool pktHandler(TCPIP_NET_HANDLE hNet, struct _tag_TCPIP_MAC_PACKET* rxPkt, uint16_t frameType, const void* hParam)
{
    struct pbuf *rx_pkt;
    
    SYS_CONSOLE_PRINT("APP: pktHandler In, size = %d\r\n", rxPkt->pDSeg->segLen);
    SYS_CONSOLE_PRINT("data: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\r\n", rxPkt->pDSeg->segLoad[0], rxPkt->pDSeg->segLoad[1], rxPkt->pDSeg->segLoad[2], rxPkt->pDSeg->segLoad[3], rxPkt->pDSeg->segLoad[4], rxPkt->pDSeg->segLoad[5], rxPkt->pDSeg->segLoad[6], rxPkt->pDSeg->segLoad[7]);

    rx_pkt = pbuf_alloc(PBUF_RAW, rxPkt->pDSeg->segLen + 14, PBUF_RAM);
    //memcpy(rx_pkt->payload, rxPkt->pDSeg->segLoad, rxPkt->pDSeg->segLen);
    memcpy(rx_pkt->payload, rxPkt->pMacLayer, rxPkt->pDSeg->segLen +14);
    //pbuf_header(rx_pkt,14);
    pic32mzw1_netif_sta.input(rx_pkt, &pic32mzw1_netif_sta);
    
    //pbuf_free(rx_pkt);
    
    TCPIP_PKT_PacketAcknowledge(rxPkt, TCPIP_MAC_PKT_ACK_RX_OK);
    
    return true;
}
TCPIP_NET_HANDLE netH;


/** Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'
/** Maximum transfer unit. */
#define NET_MTU  1500

bool macPacketAcknowledge(TCPIP_MAC_PACKET * pPkt, const void * param)
{
    TCPIP_PKT_PacketFree(pPkt);
    return false;
}

static err_t wilc_netif_tx(struct netif *netif, struct pbuf *p)
{
    SYS_CONSOLE_PRINT("lwip: wilc_netif_tx\r\n");
	return ERR_OK;
}

static err_t pic32mzw1_netif_tx(struct netif *netif, struct pbuf *p)
{
    char* payload = (char *) p->payload;
    SYS_CONSOLE_PRINT("lwip: pic32mzw1_netif_tx\r\n");
    //SYS_CONSOLE_PRINT("payload size = %d, 0x%x\r\n", p->len, payload[0]);
    SYS_CONSOLE_PRINT("payload size = %d, 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\r\n", p->len, payload[0], payload[1], payload[2],payload[3], payload[4], payload[5], payload[6], payload[7]);
	
    //TCPIP_NET_IF* pNetIf = _TCPIPStackHandleToNet(hNet);
    TCPIP_NET_IF* pNetIf = (TCPIP_NET_IF*)netH;
    TCPIP_MAC_PACKET* pTxPkt;
    
    pTxPkt = TCPIP_PKT_PacketAlloc(sizeof(TCPIP_MAC_PACKET), p->len, 0);
    if (pTxPkt != 0)
    {
        pTxPkt->ackFunc = macPacketAcknowledge;
        memcpy(pTxPkt->pDSeg->segLoad,p->payload,p->len);
        pTxPkt->pDSeg->segLen = p->len;
        pTxPkt->next = 0;
    }
    _TCPIPStackPacketTx(pNetIf, pTxPkt);
    
    return ERR_OK;
}
static void pic32mzw1_netif_low_level_init(struct netif *netif)
{
	static uint8_t mac_addr[6];
	//static uint8_t mac1[6] = {0xe8,0xeb,0x1b,0xf3,0x0b,0x24};

    if (WDRV_PIC32MZW_STATUS_OK != WDRV_PIC32MZW_InfoDeviceMACAddressGet(wdrvHandle, mac_addr))
    {
        SYS_CONSOLE_PRINT("WDRV_PIC32MZW_InfoDeviceMACAddressGet fail...");
    }
	
    netif->hwaddr_len = sizeof(mac_addr);
    memcpy(netif->hwaddr, mac_addr, sizeof(mac_addr));

	netif->mtu = NET_MTU;
	netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP
#if LWIP_IGMP
			| NETIF_FLAG_IGMP
#endif // LWIP_IGMP
	;
}

static uint8_t pic32mzw1_netif_ipv6_mac_mcast[6] = {
    0x33, 0x33, 0x00, 0x00, 0x00, 0x00
};

static err_t pic32mzw1_netif_ipv6_mac_filter(struct netif *netif, ip6_addr_t *group, u8_t action)
{   

    memcpy(&pic32mzw1_netif_ipv6_mac_mcast, &group->addr[3], 4);
    
    SYS_CONSOLE_PRINT("action = %d, mac_filter = %x:%x:%x:%x:%x:%x: \r\n", action, pic32mzw1_netif_ipv6_mac_mcast[0], pic32mzw1_netif_ipv6_mac_mcast[1], pic32mzw1_netif_ipv6_mac_mcast[2], pic32mzw1_netif_ipv6_mac_mcast[3], pic32mzw1_netif_ipv6_mac_mcast[4], pic32mzw1_netif_ipv6_mac_mcast[5]);

    return 0;
}

err_t pic32mzw1_netif_init(struct netif *netif)
{
#if LWIP_NETIF_HOSTNAME
	netif->hostname = "PIC32MZW1";
#endif /* LWIP_NETIF_HOSTNAME */
#if LWIP_SNMP
	NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, NET_LINK_SPEED);
#endif /* LWIP_SNMP */
	netif->state = NULL;
	netif->name[0] = IFNAME0;
	netif->name[1] = IFNAME1;
	netif->output = etharp_output;
	netif->linkoutput = pic32mzw1_netif_tx;
#if LWIP_IPV6
	netif->output_ip6 = ethip6_output;
#endif
#if LWIP_IGMP
	//netif->igmp_mac_filter = wilc_netif_ipv4_mac_filter;
#endif // #if LWIP_IGMP
#if LWIP_IPV6_MLD
	netif->mld_mac_filter = pic32mzw1_netif_ipv6_mac_filter;
#endif // #if LWIP_IPV6_MLD
	pic32mzw1_netif_low_level_init(netif);
	return ERR_OK;
}



DHCP_STATUS_CALLBACK dhcp_status_cb_fn;

static void status_callback(struct netif *netif)
{
    SYS_CONSOLE_PRINT("lwip: status_callback\r\n");
    
    if (netif == &pic32mzw1_netif_sta)
        dhcp_status_cb_fn(netif);
#if 0
    if (netif_is_up(netif)){
       SYS_CONSOLE_PRINT("ip addr: %s\r\n", ip4addr_ntoa(netif_ip4_addr(netif))); 


        for (uint8_t i = 0; i < 5; i++)
        {
            //if (ip6_addr_islinklocal(netif_ip6_addr(netif, i)) &&
            //    ip6_addr_isvalid(netif_ip6_addr_state(netif, i)))
            {

                SYS_CONSOLE_PRINT("IPv6 Address %i Assigned : %s", i, ip6addr_ntoa(netif_ip6_addr(netif, i)));
            }
        }
       //SYS_CONSOLE_PRINT("ip6 addr: %s\r\n", ip6addr_ntoa(netif_ip6_addr(netif,0))); 
       //SYS_CONSOLE_PRINT("ip6 addr: %s\r\n", ip6addr_ntoa(netif_ip6_addr(netif,1))); 
    }
#endif
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
void Network_Tasks ( void )
{
    SYS_STATUS tcpipStat;
    bool status;
    
    int i, nNets;
	static init_flag = 0;
    static struct pbuf *rx;
    TCPIP_MODULE_SIGNAL_HANDLE signalH;
    const void *netHandlerParams;
    
    static int counter = 0;


    switch ( networkData.state )
    {
        case NETWORK_STATE_WIFI_INIT:
        {

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
        case NETWORK_STATE_INIT:
        {
			if (init_flag == 0)
			{
                struct ip_addr ip_addr;
                memset(&ip_addr, 0, sizeof(struct ip_addr));
                netif_add(&pic32mzw1_netif_sta, &ip_addr, &ip_addr, &ip_addr, 0, pic32mzw1_netif_init, tcpip_input);
                
                netif_create_ip6_linklocal_address(&pic32mzw1_netif_sta, 1);
                pic32mzw1_netif_sta.ip6_autoconfig_enabled = 1;
                netif_set_status_callback(&pic32mzw1_netif_sta, status_callback);
                netif_set_default(&pic32mzw1_netif_sta);
                
                init_flag = 1;
                SYS_CONSOLE_PRINT("lwip: add interface\r\n");
            }
			
            networkData.state = NETWORK_STATE_TCPIP_WAIT_FOR_TCPIP_INIT;
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
        
        
        case NETWORK_STATE_TCPIP_WAIT_FOR_TCPIP_INIT:
        {
            tcpipStat = TCPIP_STACK_Status(sysObj.tcpip);

            if (tcpipStat < 0) 
            {
                SYS_CONSOLE_MESSAGE( "APP: TCP/IP stack initialization failed!\r\n" );
                //networkData.state = APP_TCPIP_ERROR;
            }
            else if (SYS_STATUS_READY == tcpipStat)
            {
                nNets = TCPIP_STACK_NumberOfNetworksGet();
                for (i = 0; i < nNets; i++)
                {
                    netH = TCPIP_STACK_IndexToNet(i);
                    TCPIP_event_handle = TCPIP_STACK_HandlerRegister(netH, TCPIP_EV_CONN_ALL, APP_TcpipStack_EventHandler, NULL);
                    //TCPIP_DHCP_HandlerRegister(netH, APP_TcpipDhcp_EventHandler, NULL);
                }
                
                TCPIP_STACK_PacketHandlerRegister(netH,pktHandler,netHandlerParams);
                 ///signalH = TCPIP_MODULE_SignalFunctionRegister( TCPIP_MODULE_MANAGER, appSignalFunc);
                //_TCPIPStackSignalHandlerRegister(TCPIP_MODULE_MANAGER,appSignalHandler,100);
                //networkData.state = APP_WIFI_CONFIG;
                networkData.state = NETWORK_STATE_IDLE;
            }
            break;
        }
        
        case NETWORK_STATE_IDLE:
        {
            
            counter ++;
            if (counter > 20)
            {
                SYS_CONSOLE_PRINT("APP: free heap = %d\r\n", xPortGetFreeHeapSize() );
                counter = 0;
            }
            break;
        }
        
        case NETWORK_STATE_ERROR:
        {
            SYS_CONSOLE_MESSAGE("APP: APP_WIFI_ERROR\r\n" );
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
