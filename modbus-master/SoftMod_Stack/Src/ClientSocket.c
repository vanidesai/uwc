/*************************************************************************
*                   Copyright (c) by Softdel Systems              
*                                                                       
*   This software is copyrighted by and is the sole property of Softdel
*   Systems. All rights, title, ownership, or other interests in the
*   software remain the property of Softdel Systems. This software
*   may only be used in accordance with the corresponding license
*   agreement. Any unauthorized use, duplication, transmission,
*   distribution, or disclosure of this software is expressly forbidden. 
*                                                                       
*   This Copyright notice may not be removed or modified without prior   
*   written consent of Softdel Systems.                               
*                                                                       
*   Softdel Systems reserves the right to modify this software       
*   without notice.                                                      
*************************************************************************/

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include "StackConfig.h"
#include "osalLinux.h"
#include <fcntl.h>
#include <safe_lib.h>

extern stDevConfig_t ModbusMasterConfig;

void (*ModbusMaster_ApplicationCallback)(uint8_t  ,
											 uint16_t ,
											 uint8_t* ,
											 uint8_t  ,
											 stException_t*,
											 uint8_t  ,
											 uint8_t* ,
											 uint16_t,
											 uint16_t);


void (*ReadFileRecord_CallbackFunction)(uint8_t, uint8_t*,uint16_t,uint8_t,
		 stException_t *,
		stMbusRdFileRecdResp_t*);

void (*WriteFileRecord_CallbackFunction)(uint8_t, uint8_t*, uint16_t,uint8_t,
		 stException_t*,
		stMbusWrFileRecdResp_t*);

void (*ReadDeviceIdentification_CallbackFunction)(uint8_t, uint8_t*, uint16_t,uint8_t,
		 stException_t*,
		 stRdDevIdResp_t*);

/**
 *
 * Description
 * Application callback handler
 *
 * @param pstMBusRequesPacket [in] Request packet
 * @param eMbusStackErr       [in] Stack error codes
 *
 */
 void ApplicationCallBackHandler(stMbusPacketVariables_t *pstMBusRequesPacket,eStackErrorCode eMbusStackErr)
 {
	stException_t  stException = {0};

	if(NULL == pstMBusRequesPacket)
		return;

	eModbusFuncCode_enum eMbusFunctionCode = ((pstMBusRequesPacket->m_u8FunctionCode) & 0x7F);
	uint8_t u8exception = (8 == ((pstMBusRequesPacket->m_u8FunctionCode) & 0x80)>>4)?1:0;

	if(u8exception)
	{
		stException.m_u8ExcStatus = MODBUS_EXCEPTION;
		stException.m_u8ExcCode = pstMBusRequesPacket->m_stMbusRxData.m_au8DataFields[0];
		pstMBusRequesPacket->m_stMbusRxData.m_au8DataFields[0] = 0;
		pstMBusRequesPacket->m_stMbusRxData.m_u8Length = 0;
		pstMBusRequesPacket->m_u8FunctionCode = (pstMBusRequesPacket->m_u8FunctionCode & (0x7F));
	}
	else if(STACK_NO_ERROR != eMbusStackErr)
	{
		stException.m_u8ExcStatus = MODBUS_STACK_ERROR;
		stException.m_u8ExcCode = eMbusStackErr;
		pstMBusRequesPacket->m_stMbusRxData.m_u8Length = 0;
	}

	switch(eMbusFunctionCode)
	{
		default:
		break;
		case READ_COIL_STATUS :
		case READ_INPUT_STATUS :
		case READ_HOLDING_REG :
		case READ_INPUT_REG :
		case READ_WRITE_MUL_REG :
		case WRITE_MULTIPLE_COILS :
		case WRITE_SINGLE_COIL :
		case WRITE_SINGLE_REG :
		case WRITE_MULTIPLE_REG :

			ModbusMaster_ApplicationCallback = pstMBusRequesPacket->pFunc;

			if(NULL != ModbusMaster_ApplicationCallback)
			{
				ModbusMaster_ApplicationCallback(
						pstMBusRequesPacket->m_u8UnitID,
						pstMBusRequesPacket->m_u16TransactionID,
						pstMBusRequesPacket->m_u8IpAddr,
						pstMBusRequesPacket->m_u8FunctionCode,
						&stException,
						pstMBusRequesPacket->m_stMbusRxData.m_u8Length,
						pstMBusRequesPacket->m_stMbusRxData.m_au8DataFields,
						pstMBusRequesPacket->m_u16StartAdd,
						pstMBusRequesPacket->m_u16Quantity);
			}
		break;

		case READ_FILE_RECORD :
		{
			stMbusRdFileRecdResp_t	*pstMbusRdFileRecdResp = NULL;
			stRdFileSubReq_t	*pstSubReq = NULL;
			stRdFileSubReq_t	*pstTempSubReq = NULL;

			pstMbusRdFileRecdResp =
					pstMBusRequesPacket->m_stMbusRxData.m_pvAdditionalData;

			ReadFileRecord_CallbackFunction = pstMBusRequesPacket->pFunc;

			if(NULL != ReadFileRecord_CallbackFunction)
				ReadFileRecord_CallbackFunction(pstMBusRequesPacket->m_u8UnitID,
						pstMBusRequesPacket->m_u8IpAddr,
						pstMBusRequesPacket->m_u16TransactionID,
						pstMBusRequesPacket->m_u8FunctionCode,
						&stException,pstMbusRdFileRecdResp);

			if(NULL != pstMbusRdFileRecdResp)
				pstSubReq = pstMbusRdFileRecdResp->m_stSubReq.pstNextNode;

			while(NULL != pstSubReq)
			{
				OSAL_Free(pstSubReq->m_pu16RecData);
				pstTempSubReq = pstSubReq->pstNextNode;
				OSAL_Free(pstSubReq);
				pstSubReq = pstTempSubReq;
			}
			if(NULL != pstMbusRdFileRecdResp)
			{
				OSAL_Free(pstMbusRdFileRecdResp);
				pstMBusRequesPacket->m_stMbusRxData.m_pvAdditionalData = NULL;
			}
		}
		break;
		case WRITE_FILE_RECORD :
		{
			stMbusWrFileRecdResp_t	*pstMbusWrFileRecdResp = NULL;
			stWrFileSubReq_t	*pstSubReq = NULL;
			stWrFileSubReq_t	*pstTempSubReq = NULL;

			pstMbusWrFileRecdResp = pstMBusRequesPacket->m_stMbusRxData.m_pvAdditionalData;

			WriteFileRecord_CallbackFunction = pstMBusRequesPacket->pFunc;

			if(NULL != WriteFileRecord_CallbackFunction)
				WriteFileRecord_CallbackFunction(pstMBusRequesPacket->m_u8UnitID,
						pstMBusRequesPacket->m_u8IpAddr,
						pstMBusRequesPacket->m_u16TransactionID,
						pstMBusRequesPacket->m_u8FunctionCode,
						&stException,pstMbusWrFileRecdResp);

			if(NULL != pstMbusWrFileRecdResp)
				pstSubReq = pstMbusWrFileRecdResp->m_stSubReq.pstNextNode;

			while(NULL != pstSubReq)
			{
				OSAL_Free(pstSubReq->m_pu16RecData);
				pstTempSubReq = pstSubReq->pstNextNode;
				OSAL_Free(pstSubReq);
				pstSubReq = pstTempSubReq;
			}
			if(NULL != pstMbusWrFileRecdResp)
			{
				OSAL_Free(pstMbusWrFileRecdResp);
				pstMBusRequesPacket->m_stMbusRxData.m_pvAdditionalData = NULL;
			}
		}
		break;

		case READ_DEVICE_IDENTIFICATION :
		{
			stRdDevIdResp_t	*pstMbusRdDevIdResp = NULL;
			SubObjList_t		*pstObjList = NULL;
			SubObjList_t		*pstTempObjReq = NULL;

			pstMbusRdDevIdResp = pstMBusRequesPacket->m_stMbusRxData.m_pvAdditionalData;

			ReadDeviceIdentification_CallbackFunction = pstMBusRequesPacket->pFunc;

			if(NULL != ReadDeviceIdentification_CallbackFunction)
				ReadDeviceIdentification_CallbackFunction(pstMBusRequesPacket->m_u8UnitID,
						pstMBusRequesPacket->m_u8IpAddr,
						pstMBusRequesPacket->m_u16TransactionID,
						pstMBusRequesPacket->m_u8FunctionCode,
						&stException,pstMbusRdDevIdResp);

			if(NULL != pstMbusRdDevIdResp)
			{
				pstObjList = pstMbusRdDevIdResp->m_pstSubObjList.pstNextNode;
				if(pstMbusRdDevIdResp->m_pstSubObjList.m_u8ObjectValue != NULL)
				{
					OSAL_Free(pstMbusRdDevIdResp->m_pstSubObjList.m_u8ObjectValue);
					pstMbusRdDevIdResp->m_pstSubObjList.m_u8ObjectValue = NULL;
				}
			}

			while(NULL != pstObjList)
			{
				OSAL_Free(pstObjList->m_u8ObjectValue);
				pstObjList->m_u8ObjectValue = NULL;
				pstTempObjReq = pstObjList->pstNextNode;
				pstObjList->pstNextNode = NULL;
				OSAL_Free(pstObjList);
				pstObjList = pstTempObjReq;
			}
			if(NULL != pstMbusRdDevIdResp)
			{
				OSAL_Free(pstMbusRdDevIdResp);
				pstMBusRequesPacket->m_stMbusRxData.m_pvAdditionalData = NULL;
			}

		}
		break;
	}
 }


/**
 *
 * Description
 * Function to decode received modbus data
 *
 * @param ServerReplyBuff [in] Input buffer
 * @param u16BuffInex 	  [in] buffer index
 * @param pstMBusRequesPacket [in] Request packet
 *
 * @return uint8_t [out] respective error codes
 *
 */
uint8_t DecodeRxMBusPDU(uint8_t *ServerReplyBuff,
						uint16_t u16BuffInex,
						stMbusPacketVariables_t *pstMBusRequesPacket)
{
	uint8_t u8ReturnType = STACK_NO_ERROR;
	uint16_t u16TempBuffInex = 0;
	uint8_t u8Count = 0;
	stEndianess_t stEndianess = { 0 };
	eModbusFuncCode_enum eMbusFunctionCode = MBUS_MIN_FUN_CODE;

	eMbusFunctionCode = (eModbusFuncCode_enum)pstMBusRequesPacket->m_u8FunctionCode;

	if(8 == ((0x80 & eMbusFunctionCode)>>4))
	{
		pstMBusRequesPacket->m_stMbusRxData.m_au8DataFields[0] = ServerReplyBuff[u16BuffInex++];
		pstMBusRequesPacket->m_stMbusRxData.m_u8Length = 0;
		pstMBusRequesPacket->m_stMbusRxData.m_pvAdditionalData = NULL;
	}
	else
	{
		switch(eMbusFunctionCode)
		{
			default:
			break;
			case READ_COIL_STATUS :
			case READ_INPUT_STATUS :
				pstMBusRequesPacket->m_stMbusRxData.m_u8Length = ServerReplyBuff[u16BuffInex++];
				memcpy_s((pstMBusRequesPacket->m_stMbusRxData.m_au8DataFields),
						sizeof(pstMBusRequesPacket->m_stMbusRxData.m_au8DataFields),
						&(ServerReplyBuff[u16BuffInex]),
						pstMBusRequesPacket->m_stMbusRxData.m_u8Length);
			break;

			case READ_HOLDING_REG :
			case READ_INPUT_REG :
			case READ_WRITE_MUL_REG :
				pstMBusRequesPacket->m_stMbusRxData.m_u8Length = ServerReplyBuff[u16BuffInex++];

				u16TempBuffInex = u16BuffInex;
				while(1)
				{
					stEndianess.stByteOrder.u8SecondByte = ServerReplyBuff[u16BuffInex++];
					stEndianess.stByteOrder.u8FirstByte = ServerReplyBuff[u16BuffInex++];
					pstMBusRequesPacket->m_stMbusRxData.m_au8DataFields[u8Count++] = stEndianess.stByteOrder.u8FirstByte;
					pstMBusRequesPacket->m_stMbusRxData.m_au8DataFields[u8Count++] = stEndianess.stByteOrder.u8SecondByte;
					if(pstMBusRequesPacket->m_stMbusRxData.m_u8Length <= (u16BuffInex - u16TempBuffInex))
						break;
				}
			break;

			case WRITE_MULTIPLE_COILS :
			case WRITE_MULTIPLE_REG :
			{
				stEndianess.stByteOrder.u8SecondByte = ServerReplyBuff[u16BuffInex++];
				stEndianess.stByteOrder.u8FirstByte = ServerReplyBuff[u16BuffInex++];
				pstMBusRequesPacket->m_u16StartAdd = stEndianess.u16word;
				stEndianess.stByteOrder.u8SecondByte = ServerReplyBuff[u16BuffInex++];
				stEndianess.stByteOrder.u8FirstByte = ServerReplyBuff[u16BuffInex++];
				pstMBusRequesPacket->m_u16Quantity = stEndianess.u16word;
			}
			break;

			case WRITE_SINGLE_REG :
			case WRITE_SINGLE_COIL :
			{
				pstMBusRequesPacket->m_stMbusRxData.m_u8Length = 2;
				stEndianess.stByteOrder.u8SecondByte = ServerReplyBuff[u16BuffInex++];
				stEndianess.stByteOrder.u8FirstByte = ServerReplyBuff[u16BuffInex++];
				pstMBusRequesPacket->m_u16StartAdd = stEndianess.u16word;
				stEndianess.stByteOrder.u8SecondByte = ServerReplyBuff[u16BuffInex++];
				stEndianess.stByteOrder.u8FirstByte = ServerReplyBuff[u16BuffInex++];
				pstMBusRequesPacket->m_stMbusRxData.m_au8DataFields[u8Count++] = stEndianess.stByteOrder.u8FirstByte;
				pstMBusRequesPacket->m_stMbusRxData.m_au8DataFields[u8Count++] = stEndianess.stByteOrder.u8SecondByte;

			}
			break;

			case READ_FILE_RECORD :
			{
				stMbusRdFileRecdResp_t	*pstMbusRdFileRecdResp = NULL;
				stRdFileSubReq_t		*pstSubReq = NULL;
				uint8_t u16TempBuffInex = 0;
				uint8_t u8Arrayindex2 = 0;
				uint16_t *pu16RecData = NULL;

				pstMbusRdFileRecdResp = OSAL_Malloc(sizeof(stMbusRdFileRecdResp_t));
				if(NULL == pstMbusRdFileRecdResp)
				{
					u8ReturnType = STACK_ERROR_MALLOC_FAILED;
					return u8ReturnType;
				}
				pstMBusRequesPacket->m_stMbusRxData.m_pvAdditionalData = pstMbusRdFileRecdResp;

				pstMbusRdFileRecdResp->m_u8RespDataLen = ServerReplyBuff[u16BuffInex++];

				pstSubReq = &(pstMbusRdFileRecdResp->m_stSubReq);

				pstSubReq->pstNextNode = NULL;

				u16TempBuffInex = u16BuffInex;
				while(1)
				{
					pstSubReq->m_u8FileRespLen = ServerReplyBuff[u16BuffInex++];
					pstSubReq->m_u8RefType = ServerReplyBuff[u16BuffInex++];

					pu16RecData = OSAL_Malloc(pstSubReq->m_u8FileRespLen-1);
					if(NULL == pu16RecData)
					{
						u8ReturnType = STACK_ERROR_MALLOC_FAILED;
						return u8ReturnType;
					}
					pstSubReq->m_pu16RecData = pu16RecData;

					for(u8Arrayindex2 = 0;
							u8Arrayindex2<((pstSubReq->m_u8FileRespLen-1)/2);u8Arrayindex2++)
					{
						stEndianess.stByteOrder.u8SecondByte = ServerReplyBuff[u16BuffInex++];
						stEndianess.stByteOrder.u8FirstByte = ServerReplyBuff[u16BuffInex++];
						*pu16RecData = stEndianess.u16word;
						pu16RecData++;
					}
					if(pstMbusRdFileRecdResp->m_u8RespDataLen > (u16BuffInex - u16TempBuffInex))
					{
						pstSubReq->pstNextNode = OSAL_Malloc(sizeof(stRdFileSubReq_t));
						if(NULL == pstSubReq->pstNextNode)
						{
							u8ReturnType = STACK_ERROR_MALLOC_FAILED;
							return u8ReturnType;
						}
						pstSubReq = pstSubReq->pstNextNode;
						pstSubReq->pstNextNode = NULL;
					}
					else
					{
						break;
					}
				}
			}
			break;
			case WRITE_FILE_RECORD :
			{
				stMbusWrFileRecdResp_t	*pstMbusWrFileRecdResp = NULL;
				stWrFileSubReq_t		*pstSubReq = NULL;
				uint8_t u16TempBuffInex = 0;
				uint8_t u8Arrayindex = 0;
				uint16_t *pu16RecData = NULL;

				pstMbusWrFileRecdResp = OSAL_Malloc(sizeof(stMbusWrFileRecdResp_t));
				if(NULL == pstMbusWrFileRecdResp )
				{
					u8ReturnType = STACK_ERROR_MALLOC_FAILED;
					return u8ReturnType;
				}
				memset(pstMbusWrFileRecdResp,00,sizeof(stMbusWrFileRecdResp_t));

				pstMBusRequesPacket->m_stMbusRxData.m_pvAdditionalData = pstMbusWrFileRecdResp;

				pstMbusWrFileRecdResp->m_u8RespDataLen = ServerReplyBuff[u16BuffInex++];

				pstSubReq = &(pstMbusWrFileRecdResp->m_stSubReq);

				u16TempBuffInex = u16BuffInex;

				while(1)
				{
					pstSubReq->m_u8RefType = ServerReplyBuff[u16BuffInex++];

					stEndianess.stByteOrder.u8SecondByte = ServerReplyBuff[u16BuffInex++];
					stEndianess.stByteOrder.u8FirstByte = ServerReplyBuff[u16BuffInex++];
					pstSubReq->m_u16FileNum = stEndianess.u16word;

					stEndianess.stByteOrder.u8SecondByte = ServerReplyBuff[u16BuffInex++];
					stEndianess.stByteOrder.u8FirstByte = ServerReplyBuff[u16BuffInex++];
					pstSubReq->m_u16RecNum = stEndianess.u16word;

					stEndianess.stByteOrder.u8SecondByte = ServerReplyBuff[u16BuffInex++];
					stEndianess.stByteOrder.u8FirstByte = ServerReplyBuff[u16BuffInex++];
					pstSubReq->m_u16RecLen = stEndianess.u16word;

					pu16RecData = OSAL_Malloc(pstSubReq->m_u16RecLen*sizeof(uint16_t));
					if(NULL == pu16RecData)
					{
						u8ReturnType = STACK_ERROR_MALLOC_FAILED;
						return u8ReturnType;
					}
					pstSubReq->m_pu16RecData = pu16RecData;

					for(u8Arrayindex = 0;
							u8Arrayindex<(pstSubReq->m_u16RecLen);u8Arrayindex++)
					{
						stEndianess.stByteOrder.u8SecondByte = ServerReplyBuff[u16BuffInex++];
						stEndianess.stByteOrder.u8FirstByte = ServerReplyBuff[u16BuffInex++];
						*pu16RecData = stEndianess.u16word;
						pu16RecData++;
					}

					if(pstMbusWrFileRecdResp->m_u8RespDataLen > (u16BuffInex - u16TempBuffInex))
					{
						pstSubReq->pstNextNode = OSAL_Malloc(sizeof(stWrFileSubReq_t));
						if(NULL == pstSubReq->pstNextNode)
						{
							u8ReturnType = STACK_ERROR_MALLOC_FAILED;
							return u8ReturnType;
						}
						pstSubReq = pstSubReq->pstNextNode;
						memset(pstSubReq,00,sizeof(stWrFileSubReq_t));
						pstSubReq->pstNextNode = NULL;
					}
					else
					{
						break;
					}
				}
			}
			break;

			case READ_DEVICE_IDENTIFICATION :
			{
				stRdDevIdResp_t	*pstMbusRdDevIdResp = NULL;
				SubObjList_t		*pstObjList = NULL;
				uint8_t u8NumObj = 0;
				bool bIsFirstObjflag = true;

				pstMbusRdDevIdResp = OSAL_Malloc(sizeof(stRdDevIdResp_t));
				if(NULL == pstMbusRdDevIdResp)
				{
					u8ReturnType = STACK_ERROR_MALLOC_FAILED;
					return u8ReturnType;
				}

				memset(pstMbusRdDevIdResp,00,sizeof(stRdDevIdResp_t));
				pstMBusRequesPacket->m_stMbusRxData.m_pvAdditionalData = pstMbusRdDevIdResp;

				pstMbusRdDevIdResp->m_u8MEIType = ServerReplyBuff[u16BuffInex++];
				pstMbusRdDevIdResp->m_u8RdDevIDCode = ServerReplyBuff[u16BuffInex++];
				pstMbusRdDevIdResp->m_u8ConformityLevel = ServerReplyBuff[u16BuffInex++];
				pstMbusRdDevIdResp->m_u8MoreFollows = ServerReplyBuff[u16BuffInex++];
				pstMbusRdDevIdResp->m_u8NextObjId = ServerReplyBuff[u16BuffInex++];
				pstMbusRdDevIdResp->m_u8NumberofObjects = ServerReplyBuff[u16BuffInex++];

				u8NumObj = pstMbusRdDevIdResp->m_u8NumberofObjects;
				pstObjList = &(pstMbusRdDevIdResp->m_pstSubObjList);
				u16TempBuffInex = u16BuffInex;
				while(1)
				{
					pstObjList->m_u8ObjectID = ServerReplyBuff[u16BuffInex++];
					pstObjList->m_u8ObjectLen = ServerReplyBuff[u16BuffInex++];
					pstObjList->m_u8ObjectValue = OSAL_Malloc(sizeof(uint8_t) * pstObjList->m_u8ObjectLen);
					if(NULL == pstObjList->m_u8ObjectValue)
					{
						u8ReturnType = STACK_ERROR_MALLOC_FAILED;
						return u8ReturnType;
					}
					memcpy_s(pstObjList->m_u8ObjectValue,
							(sizeof(uint8_t)*pstObjList->m_u8ObjectLen),
							&ServerReplyBuff[u16BuffInex++],
							pstObjList->m_u8ObjectLen);

					if(0 == pstMbusRdDevIdResp->m_u8MoreFollows && bIsFirstObjflag == true)
					{
						u8NumObj = pstMbusRdDevIdResp->m_u8NumberofObjects -
								pstMbusRdDevIdResp->m_u8NextObjId;
						bIsFirstObjflag = false;
					}
					else if(0xFF == pstMbusRdDevIdResp->m_u8MoreFollows && bIsFirstObjflag == true)
					{
						u8NumObj = pstMbusRdDevIdResp->m_u8NumberofObjects;
						bIsFirstObjflag = false;
					}

					if(u8NumObj > 0)
					{
						u8NumObj = u8NumObj - 1;
					}

					if(u8NumObj)
					{
						u16BuffInex = u16BuffInex + (pstObjList->m_u8ObjectLen - 1);
						pstObjList->pstNextNode = OSAL_Malloc(sizeof(SubObjList_t));
						if(NULL == pstObjList->pstNextNode)
						{
							u8ReturnType = STACK_ERROR_MALLOC_FAILED;
							return u8ReturnType;
						}
						pstObjList = pstObjList->pstNextNode;
						pstObjList->pstNextNode = NULL;
					}
					else
					{
						break;
					}
				}
			}
			break;
		}
	}
	return u8ReturnType;
}

/**
 * Description
 * Function to decode received modbus data
 *
 * @param ServerReplyBuff [in] Input buffer
 * @param pstMBusRequesPacket [in] Request packet
 * @return uint8_t [out] respective error codes
 *
 */
uint8_t DecodeRxPacket(uint8_t *ServerReplyBuff,stMbusPacketVariables_t *pstMBusRequesPacket)
{
	uint8_t u8ReturnType = STACK_NO_ERROR;
	uint16_t u16BuffInex = 0;
	uByteOrder_t ustByteOrder = {0};

	// Holds the received transaction ID
	uint16_t u16TransactionID = 0;
	// Holds the unit id
	uint8_t  u8UnitID = 0;
	// Holds the function code
	uint8_t u8FunctionCode = 0;


	// Transaction ID
	ustByteOrder.u16Word = 0;
	ustByteOrder.TwoByte.u8ByteTwo = ServerReplyBuff[u16BuffInex++];
	ustByteOrder.TwoByte.u8ByteOne = ServerReplyBuff[u16BuffInex++];
	u16TransactionID = ustByteOrder.u16Word;

	// Protocol ID
	ustByteOrder.TwoByte.u8ByteTwo = ServerReplyBuff[u16BuffInex++];
	ustByteOrder.TwoByte.u8ByteOne = ServerReplyBuff[u16BuffInex++];

	ustByteOrder.u16Word = 0;
	// Length
	ustByteOrder.TwoByte.u8ByteTwo = ServerReplyBuff[u16BuffInex++];
	ustByteOrder.TwoByte.u8ByteOne = ServerReplyBuff[u16BuffInex++];

	// Unit ID
	u8UnitID = ServerReplyBuff[u16BuffInex++];

	u8FunctionCode = ServerReplyBuff[u16BuffInex++];

	if((pstMBusRequesPacket->m_u16TransactionID != u16TransactionID) ||
	   (pstMBusRequesPacket->m_u8UnitID != u8UnitID) ||
	   (pstMBusRequesPacket->m_u8FunctionCode != (u8FunctionCode & 0x7F)))
	{
		u8ReturnType = STACK_TXNID_OR_UNITID_MISSMATCH;
	}
	else
	{
		pstMBusRequesPacket->m_u8FunctionCode = u8FunctionCode;
		u8ReturnType = DecodeRxMBusPDU(ServerReplyBuff,u16BuffInex, pstMBusRequesPacket);
	}
	return u8ReturnType;
}


/**
 * Description
 * Function to send packet on network
 *
 * @param pstMBusRequesPacket [in] Request packet
 * @param pi32sockfd [in] socket fd
 *
 * @return uint8_t [out] respective error codes
 *
 */
uint8_t Modbus_SendPacket(stMbusPacketVariables_t *pstMBusRequesPacket,int32_t* pi32sockfd)
{
	int32_t sockfd = 0;
	uint8_t u8CommunicationFalg = 1;
	uint8_t u8ReturnType = STACK_NO_ERROR;
	uint8_t recvBuff[260];
    uint8_t ServerReplyBuff[260];
    struct sockaddr_in serv_addr;
    IP_address_t stTempIpAdd = {0};
    long arg;
    fd_set myset;
    socklen_t lon;
    int res, valopt;
    struct timeval tv;
    //tv.tv_sec = ModbusMasterConfig.m_u8TcpConnectTimeout;  /// 3 seconds timeout
    tv.tv_usec = ModbusMasterConfig.m_u8TcpConnectTimeout;	/// 30k microseconds

    if(NULL == pstMBusRequesPacket || NULL == pi32sockfd)
    {
    	u8ReturnType = STACK_ERROR_SEND_FAILED;
    	return u8ReturnType;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    stTempIpAdd.s_un.s_un_b.IP_1 = pstMBusRequesPacket->m_u8IpAddr[0];
    stTempIpAdd.s_un.s_un_b.IP_2 = pstMBusRequesPacket->m_u8IpAddr[1];
    stTempIpAdd.s_un.s_un_b.IP_3 = pstMBusRequesPacket->m_u8IpAddr[2];
    stTempIpAdd.s_un.s_un_b.IP_4 = pstMBusRequesPacket->m_u8IpAddr[3];
    serv_addr.sin_addr.s_addr = stTempIpAdd.s_un.s_addr;

    memset(recvBuff, '0',sizeof(recvBuff));
    memcpy_s(recvBuff,sizeof(pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields),
    		pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields,
			pstMBusRequesPacket->m_stMbusTxData.m_u16Length);

    while(u8CommunicationFalg)
    {
    	if(0 == (*pi32sockfd))
    	{
			if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			{
				u8ReturnType = STACK_ERROR_SOCKET_FAILED;
				break;
			}

			arg = fcntl(sockfd, F_GETFL, NULL);
			arg |= O_NONBLOCK;
			fcntl(sockfd, F_SETFL, arg);

			/// Trying to connect with timeout
			*pi32sockfd = sockfd;
			serv_addr.sin_family = AF_INET;
			serv_addr.sin_port = htons(MODBUS_TCP_PORT);

			res = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

			if (res < 0)
			{
			     if (errno == EINPROGRESS)
			     {
			        tv.tv_sec = 0;
			        tv.tv_usec = ModbusMasterConfig.m_u8TcpConnectTimeout;
			        FD_ZERO(&myset);
			        FD_SET(sockfd, &myset);
			        if (select(sockfd+1, NULL, &myset, NULL, &tv) > 0)
			        {
			           lon = sizeof(int);
			           getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon);
			           if (valopt)
			           {
			              u8ReturnType = STACK_ERROR_CONNECT_FAILED;
			              *pi32sockfd = 0;
			              /// closing socket on error.
			              close(sockfd);
			              break;
			           }
			        }
			        else
			        {
			        	u8ReturnType = STACK_ERROR_CONNECT_FAILED;
			        	*pi32sockfd = 0;
			        	/// closing socket on error.
			        	close(sockfd);
			        	break;
			        }
			     }
			     else
			     {
			    	 u8ReturnType = STACK_ERROR_CONNECT_FAILED;
			    	 *pi32sockfd = 0;
			    	 /// closing socket on error.
			    	 close(sockfd);
			    	 break;
			     }
			}
			/// Set to blocking mode again...
			arg = fcntl(sockfd, F_GETFL, NULL);
			arg &= (~O_NONBLOCK);
			fcntl(sockfd, F_SETFL, arg);
    	}
    	else
    	{
    		sockfd = *pi32sockfd;
    	}

///		if(send(sockfd, recvBuff, (pstMBusRequesPacket->m_stMbusTxData.m_u16Length), 0) < 0)
		if(send(sockfd, recvBuff, (pstMBusRequesPacket->m_stMbusTxData.m_u16Length), MSG_NOSIGNAL) < 0)
		/// in order to avoid application stop whenever SIGPIPE gets generated,used send function with MSG_NOSIGNAL argument
		{
			u8ReturnType = STACK_ERROR_SEND_FAILED;
			close(sockfd);
			*pi32sockfd = 0;
			break;
		}

		setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,sizeof(struct timeval));

		if(recv(sockfd, ServerReplyBuff, sizeof(ServerReplyBuff), 0) < 0)
		{
			u8ReturnType = STACK_ERROR_RECV_FAILED;
			/// closing socket on error.
			close(sockfd);
			*pi32sockfd = 0;
			break;
		}
		else
		{
			u8CommunicationFalg = 0;
		}
		break;
    }
    if(0 == u8CommunicationFalg && STACK_NO_ERROR == u8ReturnType)
    	u8ReturnType = DecodeRxPacket(ServerReplyBuff,pstMBusRequesPacket);

    pstMBusRequesPacket->m_u8CommandStatus = u8ReturnType;

    return u8ReturnType;
}
