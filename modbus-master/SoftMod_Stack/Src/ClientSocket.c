/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation or Softdel Systems
 * (and licensed to Intel Corporation). Title to the Material remains with
 * Intel Corporation or Softdel Systems.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <safe_lib.h>
#include <errno.h>
#include <stddef.h>

#include "SessionControl.h"
#include "osalLinux.h"

//#define MODBUS_STACK_TCPIP_ENABLED

#ifdef MODBUS_STACK_TCPIP_ENABLED
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#else
#include <termios.h>
#include <signal.h>
#endif

/*CRC calculation is for Modbus RTU stack */
#ifndef MODBUS_STACK_TCPIP_ENABLED
/* Table of CRC values for high-order byte */
static const uint8_t table_crc_hi[] = {
		0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
		0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
		0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
		0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
		0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
		0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
		0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
		0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
		0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
		0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
		0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
		0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
		0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
		0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
		0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
		0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
		0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
		0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
		0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
		0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
		0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
		0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,
		0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,
		0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
		0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
		0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

/* Table of CRC values for low-order byte */
static const uint8_t table_crc_lo[] = {
		0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
		0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,
		0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
		0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
		0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,
		0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
		0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,
		0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
		0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
		0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,
		0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,
		0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
		0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,
		0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,
		0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
		0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
		0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,
		0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
		0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,
		0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
		0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
		0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,
		0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,
		0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
		0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,
		0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

static uint16_t crc16(uint8_t *buffer, uint16_t buffer_length)
{
	uint8_t crc_hi = 0xFF; /* high CRC byte initialized */
	uint8_t crc_lo = 0xFF; /* low CRC byte initialized */
	unsigned int i; /* will index into CRC lookup */

	/* pass through message buffer */
	while (buffer_length--) {
		i = crc_hi ^ *buffer++; /* calculate the CRC  */
		crc_hi = crc_lo ^ table_crc_hi[i];
		crc_lo = table_crc_lo[i];
	}

	return (crc_hi << 8 | crc_lo);
}
#endif //#ifndef MODBUS_STACK_TCPIP_ENABLED

#ifdef MODBUS_STACK_TCPIP_ENABLED
extern stDevConfig_t ModbusMasterConfig;
#endif

#ifndef MODBUS_STACK_TCPIP_ENABLED
int fd;
extern Mutex_H LivSerSesslist_Mutex;
uint32_t baud;

/*To check blocking serial read*/
int8_t checkforblockingread(void);

#endif

void (*ModbusMaster_ApplicationCallback)(uint8_t  ,
		uint16_t ,
		uint8_t* ,
		uint16_t ,
		uint8_t  ,
		stException_t*,
		uint8_t  ,
		uint8_t* ,
		uint16_t,
		uint16_t);


void (*ReadFileRecord_CallbackFunction)(uint8_t, uint8_t*,uint16_t, uint16_t,uint8_t,
		stException_t *,
		stMbusRdFileRecdResp_t*);

void (*WriteFileRecord_CallbackFunction)(uint8_t, uint8_t*, uint16_t,uint8_t,
		stException_t*,
		stMbusWrFileRecdResp_t*);

void (*ReadDeviceIdentification_CallbackFunction)(uint8_t, uint8_t*, uint16_t, uint16_t,uint8_t,
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
#ifdef MODBUS_STACK_TCPIP_ENABLED
			ModbusMaster_ApplicationCallback(
					pstMBusRequesPacket->m_u8UnitID,
					pstMBusRequesPacket->m_u16TransactionID,
					pstMBusRequesPacket->m_u8IpAddr,
					pstMBusRequesPacket->u16Port,
					pstMBusRequesPacket->m_u8FunctionCode,
					&stException,
					pstMBusRequesPacket->m_stMbusRxData.m_u8Length,
					pstMBusRequesPacket->m_stMbusRxData.m_au8DataFields,
					pstMBusRequesPacket->m_u16StartAdd,
					pstMBusRequesPacket->m_u16Quantity);

#else
			ModbusMaster_ApplicationCallback(
					pstMBusRequesPacket->m_u8UnitID,
					pstMBusRequesPacket->m_u16TransactionID,
					&pstMBusRequesPacket->m_u8ReceivedDestination,
					pstMBusRequesPacket->m_u8FunctionCode,
					&stException,
					pstMBusRequesPacket->m_stMbusRxData.m_u8Length,
					pstMBusRequesPacket->m_stMbusRxData.m_au8DataFields,
					pstMBusRequesPacket->m_u16StartAdd,
					pstMBusRequesPacket->m_u16Quantity);

#endif
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
#ifdef MODBUS_STACK_TCPIP_ENABLED

		if(NULL != ReadFileRecord_CallbackFunction)
			ReadFileRecord_CallbackFunction(pstMBusRequesPacket->m_u8UnitID,
					pstMBusRequesPacket->m_u8IpAddr,
					pstMBusRequesPacket->u16Port,
					pstMBusRequesPacket->m_u16TransactionID,
					pstMBusRequesPacket->m_u8FunctionCode,
					&stException,pstMbusRdFileRecdResp);


#else
		if(NULL != ReadFileRecord_CallbackFunction)
			ReadFileRecord_CallbackFunction(pstMBusRequesPacket->m_u8UnitID,
					&pstMBusRequesPacket->m_u8ReceivedDestination,
					pstMBusRequesPacket->m_u16TransactionID,
					pstMBusRequesPacket->m_u8FunctionCode,
					&stException,pstMbusRdFileRecdResp);
#endif

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

#ifdef MODBUS_STACK_TCPIP_ENABLED

		if(NULL != ReadFileRecord_CallbackFunction)
			ReadFileRecord_CallbackFunction(pstMBusRequesPacket->m_u8UnitID,
					pstMBusRequesPacket->m_u8IpAddr,
					pstMBusRequesPacket->u16Port,
					pstMBusRequesPacket->m_u16TransactionID,
					pstMBusRequesPacket->m_u8FunctionCode,
					&stException,pstMbusWrFileRecdResp);

#else
		if(NULL != WriteFileRecord_CallbackFunction)
			WriteFileRecord_CallbackFunction(pstMBusRequesPacket->m_u8UnitID,
					&pstMBusRequesPacket->m_u8ReceivedDestination,
					pstMBusRequesPacket->m_u16TransactionID,
					pstMBusRequesPacket->m_u8FunctionCode,
					&stException,pstMbusWrFileRecdResp);

#endif
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

#ifdef MODBUS_STACK_TCPIP_ENABLED

		if(NULL != ReadDeviceIdentification_CallbackFunction)
			ReadDeviceIdentification_CallbackFunction(pstMBusRequesPacket->m_u8UnitID,
					pstMBusRequesPacket->m_u8IpAddr,
					pstMBusRequesPacket->u16Port,
					pstMBusRequesPacket->m_u16TransactionID,
					pstMBusRequesPacket->m_u8FunctionCode,
					&stException,pstMbusRdDevIdResp);

#else
		if(NULL != ReadDeviceIdentification_CallbackFunction)
			ReadDeviceIdentification_CallbackFunction(pstMBusRequesPacket->m_u8UnitID,
					&pstMBusRequesPacket->m_u8ReceivedDestination,
					pstMBusRequesPacket->m_u16TransactionID,
					pstMBusRequesPacket->m_u8FunctionCode,
					&stException,pstMbusRdDevIdResp);
#endif

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
	uint8_t bIsDone = 0;

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
			while(0 == bIsDone)
			{
				stEndianess.stByteOrder.u8SecondByte = ServerReplyBuff[u16BuffInex++];
				stEndianess.stByteOrder.u8FirstByte = ServerReplyBuff[u16BuffInex++];
				pstMBusRequesPacket->m_stMbusRxData.m_au8DataFields[u8Count++] = stEndianess.stByteOrder.u8FirstByte;
				pstMBusRequesPacket->m_stMbusRxData.m_au8DataFields[u8Count++] = stEndianess.stByteOrder.u8SecondByte;
				if(pstMBusRequesPacket->m_stMbusRxData.m_u8Length <= (u16BuffInex - u16TempBuffInex))
					bIsDone = 1;
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

#ifdef MODBUS_STACK_TCPIP_ENABLED
	uByteOrder_t ustByteOrder = {0};

	// Holds the received transaction ID
	uint16_t u16TransactionID = 0;
#endif
	// Holds the unit id
	uint8_t  u8UnitID = 0;
	// Holds the function code
	uint8_t u8FunctionCode = 0;

#ifdef MODBUS_STACK_TCPIP_ENABLED

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
#endif

	u8UnitID = ServerReplyBuff[u16BuffInex++];

	u8FunctionCode = ServerReplyBuff[u16BuffInex++];

#ifdef MODBUS_STACK_TCPIP_ENABLED
	if((pstMBusRequesPacket->m_u16TransactionID != u16TransactionID) ||
			(pstMBusRequesPacket->m_u8UnitID != u8UnitID) ||
			(pstMBusRequesPacket->m_u8FunctionCode != (u8FunctionCode & 0x7F)))
#else
		if((pstMBusRequesPacket->m_u8ReceivedDestination != u8UnitID)
				||(pstMBusRequesPacket->m_u8FunctionCode != (u8FunctionCode & 0x7F)))
#endif
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

#ifndef MODBUS_STACK_TCPIP_ENABLED

/**
 * Description
 * Function to check the serial blocking read
 *
 * @param None
 * @param None
 * @return uint8_t [out] respective error codes
 *
 */
int8_t checkforblockingread(void)
{
	fd_set rset;
	struct timeval tv;
	//wait upto 1 seconds
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	FD_ZERO(&rset);

	// If fd is not set correctly.
	if(fd < 0)
	{
		return 0;
	}
	FD_SET(fd, &rset);

	return select(fd+1,&rset,NULL,NULL,&tv);
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
uint8_t Modbus_SendPacket(stMbusPacketVariables_t *pstMBusRequesPacket,int32_t *sfd)
{
	uint8_t u8ReturnType = STACK_NO_ERROR;
	uint8_t recvBuff[TCP_MODBUS_ADU_LENGTH];
	volatile int bytes = 0;
	uint16_t crc;
	uint8_t ServerReplyBuff[TCP_MODBUS_ADU_LENGTH];
	int totalRead = 0;
	//int numToRead = sizeof(ServerReplyBuff);
	uint16_t numToRead = 0;

	if(NULL == pstMBusRequesPacket)
	{
		u8ReturnType = STACK_ERROR_SEND_FAILED;
		return u8ReturnType;
	}

	memset(recvBuff, '0',sizeof(recvBuff));
	memcpy_s(recvBuff,sizeof(pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields),
			pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields,
			pstMBusRequesPacket->m_stMbusTxData.m_u16Length);

	{
		crc = crc16(recvBuff,pstMBusRequesPacket->m_stMbusTxData.m_u16Length);

		recvBuff[pstMBusRequesPacket->m_stMbusTxData.m_u16Length++] = (crc & 0xFF00) >> 8;
		recvBuff[pstMBusRequesPacket->m_stMbusTxData.m_u16Length++] = (crc & 0x00FF);
		tcflush(fd, TCIOFLUSH);
		bytes = write(fd,recvBuff,(pstMBusRequesPacket->m_stMbusTxData.m_u16Length));
		//< Note: Below framing delay is commented intentionally as read function is having blocking read() call in which
		//< select() function is having response waiting time of 1000 mSec as well as it is having blocking read call.
		//< This will not allow to start new message unless mess is received from slave within 1000mSec
		//< Please remove below commented framing delay when nonblocking call is used.
		/*if(baud > 19200)
		{
			usleep(1750);
		}
		else
		{
			usleep(3700);
		}*/

		bytes = 0;
		if((READ_COIL_STATUS == (eModbusFuncCode_enum)(pstMBusRequesPacket->m_u8FunctionCode)) ||
				(READ_INPUT_STATUS == (eModbusFuncCode_enum)(pstMBusRequesPacket->m_u8FunctionCode)))
		{
			// Identify number of bytes to be read from slave device for Read coil and read input function codes.
			// When read request is for Coil and input quantity is unit. Header length is of 5.
			numToRead = ((pstMBusRequesPacket->m_u16Quantity) + 5);
		}
		else
		{
			// Identify number of bytes to be read from slave device for WORD i.e. integer function codes.
			// When read request is for Holding register and input register it is of WORD i.e. multiple of quantity.
			// Header length is of 5.
			numToRead = ((pstMBusRequesPacket->m_u16Quantity * 2) + 5);
		}
		while(numToRead > 0){
			if(checkforblockingread() > 0)
			{
				bytes = read(fd, &ServerReplyBuff[totalRead], numToRead);
				totalRead += bytes;
				numToRead -= bytes;

				if(bytes == 0){
					break;
				}
			}
			else{
				break;
			}
		}

		if(totalRead > 0)
		{
			u8ReturnType = DecodeRxPacket(ServerReplyBuff,pstMBusRequesPacket);
		}
	}

	pstMBusRequesPacket->m_u8CommandStatus = u8ReturnType;
	return u8ReturnType;
}

// New initSerialPort Function
//#ifdef OLD_SERIALPORT
MODBUS_STACK_EXPORT int initSerialPort(uint8_t *portName, uint32_t baudrate, uint8_t  parity, uint8_t stop_bit)
{
	struct termios tios;
    speed_t speed;
    int flags;
    /* The O_NOCTTY flag tells UNIX that this program doesn't want
       to be the "controlling terminal" for that port. If you
       don't specify this then any input (such as keyboard abort
       signals and so forth) will affect your process

       Timeouts are ignored in canonical input mode or when the
       NDELAY option is set on the file via open or fcntl */

    memset(&tios, 0, sizeof(struct termios));
     flags = O_RDWR | O_NOCTTY | O_NDELAY | O_EXCL;
    //flags = O_RDWR | O_NOCTTY | O_NDELAY | O_SYNC;

    fd = open((const char*)portName, flags);

    if (fd == -1) {
        printf("ERROR Can't open the device %s (%s)\n",
                    portName, strerror(errno));
        return -1;
    }

    /* Save */
   // tcgetattr(fd, &old_tios);
    tcgetattr(fd, &tios);


    /* C_ISPEED     Input baud (new interface)
       C_OSPEED     Output baud (new interface)
    */
    switch (baudrate) {
    case 110:
        speed = B110;
        break;
    case 300:
        speed = B300;
        break;
    case 600:
        speed = B600;
        break;
    case 1200:
        speed = B1200;
        break;
    case 2400:
        speed = B2400;
        break;
    case 4800:
        speed = B4800;
        break;
    case 9600:
        speed = B9600;
        break;
    case 19200:
        speed = B19200;
        break;
    case 38400:
        speed = B38400;
        break;
#ifdef B57600
    case 57600:
        speed = B57600;
        break;
#endif
#ifdef B115200
    case 115200:
        speed = B115200;
        break;
#endif
#ifdef B230400
    case 230400:
        speed = B230400;
        break;
#endif
#ifdef B460800
    case 460800:
        speed = B460800;
        break;
#endif
#ifdef B500000
    case 500000:
        speed = B500000;
        break;
#endif
#ifdef B576000
    case 576000:
        speed = B576000;
        break;
#endif
#ifdef B921600
    case 921600:
        speed = B921600;
        break;
#endif
#ifdef B1000000
    case 1000000:
        speed = B1000000;
        break;
#endif
#ifdef B1152000
   case 1152000:
        speed = B1152000;
        break;
#endif
#ifdef B1500000
    case 1500000:
        speed = B1500000;
        break;
#endif
#ifdef B2500000
    case 2500000:
        speed = B2500000;
        break;
#endif
#ifdef B3000000
    case 3000000:
        speed = B3000000;
        break;
#endif
#ifdef B3500000
    case 3500000:
        speed = B3500000;
        break;
#endif
#ifdef B4000000
    case 4000000:
        speed = B4000000;
        break;
#endif
    default:
        speed = B9600;
		printf("ERROR Unknown baud rate %d for %s (B9600 used)\n",
				baudrate, portName);
    }

    /* Set the baud rate */
    if ((cfsetispeed(&tios, speed) < 0) ||
        (cfsetospeed(&tios, speed) < 0)) {
        close(fd);
        fd = -1;
        return -1;
    }

    /* C_CFLAG      Control options
       CLOCAL       Local line - do not change "owner" of port
       CREAD        Enable receiver
    */
    tios.c_cflag |= (CREAD | CLOCAL);
    /* CSIZE, HUPCL, CRTSCTS (hardware flow control) */

    /* Set data bits (5, 6, 7, 8 bits)
       CSIZE        Bit mask for data bits
    */
    tios.c_cflag &= ~CSIZE;
	tios.c_cflag |= CS8;

    /* Stop bit (1 or 2) */
    if (stop_bit == 1)
        tios.c_cflag &=~ CSTOPB;
    else /* 2 */
        tios.c_cflag |= CSTOPB;

    /* PARENB       Enable parity bit
       PARODD       Use odd parity instead of even */
    if (parity == NO_PARITY) {
        /* None */
        tios.c_cflag &=~ PARENB;
    } else if (parity == EVEN_PARITY) {
        /* Even */
        tios.c_cflag |= PARENB;
        tios.c_cflag &=~ PARODD;
    } else {
        /* Odd */
        tios.c_cflag |= PARENB;
        tios.c_cflag |= PARODD;
    }

    /* Read the man page of termios if you need more information. */

    /* This field isn't used on POSIX systems
       tios.c_line = 0;
    */

    /* C_LFLAG      Line options

       ISIG Enable SIGINTR, SIGSUSP, SIGDSUSP, and SIGQUIT signals
       ICANON       Enable canonical input (else raw)
       XCASE        Map uppercase \lowercase (obsolete)
       ECHO Enable echoing of input characters
       ECHOE        Echo erase character as BS-SP-BS
       ECHOK        Echo NL after kill character
       ECHONL       Echo NL
       NOFLSH       Disable flushing of input buffers after
       interrupt or quit characters
       IEXTEN       Enable extended functions
       ECHOCTL      Echo control characters as ^char and delete as ~?
       ECHOPRT      Echo erased character as character erased
       ECHOKE       BS-SP-BS entire line on line kill
       FLUSHO       Output being flushed
       PENDIN       Retype pending input at next read or input char
       TOSTOP       Send SIGTTOU for background output

       Canonical input is line-oriented. Input characters are put
       into a buffer which can be edited interactively by the user
       until a CR (carriage return) or LF (line feed) character is
       received.

       Raw input is unprocessed. Input characters are passed
       through exactly as they are received, when they are
       received. Generally you'll deselect the ICANON, ECHO,
       ECHOE, and ISIG options when using raw input
    */

    /* Raw input */
    tios.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    /* C_IFLAG      Input options

       Constant     Description
       INPCK        Enable parity check
       IGNPAR       Ignore parity errors
       PARMRK       Mark parity errors
       ISTRIP       Strip parity bits
       IXON Enable software flow control (outgoing)
       IXOFF        Enable software flow control (incoming)
       IXANY        Allow any character to start flow again
       IGNBRK       Ignore break condition
       BRKINT       Send a SIGINT when a break condition is detected
       INLCR        Map NL to CR
       IGNCR        Ignore CR
       ICRNL        Map CR to NL
       IUCLC        Map uppercase to lowercase
       IMAXBEL      Echo BEL on input line too long
    */
    if (parity == NO_PARITY) {
        /* None */
        tios.c_iflag &= ~INPCK;
    } else {
        tios.c_iflag |= INPCK;
    }

    /* Software flow control is disabled */
    tios.c_iflag &= ~(IXON | IXOFF | IXANY);

    /* C_OFLAG      Output options
       OPOST        Postprocess output (not set = raw output)
       ONLCR        Map NL to CR-NL

       ONCLR ant others needs OPOST to be enabled
    */

    /* Raw ouput */
    tios.c_oflag &=~ OPOST;

    /* C_CC         Control characters
       VMIN         Minimum number of characters to read
       VTIME        Time to wait for data (tenths of seconds)

       UNIX serial interface drivers provide the ability to
       specify character and packet timeouts. Two elements of the
       c_cc array are used for timeouts: VMIN and VTIME. Timeouts
       are ignored in canonical input mode or when the NDELAY
       option is set on the file via open or fcntl.

       VMIN specifies the minimum number of characters to read. If
       it is set to 0, then the VTIME value specifies the time to
       wait for every character read. Note that this does not mean
       that a read call for N bytes will wait for N characters to
       come in. Rather, the timeout will apply to the first
       character and the read call will return the number of
       characters immediately available (up to the number you
       request).

       If VMIN is non-zero, VTIME specifies the time to wait for
       the first character read. If a character is read within the
       time given, any read will block (wait) until all VMIN
       characters are read. That is, once the first character is
       read, the serial interface driver expects to receive an
       entire packet of characters (VMIN bytes total). If no
       character is read within the time allowed, then the call to
       read returns 0. This method allows you to tell the serial
       driver you need exactly N bytes and any read call will
       return 0 or N bytes. However, the timeout only applies to
       the first character read, so if for some reason the driver
       misses one character inside the N byte packet then the read
       call could block forever waiting for additional input
       characters.

       VTIME specifies the amount of time to wait for incoming
       characters in tenths of seconds. If VTIME is set to 0 (the
       default), reads will block (wait) indefinitely unless the
       NDELAY option is set on the port with open or fcntl.
    */
    /* Unused because we use open with the NDELAY option */
    tios.c_cc[VMIN] = 0;
    tios.c_cc[VTIME] = 0;

    if (tcsetattr(fd, TCSANOW, &tios) < 0) {
        close(fd);
        fd = -1;
        return -1;
    }
	return fd;
}

#endif

#ifdef MODBUS_STACK_TCPIP_ENABLED

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
	serv_addr.sin_port = htons(pstMBusRequesPacket->u16Port);

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
			//serv_addr.sin_port = htons(MODBUS_TCP_PORT);

			res = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

			if (res < 0)
			{
				if (errno == EINPROGRESS)
				{
					tv.tv_sec = 1;
					//tv.tv_usec = ModbusMasterConfig.m_u8TcpConnectTimeout;
					tv.tv_usec = 0;
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

		//setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,(struct timeval *)&tv,sizeof(struct timeval));

		tv.tv_sec = 1;
		//tv.tv_usec = ModbusMasterConfig.m_u8TcpConnectTimeout;
		tv.tv_usec = 0;
		FD_ZERO(&myset);
		FD_SET(sockfd, &myset);
		if (select(sockfd+1, NULL, &myset, NULL, &tv) <= 0)
		{
			printf("\nselect before recv error: %d", errno);
			u8ReturnType = STACK_ERROR_RECV_FAILED;
			/// closing socket on error.
			close(sockfd);
			*pi32sockfd = 0;
			break;
		}

		if(recv(sockfd, ServerReplyBuff, sizeof(ServerReplyBuff), 0) <= 0)
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

#endif
