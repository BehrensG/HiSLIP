/*
 * test.c
 *
 *  Created on: Jun 7, 2024
 *      Author: grzegorz
 */
#include "api.h"
#include "netif.h"
#include "mdns.h"

#if LWIP_MDNS_RESPONDER
static void
srv_txt(struct mdns_service *service, void *txt_userdata)
{
  err_t res;
  LWIP_UNUSED_ARG(txt_userdata);

  res = mdns_resp_add_service_txtitem(service, "path=/", 6);
  LWIP_ERROR("mdns add service txt failed\n", (res == ERR_OK), return);
}
#endif

#if LWIP_MDNS_RESPONDER
static void
mdns_example_report(struct netif* netif, u8_t result)
{
  LWIP_PLATFORM_DIAG(("mdns status[netif %d]: %d\n", netif->num, result));
}
#endif

//extern struct netif gnetif;
void
mdns_example_init(void)
{
	//netif_default = &gnetif ;
	netif_default->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP | NETIF_FLAG_IGMP;
#if LWIP_MDNS_RESPONDER
  mdns_resp_register_name_result_cb(mdns_example_report);
  mdns_resp_init();
  mdns_resp_add_netif(netif_default, "lwip",1);
  mdns_resp_add_service(netif_default, "hislip0", "_hislip", DNSSD_PROTO_TCP, 4880, 1, srv_txt, NULL);
 // mdns_resp_announce(netif_default);
#endif
}
