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


void MDNS_Init(void)
{

	netif_default->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP | NETIF_FLAG_IGMP;

#if LWIP_MDNS_RESPONDER

  mdns_resp_register_name_result_cb(mdns_example_report);
  mdns_resp_init();
  mdns_resp_add_netif(netif_default, "mdns_name",0xFFFFFFFF);
  mdns_resp_add_service(netif_default, "web", "_http", DNSSD_PROTO_TCP, 80, 0xFFFFFFFF, srv_txt, NULL);
  mdns_resp_add_service(netif_default, "hislip0", "_hislip", DNSSD_PROTO_TCP, 4880, 0xFFFFFFFF, test_txt, NULL);
  mdns_resp_add_service(netif_default, "spi-raw", "_scpi-raw", DNSSD_PROTO_TCP, 5025, 0xFFFFFFFF, test_txt, NULL);

  mdns_resp_announce(netif_default);

#endif
}
