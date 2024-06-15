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

extern struct netif gnetif;

struct netif *mdns_netif ;

void MDNS_Init(void)
{
	mdns_netif = &gnetif;
	mdns_netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP | NETIF_FLAG_IGMP;

#if LWIP_MDNS_RESPONDER

  mdns_resp_register_name_result_cb(mdns_example_report);
  mdns_resp_init();
  mdns_resp_add_netif(mdns_netif, "mdns_name",5);
  mdns_resp_add_service(mdns_netif, "web", "_http", DNSSD_PROTO_TCP, 80, 5, srv_txt, NULL);
  mdns_resp_add_service(mdns_netif, "hislip0", "_hislip", DNSSD_PROTO_TCP, 4880, 5, test_txt, NULL);
  mdns_resp_add_service(mdns_netif, "spi-raw", "_scpi-raw", DNSSD_PROTO_TCP, 5025, 5, test_txt, NULL);
  mdns_resp_announce(mdns_netif);

#endif
}
