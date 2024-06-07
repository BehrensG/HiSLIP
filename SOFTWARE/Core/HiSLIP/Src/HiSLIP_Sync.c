/*
 * HiSLIP_Sync.c
 *
 *  Created on: Jun 7, 2024
 *      Author: BehrensG
 */


#include "api.h"
#include "HiSLIP.h"

#include "SCPI_Def.h"


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



int8_t hislip_Data(hislip_instr_t* hislip_instr)
{
	//TBD
	return 0;
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




int8_t hislip_InitializeResponse(hislip_instr_t* hislip_instr)
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

int8_t hislip_DeviceClearAcknowledge(hislip_instr_t* hislip_instr)
{
	int8_t err = ERR_OK;
	//TBD
	return err;
}


int8_t hislip_Interrupted(hislip_instr_t* hislip_instr)
{
	int8_t err = ERR_OK;
	//TBD
	return err;
}
