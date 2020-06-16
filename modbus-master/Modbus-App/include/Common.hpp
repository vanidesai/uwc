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
#include "ConfigManager.hpp"
#include <string>
#include <iostream>
#ifdef __cplusplus
extern "C" {
#include "API.h"
}
#endif
#include <map>

#ifdef __linux

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
	std::string m_sValue;
	std::string m_sUsec;
	std::string m_sTimestamp;
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
	int32_t m_i32Ctx;
	stOnDemandRequest m_stOnDemandReqData;
}MbusAPI_t;

// Function to get function code
unsigned char GetFunctionCode(uint8_t u8ReadWrite,
								  uint16_t u16Quantity,
								  uint8_t u8MbudTable);

// This enumerator defines modbus app error codes
typedef enum MbusAppErrorCode
{
	APP_SUCCESS = 0,
	APP_ERROR_DUMMY_RESPONSE = 100,
	APP_ERROR_REQUEST_SEND_FAILED,
	APP_ERROR_INVALID_INPUT_JSON,
	APP_ERROR_CUTOFF_TIME_INTERVAL,
	APP_ERROR_INVALID_FUNCTION_CODE,
	APP_ERROR_EMPTY_DATA_RECVD_FROM_STACK,
	APP_JSON_PARSING_EXCEPTION,
	APP_ERROR_UNKNOWN_SERVICE_REQUEST,
	APP_ERROR_POINT_IS_NOT_WRITABLE,
	APP_INTERNAL_ERORR,
	APP_ERROR_INVALID_CTX,
	APP_ERROR_CODE_MAX
}eMbusAppErrorCode;

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

/**
 * get request priority from global configuration depending on the operation priority
 * @param a_Ops			:[in] global config for which to retrieve operation priority
 * @return [long]		:[out] request priority to be sent to stack.(lower is the value higher is the priority)
 */
long getReqPriority(const globalConfig::COperation a_Ops);
}

#endif /* INCLUDE_INC_COMMON_HPP_ */
