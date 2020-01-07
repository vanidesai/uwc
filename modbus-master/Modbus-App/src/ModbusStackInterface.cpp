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

extern "C" {
	#include <safe_lib.h>
}

extern src::severity_logger< severity_level > lg;

std::mutex g_RWCommonCallbackMutex;

/**
 *
 * DESCRIPTION
 * Function is used as application layer callback
 * for read/write coils,input register
 *
 * @param u8UnitID			[in] Unit ID
 * @param u16TransacID		[in] Transaction ID
 * @param pu8IpAddr			[in] IP address
 * @param u8FunCode			[in] Function code
 * @param pstException		[in] Exception status
 * @param u8numBytes		[in] byte count
 * @param pu8data			[in] input request
 *
 * @return void nothing
 *
 */
void ModbusMaster_AppCallback(uint8_t  u8UnitID,
		 	 	 	 	 	 uint16_t u16TransacID,
							 uint8_t* pu8IpAddr,
							 uint16_t u16Port,
							 uint8_t  u8FunCode,
							 stException_t  *pstException,
							 uint8_t  u8numBytes,
							 uint8_t* pu8data,
							 uint16_t  u16StartAdd,
							 uint16_t  u16Quantity)
{
	BOOST_LOG_SEV(lg, debug) << __func__ << " Start";
	const char* pcWriteRespTopic = std::getenv("WRITE_RESPONSE_TOPIC");
	if(NULL == pcWriteRespTopic)
	{
		BOOST_LOG_SEV(lg, debug) << __func__ << " WRITE_RESPONSE_TOPIC not set";
		return;
	}
	std::lock_guard<std::mutex> lock(g_RWCommonCallbackMutex);
	msg_envelope_t *msg = NULL;
	msg = msgbus_msg_envelope_new(CT_JSON);

	try
	{
		if( u8FunCode == READ_COIL_STATUS ||
				u8FunCode == READ_HOLDING_REG ||
				u8FunCode == READ_INPUT_STATUS ||
				u8FunCode == READ_INPUT_REG)
		{
			std::vector<uint8_t> datavt;
			for (uint8_t index=0; index<u8numBytes; index++)
			{
				datavt.push_back(pu8data[index]);
			}

			/// word swap is always false.
			std:: string strdata = zmq_handler::swapConversion(datavt,
															false,
															false);
			msg_envelope_elem_body_t* ptData = msgbus_msg_envelope_new_string(strdata.c_str());
			msgbus_msg_envelope_put(msg, "value", ptData);
		}

		std::string stAppSeqNum = zmq_handler::getAppSeq(u16TransacID);
		msg_envelope_elem_body_t* ptAppSeq = msgbus_msg_envelope_new_string(stAppSeqNum.c_str());
		msgbus_msg_envelope_put(msg, "app_seq", ptAppSeq);

		if(pu8IpAddr != NULL && pstException->m_u8ExcCode == 0 && pstException->m_u8ExcStatus ==0)
		{
			msg_envelope_elem_body_t* ptStatus = msgbus_msg_envelope_new_string("Good");
			msgbus_msg_envelope_put(msg, "Status", ptStatus);

			BOOST_LOG_SEV(lg, info) << "Info::"<< __func__ << "::function_code:"
					<< (unsigned)u8FunCode << "," << "exception_code:" <<(unsigned)pstException->m_u8ExcCode
					<< "," << "exception_status " << (unsigned)pstException->m_u8ExcStatus;
		}
		else
		{
			msg_envelope_elem_body_t* ptStatus = msgbus_msg_envelope_new_string("Bad");
			msgbus_msg_envelope_put(msg, "Status", ptStatus);
			BOOST_LOG_SEV(lg, info) <<"Info::" <<__func__<< " " << " NULL Pointer is Received from stack";
		}

		std::string topic(pcWriteRespTopic);
		zmq_handler::stZmqContext msgbus_ctx = zmq_handler::getCTX(topic);

		PublishJsonHandler::instance().publishJson(msg, msgbus_ctx.m_pContext, topic);
	}
	catch(const std::exception& e)
	{
		BOOST_LOG_SEV(lg, info) <<"fatal::" << __func__ << "::Exception is raised. "<<e.what();
	}

	if(msg != NULL)
	{
		msgbus_msg_envelope_destroy(msg);
		msg = NULL;
	}

	BOOST_LOG_SEV(lg, debug) << __func__ << " End";
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
	BOOST_LOG_SEV(lg, debug) << __func__ << " Start: Function Code: " << u8FunCode;
	uint8_t u8ReturnType = MBUS_JSON_APP_ERROR_NULL_POINTER;

	if(pstMbusApiPram != NULL)
	{
		BOOST_LOG_SEV(lg, info) << "Request Init Time: "
			<< std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()
			<< ", TxID: " << pstMbusApiPram->m_u16TxId
			<< ", Function Code: " << (unsigned)u8FunCode << ", DevID: " << (unsigned)pstMbusApiPram->m_u8DevId
			<< ", PointAddr: " << pstMbusApiPram->m_u16StartAddr;

		switch ((eModbusFuncCode_enum)u8FunCode)
		{
			case READ_COIL_STATUS:
				BOOST_LOG_SEV(lg, debug) << __func__ << " Read Coil Request Received";
				u8ReturnType = Modbus_Read_Coils(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					pstMbusApiPram->m_u16Port,
					vpCallBackFun);
			break;
			case READ_INPUT_STATUS:
				BOOST_LOG_SEV(lg, debug) << __func__ << " Read Input Status Request Received";
				u8ReturnType = Modbus_Read_Discrete_Inputs(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					pstMbusApiPram->m_u16Port,
					vpCallBackFun);
			break;
			case READ_HOLDING_REG:
				BOOST_LOG_SEV(lg, debug) << __func__ << " Read Holding Register Request Received";
				u8ReturnType = Modbus_Read_Holding_Registers(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					pstMbusApiPram->m_u16Port,
					vpCallBackFun);
			break;
			case READ_INPUT_REG:
				BOOST_LOG_SEV(lg, debug) << __func__ << " Read Input Register Request Received";
				u8ReturnType = Modbus_Read_Input_Registers(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					pstMbusApiPram->m_u16Port,
					vpCallBackFun);
			break;
			case WRITE_SINGLE_COIL:
			{
				BOOST_LOG_SEV(lg, debug) << __func__ << " Write Single Coil Request Received";
				uint16_t *pu16OutData = (uint16_t *)pstMbusApiPram->m_pu8Data;
				u8ReturnType = Modbus_Write_Single_Coil(pstMbusApiPram->m_u16StartAddr,
						*pu16OutData,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					pstMbusApiPram->m_u16Port,
					vpCallBackFun);
			}
			break;

			case WRITE_SINGLE_REG:
			{
				BOOST_LOG_SEV(lg, debug) << __func__ << " Write Single Register Request Received";
				uint16_t u16OutData = 0;
				memcpy_s(&u16OutData,sizeof(uint16_t),pstMbusApiPram->m_pu8Data,sizeof(uint16_t));
				u8ReturnType = Modbus_Write_Single_Register(pstMbusApiPram->m_u16StartAddr,
					*(uint16_t*)pstMbusApiPram->m_pu8Data,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					pstMbusApiPram->m_u16Port,
					vpCallBackFun);
			}
			break;
			case WRITE_MULTIPLE_COILS:
				BOOST_LOG_SEV(lg, debug) << __func__ << " Write Multiple Coils Request Received";
				u8ReturnType = Modbus_Write_Multiple_Coils(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_pu8Data,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					pstMbusApiPram->m_u16Port,
					vpCallBackFun);
			break;
			case WRITE_MULTIPLE_REG:
				BOOST_LOG_SEV(lg, debug) << __func__ << " Write Multiple Register Request Received";
				u8ReturnType = Modbus_Write_Multiple_Register(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_pu8Data,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					pstMbusApiPram->m_u16Port,
					vpCallBackFun);
			break;

			default:
				BOOST_LOG_SEV(lg, error) << __func__ << " Invalid Request Received";
				u8ReturnType = MBUS_STACK_ERROR_INVALID_INPUT_PARAMETER;
			break;
		}
	}
	BOOST_LOG_SEV(lg, debug) << __func__ << " End: Return: " << u8ReturnType;
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
