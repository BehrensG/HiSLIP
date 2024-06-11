/*
 * SCPI_Task.c
 *
 *  Created on: Jun 2, 2024
 *      Author: grzegorz
 */

#include "scpi/scpi.h"
#include "SCPI_Def.h"

#include "cmsis_os.h"
#include "HiSLIP.h"
#include "api.h"

#define PLAYLOAD_BUFFER			32
#define PAYLOAD_SIZE			sizeof(hislip_msg_t) + HISLIP_MAX_DATA_SIZE + PLAYLOAD_BUFFER + 2

static char payload[PAYLOAD_SIZE];
static size_t payload_sum = 0;
static uint32_t payload_full_count = 0;

typedef enum {
	SCPI_WRITE_END,
	SCPI_WRITE_FULL_DATA
}scpi_write_state_t;

static scpi_write_state_t scpi_write_state = SCPI_WRITE_END;

static size_t SCPI_WriteEnd(hislip_instr_t* hislip_instr, char* payload, size_t* payload_sum, uint32_t* payload_full_count)

{
	hislip_msg_t msg;
	size_t len = 0;
	size_t offset = 0;
	size_t header_size = 0;

	if(0 == *payload_full_count)
	{
		hislip_DataHeader(hislip_instr, &msg, HISLIP_DATAEND, *payload_sum + strlen(HISLIP_LINE_ENDING));

		memcpy(payload, &msg, sizeof(hislip_msg_t));
		memcpy(payload + sizeof(hislip_msg_t) + *payload_sum, HISLIP_LINE_ENDING, strlen(HISLIP_LINE_ENDING));
		offset = 0;
		header_size = sizeof(hislip_msg_t);
	}
	else
	{

		memcpy(payload + sizeof(hislip_msg_t) + *payload_sum, HISLIP_LINE_ENDING, strlen(HISLIP_LINE_ENDING));
		offset = sizeof(hislip_msg_t);
		header_size = 0;
	}


	vTaskDelay(pdMS_TO_TICKS(1));
	if(ERR_OK == netconn_write(hislip_instr->netconn.newconn, payload + offset,
			header_size + *payload_sum + strlen(HISLIP_LINE_ENDING), NETCONN_COPY))
	{
		len = 0;
	}
	else
	{
		len = *payload_sum;
	}

	*payload_sum = 0;
	*payload_full_count = 0;

	memset(payload,0, PAYLOAD_SIZE);

	return len;


}


static size_t SCPI_WriteSegment(hislip_instr_t* hislip_instr, char* payload, size_t* payload_sum, uint32_t* payload_full_count)

{
	size_t len = 0;
	size_t offset = 0;
	size_t header_size = 0;

	if(1 == *payload_full_count) // by first full segment also add header
	{
		hislip_msg_t msg;

		hislip_DataHeader(hislip_instr, &msg, HISLIP_DATAEND, HISLIP_MAX_DATA_SIZE);
		memcpy(payload, &msg, sizeof(hislip_msg_t));

		offset = 0 ;
		header_size = sizeof(hislip_msg_t);
	}
	else
	{
		offset = sizeof(hislip_msg_t) ;
		header_size = 0;
	}

	vTaskDelay(pdMS_TO_TICKS(1));
	if(ERR_OK == netconn_write(hislip_instr->netconn.newconn, payload + offset, header_size + HISLIP_MAX_DATA_SIZE, NETCONN_COPY))
	{
		len = 0;
	}

	*payload_sum = *payload_sum % HISLIP_MAX_DATA_SIZE;

	return len;


}


size_t SCPI_WriteHiSLIPV2(scpi_t * context, const char * data, size_t len) {

	hislip_instr_t* hislip_instr = (hislip_instr_t*)context->user_context;

	if(NULL != hislip_instr)
	{

		bool end = ((len == strlen(SCPI_LINE_ENDING)) && (!strcmp(data, SCPI_LINE_ENDING)));

		memcpy(payload + sizeof(hislip_msg_t) + payload_sum, data, len);
		payload_sum += len;

		if(end)
		{
			scpi_write_state = SCPI_WRITE_END;
		}
		else if(HISLIP_MAX_DATA_SIZE <= payload_sum)
		{
			scpi_write_state = SCPI_WRITE_FULL_DATA;
			payload_full_count++;
		}

		switch(scpi_write_state)
		{
			case SCPI_WRITE_END : SCPI_WriteEnd(hislip_instr,payload, &payload_sum, &payload_full_count); break;
			case SCPI_WRITE_FULL_DATA : SCPI_WriteSegment(hislip_instr, payload, &payload_sum, &payload_full_count) ; break;
			default : len = 0; break;
		}

	}

    return len;
}



size_t SCPI_WriteHiSLIP(scpi_t * context, const char * data, size_t len) {

	hislip_instr_t* hislip_instr = (hislip_instr_t*)context->user_context;

	if(NULL != hislip_instr)
	{
		hislip_msg_t msg_tx;

		bool tail = ((len == strlen(SCPI_LINE_ENDING)) && (!strcmp(data, SCPI_LINE_ENDING)));

		if(!tail)
		{
			memcpy(payload + sizeof(hislip_msg_t) + payload_sum, data, len);
			payload_sum += len;
		}


		if(tail)
		{

			hislip_DataHeader(hislip_instr, &msg_tx, HISLIP_DATAEND, payload_sum + strlen(HISLIP_LINE_ENDING));

			memcpy(payload, &msg_tx, sizeof(hislip_msg_t));

			memcpy(payload + sizeof(hislip_msg_t) + payload_sum, HISLIP_LINE_ENDING, strlen(HISLIP_LINE_ENDING));

			vTaskDelay(pdMS_TO_TICKS(1));
			if(ERR_OK == netconn_write(hislip_instr->netconn.newconn, payload,
					sizeof(hislip_msg_t) + payload_sum + strlen(HISLIP_LINE_ENDING), NETCONN_COPY))
			{
				len = 0;
			}

			payload_sum = 0;
			memset(payload,0, PAYLOAD_SIZE);
			payload_full_count = 0;

		}
		else if(HISLIP_MAX_DATA_SIZE <= payload_sum) // payload full
		{


			hislip_DataHeader(hislip_instr, &msg_tx, HISLIP_DATAEND, HISLIP_MAX_DATA_SIZE);

			memcpy(payload, &msg_tx, sizeof(hislip_msg_t));

			vTaskDelay(pdMS_TO_TICKS(1));
			if(ERR_OK == netconn_write(hislip_instr->netconn.newconn, payload, HISLIP_MAX_DATA_SIZE, NETCONN_COPY))
			{
				len = 0;
			}

			payload_sum -= HISLIP_MAX_DATA_SIZE;
			memcpy(payload + sizeof(hislip_msg_t), data, payload_sum);

		}
	}

    return len;
}

scpi_result_t SCPI_Flush(scpi_t * context) {

    return SCPI_RES_OK;
}

int SCPI_Error(scpi_t * context, int_fast16_t err) {

    return 0;
}

scpi_result_t SCPI_Control(scpi_t * context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val) {

    return SCPI_RES_OK;
}

scpi_result_t SCPI_Reset(scpi_t * context) {

    (void) context;

    return SCPI_RES_OK;
}


void SCPI_RequestControl(void) {

}

void SCPI_AddError(int16_t err) {
}



