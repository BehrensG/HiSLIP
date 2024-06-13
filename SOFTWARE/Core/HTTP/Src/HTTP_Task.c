/*
 * http_task.c
 *
 *  Created on: Jun 13, 2024
 *      Author: grzegorz
 */
#include "lwip/apps/httpd_opts.h"
#include "lwip/apps/fs.h"
#include "api.h"

char** http_cmd[] = {"GET / http/1.1", "GET /top.html", "GET /home.html", "GET /configuration.html", "GET /support.h", "GET /index.html"};

static void http_server(struct netconn *conn)
{
	struct netbuf *inbuf;
	err_t recv_err;
	char* buf;
	u16_t buflen;
	struct fs_file file;

	/* Read the data from the port, blocking if nothing yet there */
	recv_err = netconn_recv(conn, &inbuf);

	if (recv_err == ERR_OK)
	{
		if (netconn_err(conn) == ERR_OK)
		{
			/* Get the data pointer and length of the data inside a netbuf */
			netbuf_data(inbuf, (void**)&buf, &buflen);

			/* Check if request to get the index.html */
			if (strncmp((char const *)buf, http_cmd[0], strlen(http_cmd[0]))==0)
			{
				fs_open(&file, "/index.html");
				netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
				fs_close(&file);
			}
			else if(strncmp((char const *)buf, http_cmd[1], strlen(http_cmd[1]))==0)
			{
				fs_open(&file, "/top.html");
				netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
				fs_close(&file);
			}
			else if(strncmp((char const *)buf, http_cmd[2], strlen(http_cmd[2]))==0)
			{
				fs_open(&file, "/home.html");
				netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
				fs_close(&file);
			}
			else if(strncmp((char const *)buf, http_cmd[3], strlen(http_cmd[3]))==0)
			{
				fs_open(&file, "/configuration.html");
				netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
				fs_close(&file);
			}
			else if(strncmp((char const *)buf, http_cmd[4], strlen(http_cmd[4]))==0)
			{
				fs_open(&file, "/support.html");
				netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
				fs_close(&file);
			}
			else if(strncmp((char const *)buf, http_cmd[5], strlen(http_cmd[5]))==0)
			{
				fs_open(&file, "/index.html");
				netconn_write(conn, (const unsigned char*)(file.data), (size_t)file.len, NETCONN_NOCOPY);
				fs_close(&file);
			}
		}
	}
	/* Close the connection (server closes in http) */
	netconn_close(conn);

	/* Delete the buffer (netconn_recv gives us ownership,
   so we have to make sure to deallocate the buffer) */
	netbuf_delete(inbuf);
}

static void http_thread(void *arg)
{
  struct netconn *conn, *newconn;
  err_t err, accept_err;

  /* Create a new TCP connection handle */
  conn = netconn_new(NETCONN_TCP);

  if (conn!= NULL)
  {
    /* Bind to port 80 (http) with default IP address */
    err = netconn_bind(conn, IP_ADDR_ANY, 80);

    if (err == ERR_OK)
    {
      /* Put the connection into LISTEN state */
      netconn_listen(conn);

      while(1)
      {
        /* accept any incoming connection */
        accept_err = netconn_accept(conn, &newconn);
        if(accept_err == ERR_OK)
        {
          /* serve connection */
          http_server(newconn);

          /* delete connection */
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
