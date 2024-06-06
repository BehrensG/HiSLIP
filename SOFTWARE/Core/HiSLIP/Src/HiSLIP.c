/*
 * HiSLIP.c
 *
 *  Created on: Jun 3, 2024
 *      Author: grzegorz
 */


 #include <string.h>
#include <stdbool.h>
// #include "stddef.h"
#include "cmsis_os.h"
#include "udp.h"
#include "api.h"
#include "def.h"

#include "HiSLIP.h"
#include "SCPI_Def.h"

#define HISLIP_THREAD_STACKSIZE		2048
#define HISLIP_PORT					4880

#define	NETCONN_ACCEPT_ON			(u8_t)1
#define	NETCONN_ACCEPT_OFF			(u8_t)0


static	u8_t task_count = 0;


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


static void hislip_Init(hislip_instr_t* hislip_instr)
{
	memset(hislip_instr->end, 0, sizeof(hislip_instr->end));
}


void hislip_htonl(hislip_msg_t* hislip_msg)
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

void hislip_CopyMemory(char* destination, void** sources, size_t* sizes, uint32_t num_sources)
{
    size_t offset = 0;
    for (uint32_t i = 0; i < num_sources; i++)
    {
        memcpy(destination + offset, sources[i], sizes[i]);
        offset += sizes[i];
    }
}

// ----------------------------------------------------------------------------

int8_t hislip_Trigger(hislip_instr_t* hislip_instr)
{

	hislip_msg_t msg_rx;

	char scpi_trigger[] = "*TRG";
	char scpi_data[6];

	memset(scpi_data, 0, sizeof(scpi_data));

	memcpy(scpi_data, scpi_trigger, strlen(scpi_trigger));
	memcpy(scpi_data + strlen(scpi_trigger), SCPI_LINE_ENDING, strlen(SCPI_LINE_ENDING));

	msg_rx = hislip_MsgParser(hislip_instr);

	return SCPI_Input(&scpi_context, scpi_data, strlen(scpi_data));
}


int8_t hislip_DataEnd(hislip_instr_t* hislip_instr)
{

	hislip_msg_t msg_rx, msg_tx;

	char* buf;
	char* end;

	static const char* ends[3] = {LINE_ENDING_CR, LINE_ENDING_LF, LINE_ENDING_CRLF};

	msg_rx = hislip_MsgParser(hislip_instr);

	buf = &hislip_instr->netbuf.data[sizeof(hislip_msg_t)];

	for(u8_t i = 0; i < 3; i++)
	{
		end = strstr(buf, ends[i]);
		if(NULL != end)
		{
			memset(hislip_instr->end, 0, sizeof(hislip_instr->end));
			memcpy(hislip_instr->end,ends[i],strlen(ends[i]));
			break;
		}

	}

	if(NULL != end)
	{
		memcpy(end, SCPI_LINE_ENDING, strlen(SCPI_LINE_ENDING));
		msg_rx.payload_len.lo -= strlen(SCPI_LINE_ENDING);

	}
	else
	{
		memcpy(buf + msg_rx.payload_len.lo, SCPI_LINE_ENDING, strlen(SCPI_LINE_ENDING));
	}

	memcpy(&hislip_instr->msg, &msg_rx, sizeof(msg_rx));

	if(!strcmp(buf,"?")) // is it a query
	{
		return SCPI_Input(&scpi_context, buf, msg_rx.payload_len.lo + strlen(SCPI_LINE_ENDING));
	}
	else
	{
		msg_tx.prologue = HISLIP_PROLOGUE;
		msg_tx.msg_type = HISLIP_DATAEND;
		msg_tx.control_code = 0x00;
		msg_tx.msg_param = msg_rx.msg_param;
		msg_tx.payload_len.hi = 0;
		msg_tx.payload_len.lo = 0;

		SCPI_Input(&scpi_context, buf, msg_rx.payload_len.lo + strlen(SCPI_LINE_ENDING));
		return netconn_write(hislip_instr->netconn.newconn, &msg_tx, sizeof(hislip_msg_t), NETCONN_NOFLAG);
	}
}


int8_t hislip_AsyncMaximumMessageSize(hislip_instr_t* hislip_instr)
{
	int8_t err = ERR_OK;
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


int8_t hislip_AsyncInitialize(hislip_instr_t* hislip_instr)
{
	int8_t err = ERR_OK;
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


int8_t hislip_AsyncStatusQuery(hislip_instr_t* hislip_instr)
{
	int8_t err = ERR_OK;

	hislip_msg_t msg_rx, msg_tx;

	msg_rx = hislip_MsgParser(hislip_instr);


	msg_tx.prologue = HISLIP_PROLOGUE;
	msg_tx.msg_type = HISLIP_ASYNCSTATUSRESPONSE;
	msg_tx.control_code = 0x00;
	msg_tx.msg_param = 0x00000000;
	msg_tx.payload_len.hi = 0;
	msg_tx.payload_len.lo = 0;

	hislip_htonl(&msg_tx);

	vTaskDelay(pdMS_TO_TICKS(1));
	err = netconn_write(hislip_instr->netconn.newconn, &msg_tx, sizeof(hislip_msg_t), NETCONN_NOFLAG);


	return err;
}


int8_t hislip_Initialize(hislip_instr_t* hislip_instr)
{
	int8_t err = ERR_OK;
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

int8_t hislip_AsyncDeviceClear(hislip_instr_t* hislip_instr)
{
	int8_t err = ERR_OK;
	hislip_msg_t msg_rx, msg_tx;

	msg_rx = hislip_MsgParser(hislip_instr);

	hislip_instr->session_id = msg_rx.msg_param;

	msg_tx.prologue = HISLIP_PROLOGUE;
	msg_tx.msg_type = HISLIP_ASYNCINITIALIZERESPONSE;
	msg_tx.control_code = 0x00;
	msg_tx.msg_param = 0x00000000;
	msg_tx.payload_len.hi = 0;
	msg_tx.payload_len.lo = 0;

	hislip_htonl(&msg_tx);

	vTaskDelay(pdMS_TO_TICKS(1));
	err = netconn_write(hislip_instr->netconn.newconn, &msg_tx, sizeof(hislip_msg_t), NETCONN_NOFLAG);

	return err;
}

// ----------------------------------------------------------------------------

static void hislip_Callback(struct netconn *conn, enum netconn_evt even, uint16_t len)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if(NETCONN_EVT_RCVPLUS == even)
	{

		static uint32_t test = 1;
		xQueueSendFromISR(hislip_queue, &test, &xHigherPriorityTaskWoken);

	}
}


static struct netconn*  hislip_Bind(uint16_t port)
{

	struct netconn* conn;
	int8_t err;

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
	int8_t err;
	struct netbuf* buf;
	void* data;
	uint16_t len = 0;
	uint16_t offset = 0;
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


static void hislip_SyncTask(void  *arg)
{
	hislip_instr_t hislip_instr;
	hislip_Init(&hislip_instr);

	hislip_instr.netconn.newconn = (struct netconn*)arg;

    SCPI_Init(&scpi_context,
            scpi_commands,
            &scpi_interface,
            scpi_units_def,
            SCPI_IDN1, SCPI_IDN2, SCPI_IDN3, SCPI_IDN4,
            scpi_input_buffer, SCPI_INPUT_BUFFER_LENGTH,
            scpi_error_queue_data, SCPI_ERROR_QUEUE_SIZE);

    scpi_context.user_context = (void*)&hislip_instr;

	for (;;)
	{
		switch(hislip_Recv(&hislip_instr))
		{
			case Initialize : hislip_Initialize(&hislip_instr);break;
	//		case AsyncInitialize : hislip_AsyncInitialize(&hislip_instr); break;
	//		case AsyncMaximumMessageSize : hislip_AsyncMaximumMessageSize(&hislip_instr);break;
			case DataEnd : hislip_DataEnd(&hislip_instr); break;
			case Trigger : hislip_Trigger(&hislip_instr); break;

			case HISLIP_CONN_ERR :
				{
					if(task_count)
					{
						task_count--;
					}

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
			case AsyncStatusQuery : hislip_AsyncStatusQuery(&hislip_instr); break;
			case AsyncDeviceClear : hislip_AsyncDeviceClear(&hislip_instr); break;
			case DataEnd : hislip_DataEnd(&hislip_instr); break;

			case HISLIP_CONN_ERR :
				{
					if(task_count)
					{
						task_count--;
					}

					vTaskDelete(NULL);

				}; break;

			default : vTaskDelay(pdMS_TO_TICKS(2)); break;
		}
		vTaskDelay(pdMS_TO_TICKS(2));
	}

}

static void hislip_ServerTask(void const *argument)
{
	int8_t err;
	struct netconn* newconn;
	struct netconn* conn = hislip_Bind(HISLIP_PORT);

	for (;;)
	{
		err = netconn_accept(conn, &newconn);

		if(ERR_OK == err)
		{
			if(0 == task_count)
			{
				hislip_sync_handler = xTaskCreateStatic(hislip_SyncTask,"hislip_SyncTask",
						HISLIP_THREAD_STACKSIZE, (void*)newconn, tskIDLE_PRIORITY + 2,
						hislip_sync_buffer, &hislip_sync_control_block);

				task_count++;

			}

			else if(1 == task_count)
			{
				hislip_async_handler = xTaskCreateStatic(hislip_aSyncTask,"hislip_aSyncTask",
						HISLIP_THREAD_STACKSIZE, (void*)newconn, tskIDLE_PRIORITY + 2,
						hislip_async_buffer, &hislip_async_control_block);

				task_count++;
			}
		}

	}
}

void hislip_CreateTask(void)
{

	hislip_handler = xTaskCreateStatic(hislip_ServerTask,"hislip_ServerTask",
			HISLIP_THREAD_STACKSIZE, (void*)1, tskIDLE_PRIORITY + 2,
			hislip_buffer, &hislip_control_block);

}
