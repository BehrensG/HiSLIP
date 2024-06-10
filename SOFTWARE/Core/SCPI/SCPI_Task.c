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

size_t SCPI_Write(scpi_t * context, const char * data, size_t len) {

	hislip_instr_t* hislip_instr = (hislip_instr_t*)context->user_context;

	if(NULL != hislip_instr)
	{
		hislip_msg_t msg_tx;

		bool is_tail = ((len == strlen(SCPI_LINE_ENDING)) && (!strcmp(data, SCPI_LINE_ENDING)));

		if(!is_tail)
		{
			memcpy(payload + sizeof(hislip_msg_t) + payload_sum, data, len);
			payload_sum += len;
		}


		if(is_tail)
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

		}
		else if(HISLIP_MAX_DATA_SIZE <= payload_sum) // payload full
		{

			hislip_DataHeader(hislip_instr, &msg_tx, HISLIP_DATA, payload_sum);

			memcpy(payload, &msg_tx, sizeof(hislip_msg_t));

			vTaskDelay(pdMS_TO_TICKS(1));
			if(ERR_OK == netconn_write(hislip_instr->netconn.newconn, payload, payload_sum, NETCONN_COPY))
			{
				len = 0;
			}

			payload_sum = 0;
			memset(payload,0, PAYLOAD_SIZE);

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



