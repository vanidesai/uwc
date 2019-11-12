/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

//#include "RequestMapper.hpp"
//#include "Httprest.hpp"
#include "BoostLogger.hpp"
//#include "PlatformBusInterface.hpp"
#include "Common.hpp"

extern "C" {
	#include <safe_lib.h>
}

extern src::severity_logger< severity_level > lg;

#if 0
using namespace boostbeast;
/* for boost logging*/
extern src::severity_logger< severity_level > lg;
/* Mutex is used for read write common api callback*/
std::mutex g_RWCommonCallbackMutex;
/* Mutex is used for read device identification api callback*/
std::mutex g_ReadDevIdenCallbackMutex;
/* Mutex is used for read write multiple register api callback*/
std::mutex g_RWMulRegCallbackMutex;
/* Mutex is used for read file record api callback*/
std::mutex g_RFRCallbackMutex;
/* Mutex is used for write file record api callback*/
std::mutex g_WFRCallbackMutex;

/**
 *
 * DESCRIPTION
 * Function is used as application layer callback
 * for read device identification function code
 *
 * @param u8UnitID 			[in] Unit ID
 * @param pu8IpAddr 		[in] IP address
 * @param u16TransacID		[in] Transaction ID
 * @param u8FunCode 		[in] Function code
 * @param pstException 		[in] Exception status
 * @param pstRdDevIdResp 	[in] input request
 *
 * @return void nothing
 *
 */
void ModbusMasterRdDevIdentification_AppCallback(uint8_t  u8UnitID,
							 uint8_t* pu8IpAddr,
							 uint16_t u16TransacID,
							 uint8_t  u8FunCode,
							 stException_t  *pstException,
							 stRdDevIdResp_t* pstRdDevIdResp)
{
	RestRdDevIdInfo_t stModbusRxPacket = {};
	SubObjList_t *pstTempObjList = NULL;
	errno_t eSafeFunRetType;
	std::lock_guard<std::mutex> lock(g_ReadDevIdenCallbackMutex);
	//memset(&stModbusRxPacket,0x00,sizeof(RestRdDevIdInfo_t));
	stModbusRxPacket.RestRdDevIdResponse.m_pstSubObjList.pstNextNode = NULL;
	stModbusRxPacket.RestRdDevIdResponse.m_pstSubObjList.m_u8ObjectValue = NULL;

	try
	{
		if((pu8IpAddr != NULL) && (pstException != NULL))
		{
			stModbusRxPacket.m_stException.m_u8ExcCode = pstException->m_u8ExcCode;
			stModbusRxPacket.m_stException.m_u8ExcStatus = pstException->m_u8ExcStatus;

			if((0 != pstException->m_u8ExcCode) && (0 != pstException->m_u8ExcStatus))
			{
				BOOST_LOG_SEV(lg, info) <<
						"Info::"<< __func__  <<"::exception_code:"<<
						(unsigned)pstException->m_u8ExcCode << "," <<"exception_status:" <<
						(unsigned)pstException->m_u8ExcStatus;
			}

			if((0 == pstException->m_u8ExcCode) && (0 == pstException->m_u8ExcStatus)
					&& (pstRdDevIdResp != NULL))
			{
				SubObjList_t *pstSubObjList = pstRdDevIdResp->m_pstSubObjList.pstNextNode;
				stModbusRxPacket.RestRdDevIdResponse.m_u8ConformityLevel = pstRdDevIdResp->m_u8ConformityLevel;
				stModbusRxPacket.RestRdDevIdResponse.m_u8MEIType = pstRdDevIdResp->m_u8MEIType;
				stModbusRxPacket.RestRdDevIdResponse.m_u8MoreFollows = pstRdDevIdResp->m_u8MoreFollows;
				stModbusRxPacket.RestRdDevIdResponse.m_u8NextObjId = pstRdDevIdResp->m_u8NextObjId;
				stModbusRxPacket.RestRdDevIdResponse.m_u8NumberofObjects = pstRdDevIdResp->m_u8NumberofObjects;
				stModbusRxPacket.RestRdDevIdResponse.m_u8RdDevIDCode = pstRdDevIdResp->m_u8RdDevIDCode;
				stModbusRxPacket.RestRdDevIdResponse.m_pstSubObjList.m_u8ObjectID = pstRdDevIdResp->m_pstSubObjList.
						m_u8ObjectID;
				stModbusRxPacket.RestRdDevIdResponse.m_pstSubObjList.m_u8ObjectLen = pstRdDevIdResp->m_pstSubObjList
						.m_u8ObjectLen ;
				stModbusRxPacket.RestRdDevIdResponse.m_pstSubObjList.m_u8ObjectValue = new unsigned char
						[pstRdDevIdResp->m_pstSubObjList.m_u8ObjectLen]();

				eSafeFunRetType = memcpy_s(stModbusRxPacket.RestRdDevIdResponse.m_pstSubObjList.m_u8ObjectValue,
					stModbusRxPacket.RestRdDevIdResponse.m_pstSubObjList.m_u8ObjectLen,
					pstRdDevIdResp->m_pstSubObjList.m_u8ObjectValue,
					sizeof(unsigned char)*pstRdDevIdResp->m_pstSubObjList.m_u8ObjectLen);
				
				if(eSafeFunRetType != EOK)
				{
					BOOST_LOG_SEV(lg, debug) <<"fatal::"<< __func__<<
							"ObjectValue::memcpy_s return type:" << (unsigned)eSafeFunRetType;
				}

				SubObjList_t *pstRespSubObjList = &stModbusRxPacket.RestRdDevIdResponse.m_pstSubObjList;
				while(pstSubObjList != NULL)
				{
					pstRespSubObjList->pstNextNode = new SubObjList_t();
					SubObjList *pstObjNextList = pstRespSubObjList->pstNextNode;
					pstObjNextList->m_u8ObjectID = pstSubObjList->m_u8ObjectID;
					pstObjNextList->m_u8ObjectLen = pstSubObjList->m_u8ObjectLen;
					pstObjNextList->m_u8ObjectValue = new unsigned char[pstObjNextList->m_u8ObjectLen]();
					eSafeFunRetType = memcpy_s(pstObjNextList->m_u8ObjectValue,
								pstObjNextList->m_u8ObjectLen,
								pstSubObjList->m_u8ObjectValue,
								pstSubObjList->m_u8ObjectLen);

					if(eSafeFunRetType != EOK)
					{
						BOOST_LOG_SEV(lg, debug) <<"fatal::"<<__func__<<
									"::ObjectValue:memcpy_s return type:" << (unsigned)eSafeFunRetType;
					}
					pstObjNextList->pstNextNode = NULL;
					pstSubObjList = pstSubObjList->pstNextNode;
					pstRespSubObjList = pstObjNextList;
				}
			}

			eSafeFunRetType = memcpy_s(&stModbusRxPacket.RestRdDevIdRequest.m_u8IpAddr[0],
				sizeof(stModbusRxPacket.RestRdDevIdRequest.m_u8IpAddr),
				pu8IpAddr,
				sizeof(stModbusRxPacket.RestRdDevIdRequest.m_u8IpAddr));

			if(eSafeFunRetType != EOK)
			{
				BOOST_LOG_SEV(lg, debug) <<"fatal::ModbusMasterRdDevIdentification_AppCallback"
							"::IpAddr:memcpy_s return type:" << (unsigned)eSafeFunRetType;
			}
			
			boostbeast::DeviceIdentification::JsonDevIdentificationCmdEncoder(u16TransacID,
						u8FunCode,&stModbusRxPacket);
		}
		else
		{
			boostbeast::DeviceIdentification::JsonDevIdentificationCmdEncoder(u16TransacID,
									u8FunCode,NULL);
			BOOST_LOG_SEV(lg, info) <<"Error::" <<__func__<< " "<<"NULL Pointer is "
					"Received from stack";
		}
	}
	catch(const std::exception& e)
	{
		BOOST_LOG_SEV(lg, info) <<"fatal::" << __func__ <<
			"::Exception is raised. "<<e.what();
	}

	SubObjList_t *pstSubObjList = (stModbusRxPacket.RestRdDevIdResponse.
					m_pstSubObjList.pstNextNode);

	if(stModbusRxPacket.RestRdDevIdResponse.m_pstSubObjList.m_u8ObjectValue != NULL)
	{
		delete[](stModbusRxPacket.RestRdDevIdResponse.m_pstSubObjList.m_u8ObjectValue);
		stModbusRxPacket.RestRdDevIdResponse.m_pstSubObjList.m_u8ObjectValue = NULL;
	}

	while(pstSubObjList != NULL)
	{
		if(pstSubObjList->m_u8ObjectValue != NULL)
		{
			delete[](pstSubObjList->m_u8ObjectValue);
			pstSubObjList->m_u8ObjectValue = NULL;
		}
		pstTempObjList = pstSubObjList->pstNextNode;
		if(pstSubObjList!= NULL)
		{
			delete(pstSubObjList);
			pstSubObjList = NULL;
		}
		pstSubObjList = pstTempObjList;
	}
}

/**
 *
 * DESCRIPTION
 * Function is used as application layer callback
 * for read write multiple registers function code
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
void ModbusMasterRdWrMulReg_AppCallback(uint8_t  u8UnitID,
		 	 	 	 	 	 uint16_t u16TransacID,
							 uint8_t* pu8IpAddr,
							 uint8_t  u8FunCode,
							 stException_t  *pstException,
							 uint8_t  u8numBytes,
							 uint8_t* pu8data)
{
	boostbeats_ModbusInterface MbusInter;
	AuditLogMsg_t mqttMsg = {};
	json::value AuditLogJson;

	std::lock_guard<std::mutex> lock(g_RWMulRegCallbackMutex);

	try
	{
		if((pu8IpAddr != NULL) && (pstException != NULL))
		{
			if((0 == pstException->m_u8ExcCode) && (0 == pstException->m_u8ExcStatus))
			{
#ifdef TLSENABLED
				session& tempSession = RequestMapper::instance().getTLSSession(u16TransacID);
#else
				http_session& tempSession = RequestMapper::instance().
						getNonTLSSession(u16TransacID);
	#endif
				//memset(&mqttMsg,0x00,sizeof(mqttMsg));
				mqttMsg.m_u8FunCode = u8FunCode;

				boostbeast::convertCharArrToString(pu8IpAddr, mqttMsg.m_ipAddr);

				if(Create_AutditLog_Json(AuditLogJson,&mqttMsg,tempSession))
				{
					PlBusHandler::instance().publishPlBus(AuditLogJson,MQTT_AUDIT_LOG_TOPIC.c_str());
				}
			}

			if((0 != pstException->m_u8ExcCode) && (0 != pstException->m_u8ExcStatus))
			{
				BOOST_LOG_SEV(lg, info) << "Info::"<< __func__ <<"::exception_code:"<<
						(unsigned)pstException->m_u8ExcCode << ","<< "exception_status: "
						<< (unsigned)pstException->m_u8ExcStatus;
			}
			boostbeast::ModbusReadWriteMulRegCommand::
						JsonReadWriteMulRegCmdEncoder(u16TransacID,
							u8FunCode,
							pu8IpAddr,
							pstException,
							u8numBytes,
							pu8data);
		}
		else
		{
			BOOST_LOG_SEV(lg, info) <<"Error::" <<__func__<< " " <<"NULL Pointer is "
								"Received from stack";
			boostbeast::ModbusReadWriteMulRegCommand::
									JsonReadWriteMulRegCmdEncoder(u16TransacID,
										u8FunCode,
										NULL,
										NULL,
										u8numBytes,
										NULL);
		}
	}
	catch(const std::exception& e)
	{
		BOOST_LOG_SEV(lg, info) <<"fatal::" << __func__ <<
					"::Exception is raised. "<<e.what();
	}
}

/**
 *
 * DESCRIPTION
 * Function is used as application layer callback
 * for read/write coils,input register,holding register
 * discrete input function codes
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
							 uint8_t  u8FunCode,
							 stException_t  *pstException,
							 uint8_t  u8numBytes,
							 uint8_t* pu8data,
							 uint16_t  u16StartAdd,
							 uint16_t  u16Quantity)
 {
	boostbeats_ModbusInterface MbusInter;
	RestMbusReqGeneric_t stModbusRxPacket = {};
	AuditLogMsg_t mqttMsg = {};
	json::value AuditLogJson;
	errno_t eSafeFunRetType;
	std::lock_guard<std::mutex> lock(g_RWCommonCallbackMutex);
	//memset(&stModbusRxPacket,0x00,sizeof(RestMbusReqGeneric_t));
	stModbusRxPacket.m_stRespData.m_pu8Data = NULL;

	try
	{
		if((pu8IpAddr != NULL) && (pstException != NULL))
		{
			if((0 != pstException->m_u8ExcCode)&& (0 != pstException->m_u8ExcCode))
			{
				BOOST_LOG_SEV(lg, info) << "Info::"<< __func__ << "::function_code:"
						<< (unsigned)u8FunCode << "," << "exception_code:" <<(unsigned)pstException->m_u8ExcCode
						<< "," << "exception_status " << (unsigned)pstException->m_u8ExcStatus;
			}

			if(((WRITE_SINGLE_COIL == u8FunCode)|| (WRITE_SINGLE_REG == u8FunCode)||
					(WRITE_MULTIPLE_COILS == u8FunCode)|| (WRITE_MULTIPLE_REG == u8FunCode)) &&
					(0 == pstException->m_u8ExcCode) && (0 == pstException->m_u8ExcStatus))
			{
				//memset(&mqttMsg, 0x00, sizeof(mqttMsg));
				boostbeast::convertCharArrToString(pu8IpAddr, mqttMsg.m_ipAddr);
				mqttMsg.m_u8FunCode = u8FunCode;
#ifdef TLSENABLED
				session& tempSession = RequestMapper::instance().getTLSSession(u16TransacID);
#else
				http_session& tempSession = RequestMapper::instance().
						getNonTLSSession(u16TransacID);
#endif
				if(Create_AutditLog_Json(AuditLogJson,&mqttMsg,tempSession))
				{
					PlBusHandler::instance().publishPlBus(AuditLogJson,MQTT_AUDIT_LOG_TOPIC.c_str());
				}
			}

			eSafeFunRetType = memcpy_s(&stModbusRxPacket.m_u8IpAddr[0],sizeof(stModbusRxPacket.m_u8IpAddr),
				pu8IpAddr,sizeof(stModbusRxPacket.m_u8IpAddr));
			
			if(eSafeFunRetType != EOK)
			{
				BOOST_LOG_SEV(lg, debug) <<"fatal::" << __func__ <<
									"::IpAddr:memcpy_s return type:" << (unsigned)eSafeFunRetType;
			}
			
			stModbusRxPacket.m_stRespData.m_stExcStatus.m_u8ExcStatus = pstException->m_u8ExcStatus;
			stModbusRxPacket.m_stRespData.m_stExcStatus.m_u8ExcCode = pstException->m_u8ExcCode;
			stModbusRxPacket.m_u16Quantity = u16Quantity;
			stModbusRxPacket.m_u16StartAddr = u16StartAdd;

			if((0 == pstException->m_u8ExcStatus) && (pu8data != NULL))
			{
				stModbusRxPacket.m_stRespData.m_u8DataLen = u8numBytes;
				stModbusRxPacket.m_stRespData.m_pu8Data = new uint8_t[u8numBytes]();
				eSafeFunRetType = memcpy_s(stModbusRxPacket.m_stRespData.m_pu8Data,
						sizeof(unsigned char)*u8numBytes,
						pu8data,
						sizeof(unsigned char)*u8numBytes);
				if(eSafeFunRetType != EOK)
				{
					BOOST_LOG_SEV(lg, debug) <<"fatal::" << __func__ <<
										"::pu8Data:memcpy_s return type:" << (unsigned)eSafeFunRetType;
				}
			}

			u8FunCode = (u8FunCode & 0x7F);
			stModbusRxPacket.m_u8FunCode = u8FunCode;

			boostbeast::MBSpecCmd::boostbeast_JsonSpecCmdEncoder(&stModbusRxPacket,
					u16TransacID);
		}
		else
		{
			BOOST_LOG_SEV(lg, info) <<"Error::" <<__func__<< " " << "NULL Pointer is "
								"Received from stack";
			boostbeast::MBSpecCmd::boostbeast_JsonSpecCmdEncoder(NULL,
					u16TransacID);
		}
	}
	catch(const std::exception& e)
	{
		BOOST_LOG_SEV(lg, info) <<"fatal::" << __func__ << "::Exception is raised. "<<e.what();
	}

	if(stModbusRxPacket.m_stRespData.m_pu8Data != NULL)
	{
		delete[](stModbusRxPacket.m_stRespData.m_pu8Data);
		stModbusRxPacket.m_stRespData.m_pu8Data = NULL;
	}
 }

/**
 *
 * DESCRIPTION
 * Function is used as application layer callback
 * for read file record function code.
 *
 * @param u8UnitID				[in] Unit ID
 * @param u16TransacID			[in] Transaction ID
 * @param pu8IpAddr				[in] IP address
 * @param u8FunCode				[in] Function code
 * @param pstException			[in] Exception status
 * @param pstRdFileRecdResp		[in] input request
 *
 * @return void nothing
 *
 */
void AppReadFileRecord_CallbackFunction(uint8_t  u8UnitID,
		 	 	 	 	 	 	 	 	uint8_t* pu8IpAddr,
										uint16_t u16TransacID,
										uint8_t  u8FunCode,
										stException_t  *pstException,
										stMbusRdFileRecdResp_t* pstRdFileRecdResp)
{
	stRdFileSubReq_t *pstRdFileSubReq = NULL;
	RestReadFileRecordResp_t stReadFileRecordResp = {};
	stRdFileSubReq_t *ptrRdFileSubResp = NULL;
	stRdFileSubReq_t *pstTempSubReq = NULL;
	stRdFileSubReq_t *pstSubReq = NULL;
	errno_t eSafeFunRetType;

	std::lock_guard<std::mutex> lock(g_RFRCallbackMutex);

	//memset(&stReadFileRecordResp,0x00,sizeof(stReadFileRecordResp));
	stReadFileRecordResp.m_stMbusRdFileRecdResp.m_stSubReq.m_pu16RecData = NULL;
	stReadFileRecordResp.m_stMbusRdFileRecdResp.m_stSubReq.pstNextNode = NULL;

	try
	{
		if((pu8IpAddr != NULL) && (pstException != NULL))
		{
			stReadFileRecordResp.m_stException.m_u8ExcCode = pstException->m_u8ExcCode;
			stReadFileRecordResp.m_stException.m_u8ExcStatus = pstException->m_u8ExcStatus;
			stReadFileRecordResp.m_u8FunCode = (u8FunCode & 0x7F);

			if((0 != pstException->m_u8ExcCode)&& (0 != pstException->m_u8ExcCode))
			{
				BOOST_LOG_SEV(lg, info) <<
						"Info::" << __func__ << "::exception_code:"
						<< (unsigned)pstException->m_u8ExcCode << ","
						<< "exception_status:"<< (unsigned)pstException->m_u8ExcStatus;
			}

			if((0 == pstException->m_u8ExcCode) && (0 == pstException->m_u8ExcCode)
					&& (pstRdFileRecdResp != NULL))
			{
				stReadFileRecordResp.m_stMbusRdFileRecdResp.m_u8RespDataLen = pstRdFileRecdResp->m_u8RespDataLen;
				ptrRdFileSubResp = &(stReadFileRecordResp.m_stMbusRdFileRecdResp.m_stSubReq);
				pstRdFileSubReq = &pstRdFileRecdResp->m_stSubReq;

				while(NULL != pstRdFileSubReq)
				{
					ptrRdFileSubResp->m_u8RefType = pstRdFileSubReq->m_u8RefType;
					ptrRdFileSubResp->m_u8FileRespLen = pstRdFileSubReq->m_u8FileRespLen;
					ptrRdFileSubResp->m_pu16RecData = new uint16_t[pstRdFileSubReq->m_u8FileRespLen/2]();
									
					eSafeFunRetType = memcpy_s(&ptrRdFileSubResp->m_pu16RecData[0],
						((pstRdFileSubReq->m_u8FileRespLen)/2) *sizeof(uint16_t),
						pstRdFileSubReq->m_pu16RecData,
						((pstRdFileSubReq->m_u8FileRespLen)/2) *sizeof(uint16_t));

					if(eSafeFunRetType != EOK)
					{
						BOOST_LOG_SEV(lg, debug) <<"fatal::AppReadFileRecord_CallbackFunction"
											"::pu16RecData:memcpy_s return type:" << (unsigned)eSafeFunRetType;
					}

					pstRdFileSubReq = pstRdFileSubReq->pstNextNode;
					if(pstRdFileSubReq != NULL)
					{
						ptrRdFileSubResp->pstNextNode = new stRdFileSubReq_t();
						ptrRdFileSubResp = ptrRdFileSubResp->pstNextNode;
						ptrRdFileSubResp->pstNextNode = NULL;
					}
				}
			}

			boostbeast::ReadFileRecordCommand::boostbeast_JsonReadFileRecordCmdEncoder(
					u16TransacID,
					pu8IpAddr,
					&stReadFileRecordResp);
		}
		else
		{
			BOOST_LOG_SEV(lg, info) <<"Error::" <<__func__<< " " <<"NULL Pointer is "
								"Received from stack";
			boostbeast::ReadFileRecordCommand::boostbeast_JsonReadFileRecordCmdEncoder(
								u16TransacID,
								NULL,
								NULL);
		}
	}
	catch(const std::exception& e)
	{
		BOOST_LOG_SEV(lg, info) <<"fatal::" << __func__ <<
				"::Exception is raised. "<<e.what();
	}
	// Free response data
	if(stReadFileRecordResp.m_stMbusRdFileRecdResp.m_stSubReq.m_pu16RecData != NULL)
	{
		delete[](stReadFileRecordResp.m_stMbusRdFileRecdResp.m_stSubReq.m_pu16RecData);
		stReadFileRecordResp.m_stMbusRdFileRecdResp.m_stSubReq.m_pu16RecData = NULL;
	}
	pstSubReq = stReadFileRecordResp.m_stMbusRdFileRecdResp.m_stSubReq.pstNextNode;
	while(pstSubReq != NULL)
	{
		if(pstSubReq->m_pu16RecData != NULL)
		{
			delete[](pstSubReq->m_pu16RecData);
			pstSubReq->m_pu16RecData = NULL;
		}
		pstTempSubReq = pstSubReq->pstNextNode;
		pstSubReq->pstNextNode = NULL;
		delete(pstSubReq);
		pstSubReq = pstTempSubReq;
	}
}

/**
 *
 * DESCRIPTION
 * Function is used as application layer callback
 * for write file record function code.
 *
 * @param u8UnitID				[in] Unit ID
 * @param u16TransacID			[in] Transaction ID
 * @param pu8IpAddr				[in] IP address
 * @param u8FunCode				[in] Function code
 * @param pstException			[in] Exception status
 * @param pstMbusWrFileRecdResp	[in] input request
 *
 * @return void nothing
 *
 */
void AppWriteFileRecord_CallbackFunction(uint8_t  u8UnitID,
		uint8_t* pu8IpAddr,
		uint16_t u16TransacID,
		uint8_t  u8FunCode,
		stException_t  *pstException,
		stMbusWrFileRecdResp_t* pstMbusWrFileRecdResp)
{

	RestWriteFileRecordResp_t stRestWriteFileRecoResp = {};
	stWrFileSubReq_t *pstWrFileSubReq = NULL;
	stWrFileSubReq_t *stWrFileSubReqResp = NULL;
	stWrFileSubReq_t *pstTempSubTagPart = NULL;
	AuditLogMsg_t mqttMsg = {};
	json::value AuditLogJson;
	errno_t eSafeFunRetType;

	std::lock_guard<std::mutex> lock(g_WFRCallbackMutex);

	//memset(&stRestWriteFileRecoResp,0x00,sizeof(stRestWriteFileRecoResp));
	stRestWriteFileRecoResp.m_stMbusWrFileRecdResp.m_stSubReq.m_pu16RecData = NULL;
	stRestWriteFileRecoResp.m_stMbusWrFileRecdResp.m_stSubReq.pstNextNode = NULL;

	try
	{
		if((pu8IpAddr != NULL) && (pstException != NULL))
		{
			stRestWriteFileRecoResp.m_stException.m_u8ExcCode = pstException->m_u8ExcCode;
			stRestWriteFileRecoResp.m_stException.m_u8ExcStatus = pstException->m_u8ExcStatus;
			stRestWriteFileRecoResp.m_u8FunCode = (u8FunCode & 0x7F);

			if((0 != pstException->m_u8ExcCode)&& (0 != pstException->m_u8ExcCode))
			{
				BOOST_LOG_SEV(lg, info) <<
						"Info::" << __func__ << "::exception_code:"
						<< (unsigned)pstException->m_u8ExcCode << ","<< "exception_status: "
						<<(unsigned)pstException->m_u8ExcStatus;
			}

			if((0 == pstException->m_u8ExcCode)&& (0 == pstException->m_u8ExcCode)
					&& (pstMbusWrFileRecdResp != NULL))
			{
				//memset(&mqttMsg, 0x00, sizeof(mqttMsg));
				boostbeast::convertCharArrToString(pu8IpAddr, mqttMsg.m_ipAddr);
				mqttMsg.m_u8FunCode = u8FunCode;

#ifdef TLSENABLED
				session& tempSession = RequestMapper::instance().getTLSSession(u16TransacID);
#else
				http_session& tempSession = RequestMapper::instance().
						getNonTLSSession(u16TransacID);
#endif
				if(Create_AutditLog_Json(AuditLogJson,&mqttMsg,tempSession))
				{
					PlBusHandler::instance().publishPlBus(AuditLogJson,MQTT_AUDIT_LOG_TOPIC.c_str());
				}

				stRestWriteFileRecoResp.m_stMbusWrFileRecdResp.m_u8RespDataLen =
						pstMbusWrFileRecdResp->m_u8RespDataLen;
				stWrFileSubReqResp = &(stRestWriteFileRecoResp.m_stMbusWrFileRecdResp.m_stSubReq);
				pstWrFileSubReq = &(pstMbusWrFileRecdResp->m_stSubReq);
				while(NULL != pstWrFileSubReq)
				{
					stWrFileSubReqResp->m_u16FileNum = pstWrFileSubReq->m_u16FileNum;
					stWrFileSubReqResp->m_u16RecLen = pstWrFileSubReq->m_u16RecLen;
					stWrFileSubReqResp->m_u16RecNum = pstWrFileSubReq->m_u16RecNum;
					stWrFileSubReqResp->m_u8RefType = pstWrFileSubReq->m_u8RefType;
					stWrFileSubReqResp->m_pu16RecData = new uint16_t[pstWrFileSubReq->m_u16RecLen]();
					//memset(stWrFileSubReqResp->m_pu16RecData,0x00,(sizeof(uint16_t)*pstWrFileSubReq->m_u16RecLen));
					eSafeFunRetType = memcpy_s(&stWrFileSubReqResp->m_pu16RecData[0],
						sizeof(uint16_t)*pstWrFileSubReq->m_u16RecLen,
						pstWrFileSubReq->m_pu16RecData,
						sizeof(uint16_t)*pstWrFileSubReq->m_u16RecLen);
					if(eSafeFunRetType != EOK)
					{
						BOOST_LOG_SEV(lg, debug) <<"fatal::" << __func__ <<
							"::pu16RecData:memcpy_s return type:" << (unsigned)eSafeFunRetType;
					}
					
					pstWrFileSubReq = pstWrFileSubReq->pstNextNode;
					if(pstWrFileSubReq != NULL)
					{
						stWrFileSubReqResp->pstNextNode = new stWrFileSubReq_t();
						stWrFileSubReqResp = stWrFileSubReqResp->pstNextNode;
						stWrFileSubReqResp->pstNextNode = NULL;
					}
				}
			}
			boostbeast::WriteFileRecordCommand::boostbeast_JsonWriteFileRecordCmdEncoder(
					u16TransacID,
					pu8IpAddr,
					&stRestWriteFileRecoResp);
		}
		else
		{
			BOOST_LOG_SEV(lg, info) <<"Error::" <<__func__<< "NULL Pointer is "
								"Received from stack";
			boostbeast::WriteFileRecordCommand::boostbeast_JsonWriteFileRecordCmdEncoder(
								u16TransacID,
								NULL,
								NULL);
		}
	}
	catch(const std::exception& e)
	{
		BOOST_LOG_SEV(lg, info) <<"fatal::" << __func__ <<
						"::Exception is raised. "<<e.what();
	}
	// Free response data
	if(stRestWriteFileRecoResp.m_stMbusWrFileRecdResp.m_stSubReq.m_pu16RecData != NULL)
	{
		delete[](stRestWriteFileRecoResp.m_stMbusWrFileRecdResp.m_stSubReq.m_pu16RecData);
		stRestWriteFileRecoResp.m_stMbusWrFileRecdResp.m_stSubReq.m_pu16RecData = NULL;
	}

	stWrFileSubReqResp = stRestWriteFileRecoResp.m_stMbusWrFileRecdResp.m_stSubReq.pstNextNode;

	while(stWrFileSubReqResp != NULL)
	{
		if(stWrFileSubReqResp->m_pu16RecData != NULL)
		{
			delete[](stWrFileSubReqResp->m_pu16RecData);
			stWrFileSubReqResp->m_pu16RecData = NULL;
		}

		pstTempSubTagPart  = stWrFileSubReqResp->pstNextNode;
		stWrFileSubReqResp->pstNextNode = NULL;
		delete(stWrFileSubReqResp);
		stWrFileSubReqResp = pstTempSubTagPart;
	}
}

/**
 * DESCRIPTION
 * This function is to convert character array to string.
 *
 * @param ipBuffer		[in] input buffer
 * @param dstCopyStr 	[in] string
 *
 * @return bool		    [out] return true on success
 *
 */
bool boostbeast::convertCharArrToString(unsigned char *ipBuffer, string &dstCopyStr)
{
	dstCopyStr = to_string(ipBuffer[0]) + "." +
			to_string(ipBuffer[1]) + "." +
			to_string(ipBuffer[2]) + "." +
			to_string(ipBuffer[3]);
	return true;
}
#endif /*0*/

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
	uint8_t u8ReturnType = MBUS_JSON_APP_ERROR_NULL_POINTER;

	if(pstMbusApiPram != NULL)
	{
		switch ((eModbusFuncCode_enum)u8FunCode)
		{
			case READ_COIL_STATUS:
				BOOST_LOG_SEV(lg, info) << "Info::Read Coil Request Received";
				u8ReturnType = Modbus_Read_Coils(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					vpCallBackFun);
			break;
			case READ_INPUT_STATUS:
				BOOST_LOG_SEV(lg, info) << "Info::Read Input Status Request Received";
				u8ReturnType = Modbus_Read_Discrete_Inputs(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					vpCallBackFun);
			break;
			case READ_HOLDING_REG:
				BOOST_LOG_SEV(lg, info) << "Info::Read Holding Register Request Received";
				u8ReturnType = Modbus_Read_Holding_Registers(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					vpCallBackFun);
			break;
			case READ_INPUT_REG:
				BOOST_LOG_SEV(lg, info) << "Info::Read Input Register Request Received";
				u8ReturnType = Modbus_Read_Input_Registers(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					vpCallBackFun);
			break;
			case WRITE_SINGLE_COIL:
			{
				BOOST_LOG_SEV(lg, info) << "Info::Write Single Coil Request Received";
				uint16_t *pu16OutData = (uint16_t *)pstMbusApiPram->m_pu8Data;
				u8ReturnType = Modbus_Write_Single_Coil(pstMbusApiPram->m_u16StartAddr,
						*pu16OutData,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					vpCallBackFun);
			}
			break;

			case WRITE_SINGLE_REG:
			{
				BOOST_LOG_SEV(lg, info) << "Info::Write Single Register Request Received";
				uint16_t u16OutData = 0;
				memcpy_s(&u16OutData,sizeof(uint16_t),pstMbusApiPram->m_pu8Data,sizeof(uint16_t));
				u8ReturnType = Modbus_Write_Single_Register(pstMbusApiPram->m_u16StartAddr,
					*(uint16_t*)pstMbusApiPram->m_pu8Data,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					vpCallBackFun);
			}
			break;
			case WRITE_MULTIPLE_COILS:
				BOOST_LOG_SEV(lg, info) << "Info::Write Multiple Coils Request Received";
				u8ReturnType = Modbus_Write_Multiple_Coils(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_pu8Data,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					vpCallBackFun);
			break;
			case WRITE_MULTIPLE_REG:
				BOOST_LOG_SEV(lg, info) << "Info::Write Multiple Register Request Received";
				u8ReturnType = Modbus_Write_Multiple_Register(pstMbusApiPram->m_u16StartAddr,
					pstMbusApiPram->m_u16Quantity,
					pstMbusApiPram->m_u16TxId,
					pstMbusApiPram->m_pu8Data,
					pstMbusApiPram->m_u8DevId,
					pstMbusApiPram->m_u8IpAddr,
					vpCallBackFun);
			break;

			default:
				BOOST_LOG_SEV(lg, info) <<
						"Error::" << __func__ <<":Invalid Request Received";
				u8ReturnType = MBUS_STACK_ERROR_INVALID_INPUT_PARAMETER;
			break;
		}
	}
	return u8ReturnType;
}
#if 0
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
uint8_t boostbeats_ModbusInterface :: MbusApp_Process_Request(RestMbusReqGeneric_t *pstMbusReqGen)
{
	MbusAPI_t stMbusApiPram = {};
	uint8_t u8ReturnType = MBUS_JSON_APP_ERROR_NULL_POINTER;

	//memset(&stMbusApiPram, 0x00, sizeof(stMbusApiPram));

	if(pstMbusReqGen != NULL)
	{
		stMbusApiPram.m_pu8Data = pstMbusReqGen->m_stReqData.m_pu8Data;
		stMbusApiPram.m_u16ByteCount =  pstMbusReqGen->m_stReqData.m_u8DataLen;
		stMbusApiPram.m_u16Quantity = pstMbusReqGen->m_u16Quantity;
		stMbusApiPram.m_u16StartAddr = pstMbusReqGen->m_u16StartAddr;
		stMbusApiPram.m_u8DevId = pstMbusReqGen->m_u8NodeId;
		stMbusApiPram.m_u16TxId = pstMbusReqGen->m_u16ReffId;
		stMbusApiPram.m_u8IpAddr[0] = pstMbusReqGen->m_u8IpAddr[0];
		stMbusApiPram.m_u8IpAddr[1] = pstMbusReqGen->m_u8IpAddr[1];
		stMbusApiPram.m_u8IpAddr[2] = pstMbusReqGen->m_u8IpAddr[2];
		stMbusApiPram.m_u8IpAddr[3] = pstMbusReqGen->m_u8IpAddr[3];

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

char Header[][20] = { "Device_ID", "IP_Addr", "Device_Tag", "Num_DI", "Num_COILS", "Num_INPUT_REG", "Num_HOLDING_REG" };

#if 0
/**
 *
 * DESCRIPTION
 * function to read IP address.
 *
 * @param i8IpAddress [in] IP address
 * @param pi8IpAdr	  [in] IP address
 *
 * @return uint8_t	  [out]	return 0 on success
 */
uint8_t GetIpAddr(const char *i8IpAddress, char *pi8IpAdr)
{

	signed char *pi8Char = NULL;
	uint8_t indexIp = 0;
	uint8_t loop;
	uint32_t IPADDR[5] = { 0 };

	for (loop = 0; loop < sizeof(i8IpAddress); loop++)
	{
		if (i8IpAddress[loop] == '.')
			indexIp++;
	}

	if (indexIp > 3)
	{
		///continue;
	}

	/// Network address octates diff with dot symbol
	pi8Char = (signed char *)strtok((char *)i8IpAddress, ".");

	/// Check for three dot symbols and convert octactes
	if (NULL == pi8Char)
	{
		///continue;
	}
	///stDestinationInfo->Ipaddr[0] = (uint8_t)(stoi((const char *)pi8Char));
	IPADDR[0] = stoi((const char *)pi8Char);
	pi8IpAdr[0] = (uint8_t)IPADDR[0];

	pi8Char = (signed char *)strtok(NULL, ".");
	if (NULL == pi8Char)
	{
		///continue;
	}
	///	stDestinationInfo->Ipaddr[1] = (uint8_t)(stoi((const char *)pi8Char));
	IPADDR[1] = stoi((const char *)pi8Char);
	pi8IpAdr[1] = (uint8_t)IPADDR[1];
	if (NULL == pi8Char)
	{
		///continue;
	}
	pi8Char = (signed char *)strtok(NULL, ".");
	if (NULL == pi8Char)
	{
		///continue;
	}
	///stDestinationInfo->Ipaddr[2] = (uint8_t)(stoi((const char *)pi8Char));
	IPADDR[2] = stoi((const char *)pi8Char);
	pi8IpAdr[2] = (uint8_t)IPADDR[2];

	pi8Char = (signed char *)strtok(NULL, ".");
	if (NULL == pi8Char)
	{
		///continue;
	}
	///stDestinationInfo->Ipaddr[3] = (uint8_t)(stoi((const char *)pi8Char));
	IPADDR[3] = stoi((const char *)pi8Char);
	pi8IpAdr[3] = (uint8_t)IPADDR[3];
	if (IPADDR[0] > 255 || IPADDR[1] > 255 || IPADDR[2] > 255 || IPADDR[3] > 255)
	{
		///continue;
	}

	/*if (stDestinationInfo->Ipaddr[0]>255 || stDestinationInfo->Ipaddr[1]>255 || stDestinationInfo->Ipaddr[2]>255 || stDestinationInfo->Ipaddr[3]>255)
	{
	continue;
	}*/

	if ((pi8IpAdr[3] + pi8IpAdr[2] + pi8IpAdr[1] + pi8IpAdr[0]) == 0)
	{
		///continue;
	}
	return 1;
}

/**
 *	DESCRIPTION
 *	This function is used to validate input parameters in generic request.
 *	@param pstSubTagPart [in] structure to store the request.
 *	@return unit8        [out]	return 0 on success
 */
uint8_t boostbeats_ModbusInterface::GenericRequestInputDataVerifiaction(SubTagPart_t *pstSubTagPart)
{
	uint8_t u8ReturnType = MBUS_STACK_NO_ERROR;

	while(1)
	{
		/// address validations w.r.t. data length
		if(pstSubTagPart->m_u16StartAddr > pstSubTagPart->m_u16EndAddr)
		{
			u8ReturnType = MBUS_JSON_APP_ERROR_INVALID_INPUT_PARAMETER;
			break;
		}
		/// function code validations w.r.t. data length
		if(((pstSubTagPart->m_u8FunCode == WRITE_SINGLE_REG) ||
				(pstSubTagPart->m_u8FunCode == WRITE_MULTIPLE_REG) ||
				(pstSubTagPart->m_u8FunCode == READ_WRITE_MUL_REG)))
		{
			if(((pstSubTagPart->m_stReqData.m_u8DataLen >= 2) &&
			 (pstSubTagPart->m_stReqData.m_u8DataLen % 2 == 0) &&
			 (pstSubTagPart->m_stReqData.m_u8DataLen ==
					(((pstSubTagPart->m_u16EndAddr-pstSubTagPart->m_u16StartAddr)+1)*2))))
			{

			}
			else
			{
				u8ReturnType = MBUS_JSON_APP_ERROR_INVALID_INPUT_PARAMETER;
				break;
			}
		}

		if(WRITE_MULTIPLE_COILS == pstSubTagPart->m_u8FunCode)
		{
			uint8_t u8Quantity = pstSubTagPart->m_u16EndAddr-pstSubTagPart->m_u16StartAddr+1;
			uint8_t u8ByteCount = (0 != (u8Quantity%8))?((u8Quantity/8)+1):(u8Quantity/8);

			if((0 < pstSubTagPart->m_stReqData.m_u8DataLen) &&
				(247 > pstSubTagPart->m_stReqData.m_u8DataLen) &&
				pstSubTagPart->m_stReqData.m_u8DataLen == u8ByteCount)
			{
				;
			}
			else
			{
				u8ReturnType = MBUS_JSON_APP_ERROR_INVALID_INPUT_PARAMETER;
				break;
			}
		}
		if(WRITE_SINGLE_COIL == pstSubTagPart->m_u8FunCode)
		{
			if(1 == pstSubTagPart->m_stReqData.m_u8DataLen)
			{
				pstSubTagPart->m_stReqData.m_u8DataLen = 2;
				if(1 == pstSubTagPart->m_stReqData.m_pu8Data[0])
				{
					pstSubTagPart->m_stReqData.m_pu8Data[1] = 255;
				}
				else if(0 == pstSubTagPart->m_stReqData.m_pu8Data[0])
				{
					pstSubTagPart->m_stReqData.m_pu8Data[1] = 0;
				}
				else
				{
					u8ReturnType = MBUS_JSON_APP_ERROR_INVALID_INPUT_PARAMETER;
					break;
				}
				pstSubTagPart->m_stReqData.m_pu8Data[0] = 0;
			}
			else
			{
				u8ReturnType = MBUS_JSON_APP_ERROR_INVALID_INPUT_PARAMETER;
				break;
			}
		}
		break;
	}

	return u8ReturnType;
}

#endif /* 0 */

/**
 * DESCRIPTION
 * This function is used to process Read File Record request
 *
 * @param pstModbusRxPacket [in] structure to store the request.
 * @param u16TransacID		[in] transaction ID
 *
 * @return uint8_t			[out] return 0 on success
 *
 */
uint8_t boostbeats_ModbusInterface ::MbusApp_ReadFileRecord_Request(stRestReadFileRecord_t *pstModbusRxPacket,
		uint16_t u16TransacID)
{
	uint8_t u8ReturnType = MBUS_JSON_APP_ERROR_NULL_POINTER;

	if(pstModbusRxPacket != NULL)
	{
		u8ReturnType = Modbus_Read_File_Record(pstModbusRxPacket->m_u8ByteCount,
				pstModbusRxPacket->m_u8FunCode,
				&pstModbusRxPacket->SubRequest,
				u16TransacID,
				pstModbusRxPacket->m_u8UnitId,
				(unsigned char*)pstModbusRxPacket->m_u8IpAddr,
				(void*)AppReadFileRecord_CallbackFunction);
	}
	return(u8ReturnType);
}

/**
 * DESCRIPTION
 * This function is used to process write File Record request
 *
 * @param pstModbusRxPacket [in] structure to store the request.
 * @param u16TransacID		[in] transaction ID
 *
 * @return uint8_t			[out] return 0 on success
 *
 */
uint8_t boostbeats_ModbusInterface ::MbusApp_WriteFileRecord_Request(RestWriteFileRecord_t *pstModbusRxPacket,
		uint16_t u16TransacID)
{
	uint8_t u8ReturnType = MBUS_JSON_APP_ERROR_NULL_POINTER;

	if(pstModbusRxPacket != NULL)
	{
		u8ReturnType = Modbus_Write_File_Record(pstModbusRxPacket->m_u8ReqDataLen,
								pstModbusRxPacket->m_u8FunCode,
								&pstModbusRxPacket->SubRequest,
								u16TransacID,
								pstModbusRxPacket->m_u8UnitId,
								(unsigned char*)pstModbusRxPacket->m_u8IpAddr,
								(void*)AppWriteFileRecord_CallbackFunction);
	}
	return(u8ReturnType);
}

/**
 * DESCRIPTION
 *This function is used to process Read write multiple registers request
 *
 * @param pstModbusRxPacket [in] structure to store the request.
 * @param u16TransacID		[in] transaction ID
 *
 * @return uint8_t			[out] return 0 on success
 *
 */
uint8_t boostbeats_ModbusInterface ::MbusApp_ReadWriteMulReg_Request(RestRdWrMulRegReq_t *pstModbusRxPacket,
		uint16_t u16TransacID)
{
	uint8_t u8ReturnType = MBUS_JSON_APP_ERROR_NULL_POINTER;

	if(pstModbusRxPacket != NULL)
	{
		u8ReturnType = Modbus_Read_Write_Registers(pstModbusRxPacket->m_u16RdStartAddr,
				pstModbusRxPacket->m_u8FunCode,
				pstModbusRxPacket->m_u16RdQuantity,pstModbusRxPacket->m_u16WrStartAddr,
				pstModbusRxPacket->m_u16WrQuantity,u16TransacID,pstModbusRxPacket->m_pu8ReqData,
				pstModbusRxPacket->m_u8UnitId,(unsigned char*)pstModbusRxPacket->m_u8IpAddr,
				(void*)ModbusMasterRdWrMulReg_AppCallback);
	}
	return(u8ReturnType);
}

/**
 * DESCRIPTION
 * This function is used to process Read device identification request
 *
 * @param pstModbusRxPacket [in] structure to store the request.
 * @param u16TransacID		[in] transaction ID
 *
 * @return uint8_t			[out] return 0 on success
 *
 */
uint8_t boostbeats_ModbusInterface ::MbusApp_ReadDevIdentification_Request(RestRdDevIdInfo_t *pstModbusRxPacket,
		uint16_t u16TransacID)
{
	uint8_t u8ReturnType = MBUS_JSON_APP_ERROR_NULL_POINTER;

	if(pstModbusRxPacket != NULL)
	{
		u8ReturnType = Modbus_Read_Device_Identification(
				pstModbusRxPacket->RestRdDevIdRequest.m_u8MEIType,
				pstModbusRxPacket->RestRdDevIdRequest.m_u8FunCode,
							pstModbusRxPacket->RestRdDevIdRequest.m_u8RdDevIDCode,
							pstModbusRxPacket->RestRdDevIdRequest.m_u8ObjectID,
							u16TransacID,
							pstModbusRxPacket->RestRdDevIdRequest.m_u8UnitId,
							(unsigned char*)pstModbusRxPacket->RestRdDevIdRequest.m_u8IpAddr,
							(void*)ModbusMasterRdDevIdentification_AppCallback);
	}
	return(u8ReturnType);
}

#endif /*0*/
