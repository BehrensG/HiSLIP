/*-
 * Copyright (c) 2012-2013 Jan Breuer,
 *
 * All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file   scpi-def.c
 * @date   Thu Nov 15 10:58:45 UTC 2012
 *
 * @brief  SCPI parser test
 *
 *
 */

//#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmsis_os.h>

#include "api.h"
#include "main.h"
#include "scpi/scpi.h"
#include "SCPI_Def.h"
#include "HiSLIP.h"

scpi_choice_def_t scpi_boolean_select[] =
{
    {"OFF", 0},
    {"ON", 1},
	{"0", 0},
	{"1", 1},
    SCPI_CHOICE_LIST_END
};



static scpi_result_t SCPI_Rst(scpi_t * context)
{
	HAL_NVIC_SystemReset();
    return SCPI_RES_OK;
}

static err_t SCPI_NetconnWrite(hislip_instr_t* hislip_instr, const char* payload, size_t len)
{
	vTaskDelay(pdMS_TO_TICKS(1));
	return netconn_write(hislip_instr->netconn.newconn, payload, len, NETCONN_NOFLAG);

}


static scpi_result_t SCPI_IdnQ(scpi_t * context)
{
	hislip_instr_t* hislip_instr = (hislip_instr_t*)context->user_context;
	hislip_msg_t msg;

	size_t header_size = sizeof(hislip_msg_t);

	int32_t ptr = 0;
	char info[header_size + 46];

    for (uint8_t i = 0; i < 4; i++)
    {
        if (context->idn[i])
        {
        	if(3 == i)
        	{
        		ptr += snprintf(info + header_size + ptr, sizeof(info) - header_size - ptr, "%s", context->idn[i]);
        	}
        	else
        	{
        		ptr += snprintf(info + header_size + ptr, sizeof(info) - header_size - ptr, "%s,", context->idn[i]);
        	}
        }

    }


    hislip_DataHeader(hislip_instr, &msg, HISLIP_DATAEND, ptr + strlen(HISLIP_LINE_ENDING));
	memcpy(info, &msg, sizeof(hislip_msg_t));

	memcpy(info + header_size + ptr, HISLIP_LINE_ENDING, strlen(HISLIP_LINE_ENDING));

	SCPI_NetconnWrite(hislip_instr, info, ptr + header_size + strlen(HISLIP_LINE_ENDING));

    return SCPI_RES_OK;
}


#define TEX_SIZE	256

scpi_result_t SCPI_SystemErrorQ(scpi_t * context)
{
	char text[TEX_SIZE];
	const char * data;
	scpi_error_t error;
	int len;

	SCPI_ErrorPop(context, &error);
	data = SCPI_ErrorTranslate(error.error_code);
	len = snprintf(text, TEX_SIZE,"%d, %s",error.error_code, data);

	SCPI_ResultCharacters(context, text, len);

	return SCPI_RES_OK;
}

float test[10000];

scpi_result_t SCPI_MeasQ(scpi_t * context)
{

	uint32_t value;

	if(!SCPI_ParamUInt32(context, &value, true))
	{
		return SCPI_RES_ERR;
	}

	for(uint32_t i = 0; i < value; i++)
	{
		test[i] = i*0.0001;
	}

	SCPI_ResultArrayFloat(context, test, value, SCPI_FORMAT_ASCII);

    return SCPI_RES_OK;
}

const scpi_command_t scpi_commands[] = {
    /* IEEE Mandated Commands (SCPI std V1999.0 4.1.1) */
    { .pattern = "*CLS", .callback = SCPI_CoreCls,},
    { .pattern = "*ESE", .callback = SCPI_CoreEse,},
    { .pattern = "*ESE?", .callback = SCPI_CoreEseQ,},
    { .pattern = "*ESR?", .callback = SCPI_CoreEsrQ,},
    { .pattern = "*IDN?", .callback = SCPI_IdnQ,},
    { .pattern = "*OPC", .callback = SCPI_CoreOpc,},
    { .pattern = "*OPC?", .callback = SCPI_CoreOpcQ,},
    { .pattern = "*RST", .callback = SCPI_Rst,},
    { .pattern = "*SRE", .callback = SCPI_CoreSre,},
    { .pattern = "*SRE?", .callback = SCPI_CoreSreQ,},
    { .pattern = "*STB?", .callback = SCPI_CoreStbQ,},
    { .pattern = "*TST?", .callback = SCPI_CoreTstQ,},
    { .pattern = "*WAI", .callback = SCPI_CoreWai,},

    {.pattern = "STATus:QUEStionable[:EVENt]?", .callback = SCPI_StatusQuestionableEventQ,},
    /* {.pattern = "STATus:QUEStionable:CONDition?", .callback = scpi_stub_callback,}, */
    {.pattern = "STATus:QUEStionable:ENABle", .callback = SCPI_StatusQuestionableEnable,},
    {.pattern = "STATus:QUEStionable:ENABle?", .callback = SCPI_StatusQuestionableEnableQ,},

    {.pattern = "STATus:PRESet", .callback = SCPI_StatusPreset,},

    /* Required SCPI commands (SCPI std V1999.0 4.2.1) */
    {.pattern = "SYSTem:ERRor[:NEXT]?", .callback = SCPI_SystemErrorQ,},
    {.pattern = "SYSTem:ERRor:COUNt?", .callback = SCPI_SystemErrorCountQ,},
    {.pattern = "SYSTem:VERSion?", .callback = SCPI_SystemVersionQ,},

	{.pattern = "Meas?", .callback = SCPI_MeasQ,},



	SCPI_CMD_LIST_END
};

scpi_interface_t scpi_interface = {
    .error = SCPI_Error,
    .write = SCPI_WriteHiSLIP,
    .control = SCPI_Control,
    .flush = SCPI_Flush,
    .reset = SCPI_Reset,
};

char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];
scpi_error_t scpi_error_queue_data[SCPI_ERROR_QUEUE_SIZE];

scpi_t scpi_context;
