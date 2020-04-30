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

#include "SessionControl.h"
#include <safe_lib.h>
#include <unistd.h>
#include <time.h>

Thread_H SessionControl_ThreadId = 0;
extern int32_t i32MsgQueIdSC;
bool g_bThreadExit = false;

// global variable to store stack configurations
stDevConfig_t g_stModbusDevConfig = {0};

// global variable to maintain stack enable status
bool g_bIsStackEnable = false;

#ifdef MODBUS_STACK_TCPIP_ENABLED
Mutex_H LivSerSesslist_Mutex = NULL;
extern stLiveSerSessionList_t *pstSesCtlThdLstHead;
#endif   //MODBUS_STACK_TCPIP_ENABLED

/**
 * Description
 * Exported function to set stack configuration parameter
 * This API has to be called before AppMbusMaster_StackInit() API
 *
 * @param pstDevConf [in] stDevConfig_t : list of parameters to be set
 *
 * @return uint8_t [out] return success if stack is not initialized
 */
MODBUS_STACK_EXPORT uint8_t AppMbusMaster_SetStackConfigParam(stDevConfig_t *a_pstDevConf)
{
	// check stack status
	if(g_bIsStackEnable)
	{
		return STACK_ERROR_STACK_IS_ALREADY_INITIALIZED;
	}
	if(a_pstDevConf != NULL)
	{
		// convert it to us
		if(a_pstDevConf->m_lInterframedelay > 0 &&
				a_pstDevConf->m_lInterframedelay < MAX_ENV_VAR_LEN)
		{
			g_stModbusDevConfig.m_lInterframedelay = (a_pstDevConf->m_lInterframedelay) * 1000; //*1000 is to convert millisecond value to microsecond
		}
		if(a_pstDevConf->m_lResponseTimeout > 0 &&
				a_pstDevConf->m_lResponseTimeout < MAX_ENV_VAR_LEN)
		{
			g_stModbusDevConfig.m_lResponseTimeout = (a_pstDevConf->m_lResponseTimeout) * 1000;
		}
	}
	// set default parameters
	else
	{
		g_stModbusDevConfig.m_lResponseTimeout = DEFAULT_RESPONSE_TIMEOUT_MS * 1000;
		g_stModbusDevConfig.m_lInterframedelay = DEFAULT_INTERFRAME_DELAY_MS * 1000;
	}

	printf("\n\nStack config parameters ------------------------\n");
	printf("response timeout is set to :: %ld us\n", g_stModbusDevConfig.m_lResponseTimeout);
	printf("Interframe delay is set to :: %ld us\n\n", g_stModbusDevConfig.m_lInterframedelay);

	return STACK_NO_ERROR;
}

/**
 * Description
 * Exported function to get Modbus master stack configuration parameter
 *
 * @param Nothing
 *
 * @return stDevConfig_t [out] available stack configurations
 */
MODBUS_STACK_EXPORT stDevConfig_t* AppMbusMaster_GetStackConfigParam()
{
	return &g_stModbusDevConfig;
}

/**
 *
 * Description
 * Exported function to initiate modbus master stack
 * @param none
 *
 * @return uint8_t [out] success or failure in integer format
 *
 */
MODBUS_STACK_EXPORT uint8_t AppMbusMaster_StackInit()
{
	/*Local variable */
	uint8_t eStatus = STACK_NO_ERROR;
	thread_Create_t stThreadParam = { 0 };
	g_bThreadExit = false;

#ifdef MODBUS_STACK_TCPIP_ENABLED

	if(-1 == initTCPRespStructs())
	{
		printf("failed to init initTCPRespStructs\n");
		eStatus = STACK_INIT_FAILED;
	}
	LivSerSesslist_Mutex = Osal_Mutex();
	if(NULL == LivSerSesslist_Mutex)
	{
		eStatus = STACK_ERROR_MUTEX_CREATE;
	}

#endif //#ifdef MODBUS_STACK_TCPIP_ENABLED

	if(!initReqManager())
	{
		eStatus = STACK_INIT_FAILED;
	}

	i32MsgQueIdSC = OSAL_Init_Message_Queue();

	// check for error
	if(-1 == i32MsgQueIdSC)
	{
		eStatus = STACK_ERROR_QUEUE_CREATE;
	}

	stThreadParam.dwStackSize = 0;
	stThreadParam.lpStartAddress = SessionControlThread;
	stThreadParam.lpParameter = &i32MsgQueIdSC;
	stThreadParam.lpThreadId = &SessionControl_ThreadId;

	SessionControl_ThreadId = Osal_Thread_Create(&stThreadParam);
	if(-1 == SessionControl_ThreadId)
	{
		eStatus = STACK_ERROR_THREAD_CREATE;
	}

	if(STACK_NO_ERROR == eStatus)
	{
		// set the stack enable status as true
		g_bIsStackEnable = true;
	}
	return eStatus;
}

/**
 *
 * Description
 * Exported function to de-initiate modbus master stack
 * @param none
 *
 * @return void [out] none
 *
 */
MODBUS_STACK_EXPORT void AppMbusMaster_StackDeInit(void)
{
	static bool bDeInitStackFlag = false;
	/* making the function non-reentrant */
	if(true == bDeInitStackFlag)
		return;
	g_bThreadExit = true;
	
	if(SessionControl_ThreadId)
	{
		Osal_Thread_Terminate(SessionControl_ThreadId);
	}
	if(i32MsgQueIdSC)
	{
		OSAL_Delete_Message_Queue(i32MsgQueIdSC);
	}
	/*if(TransactionId_Mutex)
	{
		Osal_Close_Mutex(TransactionId_Mutex);
	}*/
	
	#ifdef MODBUS_STACK_TCPIP_ENABLED
	// Delete session list
	if(0 != Osal_Wait_Mutex(LivSerSesslist_Mutex))
	{
		// fail to lock mutex
		return;
	}
	stLiveSerSessionList_t *pstTempLivSerSesslist = pstSesCtlThdLstHead;
	while(NULL != pstSesCtlThdLstHead)
	{
		pstTempLivSerSesslist = pstSesCtlThdLstHead;
		pstSesCtlThdLstHead = pstSesCtlThdLstHead->m_pNextElm;
		pstTempLivSerSesslist->m_pNextElm = NULL;
		if(pstTempLivSerSesslist->m_ThreadId)
		{
			Osal_Thread_Terminate(pstTempLivSerSesslist->m_ThreadId);
		}
		if(pstTempLivSerSesslist->MsgQId)
		{
			OSAL_Delete_Message_Queue(pstTempLivSerSesslist->MsgQId);
		}
		if(pstTempLivSerSesslist->m_i32sockfd)
		{
			close(pstTempLivSerSesslist->m_i32sockfd);
		}
		OSAL_Free(pstTempLivSerSesslist);
		pstTempLivSerSesslist = NULL;
	}
	Osal_Release_Mutex (LivSerSesslist_Mutex);
	
	if(LivSerSesslist_Mutex)
	{
		Osal_Close_Mutex(LivSerSesslist_Mutex);
	}

	deinitTCPRespStructs();

	#endif
	/* update re-entrancy flag */
	bDeInitStackFlag = false;
	g_bThreadExit = false;
}

/**
 *
 * Description
 * This function Validates  Quantity
 * A value contained in the query data field is not an allowable
 * value for server(slave).So send exception illegal data value
 *
 * @param eFunCode 		[in] uint8_t received function code
 * @param u16Quantity 	[in] uint16_t received quantity for function code
 * @param u8ByteCount	[in] uint8_t byte count for quantity
 *
 * @return bool 		[out] True for success or false for error
 *
 */
bool ValidateQuantity( uint8_t eFunCode,
		uint16_t u16Quantity ,
		uint8_t u8ByteCount)
{
	bool bExceptionFound = false;

	switch(eFunCode)
	{
		case READ_COIL_STATUS:
		case READ_INPUT_STATUS:
		{
			// If quantity is not in range then return ILLEGAL_DATA_VAL exception.
			if (( u16Quantity < MIN_COILS)||
				(u16Quantity > MAX_COILS))
			{
				bExceptionFound = true;
			}//Else not required
		}
		break;
		case READ_INPUT_REG:
		{
			// If quantity is not in range then return ILLEGAL_DATA_VAL exception.
			if (( u16Quantity < MIN_INPUT_REGISTER)||
				(u16Quantity > MAX_INPUT_REGISTER))
			{
				bExceptionFound = true;
			}//Else not required
		}
		break;
		case READ_HOLDING_REG:
		{
			if((u16Quantity < MIN_HOLDING_REGISTERS)||
				(u16Quantity >MAX_HOLDING_REGISTERS))
			{
				bExceptionFound = true;
			}//Else not required
		}
		break;
		case WRITE_MULTIPLE_COILS:
		{
			//Decide the byte count depending on no of coils
			uint8_t u8DummyCnt = VALUE_ZERO;
			u8DummyCnt = (u16Quantity / MAX_BITS);
			if(VALUE_ZERO != (u16Quantity % MAX_BITS))
			{
				u8DummyCnt = u8DummyCnt + 1;
			}

			// For Write multiple register,byte count should be 246
			if( (u16Quantity < MIN_MULTI_COIL)||
				(u16Quantity > MAX_MULTI_COIL) ||
				(VALUE_ZERO == u8ByteCount) || (u8DummyCnt != u8ByteCount))    //Byte Count Validation
			{
				bExceptionFound = true;
			}//Else not required
		}
 		break;
		case WRITE_MULTIPLE_REG:
		{
			// If quantity is not in range then return ILLEGAL_DATA_VAL exception.
			// For multiple Register byte count will be 246

			if ((u16Quantity < MIN_MULTI_REGISTER) ||
			     (u16Quantity > MAX_MULTI_REGISTER) ||
				(VALUE_ZERO == u8ByteCount) ||   ///Byte Count Validation
				(u8ByteCount != (uint8_t)(u16Quantity<<1))) ///Byte = No.Of Points * 2
			{
				bExceptionFound = true;
			}//Else not required
		}
		break;
		case READ_WRITE_MUL_REG:
		{
			if((u16Quantity < MIN_MUL_WRITE_REG)
					|| (u16Quantity >MAX_MUL_WRITE_REG)
					|| (u8ByteCount != (uint8_t)(u16Quantity<<1)))

			{
				bExceptionFound = true;
			}
		}
		break;
		case READ_FILE_RECORD:
		{
			if((u16Quantity < MIN_FILE_BYTE_COUNT)||
							(u16Quantity >MAX_FILE_BYTE_COUNT))

			{
				bExceptionFound = true;
			}
		}
		break;
		default:
		{
			// if there is no exception return false
			bExceptionFound = false;
		}
		break;
	}//End of switch

	return bExceptionFound;
}


/**
 *
 * Description
 * This function is to create header for modbus
 * read write coil or register request
 *
 * @param u16StartAddr 			[in] uint16_t start address
 * @param u8UnitId				[in] uint8_t unit id
 * @param u16HeaderLength		[in] uint16_t header length
 * @param u16TransacID			[in] uint16_t transaction id
 * @param u8FunctionCode		[in] uint8_t Function code
 * @param pstMBusRequesPacket 	[in] pointer to request packet struct of type stMbusPacketVariables_t
 *
 * @return uint8_t				[out] success or failure in integer format
 *
 */
uint8_t CreateHeaderForModbusRequest(uint16_t u16StartAddr,
							uint8_t u8UnitId,
							uint16_t u16HeaderLength,
							uint16_t u16TransacID,
							uint8_t u8FunctionCode,
							stMbusPacketVariables_t *pstMBusRequesPacket)
{
	stEndianess_t stEndianess = { 0 };
	uint16_t u16PacketIndex = 0;

#ifdef MODBUS_STACK_TCPIP_ENABLED

	stEndianess.u16word = u16TransacID;

	// TODO: Release Mutex lock
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8SecondByte;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8FirstByte;
	pstMBusRequesPacket->m_u16TransactionID = stEndianess.u16word;
#else
	pstMBusRequesPacket->m_u16TransactionID = u16TransacID;
#endif

#ifdef MODBUS_STACK_TCPIP_ENABLED
	// Protocol ID
	stEndianess.u16word = 0;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8SecondByte;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8FirstByte;

	// Length
	stEndianess.u16word = u16HeaderLength;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8SecondByte;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8FirstByte;

#else

	pstMBusRequesPacket->m_u8ReceivedDestination = u8UnitId;

#endif //MODBUS_STACK_TCPIP_ENABLED

	// Unit Id
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =  u8UnitId;
	pstMBusRequesPacket->m_u8UnitID = u8UnitId;

	// Function Code
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] = u8FunctionCode;
	pstMBusRequesPacket->m_u8FunctionCode = u8FunctionCode;

	// Starting address or offset address
	stEndianess.u16word = u16StartAddr;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8SecondByte;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8FirstByte;

	return u16PacketIndex;
}

/**
 *
 * Description
 * This function is to create header for modbus
 * read device identification request
 *
 * @param u8UnitId 				[in] unit id
 * @param u16HeaderLength 		[in] Header length
 * @param u16TransacID 			[in] transaction id
 * @param u8FunctionCode 		[in] Function code
 * @param pstMBusRequesPacket 	[in] Request packet
 *
 * @return uint8_t				[out] success or failure in integer format
 *
 */
uint8_t CreateHeaderForDevIdentificationModbusRequest(uint8_t u8UnitId,
							uint16_t u16HeaderLength,
							uint16_t u16TransacID,
							uint8_t u8FunctionCode,
							stMbusPacketVariables_t *pstMBusRequesPacket)
{

	uint16_t u16PacketIndex = 0;

#ifdef MODBUS_STACK_TCPIP_ENABLED  // Not required for RTU
	stEndianess_t stEndianess = { 0 };

	stEndianess.u16word = u16TransacID;
	/* TODO: Release Mutex lock */
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8SecondByte;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8FirstByte;

	pstMBusRequesPacket->m_u16TransactionID = stEndianess.u16word;
#else
	pstMBusRequesPacket->m_u16TransactionID = u16TransacID;
#endif

#ifdef MODBUS_STACK_TCPIP_ENABLED
	/* Protocol ID */
	stEndianess.u16word = 0;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8SecondByte;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8FirstByte;

	/* Length */
	stEndianess.u16word = u16HeaderLength;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8SecondByte;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8FirstByte;
#endif  //#ifdef MODBUS_STACK_TCPIP_ENABLED


		/* Unit Id */
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =  u8UnitId;
	pstMBusRequesPacket->m_u8UnitID = u8UnitId;

	/* Function Code */
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] = u8FunctionCode;
	pstMBusRequesPacket->m_u8FunctionCode = u8FunctionCode;

	return u16PacketIndex;
}

/**
 *
 * Description
 * This function is to validate input parameters
 *
 * @param u16StartCoilOrReg 	[in] uint16_t start address
 * @param u16NumberOfcoilsOrReg [in] uint16_t number of coils or registers
 * @param u8UnitID 				[in] uint8_t unit id
 * @param pFunCallBack 			[in] void callback function pointer
 * @param u8FunctionCode 		[in] uint8_t function code
 * @param u8ByteCount 			[in] uint16_t byte count
 *
 * @return uint8_t				[out] success or failure in integer format
 *
 */
uint8_t InputParameterVerification(uint16_t u16StartCoilOrReg, uint16_t u16NumberOfcoilsOrReg,
		uint8_t u8UnitID, void* pFunCallBack, uint8_t u8FunctionCode, uint8_t u8ByteCount)
{
	if (NULL == pFunCallBack)
	{
		//printf("STACK_ERROR_INVALID_INPUT_PARAMETER 0");
		return STACK_ERROR_INVALID_INPUT_PARAMETER;
	}

	/* Maximum allowed slave 0- 247 */
	if((0 == u8UnitID) || ((u8UnitID >= 248) && (u8UnitID < 255)))
	{
		//printf("STACK_ERROR_INVALID_INPUT_PARAMETER 1");
		return STACK_ERROR_INVALID_INPUT_PARAMETER;
	}

	if(ValidateQuantity(u8FunctionCode,u16NumberOfcoilsOrReg,u8ByteCount))
	{
		//printf("STACK_ERROR_INVALID_INPUT_PARAMETER 2");
		return STACK_ERROR_INVALID_INPUT_PARAMETER;
	}

	return STACK_NO_ERROR;
}

/**
 *
 * Description
 * Exported API to read coil
 *
 * @param u16StartCoil 	[in] uint16_t start address
 * @param u16NumOfcoils [in] uint16_t number of coils
 * @param u16TransacID 	[in] uint16_t transaction ID
 * @param u8UnitId 		[in] uint8_t unit id
 * @param pu8SerIpAddr 	[in] uint8_t server IP address
 * @param u16Port		[in] uint16_t port number
 * @param lPriority		[in] long priority
 * @param pFunCallBack 	[in] callback function pointer
 *
 * @return uint8_t		[out] success or failure in integer format
 *
 */
MODBUS_STACK_EXPORT uint8_t Modbus_Read_Coils(uint16_t u16StartCoil,
											  uint16_t u16NumOfcoils,
											  uint16_t u16TransacID,
											  uint8_t u8UnitId,
											  uint8_t *pu8SerIpAddr,
											  uint16_t u16Port,
											  long lPriority,
											  void* pFunCallBack)
{
	uint8_t	u8ReturnType = STACK_NO_ERROR;
	uint16_t u16PacketIndex = 0;
	uint8_t u8FunctionCode = READ_COIL_STATUS;
	stEndianess_t stEndianess = { 0 };
	stMbusPacketVariables_t *pstMBusRequesPacket = NULL;
	Post_Thread_Msg_t stPostThreadMsg = { 0 };
	struct timespec tsReqRcvd = (struct timespec){0};

	// Init req rcvd timestamp
	timespec_get(&tsReqRcvd, TIME_UTC);

	u8ReturnType = InputParameterVerification(u16StartCoil, u16NumOfcoils, u8UnitId, pFunCallBack, u8FunctionCode,0);
	if(STACK_NO_ERROR != u8ReturnType)
	{
		return u8ReturnType;
	}

	pstMBusRequesPacket = emplaceNewRequest(tsReqRcvd);

	if(NULL == pstMBusRequesPacket)
	{
		return STACK_ERROR_MAX_REQ_SENT;
	}

	pstMBusRequesPacket->m_u16AppTxID = u16TransacID;
#ifdef MODBUS_STACK_TCPIP_ENABLED
	u16PacketIndex = CreateHeaderForModbusRequest(	u16StartCoil,
													u8UnitId,
													6,
													pstMBusRequesPacket->m_ulMyId,
													u8FunctionCode,
													pstMBusRequesPacket);
#else

	u16PacketIndex = CreateHeaderForModbusRequest(	u16StartCoil,
													u8UnitId,
													0,
													pstMBusRequesPacket->m_ulMyId,
													u8FunctionCode,
													pstMBusRequesPacket);
#endif

	stEndianess.u16word = u16NumOfcoils;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8SecondByte;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8FirstByte;

	pstMBusRequesPacket->m_stMbusTxData.m_u16Length = (u16PacketIndex);

	pstMBusRequesPacket->pFunc = pFunCallBack;

#ifdef MODBUS_STACK_TCPIP_ENABLED
	pstMBusRequesPacket->m_u8IpAddr[0] = pu8SerIpAddr[0];
	pstMBusRequesPacket->m_u8IpAddr[1] = pu8SerIpAddr[1];
	pstMBusRequesPacket->m_u8IpAddr[2] = pu8SerIpAddr[2];
	pstMBusRequesPacket->m_u8IpAddr[3] = pu8SerIpAddr[3];
	pstMBusRequesPacket->u16Port = u16Port;
#else
	pstMBusRequesPacket->m_u8ReceivedDestination = u8UnitId;
#endif   //#ifdef MODBUS_STACK_TCPIP_ENABLED


	pstMBusRequesPacket->m_u16StartAdd = u16StartCoil;

	pstMBusRequesPacket->m_u16Quantity = u16NumOfcoils;

	if(u16PacketIndex >= TCP_MODBUS_ADU_LENGTH)
	{
		freeReqNode(pstMBusRequesPacket);
		return STACK_ERROR_PACKET_LENGTH_EXCEEDED;
	}

	pstMBusRequesPacket->m_lPriority = lPriority;
	stPostThreadMsg.idThread = i32MsgQueIdSC;
	stPostThreadMsg.lParam = NULL;
	stPostThreadMsg.wParam = pstMBusRequesPacket;
	stPostThreadMsg.MsgType = lPriority;

	if(!OSAL_Post_Message(&stPostThreadMsg))
	{
		u8ReturnType = STACK_ERROR_QUEUE_SEND;
		freeReqNode(pstMBusRequesPacket);
	}

	return u8ReturnType;
}

/**
 *
 * Description
 * Exported API to read discrete inputs
 *
 * @param u16StartDI 	[in] uint16_t start address
 * @param u16NumOfDI 	[in] uint16_t number of discrete inputs
 * @param u16TransacID 	[in] uint16_t transaction ID
 * @param u8UnitId 		[in] uint8_t unit id
 * @param pu8SerIpAddr 	[in] uint8_t server IP address
 * @param u16Port		[in] uint16_t port number
 * @param lPriority		[in] long priority
 * @param pFunCallBack 	[in] callback function pointer
 *
 * @return uint8_t		[out] success or failure in integer format
 *
 */
MODBUS_STACK_EXPORT uint8_t Modbus_Read_Discrete_Inputs(uint16_t u16StartDI,
														uint16_t u16NumOfDI,
														uint16_t u16TransacID,
														uint8_t u8UnitId,
														uint8_t *pu8SerIpAddr,
														uint16_t u16Port,
														long lPriority,
														void* pFunCallBack)
{
	uint8_t	u8ReturnType = STACK_NO_ERROR;
	uint16_t u16PacketIndex = 0;
	uint8_t u8FunctionCode = READ_INPUT_STATUS;
	stEndianess_t stEndianess = { 0 };
	stMbusPacketVariables_t *pstMBusRequesPacket = NULL;
	Post_Thread_Msg_t stPostThreadMsg = { 0 };
	struct timespec tsReqRcvd = (struct timespec){0};

	// Init req rcvd timestamp
	timespec_get(&tsReqRcvd, TIME_UTC);

	u8ReturnType = InputParameterVerification(u16StartDI, u16NumOfDI, u8UnitId, pFunCallBack, u8FunctionCode, 0);
	if(STACK_NO_ERROR != u8ReturnType)
	{
		return u8ReturnType;
	}

	pstMBusRequesPacket = emplaceNewRequest(tsReqRcvd);
	if(NULL == pstMBusRequesPacket)
	{
		return STACK_ERROR_MAX_REQ_SENT;
	}
	pstMBusRequesPacket->m_u16AppTxID = u16TransacID;

#ifdef MODBUS_STACK_TCPIP_ENABLED

	u16PacketIndex = CreateHeaderForModbusRequest(	u16StartDI,
													u8UnitId,
													6,
													pstMBusRequesPacket->m_ulMyId,
													u8FunctionCode,
													pstMBusRequesPacket);

#else
	u16PacketIndex = CreateHeaderForModbusRequest(	u16StartDI,
													u8UnitId,
													0,
													pstMBusRequesPacket->m_ulMyId,
													u8FunctionCode,
													pstMBusRequesPacket);
#endif

	stEndianess.u16word = u16NumOfDI;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8SecondByte;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8FirstByte;

	pstMBusRequesPacket->m_stMbusTxData.m_u16Length = (u16PacketIndex);

	pstMBusRequesPacket->pFunc = pFunCallBack;

#ifdef MODBUS_STACK_TCPIP_ENABLED

	pstMBusRequesPacket->m_u8IpAddr[0] = pu8SerIpAddr[0];
	pstMBusRequesPacket->m_u8IpAddr[1] = pu8SerIpAddr[1];
	pstMBusRequesPacket->m_u8IpAddr[2] = pu8SerIpAddr[2];
	pstMBusRequesPacket->m_u8IpAddr[3] = pu8SerIpAddr[3];
	pstMBusRequesPacket->u16Port = u16Port;

#else
	pstMBusRequesPacket->m_u8ReceivedDestination = u8UnitId;
#endif

	pstMBusRequesPacket->m_u16StartAdd = u16StartDI;

	pstMBusRequesPacket->m_u16Quantity = u16NumOfDI;

	if(u16PacketIndex >= TCP_MODBUS_ADU_LENGTH)
	{
		freeReqNode(pstMBusRequesPacket);
		return STACK_ERROR_PACKET_LENGTH_EXCEEDED;
	}

	pstMBusRequesPacket->m_lPriority = lPriority;
	stPostThreadMsg.idThread = i32MsgQueIdSC;
	stPostThreadMsg.lParam = NULL;
	stPostThreadMsg.wParam = pstMBusRequesPacket;
	stPostThreadMsg.MsgType = lPriority;

	if(!OSAL_Post_Message(&stPostThreadMsg))
	{
		u8ReturnType = STACK_ERROR_QUEUE_SEND;
		freeReqNode(pstMBusRequesPacket);
	}

	return u8ReturnType;

}

/**
 *
 * Description
 * Exported API to read Holding Registers
 *
 * @param u16StartReg 			[in] uint16_t start address
 * @param u16NumberOfRegisters 	[in] uint16_t number of registers
 * @param u16TransacID 			[in] uint16_t transaction ID
 * @param u8UnitId 				[in] uint8_t unit id
 * @param pu8SerIpAddr 			[in] uint8_t server IP address
 * @param u16Port				[in] uint16_t port number
 * @param lPriority				[in] long priority
 * @param pFunCallBack 			[in] callback function pointer
 *
 * @return uint8_t		[out] success or failure in integer format
 *
 */
MODBUS_STACK_EXPORT uint8_t Modbus_Read_Holding_Registers(uint16_t u16StartReg,
														  uint16_t u16NumberOfRegisters,
														  uint16_t u16TransacID,
														  uint8_t u8UnitId,
														  uint8_t *pu8SerIpAddr,
														  uint16_t u16Port,
														  long lPriority,
														  void* pFunCallBack)
{
	uint8_t	u8ReturnType = STACK_NO_ERROR;
	uint16_t u16PacketIndex = 0;
	uint8_t u8FunctionCode = READ_HOLDING_REG;
	stEndianess_t stEndianess = { 0 };
	stMbusPacketVariables_t *pstMBusRequesPacket = NULL;
	Post_Thread_Msg_t stPostThreadMsg = { 0 };
	struct timespec tsReqRcvd = (struct timespec){0};

	// Init req rcvd timestamp
	timespec_get(&tsReqRcvd, TIME_UTC);

	u8ReturnType = InputParameterVerification(u16StartReg, u16NumberOfRegisters, u8UnitId, pFunCallBack, u8FunctionCode,0);
	if(STACK_NO_ERROR != u8ReturnType)
	{
		return u8ReturnType;
	}

	pstMBusRequesPacket = emplaceNewRequest(tsReqRcvd);

	if(NULL == pstMBusRequesPacket)
	{
		return STACK_ERROR_MAX_REQ_SENT;
	}

	pstMBusRequesPacket->m_u16AppTxID = u16TransacID;
#ifdef MODBUS_STACK_TCPIP_ENABLED
	u16PacketIndex = CreateHeaderForModbusRequest(u16StartReg,
													u8UnitId,
													6,
													pstMBusRequesPacket->m_ulMyId,
													u8FunctionCode,
													pstMBusRequesPacket);
#else

	u16PacketIndex = CreateHeaderForModbusRequest(u16StartReg,
													u8UnitId,
													0,
													pstMBusRequesPacket->m_ulMyId,
													u8FunctionCode,
													pstMBusRequesPacket);
#endif
	stEndianess.u16word = u16NumberOfRegisters;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8SecondByte;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8FirstByte;

	pstMBusRequesPacket->m_stMbusTxData.m_u16Length = (u16PacketIndex);

	pstMBusRequesPacket->pFunc = pFunCallBack;

#ifdef MODBUS_STACK_TCPIP_ENABLED

	pstMBusRequesPacket->m_u8IpAddr[0] = pu8SerIpAddr[0];
	pstMBusRequesPacket->m_u8IpAddr[1] = pu8SerIpAddr[1];
	pstMBusRequesPacket->m_u8IpAddr[2] = pu8SerIpAddr[2];
	pstMBusRequesPacket->m_u8IpAddr[3] = pu8SerIpAddr[3];
	pstMBusRequesPacket->u16Port = u16Port;

#else
	pstMBusRequesPacket->m_u8ReceivedDestination = u8UnitId;
#endif
	pstMBusRequesPacket->m_u16StartAdd = u16StartReg;

	pstMBusRequesPacket->m_u16Quantity = u16NumberOfRegisters;

	if(u16PacketIndex >= TCP_MODBUS_ADU_LENGTH)
	{
		freeReqNode(pstMBusRequesPacket);
		return STACK_ERROR_PACKET_LENGTH_EXCEEDED;
	}

	pstMBusRequesPacket->m_lPriority = lPriority;
	stPostThreadMsg.idThread = i32MsgQueIdSC;
	stPostThreadMsg.lParam = NULL;
	stPostThreadMsg.wParam = pstMBusRequesPacket;
	stPostThreadMsg.MsgType = lPriority;

	if(!OSAL_Post_Message(&stPostThreadMsg))
	{
		u8ReturnType = STACK_ERROR_QUEUE_SEND;
		freeReqNode(pstMBusRequesPacket);
	}

	return u8ReturnType;
}

/**
 *
 * Description
 * Read Input Registers API
 *
 * @param u16StartReg 			[in] uint16_t start address
 * @param u16NumberOfRegisters 	[in] uint16_t number of registers
 * @param u16TransacID 			[in] uint16_t transaction ID
 * @param u8UnitId 				[in] uint8_t unit id
 * @param pu8SerIpAddr 			[in] uint8_t server IP address
 * @param u16Port				[in] uint16_t port number
 * @param lPriority				[in] long priority
 * @param pFunCallBack 			[in] callback function pointer
 *
 * @return uint8_t		[out] success or failure in integer format
 *
 */
MODBUS_STACK_EXPORT uint8_t Modbus_Read_Input_Registers(uint16_t u16StartReg,
													    uint16_t u16NumberOfRegisters,
														uint16_t u16TransacID,
														uint8_t u8UnitId,
														uint8_t *pu8SerIpAddr,
														uint16_t u16Port,
														long lPriority,
														void* pFunCallBack)
{
	uint8_t	u8ReturnType = STACK_NO_ERROR;
	uint16_t u16PacketIndex = 0;
	uint8_t u8FunctionCode = READ_INPUT_REG;

	stEndianess_t stEndianess = { 0 };
	stMbusPacketVariables_t *pstMBusRequesPacket = NULL;
	Post_Thread_Msg_t stPostThreadMsg = { 0 };
	struct timespec tsReqRcvd = (struct timespec){0};

	// Init req rcvd timestamp
	timespec_get(&tsReqRcvd, TIME_UTC);

	u8ReturnType = InputParameterVerification(u16StartReg, u16NumberOfRegisters, u8UnitId, pFunCallBack, u8FunctionCode, 0);
	if(STACK_NO_ERROR != u8ReturnType)
	{
		return u8ReturnType;
	}
	pstMBusRequesPacket = emplaceNewRequest(tsReqRcvd);

	if(NULL == pstMBusRequesPacket)
	{
		return STACK_ERROR_MAX_REQ_SENT;
	}

	pstMBusRequesPacket->m_u16AppTxID = u16TransacID;
#ifdef MODBUS_STACK_TCPIP_ENABLED
	u16PacketIndex = CreateHeaderForModbusRequest(	u16StartReg,
													u8UnitId,
													6,
													pstMBusRequesPacket->m_ulMyId,
													u8FunctionCode,
													pstMBusRequesPacket);
#else

	u16PacketIndex = CreateHeaderForModbusRequest(	u16StartReg,
													u8UnitId,
													0,
													pstMBusRequesPacket->m_ulMyId,
													u8FunctionCode,
													pstMBusRequesPacket);

#endif

	stEndianess.u16word = u16NumberOfRegisters;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8SecondByte;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8FirstByte;

	pstMBusRequesPacket->m_stMbusTxData.m_u16Length = (u16PacketIndex);

	pstMBusRequesPacket->pFunc = pFunCallBack;


#ifdef MODBUS_STACK_TCPIP_ENABLED

	pstMBusRequesPacket->m_u8IpAddr[0] = pu8SerIpAddr[0];
	pstMBusRequesPacket->m_u8IpAddr[1] = pu8SerIpAddr[1];
	pstMBusRequesPacket->m_u8IpAddr[2] = pu8SerIpAddr[2];
	pstMBusRequesPacket->m_u8IpAddr[3] = pu8SerIpAddr[3];
	pstMBusRequesPacket->u16Port = u16Port;

#else
	pstMBusRequesPacket->m_u8ReceivedDestination = u8UnitId;
#endif
	pstMBusRequesPacket->m_u16StartAdd = u16StartReg;

	pstMBusRequesPacket->m_u16Quantity = u16NumberOfRegisters;

	if(u16PacketIndex >= TCP_MODBUS_ADU_LENGTH)
	{
		freeReqNode(pstMBusRequesPacket);
		return STACK_ERROR_PACKET_LENGTH_EXCEEDED;
	}

	pstMBusRequesPacket->m_lPriority = lPriority;
	stPostThreadMsg.idThread = i32MsgQueIdSC;
	stPostThreadMsg.lParam = NULL;
	stPostThreadMsg.wParam = pstMBusRequesPacket;
	stPostThreadMsg.MsgType = lPriority;

	if(!OSAL_Post_Message(&stPostThreadMsg))
	{
		u8ReturnType = STACK_ERROR_QUEUE_SEND;
		freeReqNode(pstMBusRequesPacket);
	}

	return u8ReturnType;
}

/**
 *
 * Description
 * Write Single Coil API
 *
 * @param u16StartCoil 		[in] start address
 * @param u16OutputVal 		[in] value to be write
 * @param u16TransacID 		[in] transaction ID
 * @param u8UnitId 			[in] unit id
 * @param pu8SerIpAddr 		[in] server IP address
 * @param u16Port			[in] uint16_t port number
 * @param lPriority			[in] long priority
 * @param pFunCallBack 		[in] callback function pointer
 * @return uint8_t			[out] success or failure in integer format
 *
 */
MODBUS_STACK_EXPORT uint8_t Modbus_Write_Single_Coil(uint16_t u16StartCoil,
													 uint16_t u16OutputVal,
													 uint16_t u16TransacID,
													 uint8_t u8UnitId,
													 uint8_t *pu8SerIpAddr,
													 uint16_t u16Port,
													 long lPriority,
													 void* pFunCallBack)
{
	uint8_t	u8ReturnType = STACK_NO_ERROR;
	uint16_t u16PacketIndex = 0;
	uint8_t u8FunctionCode = WRITE_SINGLE_COIL;
	stEndianess_t stEndianess = { 0 };
	stMbusPacketVariables_t *pstMBusRequesPacket = NULL;
	Post_Thread_Msg_t stPostThreadMsg = { 0 };
	struct timespec tsReqRcvd = (struct timespec){0};

	// Init req rcvd timestamp
	timespec_get(&tsReqRcvd, TIME_UTC);

	u8ReturnType = InputParameterVerification(u16StartCoil,u16OutputVal, u8UnitId, pFunCallBack, u8FunctionCode,0);
	if(STACK_NO_ERROR != u8ReturnType)
	{
		return u8ReturnType;
	}

	pstMBusRequesPacket = emplaceNewRequest(tsReqRcvd);

	if(NULL == pstMBusRequesPacket)
	{
		return STACK_ERROR_MAX_REQ_SENT;
	}

	pstMBusRequesPacket->m_u16AppTxID = u16TransacID;
#ifdef MODBUS_STACK_TCPIP_ENABLED
	u16PacketIndex = CreateHeaderForModbusRequest(	u16StartCoil,
													u8UnitId,
													6,
													pstMBusRequesPacket->m_ulMyId,
													u8FunctionCode,
													pstMBusRequesPacket);
#else
	u16PacketIndex = CreateHeaderForModbusRequest(	u16StartCoil,
													u8UnitId,
													0,
													pstMBusRequesPacket->m_ulMyId,
													u8FunctionCode,
													pstMBusRequesPacket);

#endif

	stEndianess.u16word = u16OutputVal;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8SecondByte;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8FirstByte;

	pstMBusRequesPacket->m_stMbusTxData.m_u16Length = (u16PacketIndex);

	pstMBusRequesPacket->pFunc = pFunCallBack;


#ifdef MODBUS_STACK_TCPIP_ENABLED

	pstMBusRequesPacket->m_u8IpAddr[0] = pu8SerIpAddr[0];
	pstMBusRequesPacket->m_u8IpAddr[1] = pu8SerIpAddr[1];
	pstMBusRequesPacket->m_u8IpAddr[2] = pu8SerIpAddr[2];
	pstMBusRequesPacket->m_u8IpAddr[3] = pu8SerIpAddr[3];
	pstMBusRequesPacket->u16Port = u16Port;

#else
	pstMBusRequesPacket->m_u8ReceivedDestination = u8UnitId;
#endif

	if(u16PacketIndex >= TCP_MODBUS_ADU_LENGTH)
	{
		freeReqNode(pstMBusRequesPacket);
		return STACK_ERROR_PACKET_LENGTH_EXCEEDED;
	}

	pstMBusRequesPacket->m_lPriority = lPriority;
	stPostThreadMsg.idThread = i32MsgQueIdSC;
	stPostThreadMsg.lParam = NULL;
	stPostThreadMsg.wParam = pstMBusRequesPacket;
	stPostThreadMsg.MsgType = lPriority;

	if(!OSAL_Post_Message(&stPostThreadMsg))
	{
		u8ReturnType = STACK_ERROR_QUEUE_SEND;
		freeReqNode(pstMBusRequesPacket);
	}

	return u8ReturnType;
}

/**
 *
 * Description
 * Write Single register API
 *
 * @param u16StartReg 		[in] start address
 * @param u16RegOutputVal 		[in] value to be write
 * @param u16TransacID 		[in] transaction ID
 * @param u8UnitId 			[in] unit id
 * @param pu8SerIpAddr 		[in] server IP address
 * @param u16Port			[in] uint16_t port number
 * @param lPriority			[in] long priority
 * @param pFunCallBack 		[in] callback function pointer
 * @return uint8_t			[out] success or failure in integer format
 *
 */
MODBUS_STACK_EXPORT uint8_t Modbus_Write_Single_Register(uint16_t u16StartReg,
														 uint16_t u16RegOutputVal,
														 uint16_t u16TransacID,
														 uint8_t u8UnitId,
														 uint8_t *pu8SerIpAddr,
														 uint16_t u16Port,
														 long lPriority,
														 void* pFunCallBack)
{
	uint8_t	u8ReturnType = STACK_NO_ERROR;
	uint16_t u16PacketIndex = 0;
	uint8_t u8FunctionCode = WRITE_SINGLE_REG;
	stEndianess_t stEndianess = { 0 };
	stMbusPacketVariables_t *pstMBusRequesPacket = NULL;
	Post_Thread_Msg_t stPostThreadMsg = { 0 };
	struct timespec tsReqRcvd = (struct timespec){0};

	// Init req rcvd timestamp
	timespec_get(&tsReqRcvd, TIME_UTC);

	u8ReturnType = InputParameterVerification(u16StartReg, u16RegOutputVal, u8UnitId, pFunCallBack, u8FunctionCode,0);
	if(STACK_NO_ERROR != u8ReturnType)
	{
		return u8ReturnType;
	}
	pstMBusRequesPacket = emplaceNewRequest(tsReqRcvd);

	if(NULL == pstMBusRequesPacket)
	{
		return STACK_ERROR_MAX_REQ_SENT;
	}

	pstMBusRequesPacket->m_u16AppTxID = u16TransacID;

#ifdef MODBUS_STACK_TCPIP_ENABLED
	u16PacketIndex = CreateHeaderForModbusRequest(	u16StartReg,
													u8UnitId,
													6,
													pstMBusRequesPacket->m_ulMyId,
													u8FunctionCode,
													pstMBusRequesPacket);
#else
	u16PacketIndex = CreateHeaderForModbusRequest(	u16StartReg,
													u8UnitId,
													0,
													pstMBusRequesPacket->m_ulMyId,
													u8FunctionCode,
													pstMBusRequesPacket);
#endif //MODBUS_STACK_TCPIP_ENABLED

	stEndianess.u16word = u16RegOutputVal;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8SecondByte;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8FirstByte;

	pstMBusRequesPacket->m_stMbusTxData.m_u16Length = (u16PacketIndex);

	pstMBusRequesPacket->pFunc = pFunCallBack;


#ifdef MODBUS_STACK_TCPIP_ENABLED

	pstMBusRequesPacket->m_u8IpAddr[0] = pu8SerIpAddr[0];
	pstMBusRequesPacket->m_u8IpAddr[1] = pu8SerIpAddr[1];
	pstMBusRequesPacket->m_u8IpAddr[2] = pu8SerIpAddr[2];
	pstMBusRequesPacket->m_u8IpAddr[3] = pu8SerIpAddr[3];
	pstMBusRequesPacket->u16Port = u16Port;

#else
	pstMBusRequesPacket->m_u8ReceivedDestination = u8UnitId;
#endif
	if(u16PacketIndex >= TCP_MODBUS_ADU_LENGTH)
	{
		freeReqNode(pstMBusRequesPacket);
		return STACK_ERROR_PACKET_LENGTH_EXCEEDED;
	}

	pstMBusRequesPacket->m_lPriority = lPriority;
	stPostThreadMsg.idThread = i32MsgQueIdSC;
	stPostThreadMsg.lParam = NULL;
	stPostThreadMsg.wParam = pstMBusRequesPacket;
	stPostThreadMsg.MsgType = lPriority;

	if(!OSAL_Post_Message(&stPostThreadMsg))
	{
		u8ReturnType = STACK_ERROR_QUEUE_SEND;
		//free(pstMBusRequesPacket);
		freeReqNode(pstMBusRequesPacket);
	}

	return u8ReturnType;
}

/**
 *
 * Description
 * Write Multiple Coils API
 *
 * @param u16Startcoil 		[in] start address
 * @param u16NumOfCoil 		[in] Numer of coilss
 * @param pu8OutputVal 		[in] value to be write
 * @param u16TransacID 		[in] transaction ID
 * @param u8UnitId 			[in] unit id
 * @param pu8SerIpAddr 		[in] server IP address
 * @param u16Port			[in] uint16_t port number
 * @param lPriority			[in] long priority
 * @param pFunCallBack 		[in] callback function pointer
 *
 * @return uint8_t			[out] success or failure in integer format
 *
 */
MODBUS_STACK_EXPORT uint8_t Modbus_Write_Multiple_Coils(uint16_t u16Startcoil,
									   uint16_t u16NumOfCoil,
									   uint16_t u16TransacID,
									   uint8_t  *pu8OutputVal,
									   uint8_t  u8UnitId,
									   uint8_t  *pu8SerIpAddr,
									   uint16_t u16Port,
									   long lPriority,
									   void*    pFunCallBack)
{
	uint8_t	u8ReturnType = STACK_NO_ERROR;
	uint16_t u16PacketIndex = 0;
	uint8_t u8FunctionCode = WRITE_MULTIPLE_COILS;
	uint8_t u8ByteCount = (0 != (u16NumOfCoil%8))?((u16NumOfCoil/8)+1):(u16NumOfCoil/8);

	stEndianess_t stEndianess = { 0 };
	stMbusPacketVariables_t *pstMBusRequesPacket = NULL;
	Post_Thread_Msg_t stPostThreadMsg = { 0 };
	struct timespec tsReqRcvd = (struct timespec){0};

	// Init req rcvd timestamp
	timespec_get(&tsReqRcvd, TIME_UTC);

	u8ReturnType = InputParameterVerification(u16Startcoil, u16NumOfCoil, u8UnitId, pFunCallBack, u8FunctionCode,u8ByteCount);
	if(STACK_NO_ERROR != u8ReturnType)
	{
		return u8ReturnType;
	}
	pstMBusRequesPacket = emplaceNewRequest(tsReqRcvd);

	if(NULL == pstMBusRequesPacket)
	{
		return STACK_ERROR_MAX_REQ_SENT;
	}
	pstMBusRequesPacket->m_u16AppTxID = u16TransacID;
#ifdef MODBUS_STACK_TCPIP_ENABLED
	u16PacketIndex = CreateHeaderForModbusRequest(	u16Startcoil,
													u8UnitId,
													7+u8ByteCount,
													pstMBusRequesPacket->m_ulMyId,
													u8FunctionCode,
													pstMBusRequesPacket);
#else
	u16PacketIndex = CreateHeaderForModbusRequest(	u16Startcoil,
													u8UnitId,
													0,
													pstMBusRequesPacket->m_ulMyId,
													u8FunctionCode,
													pstMBusRequesPacket);
#endif

	stEndianess.u16word = u16NumOfCoil;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8SecondByte;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8FirstByte;

	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] = u8ByteCount;

	while(u8ByteCount > 0 && NULL != pu8OutputVal)
	{
		if(u16PacketIndex >= TCP_MODBUS_ADU_LENGTH)
		{
			freeReqNode(pstMBusRequesPacket);
			return STACK_ERROR_PACKET_LENGTH_EXCEEDED;
		}
		pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
				*pu8OutputVal;
		pu8OutputVal++;
		u8ByteCount--;
	}

	pstMBusRequesPacket->m_stMbusTxData.m_u16Length = (u16PacketIndex);

	pstMBusRequesPacket->pFunc = pFunCallBack;


#ifdef MODBUS_STACK_TCPIP_ENABLED

	pstMBusRequesPacket->m_u8IpAddr[0] = pu8SerIpAddr[0];
	pstMBusRequesPacket->m_u8IpAddr[1] = pu8SerIpAddr[1];
	pstMBusRequesPacket->m_u8IpAddr[2] = pu8SerIpAddr[2];
	pstMBusRequesPacket->m_u8IpAddr[3] = pu8SerIpAddr[3];
	pstMBusRequesPacket->u16Port = u16Port;

#else
	pstMBusRequesPacket->m_u8ReceivedDestination = u8UnitId;
#endif
	if(u16PacketIndex >= TCP_MODBUS_ADU_LENGTH)
	{
		freeReqNode(pstMBusRequesPacket);
		return STACK_ERROR_PACKET_LENGTH_EXCEEDED;
	}

	pstMBusRequesPacket->m_lPriority = lPriority;
	stPostThreadMsg.idThread = i32MsgQueIdSC;
	stPostThreadMsg.lParam = NULL;
	stPostThreadMsg.wParam = pstMBusRequesPacket;
	stPostThreadMsg.MsgType = lPriority;

	if(!OSAL_Post_Message(&stPostThreadMsg))
	{
		u8ReturnType = STACK_ERROR_QUEUE_SEND;
		freeReqNode(pstMBusRequesPacket);
	}

	return u8ReturnType;
}

/**
 *
 * Description
 * Write Multiple Registers API
 *
 * @param u16StartReg 		[in] start address
 * @param u16NumOfReg 		[in] Number of registers
 * @param pu8OutputVal 		[in] value to be write
 * @param u16TransacID 		[in] transaction ID
 * @param u8UnitId 			[in] unit id
 * @param pu8SerIpAddr 		[in] server IP address
 * @param u16Port			[in] uint16_t port number
 * @param lPriority			[in] long priority
 * @param pFunCallBack 		[in] callback function pointer
 *
 * @return uint8_t			[out] success or failure in integer format
 *
 */
MODBUS_STACK_EXPORT uint8_t Modbus_Write_Multiple_Register(uint16_t u16StartReg,
									   uint16_t u16NumOfReg,
									   uint16_t u16TransacID,
									   uint8_t  *pu8OutputVal,
									   uint8_t  u8UnitId,
									   uint8_t  *pu8SerIpAddr,
									   uint16_t u16Port,
									   long lPriority,
									   void*    pFunCallBack)
{
	uint8_t	u8ReturnType = STACK_NO_ERROR;
	uint16_t u16PacketIndex = 0;
	uint8_t  u8ByteCount = ((u16NumOfReg * 2) > 246)?246:(u16NumOfReg * 2);
	uint8_t u8FunctionCode = WRITE_MULTIPLE_REG;
	uint16_t *pu16OutputVal = (uint16_t *)pu8OutputVal;
	stEndianess_t stEndianess = { 0 };
	stMbusPacketVariables_t *pstMBusRequesPacket = NULL;
	Post_Thread_Msg_t stPostThreadMsg = { 0 };
	struct timespec tsReqRcvd = (struct timespec){0};

	// Init req rcvd timestamp
	timespec_get(&tsReqRcvd, TIME_UTC);

	u8ReturnType = InputParameterVerification(u16StartReg, u16NumOfReg, u8UnitId, pFunCallBack, u8FunctionCode,u8ByteCount);
	if(STACK_NO_ERROR != u8ReturnType)
	{
		return u8ReturnType;
	}
	pstMBusRequesPacket = emplaceNewRequest(tsReqRcvd);

	if(NULL == pstMBusRequesPacket)
	{
		return STACK_ERROR_MAX_REQ_SENT;
	}
	pstMBusRequesPacket->m_u16AppTxID = u16TransacID;
#ifdef MODBUS_STACK_TCPIP_ENABLED
	u16PacketIndex = CreateHeaderForModbusRequest(	u16StartReg,
													u8UnitId,
													7+u8ByteCount,
													pstMBusRequesPacket->m_ulMyId,
													u8FunctionCode,
													pstMBusRequesPacket);
#else

	u16PacketIndex = CreateHeaderForModbusRequest(	u16StartReg,
													u8UnitId,
													0,
													pstMBusRequesPacket->m_ulMyId,
													u8FunctionCode,
													pstMBusRequesPacket);
#endif

	stEndianess.u16word = u16NumOfReg;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8SecondByte;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8FirstByte;

#ifdef MODBUS_STACK_TCPIP_ENABLED

	pstMBusRequesPacket->m_u8IpAddr[0] = pu8SerIpAddr[0];
	pstMBusRequesPacket->m_u8IpAddr[1] = pu8SerIpAddr[1];
	pstMBusRequesPacket->m_u8IpAddr[2] = pu8SerIpAddr[2];
	pstMBusRequesPacket->m_u8IpAddr[3] = pu8SerIpAddr[3];
	pstMBusRequesPacket->u16Port = u16Port;

#else
	pstMBusRequesPacket->m_u8ReceivedDestination = u8UnitId;
#endif
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] = u8ByteCount;

	while(u16NumOfReg > 0 && NULL != pu16OutputVal)
	{
		if(u16PacketIndex >= TCP_MODBUS_ADU_LENGTH)
		{
			freeReqNode(pstMBusRequesPacket);
			return STACK_ERROR_PACKET_LENGTH_EXCEEDED;
		}
		stEndianess.u16word = *pu16OutputVal;
		pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
					stEndianess.stByteOrder.u8SecondByte;
		if(u16PacketIndex >= TCP_MODBUS_ADU_LENGTH)
		{
			freeReqNode(pstMBusRequesPacket);
			return STACK_ERROR_PACKET_LENGTH_EXCEEDED;
		}
		pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
					stEndianess.stByteOrder.u8FirstByte;
		pu16OutputVal++;
		u16NumOfReg--;
	}

	pstMBusRequesPacket->m_stMbusTxData.m_u16Length = (u16PacketIndex);

	pstMBusRequesPacket->pFunc = pFunCallBack;

	if(u16PacketIndex >= TCP_MODBUS_ADU_LENGTH)
	{
		freeReqNode(pstMBusRequesPacket);
		return STACK_ERROR_PACKET_LENGTH_EXCEEDED;
	}

	pstMBusRequesPacket->m_lPriority = lPriority;
	stPostThreadMsg.idThread = i32MsgQueIdSC;
	stPostThreadMsg.lParam = NULL;
	stPostThreadMsg.wParam = pstMBusRequesPacket;
	stPostThreadMsg.MsgType = lPriority;

	if(!OSAL_Post_Message(&stPostThreadMsg))
	{
		u8ReturnType = STACK_ERROR_QUEUE_SEND;
		freeReqNode(pstMBusRequesPacket);
	}

	return u8ReturnType;
}

/**
 *
 * Description
 * Read File Record API
 *
 * @param u8byteCount 		[in] byte count
 * @param u8FunCode			[in] function code
 * @param pstFileRecord     [in] pointer to records
 * @param u16TransacID 		[in] transaction ID
 * @param u8UnitId 			[in] unit id
 * @param pu8SerIpAddr 		[in] server IP address
 * @param u16Port			[in] uint16_t port number
 * @param lPriority			[in] long priority
 * @param pFunCallBack 		[in] callback function pointer
 *
 * @return uint8_t			[out] success or failure in integer format
 *
 */
MODBUS_STACK_EXPORT uint8_t Modbus_Read_File_Record(uint8_t u8byteCount,
													uint8_t u8FunCode,
													stMbusReadFileRecord_t *pstFileRecord,
													uint16_t u16TransacID,
													uint8_t u8UnitId,
													uint8_t *pu8SerIpAddr,
													uint16_t u16Port,
													long lPriority,
													void* pFunCallBack)
{
	uint8_t	u8ReturnType = STACK_NO_ERROR;
	uint16_t u16PacketIndex = 0;
	//uint8_t u8FunctionCode = u8FunCode;
	stEndianess_t stEndianess = { 0 };
	stMbusPacketVariables_t *pstMBusRequesPacket = NULL;
	Post_Thread_Msg_t stPostThreadMsg = { 0 };
	struct timespec tsReqRcvd = (struct timespec){0};

	// Init req rcvd timestamp
	timespec_get(&tsReqRcvd, TIME_UTC);

	u8ReturnType = InputParameterVerification(0,
			u8byteCount,
			u8UnitId,
			pFunCallBack,
			u8FunCode,
			0);

	if(STACK_NO_ERROR != u8ReturnType)
	{
		return u8ReturnType;
	}

	pstMBusRequesPacket = emplaceNewRequest(tsReqRcvd);

	if(NULL == pstMBusRequesPacket)
	{
		return STACK_ERROR_MAX_REQ_SENT;
	}

	pstMBusRequesPacket->m_u16AppTxID = u16TransacID;

#ifdef MODBUS_STACK_TCPIP_ENABLED
	/// Transaction ID
	stEndianess.u16word = pstMBusRequesPacket->m_ulMyId;

	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8SecondByte;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8FirstByte;
	pstMBusRequesPacket->m_u16TransactionID = stEndianess.u16word;
#else
	pstMBusRequesPacket->m_u16TransactionID = u16TransacID;
#endif

#ifdef MODBUS_STACK_TCPIP_ENABLED
	/// Protocol ID
	stEndianess.u16word = 0;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8SecondByte;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8FirstByte;

	/// Length
	stEndianess.u16word = u8byteCount+3;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8SecondByte;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8FirstByte;
#endif

	/// Unit Id
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =  u8UnitId;
	pstMBusRequesPacket->m_u8UnitID = u8UnitId;

	/// Function Code
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] = u8FunCode;
	pstMBusRequesPacket->m_u8FunctionCode = u8FunCode;

	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] = u8byteCount;

	while(NULL != pstFileRecord)
	{
		pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++]
					= pstFileRecord->m_u8RefType;
		if(FILE_RECORD_REFERENCE_TYPE != pstFileRecord->m_u8RefType)
		{
			freeReqNode(pstMBusRequesPacket);
			return STACK_ERROR_INVALID_INPUT_PARAMETER;
		}

		stEndianess.u16word = pstFileRecord->m_u16FileNum;
		pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
				stEndianess.stByteOrder.u8SecondByte;
		pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
				stEndianess.stByteOrder.u8FirstByte;

		stEndianess.u16word = pstFileRecord->m_u16RecordNum;
		pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
				stEndianess.stByteOrder.u8SecondByte;
		pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
				stEndianess.stByteOrder.u8FirstByte;

		stEndianess.u16word = pstFileRecord->m_u16RecordLength;
		pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
				stEndianess.stByteOrder.u8SecondByte;
		pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
				stEndianess.stByteOrder.u8FirstByte;

		pstFileRecord = pstFileRecord->pstNextNode;
	}

	pstMBusRequesPacket->m_stMbusTxData.m_u16Length = (u16PacketIndex);

	pstMBusRequesPacket->pFunc = pFunCallBack;

#ifdef MODBUS_STACK_TCPIP_ENABLED

	pstMBusRequesPacket->m_u8IpAddr[0] = pu8SerIpAddr[0];
	pstMBusRequesPacket->m_u8IpAddr[1] = pu8SerIpAddr[1];
	pstMBusRequesPacket->m_u8IpAddr[2] = pu8SerIpAddr[2];
	pstMBusRequesPacket->m_u8IpAddr[3] = pu8SerIpAddr[3];
	pstMBusRequesPacket->u16Port = u16Port;

#else
	pstMBusRequesPacket->m_u8ReceivedDestination = u8UnitId;
#endif

	if(u16PacketIndex >= TCP_MODBUS_ADU_LENGTH)
	{
		freeReqNode(pstMBusRequesPacket);
		return STACK_ERROR_PACKET_LENGTH_EXCEEDED;
	}

	pstMBusRequesPacket->m_lPriority = lPriority;
	stPostThreadMsg.idThread = i32MsgQueIdSC;
	stPostThreadMsg.lParam = NULL;
	stPostThreadMsg.wParam = pstMBusRequesPacket;
	stPostThreadMsg.MsgType = lPriority;

	if(!OSAL_Post_Message(&stPostThreadMsg))
	{
		u8ReturnType = STACK_ERROR_QUEUE_SEND;
		freeReqNode(pstMBusRequesPacket);
	}

	return u8ReturnType;
}

/**
 *
 * Description
 * Write File Record API
 *
 * @param u8ReqDataLen 		[in] Request data length
 * @param u8FunCode			[in] function code
 * @param pstFileRecord     [in] pointer to records
 * @param u16TransacID 		[in] transaction ID
 * @param u8UnitId 			[in] unit id
 * @param pu8SerIpAddr 		[in] server IP address
 * @param u16Port			[in] uint16_t port number
 * @param lPriority			[in] long priority
 * @param pFunCallBack 		[in] callback function pointer
 *
 * @return uint8_t			[out] success or failure in integer format
 *
 */
MODBUS_STACK_EXPORT uint8_t Modbus_Write_File_Record(uint8_t u8ReqDataLen,
													 uint8_t u8FunCode,
													 stWrFileSubReq_t *pstFileRecord,
													 uint16_t u16TransacID,
													 uint8_t u8UnitId,
													 uint8_t *pu8SerIpAddr,
													 uint16_t u16Port,
													 long lPriority,
													 void* pFunCallBack)
{
	uint8_t	u8ReturnType = STACK_NO_ERROR;
	uint16_t u16PacketIndex = 0;
	uint8_t	u8TempCount = 0;
	stEndianess_t stEndianess = { 0 };
	stMbusPacketVariables_t *pstMBusRequesPacket = NULL;
	Post_Thread_Msg_t stPostThreadMsg = { 0 };
	struct timespec tsReqRcvd = (struct timespec){0};

	// Init req rcvd timestamp
	timespec_get(&tsReqRcvd, TIME_UTC);

	u8ReturnType = InputParameterVerification(0,
			u8ReqDataLen,
			u8UnitId,
			pFunCallBack,
			u8FunCode,
			0);

	if(STACK_NO_ERROR != u8ReturnType)
	{
		return u8ReturnType;
	}

	pstMBusRequesPacket = emplaceNewRequest(tsReqRcvd);

	if(NULL == pstMBusRequesPacket)
	{
		return STACK_ERROR_MAX_REQ_SENT;
	}

	pstMBusRequesPacket->m_u16AppTxID = u16TransacID;

#ifdef MODBUS_STACK_TCPIP_ENABLED
	/// Transaction ID
	stEndianess.u16word = pstMBusRequesPacket->m_ulMyId;

	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8SecondByte;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8FirstByte;
	pstMBusRequesPacket->m_u16TransactionID = stEndianess.u16word;
#else
	/// Transaction ID
	pstMBusRequesPacket->m_u16TransactionID = u16TransacID;
#endif

#ifdef MODBUS_STACK_TCPIP_ENABLED
	/// Protocol ID
	stEndianess.u16word = 0;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8SecondByte;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8FirstByte;

	/// Length
	stEndianess.u16word = u8ReqDataLen+3;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8SecondByte;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8FirstByte;
#endif
	/// Unit Id
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =  u8UnitId;
	pstMBusRequesPacket->m_u8UnitID = u8UnitId;

	/// Function Code
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] = u8FunCode;
	pstMBusRequesPacket->m_u8FunctionCode = u8FunCode;

	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] = u8ReqDataLen;

	while(NULL != pstFileRecord)
	{
		pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++]
					= pstFileRecord->m_u8RefType;
		if(FILE_RECORD_REFERENCE_TYPE != pstFileRecord->m_u8RefType)
		{
			freeReqNode(pstMBusRequesPacket);
			return STACK_ERROR_INVALID_INPUT_PARAMETER;
		}

		stEndianess.u16word = pstFileRecord->m_u16FileNum;
		pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
				stEndianess.stByteOrder.u8SecondByte;
		pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
				stEndianess.stByteOrder.u8FirstByte;

		stEndianess.u16word = pstFileRecord->m_u16RecNum;
		pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
				stEndianess.stByteOrder.u8SecondByte;
		pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
				stEndianess.stByteOrder.u8FirstByte;

		stEndianess.u16word = pstFileRecord->m_u16RecLen;
		pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
				stEndianess.stByteOrder.u8SecondByte;
		pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
				stEndianess.stByteOrder.u8FirstByte;

		while(u8TempCount < (pstFileRecord->m_u16RecLen))
		{
			stEndianess.u16word = pstFileRecord->m_pu16RecData[u8TempCount];
			pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
					stEndianess.stByteOrder.u8SecondByte;
			pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
					stEndianess.stByteOrder.u8FirstByte;
			u8TempCount++;
		}
		u8TempCount = 0;
		pstFileRecord = pstFileRecord->pstNextNode;
	}

	pstMBusRequesPacket->m_stMbusTxData.m_u16Length = (u16PacketIndex);

	pstMBusRequesPacket->pFunc = pFunCallBack;

#ifdef MODBUS_STACK_TCPIP_ENABLED

	pstMBusRequesPacket->m_u8IpAddr[0] = pu8SerIpAddr[0];
	pstMBusRequesPacket->m_u8IpAddr[1] = pu8SerIpAddr[1];
	pstMBusRequesPacket->m_u8IpAddr[2] = pu8SerIpAddr[2];
	pstMBusRequesPacket->m_u8IpAddr[3] = pu8SerIpAddr[3];
	pstMBusRequesPacket->u16Port = u16Port;

#else
	pstMBusRequesPacket->m_u8ReceivedDestination = u8UnitId;
#endif

	if(u16PacketIndex >= TCP_MODBUS_ADU_LENGTH)
	{
		freeReqNode(pstMBusRequesPacket);
		return STACK_ERROR_PACKET_LENGTH_EXCEEDED;
	}

	pstMBusRequesPacket->m_lPriority = lPriority;
	stPostThreadMsg.idThread = i32MsgQueIdSC;
	stPostThreadMsg.lParam = NULL;
	stPostThreadMsg.wParam = pstMBusRequesPacket;
	stPostThreadMsg.MsgType = lPriority;

	if(!OSAL_Post_Message(&stPostThreadMsg))
	{
		u8ReturnType = STACK_ERROR_QUEUE_SEND;
		freeReqNode(pstMBusRequesPacket);
	}

	return u8ReturnType;
}

/**
 *
 * Description
 * Read Write Multiple Registers API
 *
 * @param u16ReadRegAddress [in] start address of read register
 * @param u8FunCode			[in] function code
 * @param u16NoOfReadReg 	[in] Number of read registers
 * @param u16WriteRegAddress [in] start address of write register
 * @param u16NoOfWriteReg 	[in] Number of write registers
 * @param pu8OutputVal 		[in] value to be write
 * @param u16TransacID 		[in] transaction ID
 * @param u8UnitId 			[in] unit id
 * @param pu8SerIpAddr 		[in] server IP address
 * @param u16Port			[in] uint16_t port number
 * @param lPriority			[in] long priority
 * @param pFunCallBack 		[in] callback function pointer
 *
 * @return uint8_t			[out] success or failure in integer format
 *
 */
MODBUS_STACK_EXPORT uint8_t Modbus_Read_Write_Registers(uint16_t u16ReadRegAddress,
									uint8_t u8FunCode,
									uint16_t u16NoOfReadReg,
									uint16_t u16WriteRegAddress,
									uint16_t u16NoOfWriteReg,
									uint16_t u16TransacID,
									uint8_t *pu8OutputVal,
									uint8_t u8UnitId,
									uint8_t *pu8SerIpAddr,
									uint16_t u16Port,
									long lPriority,
									void* pFunCallBack)
{
	uint8_t	u8ReturnType = STACK_NO_ERROR;
	uint16_t u16PacketIndex = 0;
	uint8_t u8WriteByteCount = ((u16NoOfWriteReg * 2) > 244)?244:(u16NoOfWriteReg * 2);
	uint16_t *pu16OutputVal = (uint16_t *)pu8OutputVal;
	stEndianess_t stEndianess = { 0 };
	stMbusPacketVariables_t *pstMBusRequesPacket = NULL;
	Post_Thread_Msg_t stPostThreadMsg = { 0 };
	struct timespec tsReqRcvd = (struct timespec){0};

	// Init req rcvd timestamp
	timespec_get(&tsReqRcvd, TIME_UTC);

	u8ReturnType = InputParameterVerification(u16ReadRegAddress,
			u16NoOfReadReg,
			u8UnitId,
			pFunCallBack,
			READ_HOLDING_REG,
			0);
		if(STACK_NO_ERROR != u8ReturnType)
	{
			return u8ReturnType;
	}
	pstMBusRequesPacket = emplaceNewRequest(tsReqRcvd);

	if(NULL == pstMBusRequesPacket)
	{
		return STACK_ERROR_MAX_REQ_SENT;
	}

	pstMBusRequesPacket->m_u16AppTxID = u16TransacID;

#ifdef MODBUS_STACK_TCPIP_ENABLED
		u16PacketIndex = CreateHeaderForModbusRequest(u16ReadRegAddress,
														u8UnitId,
														11+u8WriteByteCount,
														pstMBusRequesPacket->m_ulMyId,
														u8FunCode,
														pstMBusRequesPacket);
#else
	u16PacketIndex = CreateHeaderForModbusRequest(u16ReadRegAddress,
												u8UnitId,
												0,
												pstMBusRequesPacket->m_ulMyId,
												u8FunCode,
												pstMBusRequesPacket);
#endif

	stEndianess.u16word = u16NoOfReadReg;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8SecondByte;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8FirstByte;

	/// writing
	u8ReturnType = InputParameterVerification(u16WriteRegAddress,
			u16NoOfWriteReg,
			u8UnitId,
			pFunCallBack,
			READ_WRITE_MUL_REG,
			u8WriteByteCount);

	if(STACK_NO_ERROR != u8ReturnType)
	{
		freeReqNode(pstMBusRequesPacket);
		return u8ReturnType;
	}

	/// write starting address
	stEndianess.u16word = u16WriteRegAddress;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8SecondByte;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8FirstByte;

	/// Quantity to write
	stEndianess.u16word = u16NoOfWriteReg;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8SecondByte;
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
			stEndianess.stByteOrder.u8FirstByte;

	/// calculate byte count for writing
	u8WriteByteCount = u16NoOfWriteReg * MBUS_INDEX_2;

	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] = u8WriteByteCount;

	while(u16NoOfWriteReg > 0 && NULL != pu8OutputVal)
	{
		if(u16PacketIndex >= TCP_MODBUS_ADU_LENGTH)
		{
			freeReqNode(pstMBusRequesPacket);
			return STACK_ERROR_PACKET_LENGTH_EXCEEDED;
		}
		stEndianess.u16word = *pu16OutputVal;
		pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
					stEndianess.stByteOrder.u8SecondByte;
		if(u16PacketIndex >= TCP_MODBUS_ADU_LENGTH)
		{
			freeReqNode(pstMBusRequesPacket);
			return STACK_ERROR_PACKET_LENGTH_EXCEEDED;
		}
		pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] =
					stEndianess.stByteOrder.u8FirstByte;
		pu16OutputVal++;
		u16NoOfWriteReg--;
	}

	pstMBusRequesPacket->m_stMbusTxData.m_u16Length = (u16PacketIndex);

	pstMBusRequesPacket->pFunc = pFunCallBack;

	if(NULL == pu8OutputVal && u8WriteByteCount > 0)
		u8ReturnType = STACK_ERROR_INVALID_INPUT_PARAMETER;

#ifdef MODBUS_STACK_TCPIP_ENABLED

	pstMBusRequesPacket->m_u8IpAddr[0] = pu8SerIpAddr[0];
	pstMBusRequesPacket->m_u8IpAddr[1] = pu8SerIpAddr[1];
	pstMBusRequesPacket->m_u8IpAddr[2] = pu8SerIpAddr[2];
	pstMBusRequesPacket->m_u8IpAddr[3] = pu8SerIpAddr[3];
	pstMBusRequesPacket->u16Port = u16Port;

#else
	pstMBusRequesPacket->m_u8ReceivedDestination = u8UnitId;
#endif

	if(u16PacketIndex >= TCP_MODBUS_ADU_LENGTH)
	{
		freeReqNode(pstMBusRequesPacket);
		return STACK_ERROR_PACKET_LENGTH_EXCEEDED;
	}

	pstMBusRequesPacket->m_lPriority = lPriority;
	stPostThreadMsg.idThread = i32MsgQueIdSC;
	stPostThreadMsg.lParam = NULL;
	stPostThreadMsg.wParam = pstMBusRequesPacket;
	stPostThreadMsg.MsgType = lPriority;

	if(!OSAL_Post_Message(&stPostThreadMsg))
	{
		u8ReturnType = STACK_ERROR_QUEUE_SEND;
		freeReqNode(pstMBusRequesPacket);
	}

	return u8ReturnType;

}

/**
 *
 * Description
 * Read Device Identification API
 *
 * @param u8MEIType [in] MEI type
 * @param u8FunCode			[in] function code
 * @param u8ReadDevIdCode 	[in] read device id code
 * @param u8ObjectId 		[in] object id
 * @param u16TransacID 		[in] transaction ID
 * @param u8UnitId 			[in] unit id
 * @param pu8SerIpAddr 		[in] server IP address
 * @param u16Port			[in] uint16_t port number
 * @param lPriority			[in] long priority
 * @param pFunCallBack 		[in] callback function pointer
 *
 * @return uint8_t			[out] success or failure in integer format
 *
 */
MODBUS_STACK_EXPORT uint8_t Modbus_Read_Device_Identification(uint8_t u8MEIType,
		uint8_t u8FunCode,
		uint8_t u8ReadDevIdCode,
		uint8_t u8ObjectId,
		uint16_t u16TransacID,
		uint8_t u8UnitId,
		uint8_t *pu8SerIpAddr,
		uint16_t u16Port,
		long lPriority,
		void* pFunCallBack)
{
	uint8_t	u8ReturnType = STACK_NO_ERROR;
	uint16_t u16PacketIndex = 0;
	stMbusPacketVariables_t *pstMBusRequesPacket = NULL;
	Post_Thread_Msg_t stPostThreadMsg = { 0 };
	struct timespec tsReqRcvd = (struct timespec){0};

	// Init req rcvd timestamp
	timespec_get(&tsReqRcvd, TIME_UTC);

	if(NULL == pFunCallBack)
	{
		return STACK_ERROR_INVALID_INPUT_PARAMETER;
	}

	/// Maximum allowed slave 1- 247 or 255
	if((0 == u8UnitId) || ((u8UnitId >= 248) && (u8UnitId < 255)))
	/// Maximum allowed slave 1- 247
//	if(u8UnitId > MAX_ALLOWED_SLAVES )
	{
		return STACK_ERROR_INVALID_INPUT_PARAMETER;
	}

	/// Validate MEI type
	if(u8MEIType != MEI_TYPE)
	{
		return STACK_ERROR_INVALID_INPUT_PARAMETER;
	}

	/// Validate read device id code type
	if((u8ReadDevIdCode == 1) && (u8ReadDevIdCode == 2) && (u8ReadDevIdCode == 3)
			&& (u8ReadDevIdCode == 4))
	{
		return STACK_ERROR_INVALID_INPUT_PARAMETER;
	}

	pstMBusRequesPacket = emplaceNewRequest(tsReqRcvd);

	if(NULL == pstMBusRequesPacket)
	{
		return STACK_ERROR_MAX_REQ_SENT;
	}

	pstMBusRequesPacket->m_u16AppTxID = u16TransacID;

#ifdef MODBUS_STACK_TCPIP_ENABLED
	/// here 5 = len of trans ID + fun code + MEI + devId + ObjeId
	u16PacketIndex = CreateHeaderForDevIdentificationModbusRequest(u8UnitId,
														5,
														pstMBusRequesPacket->m_ulMyId,
														u8FunCode,
														pstMBusRequesPacket);
#else
	u16PacketIndex = CreateHeaderForDevIdentificationModbusRequest(u8UnitId,
														0,
														pstMBusRequesPacket->m_ulMyId,
														u8FunCode,
														pstMBusRequesPacket);

#endif

	/// Fill MEI Type
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] = u8MEIType;

	/// Fill Read Device ID  Code
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] = u8ReadDevIdCode;

	/// Fill Object ID
	pstMBusRequesPacket->m_stMbusTxData.m_au8DataFields[u16PacketIndex++] = u8ObjectId;

	pstMBusRequesPacket->m_stMbusTxData.m_u16Length = (u16PacketIndex);

	pstMBusRequesPacket->pFunc = pFunCallBack;

#ifdef MODBUS_STACK_TCPIP_ENABLED

	pstMBusRequesPacket->m_u8IpAddr[0] = pu8SerIpAddr[0];
	pstMBusRequesPacket->m_u8IpAddr[1] = pu8SerIpAddr[1];
	pstMBusRequesPacket->m_u8IpAddr[2] = pu8SerIpAddr[2];
	pstMBusRequesPacket->m_u8IpAddr[3] = pu8SerIpAddr[3];
	pstMBusRequesPacket->u16Port = u16Port;

#else
	pstMBusRequesPacket->m_u8ReceivedDestination = u8UnitId;
#endif

	if(u16PacketIndex >= TCP_MODBUS_ADU_LENGTH)
	{
		freeReqNode(pstMBusRequesPacket);
		return STACK_ERROR_PACKET_LENGTH_EXCEEDED;
	}

	pstMBusRequesPacket->m_lPriority = lPriority;
	stPostThreadMsg.idThread = i32MsgQueIdSC;
	stPostThreadMsg.lParam = NULL;
	stPostThreadMsg.wParam = pstMBusRequesPacket;
	stPostThreadMsg.MsgType = lPriority;

	if(!OSAL_Post_Message(&stPostThreadMsg))
	{
		u8ReturnType = STACK_ERROR_QUEUE_SEND;
		freeReqNode(pstMBusRequesPacket);
	}

	return u8ReturnType;
}
