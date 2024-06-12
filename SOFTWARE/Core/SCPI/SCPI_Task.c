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
#include "lwipopts.h"



size_t SCPI_WriteHiSLIP(scpi_t * context, const char * data, size_t len)
{
    return SCPI_RES_ERR;
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



