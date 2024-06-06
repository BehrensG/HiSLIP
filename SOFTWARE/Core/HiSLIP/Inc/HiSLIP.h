/*
 * HiSLIP.h
 *
 *  Created on: Jun 3, 2024
 *      Author: grzegorz
 */

#ifndef HISLIP_INC_HISLIP_H_
#define HISLIP_INC_HISLIP_H_

#include "arch.h"
#include "ip_addr.h"
#include "err.h"

#define HISLIP_INITIALIZE									(uint8_t)0
#define HISLIP_INITIALIZERESPONSE							(uint8_t)1
#define	HISLIP_FATALERROR									(uint8_t)2
#define	HISLIP_ERROR										(uint8_t)3
#define HISLIP_ASYNCLOCK									(uint8_t)4
#define HISLIP_ASYNCLOCKRESPONSE							(uint8_t)5
#define HISLIP_DATA											(uint8_t)6
#define HISLIP_DATAEND										(uint8_t)7
#define HISLIP_DEVICECLEARCOMPLETE							(uint8_t)8
#define HISLIP_DEVICECLEARACKNOWLEDGE						(uint8_t)9
#define HISLIP_ASYNCREMOTELOCALCONTROL						(uint8_t)10
#define HISLIP_ASYNCREMOTELOCALRESPONSE						(uint8_t)11
#define HISLIP_TRIGGER										(uint8_t)12
#define HISLIP_INTERRUPTED									(uint8_t)13
#define HISLIP_ASYNCINTERRUPTED								(uint8_t)14
#define HISLIP_ASYNCMAXIMUMMESSAGESIZE						(uint8_t)15
#define HISLIP_ASYNCMAXIMUMMESSAGESIZERESPONSE				(uint8_t)16
#define HISLIP_ASYNCINITIALIZE								(uint8_t)17
#define HISLIP_ASYNCINITIALIZERESPONSE						(uint8_t)18
#define HISLIP_ASYNCDEVICECLEAR								(uint8_t)19
#define HISLIP_ASYNCSERVICEREQUEST							(uint8_t)20
#define HISLIP_ASYNCSTATUSQUERY								(uint8_t)21
#define HISLIP_ASYNCSTATUSRESPONSE							(uint8_t)22
#define HISLIP_ASYNCDEVICECLEARACKNOWLEDGE					(uint8_t)23
#define HISLIP_ASYNCLOCKINFO								(uint8_t)24
#define HISLIP_ASYNCLOCKINFORESPONSE						(uint8_t)25
#define HISLIP_GETDESCRIPTORS								(uint8_t)26
#define HISLIP_GETDESCRIPTORSRESPONSE						(uint8_t)27
#define HISLIP_STARTTLS										(uint8_t)28
#define HISLIP_ASYNCSTARTTLS								(uint8_t)29
#define HISLIP_ASYNCSTARTTLSRESPONSE						(uint8_t)30
#define HISLIP_ENDTLS										(uint8_t)31
#define HISLIP_ASYNCENDTLS									(uint8_t)32
#define HISLIP_ASYNCENDTLSRESPONSE							(uint8_t)33
#define HISLIP_GETSASLMECHANISMLIST							(uint8_t)34
#define HISLIP_GETSASLMECHANISMLISTRESPONSE					(uint8_t)35
#define HISLIP_AUTHENTICATIONSTART							(uint8_t)36
#define HISLIP_AUTHENTICATIONEXCHANGE						(uint8_t)37
#define HISLIP_AUTHENTICATIONRESULT							(uint8_t)38

#define HISLIP_PROLOGUE									0x4853
// TBD
#define HISLIP_VENDOR_ID								0x1111
#define HISLIP_MAX_DATA_SIZE							1024

#define HISLIP_LINE_ENDING								"\n"

typedef enum
{
	Initialize = HISLIP_INITIALIZE,
	InitializeResponse = HISLIP_INITIALIZERESPONSE,
	FatalError = HISLIP_FATALERROR,
	Error = HISLIP_ERROR,
	AsyncLock = HISLIP_ASYNCLOCK,
	AsyncLockResponse = HISLIP_ASYNCLOCKRESPONSE,
	Data = HISLIP_DATA,
	DataEnd = HISLIP_DATAEND,
	DeviceClearComplete = HISLIP_DEVICECLEARCOMPLETE,
	DeviceClearAcknowledge = HISLIP_DEVICECLEARACKNOWLEDGE,
	AsyncRemoteLocalControl = HISLIP_ASYNCREMOTELOCALCONTROL,
	AsyncRemoteLocalResponse = HISLIP_ASYNCREMOTELOCALRESPONSE,
	Trigger = HISLIP_TRIGGER,
	Interrupted = HISLIP_INTERRUPTED,
	AsyncInterrupted = HISLIP_ASYNCINTERRUPTED,
	AsyncMaximumMessageSize = HISLIP_ASYNCMAXIMUMMESSAGESIZE,
	AsyncMaximumMessageSizeResponse = HISLIP_ASYNCMAXIMUMMESSAGESIZERESPONSE,
	AsyncInitialize = HISLIP_ASYNCINITIALIZE,
	AsyncInitializeResponse = HISLIP_ASYNCINITIALIZERESPONSE,
	AsyncDeviceClear = HISLIP_ASYNCDEVICECLEAR,
	AsyncServiceRequest = HISLIP_ASYNCSERVICEREQUEST,
	AsyncStatusQuery = HISLIP_ASYNCSTATUSQUERY,
	AsyncStatusResponse = HISLIP_ASYNCSTATUSRESPONSE,
	AsyncDeviceClearAcknowledge = HISLIP_ASYNCDEVICECLEARACKNOWLEDGE,
	AsyncLockInfo = HISLIP_ASYNCLOCKINFO,
	AsyncLockInfoResponse = HISLIP_ASYNCLOCKINFORESPONSE,
	GetDescriptors = HISLIP_GETDESCRIPTORS,
	GetDescriptorsResponse = HISLIP_GETDESCRIPTORSRESPONSE,
	StartTLS = HISLIP_STARTTLS,
	AsyncStartTLS = HISLIP_ASYNCSTARTTLS,
	AsyncStartTLSResponse = HISLIP_ASYNCSTARTTLSRESPONSE,
	EndTLS = HISLIP_ENDTLS,
	AsyncEndTLS = HISLIP_ASYNCENDTLS,
	AsyncEndTLSResponse = HISLIP_ASYNCENDTLSRESPONSE,
	GetSaslMechanismList = HISLIP_GETSASLMECHANISMLIST,
	GetSaslMechanismListResponse = HISLIP_GETSASLMECHANISMLISTRESPONSE,
	AuthenticationStart = HISLIP_AUTHENTICATIONSTART,
	AuthenticationExchange = HISLIP_AUTHENTICATIONEXCHANGE,
	AuthenticationResult = HISLIP_AUTHENTICATIONRESULT,
	HISLIP_CONN_ERR = 256

}hislip_msg_type_t;

#pragma pack(push, 1)

typedef struct
{
	uint32_t hi;
	uint32_t lo;
}payload_len_t;

typedef struct
{
	uint16_t prologue;
	uint8_t msg_type;
	uint8_t control_code;
	uint32_t msg_param;
	payload_len_t payload_len;
}hislip_msg_t;

#pragma pack(pop)


typedef struct {
	struct netconn* newconn;
}hislip_netconn_t;

typedef struct {
	char data[sizeof(hislip_msg_t) + HISLIP_MAX_DATA_SIZE];
	uint16_t len;
}hislip_netbuf_t;


typedef struct {
	hislip_msg_t msg;
	hislip_netbuf_t netbuf;
	hislip_netconn_t netconn;
	uint16_t session_id;
	char end[2];
}hislip_instr_t;


void hislip_CreateTask(void);
void hislip_htonl(hislip_msg_t* hislip_msg);

#endif /* HISLIP_INC_HISLIP_H_ */
