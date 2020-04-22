/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#ifndef INCLUDE_INC_COMMON_HPP_
#define INCLUDE_INC_COMMON_HPP_

#include "ZmqHandler.hpp"
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#ifdef __cplusplus
extern "C" {
#include "API.h"
}
#endif
#include <map>

#ifdef __linux

typedef bool            BOOLEAN;

#define TRUE true
#define FALSE false
#endif

#define MODBUS_SINGLE_REGISTER_LENGTH (2)

using namespace std;

struct stOnDemandRequest
{
	std::string m_strAppSeq;
	std::string m_strWellhead;
	std::string m_strMetric;
	std::string m_strVersion;
	std::string m_strTopic;
	bool m_isByteSwap;
	bool m_isWordSwap;
	bool m_isRT;
	struct timespec m_obtReqRcvdTS;
	//long m_lPriority;
	std::string m_strMqttTime;
	std::string m_strEisTime;
};

// This structure defines generic parameters for modbus common API
typedef struct MbusAPI
{
	unsigned char	m_u8DevId;
	unsigned char	m_u8IpAddr[4];
	uint16_t		m_u16Port;
	unsigned short  m_u16TxId;
	unsigned short  m_u16Quantity;
	unsigned short  m_u16StartAddr;
	unsigned short  m_u16ByteCount;
	unsigned char  m_pu8Data[260];
	/** Holds the Msg Priority  */
	long m_lPriority;
	/** Holds the Mse Timeout  */
	//uint32_t m_u32mseTimeout;
	int m_nRetry;
	stOnDemandRequest m_stOnDemandReqData;
}MbusAPI_t;

// Function to get function code
unsigned char GetFunctionCode(uint8_t u8ReadWrite,
								  uint16_t u16Quantity,
								  uint8_t u8MbudTable);

// This enumerator defines MODBUS error codes
typedef enum MbusStackErrorCode
{
	MBUS_STACK_NO_ERROR,
	MBUS_STACK_TXNID_OR_UNITID_MISSMATCH,
	MBUS_STACK_ERROR_SOCKET_FAILED,
	MBUS_STACK_ERROR_CONNECT_FAILED,
	MBUS_STACK_ERROR_SEND_FAILED,
	MBUS_STACK_ERROR_RECV_FAILED,
	MBUS_STACK_ERROR_RECV_TIMEOUT,
	MBUS_STACK_ERROR_MALLOC_FAILED,
	MBUS_STACK_ERROR_QUEUE_SEND,
	MBUS_STACK_ERROR_QUEUE_RECIVE,
	MBUS_STACK_ERROR_THREAD_CREATE,
	MBUS_STACK_ERROR_INVALID_INPUT_PARAMETER,
	MBUS_STACK_ERROR_PACKET_LENGTH_EXCEEDED,
	MBUS_JSON_APP_ERROR_MALLOC_FAILED,
	MBUS_JSON_APP_ERROR_INVALID_INPUT_PARAMETER,
	MBUS_JSON_APP_ERROR_PACKET_LENGTH_EXCEEDED,
	MBUS_JSON_APP_ERROR_NULL_POINTER,
	MBUS_JSON_APP_ERROR_INVALID_FUN_CODE,
	MBUS_JSON_APP_ERROR_EXCEPTION_RISE,
	MBUS_APP_TAG_OR_SUBTAG_NOT_FOUND,
	MBUS_APP_SUBSCRIPTION_NOT_FOUND,
	MBUS_PERIODIC_FAILED_TO_FIND_TOPIC,
	MBUS_PERIODIC_INTERVAL_OUT_OF_RANG,
	MBUS_INVALID_ENV_VARIABLE,
	MBUS_APP_ERROR_UNKNOWN_SERVICE_REQUEST,
	MBUS_APP_ERROR_IMPROPER_METHOD,
	MBUS_APP_ERROR_CONTENT_TYPE_NOT_JSON,
	MBUS_APP_ERROR_AUTHENTICATION_FAILED,
}eMbusStackErrorCode;

enum eMbusRequestType
{
	MBUS_REQUEST_NONE,
	MBUS_REQUEST_READ,
	MBUS_REQUEST_READRT,
	MBUS_REQUEST_WRITE,
	MBUS_REQUEST_WRITERT,
};

enum eMbusCallbackType
{
	MBUS_CALLBACK_POLLING,
	MBUS_CALLBACK_POLLING_RT,
	MBUS_CALLBACK_ONDEMAND_READ,
	MBUS_CALLBACK_ONDEMAND_READ_RT,
	MBUS_CALLBACK_ONDEMAND_WRITE,
	MBUS_CALLBACK_ONDEMAND_WRITE_RT,
};

/// function to call Modbus stack APIs
uint8_t Modbus_Stack_API_Call(unsigned char u8FunCode,
								MbusAPI_t *pstMbusApiPram,void* vpCallBackFun);

void OnDemandRead_AppCallback(stMbusAppCallbackParams_t *pstMbusAppCallbackParams, uint16_t uTxID);
void OnDemandReadRT_AppCallback(stMbusAppCallbackParams_t *pstMbusAppCallbackParams, uint16_t uTxID);
void OnDemandWrite_AppCallback(stMbusAppCallbackParams_t *pstMbusAppCallbackParams, uint16_t uTxID);
void OnDemandWriteRT_AppCallback(stMbusAppCallbackParams_t *pstMbusAppCallbackParams, uint16_t uTxID);

using std::string;
#include <string>
#include <vector>

namespace common_Handler
{
/// function for byteswap and wordswap
std::string swapConversion(std::vector<unsigned char> vt, bool a_bIsByteSwap = false, bool a_bIsWordSwap = false);

/// function to read current time and usec
void getTimeParams(std::string &a_sTimeStamp, std::string &a_sUsec);

/// function to get request json
bool getReqData(unsigned short seqno, MbusAPI_t& reqData);

/// function to insert new entry in map
bool insertReqData(unsigned short, MbusAPI_t);

/// function to update map value
bool updateReqData(unsigned short, MbusAPI_t);

/// function to remove entry from the map
void removeReqData(unsigned short);
}

#endif /* INCLUDE_INC_COMMON_HPP_ */
