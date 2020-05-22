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
 * Function is used as application layer callback for on-demand read response from stack.
 * @param pstMbusAppCallbackParams :[in] pointer to structure containing response from stack
 * @uTxID :[in] response Transaction ID
 */
void OnDemandRead_AppCallback(stMbusAppCallbackParams_t *pstMbusAppCallbackParams, uint16_t uTxID)
{
	DO_LOG_DEBUG("Start");
	if(pstMbusAppCallbackParams == NULL)
	{
		DO_LOG_DEBUG("Response received from stack is null for on-demand read");
		return;
	}

	/// handle response to process response
	CPeriodicReponseProcessor::Instance().handleResponse(pstMbusAppCallbackParams,
															MBUS_CALLBACK_ONDEMAND_READ,
															PublishJsonHandler::instance().getSReadResponseTopic(),
															false);

	DO_LOG_DEBUG("End");
}

/**
 * Function is used as application layer callback for on-demand readRT response from stack.
 * @param pstMbusAppCallbackParams :[in] pointer to struct containing response from stack
 * @uTxID :[in] response Transaction ID
 */
void OnDemandReadRT_AppCallback(stMbusAppCallbackParams_t *pstMbusAppCallbackParams, uint16_t uTxID)
{
	DO_LOG_DEBUG("Start");
	if(pstMbusAppCallbackParams == NULL)
	{
		DO_LOG_DEBUG("Response received from stack is null for realtime on-demand read");
		return;
	}

	/// handle response to process response
	CPeriodicReponseProcessor::Instance().handleResponse(pstMbusAppCallbackParams,
															MBUS_CALLBACK_ONDEMAND_READ_RT,
															PublishJsonHandler::instance().getSReadResponseTopicRT(),
															true);

	DO_LOG_DEBUG("End");
}

/**
 * Function is used as application layer callback for on-demand write response from stack.
 * @param pstMbusAppCallbackParams :[in] pointer to struct containing response from stack
 * @uTxID :[in] response transaction ID
 *
 */
void OnDemandWrite_AppCallback(stMbusAppCallbackParams_t *pstMbusAppCallbackParams, uint16_t uTxID)
{
	DO_LOG_DEBUG("Start");
	if(pstMbusAppCallbackParams == NULL)
	{
		DO_LOG_DEBUG("Response received from stack is null for on-demand write");
		return;
	}

	/// handle response to process response
	CPeriodicReponseProcessor::Instance().handleResponse(pstMbusAppCallbackParams,
														MBUS_CALLBACK_ONDEMAND_WRITE,
														PublishJsonHandler::instance().getSWriteResponseTopic(),
														false);

	DO_LOG_DEBUG("End");
}

/**
 * Function is used as application layer callback for on-demand WriteRT response from stack.
 * @param pstMbusAppCallbackParams :[in] pointer to struct containing response from stack
 * @uTxID :[in] response transaction ID
 */
void OnDemandWriteRT_AppCallback(stMbusAppCallbackParams_t *pstMbusAppCallbackParams, uint16_t uTxID)
{
	DO_LOG_DEBUG("Start");
	if(pstMbusAppCallbackParams == NULL)
	{
		DO_LOG_DEBUG("Response received from stack is null for realtime on-demand write");
		return;
	}

	/// handle response to process response
	CPeriodicReponseProcessor::Instance().handleResponse(pstMbusAppCallbackParams,
														MBUS_CALLBACK_ONDEMAND_WRITE_RT,
														PublishJsonHandler::instance().getSWriteResponseTopicRT(),
														true);

	DO_LOG_DEBUG("End");
}

/**
 * Function to call modbus stack APIs depending updon function code
 * @param u8FunCode 		[in] function code of request
 * @param pstMbusApiPram 	[in] Input request packet
 * @param vpCallBackFun		[in] callback function pointer
 * @return uint8_t			[out] return 0 on success else error code
 */
uint8_t Modbus_Stack_API_Call(unsigned char u8FunCode,
		MbusAPI_t *pstMbusApiPram,
		void* vpCallBackFun)
{
	string temp; /// temporary string for logging
	uint8_t u8ReturnType = APP_SUCCESS;

	if(pstMbusApiPram != NULL)
	{
		switch ((eModbusFuncCode_enum)u8FunCode)
		{
			case READ_COIL_STATUS:
				DO_LOG_DEBUG("Read Coil Request Received");
				u8ReturnType = Modbus_Read_Coils(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					pstMbusApiPram->m_u16Port,
					pstMbusApiPram->m_lPriority,
					vpCallBackFun);
			break;
			case READ_INPUT_STATUS:
				DO_LOG_DEBUG("Read Input Status Request Received");
				u8ReturnType = Modbus_Read_Discrete_Inputs(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					pstMbusApiPram->m_u16Port,
					pstMbusApiPram->m_lPriority,
					vpCallBackFun);
			break;
			case READ_HOLDING_REG:
				DO_LOG_DEBUG("Read Holding Register Request Received");
				u8ReturnType = Modbus_Read_Holding_Registers(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					pstMbusApiPram->m_u16Port,
					pstMbusApiPram->m_lPriority,
					vpCallBackFun);
			break;
			case READ_INPUT_REG:
				DO_LOG_DEBUG("Read Input Register Request Received");
				u8ReturnType = Modbus_Read_Input_Registers(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					pstMbusApiPram->m_u16Port,
					pstMbusApiPram->m_lPriority,
					vpCallBackFun);
			break;
			case WRITE_SINGLE_COIL:
			{
				DO_LOG_DEBUG("Write Single Coil Request Received");
				uint16_t *pu16OutData = (uint16_t *)pstMbusApiPram->m_pu8Data;
				u8ReturnType = Modbus_Write_Single_Coil(pstMbusApiPram->m_u16StartAddr,
						*pu16OutData,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					pstMbusApiPram->m_u16Port,
					pstMbusApiPram->m_lPriority,
					vpCallBackFun);
			}
			break;

			case WRITE_SINGLE_REG:
			{
				DO_LOG_DEBUG("Write Single Register Request Received");
				u8ReturnType = Modbus_Write_Single_Register(pstMbusApiPram->m_u16StartAddr,
					*(uint16_t*)pstMbusApiPram->m_pu8Data,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					pstMbusApiPram->m_u16Port,
					pstMbusApiPram->m_lPriority,
					vpCallBackFun);
			}
			break;
			case WRITE_MULTIPLE_COILS:
				DO_LOG_DEBUG("Write Multiple Coils Request Received");
				u8ReturnType = Modbus_Write_Multiple_Coils(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_pu8Data,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					pstMbusApiPram->m_u16Port,
					pstMbusApiPram->m_lPriority,
					vpCallBackFun);
			break;
			case WRITE_MULTIPLE_REG:
				DO_LOG_DEBUG("Write Multiple Register Request Received");
				u8ReturnType = Modbus_Write_Multiple_Register(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_pu8Data,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					pstMbusApiPram->m_u16Port,
					pstMbusApiPram->m_lPriority,
					vpCallBackFun);
			break;

			default:
				DO_LOG_ERROR("Invalid Request Received");
				u8ReturnType = APP_ERROR_INVALID_FUNCTION_CODE;
			break;
		}
	}

	return u8ReturnType;
}
