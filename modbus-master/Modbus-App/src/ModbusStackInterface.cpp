/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#include <mutex>
#include "ZmqHandler.hpp"
#include "Logger.hpp"
#include "PeriodicReadFeature.hpp"
#include "PublishJson.hpp"
#include "API.h"
#include "PeriodicRead.hpp"

extern "C" {
	#include <safe_lib.h>
}

std::mutex g_RWCommonCallbackMutex;

/**
 * Get time in nano seconds
 * @param ts
 * @return time in nano seconds
 */
static unsigned long get_nanos(struct timespec ts) {
    return (unsigned long)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

/**
 *
 * DESCRIPTION
 * Function is used as application layer callback
 * for read/write coils,input register
 *
 * @param pstMbusAppCallbackParams :[in] pointer to struct containing response from stack
 * @return void nothing
 *
 */
void ModbusMaster_AppCallback(stMbusAppCallbackParams_t *pstMbusAppCallbackParams)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start"));
	if(pstMbusAppCallbackParams == NULL)
	{
		CLogger::getInstance().log(DEBUG, LOGDETAILS("Response received from stack is null"));
		return;
	}

	/// validate pointer
	if(NULL == pstMbusAppCallbackParams->m_au8MbusRXDataDataFields)
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS(" No data received from stack"));
		return;
	}

	eMbusResponseType respType = MBUS_RESPONSE_ONDEMAND;

	// handle response
	CPeriodicReponseProcessor::Instance().handleResponse(pstMbusAppCallbackParams, respType);

	CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));
}

/**
 * Description
 * function to call Modbus stack APIs
 *
 * @param u8FunCode 		[in] function code
 * @param pstMbusApiPram 	[in] Input request packet
 * @param vpCallBackFun		[in] callback function pointer
 *
 * @return uint8_t			[out] return 0 on success
 *
 */
uint8_t Modbus_Stack_API_Call(unsigned char u8FunCode, MbusAPI_t *pstMbusApiPram,
					void* vpCallBackFun)
{
	string temp; //temporary string for logging

	temp = "Start: Function Code: ";
	temp.append(to_string(u8FunCode));
	CLogger::getInstance().log(DEBUG, LOGDETAILS(temp));

	uint8_t u8ReturnType = MBUS_JSON_APP_ERROR_NULL_POINTER;

	if(pstMbusApiPram != NULL)
	{
		temp = "Request Init Time: ";
		temp.append(to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()));
		temp.append(", TxID: ");
		temp.append(to_string(pstMbusApiPram->m_u16TxId));
		temp.append(", Function Code: ");
		temp.append(to_string((unsigned)u8FunCode));
		temp.append(", DevID: ");
		temp.append(to_string((unsigned)pstMbusApiPram->m_u8DevId));
		temp.append(", PointAddr: ");
		temp.append(to_string(pstMbusApiPram->m_u16StartAddr));

		CLogger::getInstance().log(DEBUG, LOGDETAILS(temp));

		switch ((eModbusFuncCode_enum)u8FunCode)
		{
			case READ_COIL_STATUS:
				CLogger::getInstance().log(DEBUG, LOGDETAILS("Read Coil Request Received"));
				u8ReturnType = Modbus_Read_Coils(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					pstMbusApiPram->m_u16Port,
					pstMbusApiPram->m_lPriority,
					pstMbusApiPram->m_u32mseTimeout,
					vpCallBackFun);
			break;
			case READ_INPUT_STATUS:
				CLogger::getInstance().log(DEBUG, LOGDETAILS("Read Input Status Request Received"));
				u8ReturnType = Modbus_Read_Discrete_Inputs(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					pstMbusApiPram->m_u16Port,
					pstMbusApiPram->m_lPriority,
					pstMbusApiPram->m_u32mseTimeout,
					vpCallBackFun);
			break;
			case READ_HOLDING_REG:
				CLogger::getInstance().log(DEBUG, LOGDETAILS("Read Holding Register Request Received"));
				u8ReturnType = Modbus_Read_Holding_Registers(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					pstMbusApiPram->m_u16Port,
					pstMbusApiPram->m_lPriority,
					pstMbusApiPram->m_u32mseTimeout,
					vpCallBackFun);
			break;
			case READ_INPUT_REG:
				CLogger::getInstance().log(DEBUG, LOGDETAILS("Read Input Register Request Received"));
				u8ReturnType = Modbus_Read_Input_Registers(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					pstMbusApiPram->m_u16Port,
					pstMbusApiPram->m_lPriority,
					pstMbusApiPram->m_u32mseTimeout,
					vpCallBackFun);
			break;
			case WRITE_SINGLE_COIL:
			{
				CLogger::getInstance().log(DEBUG, LOGDETAILS("Write Single Coil Request Received"));
				uint16_t *pu16OutData = (uint16_t *)pstMbusApiPram->m_pu8Data;
				u8ReturnType = Modbus_Write_Single_Coil(pstMbusApiPram->m_u16StartAddr,
						*pu16OutData,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					pstMbusApiPram->m_u16Port,
					pstMbusApiPram->m_lPriority,
					pstMbusApiPram->m_u32mseTimeout,
					vpCallBackFun);
			}
			break;

			case WRITE_SINGLE_REG:
			{
				CLogger::getInstance().log(DEBUG, LOGDETAILS("Write Single Register Request Received"));
				uint16_t u16OutData = 0;
				memcpy_s(&u16OutData,sizeof(uint16_t),pstMbusApiPram->m_pu8Data,sizeof(uint16_t));
				u8ReturnType = Modbus_Write_Single_Register(pstMbusApiPram->m_u16StartAddr,
					*(uint16_t*)pstMbusApiPram->m_pu8Data,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					pstMbusApiPram->m_u16Port,
					pstMbusApiPram->m_lPriority,
					pstMbusApiPram->m_u32mseTimeout,
					vpCallBackFun);
			}
			break;
			case WRITE_MULTIPLE_COILS:
				CLogger::getInstance().log(DEBUG, LOGDETAILS("Write Multiple Coils Request Received"));
				u8ReturnType = Modbus_Write_Multiple_Coils(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_pu8Data,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					pstMbusApiPram->m_u16Port,
					pstMbusApiPram->m_lPriority,
					pstMbusApiPram->m_u32mseTimeout,
					vpCallBackFun);
			break;
			case WRITE_MULTIPLE_REG:
				CLogger::getInstance().log(DEBUG, LOGDETAILS("Write Multiple Register Request Received"));
				u8ReturnType = Modbus_Write_Multiple_Register(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_pu8Data,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					pstMbusApiPram->m_u16Port,
					pstMbusApiPram->m_lPriority,
					pstMbusApiPram->m_u32mseTimeout,
					vpCallBackFun);
			break;

			default:
				CLogger::getInstance().log(ERROR, LOGDETAILS("Invalid Request Received"));
				u8ReturnType = MBUS_STACK_ERROR_INVALID_INPUT_PARAMETER;
			break;
		}
	}

	temp = "End: Return: ";
	temp.append(to_string(u8ReturnType));
	CLogger::getInstance().log(DEBUG, LOGDETAILS(temp));

	return u8ReturnType;
}

/**
 *
 * DESCRIPTION
 * function to process or call stack APIs.
 *
 * @param pstMbusReqGen [in] pointer to the request packet
 *
 * @return uint8_t		[out] return 0 on success
 *
 */
uint8_t modbusInterface::MbusApp_Process_Request(RestMbusReqGeneric_t *pstMbusReqGen)
{
	MbusAPI_t stMbusApiPram = {};
	uint8_t u8ReturnType = MBUS_JSON_APP_ERROR_NULL_POINTER;

	if(pstMbusReqGen != NULL)
	{
		stMbusApiPram.m_pu8Data = pstMbusReqGen->m_stReqData.m_pu8Data;
		stMbusApiPram.m_u16ByteCount =  pstMbusReqGen->m_stReqData.m_u8DataLen;
		stMbusApiPram.m_u16Quantity = pstMbusReqGen->m_u16Quantity;
		stMbusApiPram.m_u16StartAddr = pstMbusReqGen->m_u16StartAddr;
		stMbusApiPram.m_u8DevId = pstMbusReqGen->m_u8DevId;
		stMbusApiPram.m_u16TxId = pstMbusReqGen->m_u16ReffId;
#ifdef MODBUS_STACK_TCPIP_ENABLED
		stMbusApiPram.m_u8IpAddr[0] = pstMbusReqGen->m_u8IpAddr[0];
		stMbusApiPram.m_u8IpAddr[1] = pstMbusReqGen->m_u8IpAddr[1];
		stMbusApiPram.m_u8IpAddr[2] = pstMbusReqGen->m_u8IpAddr[2];
		stMbusApiPram.m_u8IpAddr[3] = pstMbusReqGen->m_u8IpAddr[3];
		stMbusApiPram.m_u16Port = pstMbusReqGen->m_u16Port;
#endif

		if(MBUS_MIN_FUN_CODE != pstMbusReqGen->m_u8FunCode)
		{
			u8ReturnType = Modbus_Stack_API_Call(
					pstMbusReqGen->m_u8FunCode,
					&stMbusApiPram,
					(void*)ModbusMaster_AppCallback);
		}
	}
	return u8ReturnType;
}
