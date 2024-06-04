/*
 * HiSLIP.c
 *
 *  Created on: Jun 3, 2024
 *      Author: grzegorz
 */


 #include <string.h>
// #include "stddef.h"
#include "cmsis_os.h"
#include "udp.h"
#include "api.h"
#include "def.h"

#include "HiSLIP.h"

#define HISLIP_THREAD_STACKSIZE		2048
#define HISLIP_PORT					4880

#define	NETCONN_ACCEPT_ON			(u8_t)1
#define	NETCONN_ACCEPT_OFF			(u8_t)0


typedef struct {
	struct netconn* newconn;
}hislip_netconn_t;

typedef struct
{
	char data[HISLIP_MAX_DATA_SIZE + sizeof(hislip_msg_t)];
	u16_t len;
}hislip_netbuf_t;

typedef struct {
	hislip_msg_t msg;
	hislip_netbuf_t netbuf;
	hislip_netconn_t netconn;
	u16_t session_id;
}hislip_instr_t;


static u8_t sync_task_running = 0;
static	u8_t async_task_runnig = 0;


hislip_task_control_t hislip_task_control;

TaskHandle_t hislip_handler;
uint32_t hislip_buffer[HISLIP_THREAD_STACKSIZE];
StaticTask_t hislip_control_block;

TaskHandle_t hislip_sync_handler;
uint32_t hislip_sync_buffer[HISLIP_THREAD_STACKSIZE];
StaticTask_t hislip_sync_control_block;

TaskHandle_t hislip_async_handler;
uint32_t hislip_async_buffer[HISLIP_THREAD_STACKSIZE];
StaticTask_t hislip_async_control_block;

xQueueHandle hislip_queue;


static hislip_msg_t hislip_MsgParser(hislip_instr_t* hislip_instr);

// ----------------------------------------------------------------------------

static void hislip_htonl(hislip_msg_t* hislip_msg)
{
	hislip_msg->msg_param = htonl(hislip_msg->msg_param);
	hislip_msg->prologue = htons(hislip_msg->prologue);
	hislip_msg->payload_len.hi = htonl(hislip_msg->payload_len.hi);
	hislip_msg->payload_len.lo = htonl(hislip_msg->payload_len.lo);
}

size_t hislip_SumSize(size_t* sizes, size_t len)
{
	size_t sum = 0;

	for(u_char i = 0; i < len; i++)
	{
		sum += sizes[i];
	}

	return sum;
}

void hislip_CopyMemory(char* destination, void** sources, size_t* sizes, u32_t num_sources)
{
    size_t offset = 0;
    for (u32_t i = 0; i < num_sources; i++)
    {
        memcpy(destination + offset, sources[i], sizes[i]);
        offset += sizes[i];
    }
}

// ----------------------------------------------------------------------------


err_t hislip_DataEnd(hislip_instr_t* hislip_instr)
{
	err_t err = ERR_OK;

	hislip_msg_t msg_rx, msg_tx;
	payload_len_t max_msg_size;

	char data[] = "ABCad,123123,432,123213\n";

	void* sources[] = {&msg_tx, &data};
	size_t sizes[] = {sizeof(hislip_msg_t), strlen(data)};

	msg_rx = hislip_MsgParser(hislip_instr);

	size_t sum = hislip_SumSize(sizes, 2);

	char payload[sum];

	msg_tx.prologue = HISLIP_PROLOGUE;
	msg_tx.msg_type = HISLIP_DATAEND;
	msg_tx.control_code = 0x00;
	msg_tx.msg_param = msg_rx.msg_param;
	msg_tx.payload_len.hi = 0;
	msg_tx.payload_len.lo = strlen(data);

	hislip_htonl(&msg_tx);

	hislip_CopyMemory(payload, sources, sizes, 2);

	vTaskDelay(pdMS_TO_TICKS(1));
	err = netconn_write(hislip_instr->netconn.newconn, payload, sum, NETCONN_NOFLAG);

	return err;
}

err_t hislip_AsyncMaximumMessageSize(hislip_instr_t* hislip_instr)
{
	err_t err = ERR_OK;
	hislip_msg_t msg_tx;
	payload_len_t max_msg_size;


	void* sources[] = {&msg_tx, &max_msg_size};
	size_t sizes[] = {sizeof(hislip_msg_t), sizeof(payload_len_t)};

	size_t sum = hislip_SumSize(sizes, 2);

	char payload[sum];

	msg_tx.prologue = HISLIP_PROLOGUE;
	msg_tx.msg_type = HISLIP_ASYNCMAXIMUMMESSAGESIZERESPONSE;
	msg_tx.control_code = 0x00;
	msg_tx.msg_param = 0x00000000;
	msg_tx.payload_len.hi = 0;
	msg_tx.payload_len.lo = 8;

	max_msg_size.hi = 0;
	max_msg_size.lo = 1024;

	hislip_htonl(&msg_tx);

	hislip_CopyMemory(payload, sources, sizes, 2);

	vTaskDelay(pdMS_TO_TICKS(1));
	err = netconn_write(hislip_instr->netconn.newconn, payload, sum, NETCONN_NOFLAG);

	return err;
}

err_t hislip_AsyncInitialize(hislip_instr_t* hislip_instr)
{
	err_t err = ERR_OK;
	hislip_msg_t msg_rx, msg_tx;

	msg_rx = hislip_MsgParser(hislip_instr);

	hislip_instr->session_id = msg_rx.msg_param;

	msg_tx.prologue = HISLIP_PROLOGUE;
	msg_tx.msg_type = HISLIP_ASYNCINITIALIZERESPONSE;
	msg_tx.control_code = 0x00;
	msg_tx.msg_param = HISLIP_VENDOR_ID;
	msg_tx.payload_len.hi = 0;
	msg_tx.payload_len.lo = 0;

	hislip_htonl(&msg_tx);

	vTaskDelay(pdMS_TO_TICKS(1));
	err = netconn_write(hislip_instr->netconn.newconn, &msg_tx, sizeof(hislip_msg_t), NETCONN_NOFLAG);

	return err;
}

err_t hislip_Initialize(hislip_instr_t* hislip_instr)
{
	err_t err = ERR_OK;
	char name_ref[] = "hislip0";
	char name_rx[12];
	char* data;
	hislip_msg_t msg_rx, msg_tx;


	memset(name_rx, 0, 12);

	msg_rx = hislip_MsgParser(hislip_instr);

	data = &hislip_instr->netbuf.data;

	memcpy(name_rx, data + sizeof(hislip_msg_t), msg_rx.payload_len.lo);

	if(!strcmp(name_ref, name_rx))
	{
		msg_tx.prologue = HISLIP_PROLOGUE;
		msg_tx.msg_type = HISLIP_INITIALIZERESPONSE;
		msg_tx.control_code = 0x00;
		msg_tx.msg_param = 0x01000001;
		msg_tx.payload_len.hi = 0;
		msg_tx.payload_len.lo = 0;

		hislip_htonl(&msg_tx);

		vTaskDelay(pdMS_TO_TICKS(1));
		err = netconn_write(hislip_instr->netconn.newconn, &msg_tx, sizeof(hislip_msg_t), NETCONN_NOFLAG);
	}

	return err;
}

// ----------------------------------------------------------------------------

static void hislip_Callback(struct netconn *conn, enum netconn_evt even, u16_t len)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if(NETCONN_EVT_RCVPLUS == even)
	{

		static u32_t test = 1;
		xQueueSendFromISR(hislip_queue, &test, &xHigherPriorityTaskWoken);

	}
}

static struct netconn*  hislip_Bind(u16_t port)
{

	struct netconn* conn;
	err_t err;

	conn = netconn_new(NETCONN_TCP);
	err = netconn_bind(conn, IP_ADDR_ANY, port);
	err = netconn_listen(conn);

#if LWIP_SO_RCVTIMEO == 1
	netconn_set_recvtimeout(conn, 1000);
#endif

	return conn;
}


static hislip_msg_t hislip_MsgParser(hislip_instr_t* hislip_instr)
{
	hislip_msg_t hislip_msg;


	size_t hislip_msg_size = sizeof(hislip_msg_t);

	memcpy(&hislip_msg, hislip_instr->netbuf.data, hislip_msg_size);


	hislip_htonl(&hislip_msg);

	return hislip_msg;
}

static hislip_msg_type_t hislip_Recv(hislip_instr_t* hislip_instr)
{
	err_t err;
	struct netbuf* buf;
	void* data;
	u16_t len = 0;
	u16_t offset = 0;
	hislip_msg_t hislip_msg;

	hislip_instr->netbuf.len = 0;

#if LWIP_SO_RCVTIMEO == 1
	netconn_set_recvtimeout(hislip_instr->netconn.newconn, 100);
#endif

	err = netconn_recv(hislip_instr->netconn.newconn, &buf);

	if(ERR_OK == err)
	{
		do
		{
			netbuf_data(buf, &data, &len);
			hislip_instr->netbuf.len +=len;

			if(hislip_instr->netbuf.len <= sizeof(hislip_instr->netbuf.data))
			{
				memcpy(hislip_instr->netbuf.data + offset, data, len);

			}
			else
			{

			}
			offset += len;


		}
		while(netbuf_next(buf) >= 0);

		netbuf_delete(buf);

		hislip_msg = hislip_MsgParser(hislip_instr);
	}
	else
	{
		netbuf_delete(buf);

		netconn_close(hislip_instr->netconn.newconn);
		netconn_delete(hislip_instr->netconn.newconn);


		return HISLIP_CONN_ERR;
	}

	return (hislip_msg_type_t)hislip_msg.msg_type;

}

static err_t hislip_Accept(hislip_instr_t* hislip_instr)
{
	struct netconn *newconn;
	err_t err = ERR_OK;

	if(NETCONN_ACCEPT_OFF == hislip_instr->netconn.accept)
	{
		err = netconn_accept(hislip_instr->netconn.conn, &newconn);

		if(ERR_OK == err)
		{
			hislip_instr->netconn.newconn = newconn;
			hislip_instr->netconn.accept = NETCONN_ACCEPT_ON;

			#if LWIP_SO_RCVTIMEO == 1
				netconn_set_recvtimeout(hislip_instr->netconn.newconn, 1000);
			#endif
		}
	}
	else
	{
		hislip_instr->netconn.accept = NETCONN_ACCEPT_OFF;
		err = ERR_OK;
	}

	return err;
}

static void hislip_Init(hislip_instr_t* hislip_instr)
{
	hislip_instr->netconn.accept = NETCONN_ACCEPT_OFF;
}


static void hislip_SyncTask(void  *arg)
{
	hislip_instr_t hislip_instr;
	hislip_Init(&hislip_instr);

	hislip_instr.netconn.newconn = (struct netconn*)arg;

	for (;;)
	{
		switch(hislip_Recv(&hislip_instr))
		{
			case Initialize : hislip_Initialize(&hislip_instr);break;
	//		case AsyncInitialize : hislip_AsyncInitialize(&hislip_instr); break;
	//		case AsyncMaximumMessageSize : hislip_AsyncMaximumMessageSize(&hislip_instr);break;
			case DataEnd : hislip_DataEnd(&hislip_instr); break;

			case HISLIP_CONN_ERR :
				{
					sync_task_running = false;
					vTaskDelete(NULL);
				}; break;

			default : vTaskDelay(pdMS_TO_TICKS(2)); break;
		}
		vTaskDelay(pdMS_TO_TICKS(2));
	}

}

static void hislip_aSyncTask(void  *arg)
{
	hislip_instr_t hislip_instr;
	hislip_Init(&hislip_instr);

	hislip_instr.netconn.newconn = (struct netconn*)arg;

	for (;;)
	{
		switch(hislip_Recv(&hislip_instr))
		{
		//	case Initialize : hislip_Initialize(&hislip_instr);break;
			case AsyncInitialize : hislip_AsyncInitialize(&hislip_instr); break;
			case AsyncMaximumMessageSize : hislip_AsyncMaximumMessageSize(&hislip_instr);break;
			case DataEnd : hislip_DataEnd(&hislip_instr); break;

			case HISLIP_CONN_ERR :
				{
					async_task_running = false;
					vTaskDelete(NULL);
				}; break;

			default : vTaskDelay(pdMS_TO_TICKS(2)); break;
		}
		vTaskDelay(pdMS_TO_TICKS(2));
	}

}

static void hislip_ServerTask(void const *argument)
{
	err_t err;
	struct netconn* newconn;
	struct netconn* conn = hislip_Bind(HISLIP_PORT);

	for (;;)
	{
		err = netconn_accept(conn, &newconn);

		if(ERR_OK == err)
		{
			if(!sync_task_running)
			{
				hislip_sync_handler = xTaskCreateStatic(hislip_SyncTask,"hislip_SyncTask",
						HISLIP_THREAD_STACKSIZE, (void*)newconn, tskIDLE_PRIORITY + 2,
						hislip_sync_buffer, &hislip_sync_control_block);

				sync_task_running = true;

			}

			if(!async_task_running)
			{
				hislip_async_handler = xTaskCreateStatic(hislip_aSyncTask,"hislip_aSyncTask",
						HISLIP_THREAD_STACKSIZE, (void*)newconn, tskIDLE_PRIORITY + 2,
						hislip_async_buffer, &hislip_async_control_block);

				async_task_running = true;
			}
		}

	}
}



/*
static void hislip_Task(void const *argument)
{

	hislip_instr_t hislip_instr;
	hislip_Init(&hislip_instr);

	u32_t test;

	hislip_instr.netconn.conn = hislip_Bind(HISLIP_PORT);

	for (;;)
	{
		if(pdTRUE == xQueueReceive(hislip_queue, &test, portMAX_DELAY))
		{
			if(ERR_OK == hislip_Accept(&hislip_instr))
			{
				switch(hislip_Recv(&hislip_instr))
				{
					case Initialize : hislip_Initialize(&hislip_instr);break;
					case AsyncInitialize : hislip_AsyncInitialize(&hislip_instr); break;
					case AsyncMaximumMessageSize : hislip_AsyncMaximumMessageSize(&hislip_instr);break;
					case DataEnd : hislip_DataEnd(&hislip_instr); break;
					default : vTaskDelay(pdMS_TO_TICKS(2)); break;
				}
			}
			vTaskDelay(pdMS_TO_TICKS(2));
		}
	}
}
*/

void hislip_CreateTask(void)
{

	hislip_handler = xTaskCreateStatic(hislip_ServerTask,"hislip_ServerTask",
			HISLIP_THREAD_STACKSIZE, (void*)1, tskIDLE_PRIORITY + 2,
			hislip_buffer, &hislip_control_block);

}
