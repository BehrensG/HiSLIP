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

#define HISLIP_INITIALIZE									(u8_t)0
#define HISLIP_INITIALIZERESPONSE							(u8_t)1
#define	HISLIP_FATALERROR									(u8_t)2
#define	HISLIP_ERROR										(u8_t)3
#define HISLIP_ASYNCLOCK									(u8_t)4
#define HISLIP_ASYNCLOCKRESPONSE							(u8_t)5
#define HISLIP_DATA											(u8_t)6
#define HISLIP_DATAEND										(u8_t)7
#define HISLIP_DEVICECLEARCOMPLETE							(u8_t)8
#define HISLIP_DEVICECLEARACKNOWLEDGE						(u8_t)9
#define HISLIP_ASYNCREMOTELOCALCONTROL						(u8_t)10
#define HISLIP_ASYNCREMOTELOCALRESPONSE						(u8_t)11
#define HISLIP_TRIGGER										(u8_t)12
#define HISLIP_INTERRUPTED									(u8_t)13
#define HISLIP_ASYNCINTERRUPTED								(u8_t)14
#define HISLIP_ASYNCMAXIMUMMESSAGESIZE						(u8_t)15
#define HISLIP_ASYNCMAXIMUMMESSAGESIZERESPONSE				(u8_t)16
#define HISLIP_ASYNCINITIALIZE								(u8_t)17
#define HISLIP_ASYNCINITIALIZERESPONSE						(u8_t)18
#define HISLIP_ASYNCDEVICECLEAR								(u8_t)19
#define HISLIP_ASYNCSERVICEREQUEST							(u8_t)20
#define HISLIP_ASYNCSTATUSQUERY								(u8_t)21
#define HISLIP_ASYNCSTATUSRESPONSE							(u8_t)22
#define HISLIP_ASYNCDEVICECLEARACKNOWLEDGE					(u8_t)23
#define HISLIP_ASYNCLOCKINFO								(u8_t)24
#define HISLIP_ASYNCLOCKINFORESPONSE						(u8_t)25
#define HISLIP_GETDESCRIPTORS								(u8_t)26
#define HISLIP_GETDESCRIPTORSRESPONSE						(u8_t)27
#define HISLIP_STARTTLS										(u8_t)28
#define HISLIP_ASYNCSTARTTLS								(u8_t)29
#define HISLIP_ASYNCSTARTTLSRESPONSE						(u8_t)30
#define HISLIP_ENDTLS										(u8_t)31
#define HISLIP_ASYNCENDTLS									(u8_t)32
#define HISLIP_ASYNCENDTLSRESPONSE							(u8_t)33
#define HISLIP_GETSASLMECHANISMLIST							(u8_t)34
#define HISLIP_GETSASLMECHANISMLISTRESPONSE					(u8_t)35
#define HISLIP_AUTHENTICATIONSTART							(u8_t)36
#define HISLIP_AUTHENTICATIONEXCHANGE						(u8_t)37
#define HISLIP_AUTHENTICATIONRESULT							(u8_t)38

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
	MSG_UNKNOWN = 256
}hislip_msg_type_t;

#define HISLIP_PROLOGUE									0x4853

// TBD
#define HISLIP_VENDOR_ID								0x1111

#define HISLIP_MAX_DATA_SIZE							1024

#pragma pack(push, 1)

typedef struct
{
	u32_t hi;
	u32_t lo;
}payload_len_t;

typedef struct
{
	u16_t prologue;
	u8_t msg_type;
	u8_t control_code;
	u32_t msg_param;
	payload_len_t payload_len;
}hislip_msg_t;

#pragma pack(pop)


void hislip_ServerTask(void);

#endif /* HISLIP_INC_HISLIP_H_ */
