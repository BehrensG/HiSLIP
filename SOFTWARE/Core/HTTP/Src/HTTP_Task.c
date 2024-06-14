/*
 * http_task.c
 *
 *  Created on: Jun 13, 2024
 *      Author: grzegorz
 */
#include <string.h>
#include <stdbool.h>

#include "lwip/apps/httpd_opts.h"
#include "lwip/apps/fs.h"
#include "api.h"

char* http_get_site[] = {"GET / HTTP/1.1", "GET /top.html", "GET /home.html", "GET /setup.html",
						"GET /support.html", "GET /index.html", "GET /help.html"};

char* http_get_home[] ={"GET /info_instr", "GET /info_sn", "GET /info_descr", "GET /info_ipv4", "GET /info_netmask",
					"GET /info_gateway", "GET /info_mac", "GET /info_visa", "GET /info_mdix", "GET /info_mdns", "GET /info_udp"};

typedef enum{
	HTTP_GET_SITE_INIT,
	HTTP_GET_SITE_TOP,
	HTTP_GET_SITE_HOME,
	HTTP_GET_SITE_SETUP,
	HTTP_GET_SITE_SUPPORT,
	HTTP_GET_SITE_INDEX,
	HTTP_GET_SITE_HELP

}http_get_site_t;

typedef enum{
	HTTP_GET_HOME_INSTR,
	HTTP_GET_HOME_SN,
	HTTP_GET_HOME_DESCR,
	HTTP_GET_HOME_IPV4,
	HTTP_GET_HOME_NETMASK,
	HTTP_GET_HOME_GATEWAY,
	HTTP_GET_HOME_MAC,
	HTTP_GET_HOME_VISA,
	HTTP_GET_HOME_MDIX,
	HTTP_GET_HOME_MDNS,
	HTTP_GET_HOME_UDP

}http_get_home_t;

static bool http_load_website(struct netconn *conn, char* buf, u16_t buflen)
{
	u32_t size = sizeof(http_get_site)/sizeof(http_get_site[0]);
	s32_t cmp = -1;
	struct fs_file file;
	bool status = false;

	for(u8_t i = 0; i < size; i++)
	{
		if (strncmp((char const *)buf, http_get_site[i], strlen(http_get_site[i]))==0)
		{
			cmp = i;
		}
	}

	switch(cmp)
	{
		case HTTP_GET_SITE_INIT: fs_open(&file, "/index.html"); break;
		case HTTP_GET_SITE_TOP: fs_open(&file, "/top.html"); break;
		case HTTP_GET_SITE_HOME: fs_open(&file, "/home.html"); break;
		case HTTP_GET_SITE_SETUP: fs_open(&file, "/setup.html"); break;
		case HTTP_GET_SITE_SUPPORT: fs_open(&file, "/support.html"); break;
		case HTTP_GET_SITE_INDEX: fs_open(&file, "/index.html"); break;
		case HTTP_GET_SITE_HELP: fs_open(&file, "/help.html"); break;

		default: /* DO NOTHING */; break;
	}

	if(cmp >= 0)
	{
		netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
		fs_close(&file);
		status = true;
	}

	return status;
}


static err_t http_home_page_send(struct netconn *conn, char* pagedata)
{
	return netconn_write(conn, (const unsigned char*)pagedata, strlen(pagedata), NETCONN_NOCOPY);
}

static bool http_home_page(struct netconn *conn, char* buf, u16_t buflen)
{
	u32_t size = sizeof(http_get_home)/sizeof(http_get_home[0]);
	s32_t cmp = -1;
	bool status = false;

	char pagedata[128];
	memset(pagedata,0,128);

	for(u8_t i = 0; i < size; i++)
	{
		if (strncmp((char const *)buf, http_get_home[i], strlen(http_get_home[i]))==0)
		{
			cmp = i;
		}
	}

	switch(cmp)
	{
		case HTTP_GET_HOME_INSTR: strcpy(pagedata,"Test device"); break;
		case HTTP_GET_HOME_SN: strcpy(pagedata,"SN1234"); break;
		case HTTP_GET_HOME_DESCR: strcpy(pagedata,"Test description"); break;
		case HTTP_GET_HOME_IPV4: strcpy(pagedata,"192.168.1.123"); break;
		case HTTP_GET_HOME_NETMASK: strcpy(pagedata,"255.255.255.0"); break;
		case HTTP_GET_HOME_GATEWAY: strcpy(pagedata,"192.168.1.254"); break;
		case HTTP_GET_HOME_MAC: strcpy(pagedata,"00-80-e1-00-00-00"); break;
		case HTTP_GET_HOME_VISA: strcpy(pagedata,"TCPIP0::192.168.1.123::5025::SOCKET\r\nTCPIP0::192.168.1.123::hislip0"); break;
		case HTTP_GET_HOME_MDIX: strcpy(pagedata,"YES"); break;
		case HTTP_GET_HOME_MDNS: strcpy(pagedata,"NO"); break;
		case HTTP_GET_HOME_UDP: strcpy(pagedata,"5025"); break;

		default: /* DO NOTHING */; break;
	}

	if(cmp >= 0)
	{
		http_home_page_send(conn,pagedata);
		status = true;
	}

	return status;
}


static void http_server(struct netconn *conn)
{
	struct netbuf *inbuf;
	err_t recv_err;
	char* buf;
	u16_t buflen;
	bool done = false;
	recv_err = netconn_recv(conn, &inbuf);

	if (recv_err == ERR_OK)
	{
		if (netconn_err(conn) == ERR_OK)
		{
			netbuf_data(inbuf, (void**)&buf, &buflen);

			done = http_load_website(conn, buf, buflen);

			if(!done)
			{
				http_home_page(conn, buf, buflen);
			}

		}
	}

	netconn_close(conn);
	netbuf_delete(inbuf);
}

/*
static void http_server(struct netconn *conn)
{
	struct netbuf *inbuf;
	err_t recv_err;
	char* buf;
	u16_t buflen;
	struct fs_file file;

	recv_err = netconn_recv(conn, &inbuf);

	if (recv_err == ERR_OK)
	{
		if (netconn_err(conn) == ERR_OK)
		{
			netbuf_data(inbuf, (void**)&buf, &buflen);

			if (strncmp((char const *)buf, http_get_site[0], strlen(http_get_site[0]))==0)
			{
				fs_open(&file, "/index.html");
				netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
				fs_close(&file);
			}
			else if(strncmp((char const *)buf, http_get_site[1], strlen(http_get_site[1]))==0)
			{
				fs_open(&file, "/top.html");
				netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
				fs_close(&file);
			}
			else if(strncmp((char const *)buf, http_get_site[2], strlen(http_get_site[2]))==0)
			{
				fs_open(&file, "/home.html");
				netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
				fs_close(&file);
			}
			else if(strncmp((char const *)buf, http_get_site[3], strlen(http_get_site[3]))==0)
			{
				fs_open(&file, "/configuration.html");
				netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
				fs_close(&file);
			}
			else if(strncmp((char const *)buf, http_get_site[4], strlen(http_get_site[4]))==0)
			{
				fs_open(&file, "/support.html");
				netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
				fs_close(&file);
			}
			else if(strncmp((char const *)buf, http_get_site[5], strlen(http_get_site[5]))==0)
			{
				fs_open(&file, "/index.html");
				netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
				fs_close(&file);
			}
		}
	}

	netconn_close(conn);
	netbuf_delete(inbuf);
}
*/

static void http_thread(void *arg)
{
  struct netconn *conn, *newconn;
  err_t err, accept_err;


  conn = netconn_new(NETCONN_TCP);

  if (conn!= NULL)
  {

    err = netconn_bind(conn, IP_ADDR_ANY, 80);

    if (err == ERR_OK)
    {

      netconn_listen(conn);

      while(1)
      {

        accept_err = netconn_accept(conn, &newconn);
        if(accept_err == ERR_OK)
        {

          http_server(newconn);

          netconn_delete(newconn);
        }
      }
    }
  }
}

TaskHandle_t http_handler;
uint32_t http_buffer[2*1024];
StaticTask_t http_control_block;

void HTTP_CreateTask()
{
	http_handler = xTaskCreateStatic(http_thread,"http_ServerTask",
			2*1024, (void*)1, tskIDLE_PRIORITY + 3,
			http_buffer, &http_control_block);
}
