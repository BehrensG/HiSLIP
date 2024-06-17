/*
 * test.c
 *
 *  Created on: Jun 7, 2024
 *      Author: grzegorz
 */
#include "api.h"
#include "netif.h"
#include "mdns.h"

const char manufacturer[] = "Manufacurer= ADD TEXT";

#if LWIP_MDNS_RESPONDER

static void srv_txt(struct mdns_service *service, void *txt_userdata)
{
  err_t res;
  LWIP_UNUSED_ARG(txt_userdata);

  res = mdns_resp_add_service_txtitem(service, "path=/", 6);
  LWIP_ERROR("mdns add service txt failed\n", (res == ERR_OK), return);
}


static void test_txt(struct mdns_service *service, void *txt_userdata)
{
  err_t res;
  LWIP_UNUSED_ARG(txt_userdata);

  res = mdns_resp_add_service_txtitem(service, manufacturer, strlen(manufacturer));
  LWIP_ERROR("mdns add service txt failed\n", (res == ERR_OK), return);
}
#endif

#if LWIP_MDNS_RESPONDER

static void mdns_example_report(struct netif* netif, u8_t result)
{
  LWIP_PLATFORM_DIAG(("mdns status[netif %d]: %d\n", netif->num, result));
}

#endif
extern ip4_addr_t ipaddr;
ip4_addr_t groupaddr;

 void MDNS_Init(void)
{


#if LWIP_MDNS_RESPONDER

  mdns_resp_register_name_result_cb(mdns_example_report);
  mdns_resp_init();
  mdns_resp_add_netif(netif_default, "mdns_name",20);
  mdns_resp_add_service(netif_default, "web", "_http", DNSSD_PROTO_TCP, 80, 20, srv_txt, NULL);
  mdns_resp_add_service(netif_default, "hislip0", "_hislip", DNSSD_PROTO_TCP, 4880, 20, test_txt, NULL);
  mdns_resp_add_service(netif_default, "spi-raw", "_scpi-raw", DNSSD_PROTO_TCP, 5025, 20, test_txt, NULL);

  mdns_resp_announce(netif_default);

#endif
}


osThreadId UDPTaskHandle;
uint32_t UDPTaskBuffer[1024];
osStaticThreadDef_t UDPTaskControlBlock;
static struct netconn *conn;

void UDP_NetconnCallback(struct netconn *conn, enum netconn_evt even, u16_t len) {
	HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);

	if (NETCONN_EVT_RCVPLUS == even) {



	}

}

