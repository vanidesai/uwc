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

extern "C" {
	#include <safe_lib.h>
}

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
#ifdef MODBUS_STACK_TCPIP_ENABLED
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
#else
void ModbusMaster_AppCallback(uint8_t  u8UnitID,
		 	 	 	 	 	 uint16_t u16TransacID,
							 uint8_t* pu8IpAddr,
							 uint8_t  u8FunCode,
							 stException_t  *pstException,
							 uint8_t  u8numBytes,
							 uint8_t* pu8data,
							 uint16_t  u16StartAdd,
							 uint16_t  u16Quantity)
#endif
{
	string temp; //temporary string for logging
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start"));
	std::string sRespTopic = "";

	std::lock_guard<std::mutex> lock(g_RWCommonCallbackMutex);
	msg_envelope_t *msg = NULL;
	msg = msgbus_msg_envelope_new(CT_JSON);

	try
	{
		stOnDemandRequest onDemandReqData;
		zmq_handler::getOnDemandReqData(u16TransacID, onDemandReqData);

		if( u8FunCode == READ_COIL_STATUS ||
				u8FunCode == READ_HOLDING_REG ||
				u8FunCode == READ_INPUT_STATUS ||
				u8FunCode == READ_INPUT_REG)
		{
			sRespTopic = PublishJsonHandler::instance().getSReadResponseTopic();
			if(NULL != pu8data && u8numBytes > 0)
			{
				std::vector<uint8_t> datavt;
				for (uint8_t index=0; index<u8numBytes; index++)
				{
					datavt.push_back(pu8data[index]);
				}

				/// word swap is always false.
				std:: string strdata = zmq_handler::swapConversion(datavt,
						onDemandReqData.m_isByteSwap,
						onDemandReqData.m_isWordSwap);
				/// value
				msg_envelope_elem_body_t* ptData = msgbus_msg_envelope_new_string(strdata.c_str());

				if(NULL != ptData)
				{
					msgbus_msg_envelope_put(msg, "value", ptData);
				}
				else
				{
					//error
					std::cout << "Pointer is null " << endl;
				}
			}

		}
		else
		{
			sRespTopic = PublishJsonHandler::instance().getSWriteResponseTopic();
		}

		/// topic
		msg_envelope_elem_body_t* ptResTopic = msgbus_msg_envelope_new_string(onDemandReqData.m_strTopic.append("Response").c_str());
		msgbus_msg_envelope_put(msg, "topic", ptResTopic);
		/// wellhead
		msg_envelope_elem_body_t* ptWellhead = msgbus_msg_envelope_new_string(onDemandReqData.m_strWellhead.c_str());
		msgbus_msg_envelope_put(msg, "wellhead", ptWellhead);
		/// application sequence
		msg_envelope_elem_body_t* ptAppSeq = msgbus_msg_envelope_new_string(onDemandReqData.m_strAppSeq.c_str());
		msgbus_msg_envelope_put(msg, "app_seq", ptAppSeq);
		/// metric
		msg_envelope_elem_body_t* ptMetric = msgbus_msg_envelope_new_string(onDemandReqData.m_strMetric.c_str());
		msgbus_msg_envelope_put(msg, "metric", ptMetric);
		/// version
		msg_envelope_elem_body_t* ptVersion = msgbus_msg_envelope_new_string(onDemandReqData.m_strVersion.c_str());
		msgbus_msg_envelope_put(msg, "version", ptVersion);

		std::string strTimestamp, strUsec;
		zmq_handler::getTimeParams(strTimestamp, strUsec);
		/// usec
		msg_envelope_elem_body_t* ptUsec = msgbus_msg_envelope_new_string(strUsec.c_str());
		msgbus_msg_envelope_put(msg, "usec", ptUsec);
		/// timestamp
		msg_envelope_elem_body_t* ptTimestamp = msgbus_msg_envelope_new_string(strTimestamp.c_str());
		msgbus_msg_envelope_put(msg, "timestamp", ptTimestamp);

		if(pu8IpAddr != NULL && pstException->m_u8ExcCode == 0 && pstException->m_u8ExcStatus ==0)
		{
			msg_envelope_elem_body_t* ptStatus = msgbus_msg_envelope_new_string("Good");
			msgbus_msg_envelope_put(msg, "status", ptStatus);

			temp = "Info::";
			temp.append("::function_code:");
			temp.append(to_string((unsigned)u8FunCode));
			temp.append("exception_code:");
			temp.append(to_string((unsigned)pstException->m_u8ExcCode));
			temp.append(",");
			temp.append("exception_status ");
			temp.append(to_string((unsigned)pstException->m_u8ExcStatus));
			CLogger::getInstance().log(INFO, LOGDETAILS(temp));

		}
		else
		{
			msg_envelope_elem_body_t* ptStatus = msgbus_msg_envelope_new_string("Bad");
			msgbus_msg_envelope_put(msg, "status", ptStatus);

			msg_envelope_elem_body_t* ptErrorDetails = msgbus_msg_envelope_new_string(((to_string(pstException->m_u8ExcCode)) + ", " +  (to_string(pstException->m_u8ExcStatus))).c_str());
			msgbus_msg_envelope_put(msg, "error_code", ptErrorDetails);

			temp = "Info::";
			temp.append("NULL Pointer is Received from stack");
			CLogger::getInstance().log(INFO, LOGDETAILS(temp));
		}

		zmq_handler::stZmqContext msgbus_ctx = zmq_handler::getCTX(sRespTopic);
		zmq_handler::stZmqPubContext pubCtx = zmq_handler::getPubCTX(sRespTopic);

		std::cout <<"****************************************************************" <<endl;
		std::cout << "on-demand response received with following parameters ::" << endl;
		std::cout << "app_seq: " << onDemandReqData.m_strAppSeq<< endl;;
		std::cout << "byte_swap: "<< onDemandReqData.m_isByteSwap<< endl;;
		std::cout << "word_swap: "<< onDemandReqData.m_isWordSwap<< endl;;
		std::cout <<"metric: "<< onDemandReqData.m_strMetric << endl;
		std::cout <<"version: "<<onDemandReqData.m_strVersion<< endl;
		std::cout <<"wellhead: "<<onDemandReqData.m_strWellhead<< endl;;
		std::cout <<"topic: "<<onDemandReqData.m_strTopic<< endl;
		std::cout <<"usec: "<<strUsec<< endl;
		std::cout <<"timestamp: "<<strTimestamp<< endl;
		std::cout <<"****************************************************************" <<endl;

		PublishJsonHandler::instance().publishJson(msg, msgbus_ctx.m_pContext, pubCtx.m_pContext, sRespTopic);
	}
	catch(const std::exception& e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}

	if(msg != NULL)
	{
		msgbus_msg_envelope_destroy(msg);
		msg = NULL;
	}

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

		CLogger::getInstance().log(INFO, LOGDETAILS(temp));

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
