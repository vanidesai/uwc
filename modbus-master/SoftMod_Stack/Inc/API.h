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

#ifndef API_H_
#define API_H_

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

#define MODBUS_STACK_EXPORT __attribute__ ((visibility ("default")))
#define MODBUS_STACK_IMPORT __attribute__ ((visibility ("default")))

typedef unsigned char   	uint8_t;    /* 1 byte  0 to 255 */
typedef signed char     	int8_t;     /* 1 byte -127 to 127 */
typedef unsigned short  	uint16_t;   /* 2 bytes 0 to 65535 */
typedef signed short    	int16_t;    /* 2 bytes -32767 to 32767 */
#ifdef _WIN64
typedef long long          	ilong32_t;  /* singed long long declarations */
typedef unsigned long long 	ulong32_t;  /* unsinged long long declarations */
#else
typedef long            	ilong32_t;  /* singed long declarations */
typedef unsigned long   	ulong32_t;  /* unsinged long declarations */
#endif


#define MODBUS_EXCEPTION 1
#define MODBUS_STACK_ERROR 2
#define MODBUS_DATA_LENGTH (260)

#define DEFAULT_RESPONSE_TIMEOUT_MS 80
#define DEFAULT_INTERFRAME_DELAY_MS 0

/**
 @enum MODBUS_ERROR_CODE
 @brief
    This enumerator defines MODBUS error codes
*/
typedef enum StackErrorCode
{
	STACK_NO_ERROR,
	STACK_TXNID_OR_UNITID_MISSMATCH,
	STACK_ERROR_SOCKET_FAILED,
	STACK_ERROR_CONNECT_FAILED,
	STACK_ERROR_SEND_FAILED,
	STACK_ERROR_RECV_FAILED,
	STACK_ERROR_RECV_TIMEOUT,
	STACK_ERROR_MALLOC_FAILED,
	STACK_ERROR_QUEUE_SEND,
	STACK_ERROR_QUEUE_RECIVE,
	STACK_ERROR_THREAD_CREATE,
	STACK_ERROR_INVALID_INPUT_PARAMETER,
	STACK_ERROR_PACKET_LENGTH_EXCEEDED,
	STACK_ERROR_SOCKET_LISTEN_FAILED,
	STACK_ERROR_MAX_REQ_SENT,
	STACK_ERROR_FAILED_Q_SENT_REQ,
	STACK_INIT_FAILED,
	STACK_ERROR_QUEUE_CREATE,
	STACK_ERROR_MUTEX_CREATE,
	STACK_ERROR_STACK_IS_NOT_INITIALIZED,
	STACK_ERROR_STACK_IS_ALREADY_INITIALIZED,
	STACK_ERROR_NULL_POINTER=21
}eStackErrorCode;

/**
 @enum MODBUS_FUNC_CODE
 @brief
    This enumerator defines MODBUS function codes
*/
typedef enum {
	MBUS_MIN_FUN_CODE = 0,
	READ_COIL_STATUS,			///01
	READ_INPUT_STATUS,			///02
	READ_HOLDING_REG,			///03
	READ_INPUT_REG,				///04
	WRITE_SINGLE_COIL,			///05
	WRITE_SINGLE_REG,			///06
	WRITE_MULTIPLE_COILS = 15,	///15
	WRITE_MULTIPLE_REG,			///16
	READ_FILE_RECORD = 20,		///20
	WRITE_FILE_RECORD,			///21
	READ_WRITE_MUL_REG = 23,	///23
	READ_DEVICE_IDENTIFICATION = 43,	///43
	MBUS_MAX_FUN_CODE
} eModbusFuncCode_enum;

typedef enum
{
	eNone,
	eOdd,
	eEven
}eParity;

/**
 @struct MbusReadFileRecord
 @brief
    This structure defines request of modbus read file record function code
*/
typedef struct MbusReadFileRecord
{
	uint8_t			m_u8RefType;
	uint16_t		m_u16FileNum;
	uint16_t		m_u16RecordNum;//m_u16StartingAddr;
	uint16_t		m_u16RecordLength;//m_u16NoOfRecords;
	struct MbusReadFileRecord *pstNextNode;
}stMbusReadFileRecord_t;

/**
 @struct SubReq
 @brief
    This structure defines sub request of modbus read file record function code
*/
typedef struct SubReq
{
	uint8_t			m_u8FileRespLen;
	uint8_t			m_u8RefType;
	uint16_t		*m_pu16RecData;
	struct SubReq   *pstNextNode;
}stRdFileSubReq_t;

/**
 @struct MbusRdFileRecdResp
 @brief
    This structure defines response of modbus read file record function code
*/
typedef struct MbusRdFileRecdResp
{
	uint8_t				m_u8RespDataLen;
	stRdFileSubReq_t	m_stSubReq;
}stMbusRdFileRecdResp_t;

/**
 @struct WrFileSubReq
 @brief
    This structure defines request of modbus write file record function code
*/
typedef struct WrFileSubReq
{
	uint8_t			m_u8RefType;
	uint16_t		m_u16FileNum;
	uint16_t		m_u16RecNum;
	uint16_t		m_u16RecLen;
	uint16_t		*m_pu16RecData;
	struct WrFileSubReq   *pstNextNode;
}stWrFileSubReq_t;

/**
 @struct MbusWrFileRecdResp
 @brief
    This structure defines response of modbus write file record function code
*/
typedef struct MbusWrFileRecdResp
{
	uint8_t				m_u8RespDataLen;
	stWrFileSubReq_t	m_stSubReq;
}stMbusWrFileRecdResp_t;

/**
 @struct Exception
 @brief
    This structure defines exception code
*/
typedef struct Exception
{
	uint8_t		m_u8ExcStatus;
	uint8_t		m_u8ExcCode;
}stException_t;

/**
 @struct SubObjList
 @brief
    This structure defines sub request of read device identification function code
*/
typedef struct SubObjList
{
	unsigned char 	m_u8ObjectID;
	unsigned char 	m_u8ObjectLen;
	unsigned char	*m_u8ObjectValue;
	struct SubObjList   *pstNextNode;
}SubObjList_t;

/**
 @struct RdDevIdReq
 @brief
    This structure defines request of read device identification function code
*/
typedef struct RdDevIdReq
{
#ifdef MODBUS_STACK_TCPIP_ENABLED
	unsigned char	m_u8IpAddr[4];
#else
	unsigned char	m_u8DestAddr;
#endif
	unsigned char 	m_u8UnitId;
	unsigned char 	m_u8MEIType;
	unsigned char 	m_u8RdDevIDCode;
	unsigned char 	m_u8ObjectID;
	unsigned char 	m_u8FunCode;
}stRdDevIdReq_t;

/**
 @struct RdDevIdResp
 @brief
    This structure defines response of read device identification function code
*/
typedef struct RdDevIdResp
{
	unsigned char 	m_u8MEIType;
	unsigned char 	m_u8RdDevIDCode;
	unsigned char 	m_u8ConformityLevel;
	unsigned char 	m_u8MoreFollows;
	unsigned char 	m_u8NextObjId;
	unsigned char 	m_u8NumberofObjects;
	SubObjList_t 	m_pstSubObjList;
}stRdDevIdResp_t;

typedef struct DevConfig
{
	long 			m_lInterframedelay;
	long 			m_lResponseTimeout;
}stDevConfig_t;

typedef struct TimeStamps
{
	struct timespec tsReqRcvd;
	struct timespec tsReqSent;
	struct timespec tsRespRcvd;
	struct timespec tsRespSent;
}stTimeStamps;

/// Modbus master stack initialization function
MODBUS_STACK_EXPORT uint8_t AppMbusMaster_StackInit();

/// Modbus master stack de-initialization function
MODBUS_STACK_EXPORT void AppMbusMaster_StackDeInit(void);

/// Modbus master stack configuration function
MODBUS_STACK_EXPORT uint8_t AppMbusMaster_SetStackConfigParam(stDevConfig_t *pstDevConf);

/// Modbus master stack configuration function
MODBUS_STACK_EXPORT stDevConfig_t* AppMbusMaster_GetStackConfigParam();

/// Read coil API
MODBUS_STACK_EXPORT uint8_t Modbus_Read_Coils(uint16_t u16StartCoil,
											  uint16_t u16NumOfcoils,
											  uint16_t u16TransacID,
											  uint8_t u8UnitId,
											  uint8_t *pu8SerIpAddr,
											  uint16_t u16Port,
											  long lPriority,
											  void* pFunCallBack);

/// Read discrete input API
MODBUS_STACK_EXPORT uint8_t Modbus_Read_Discrete_Inputs(uint16_t u16StartDI,
														uint16_t u16NumOfDI,
														uint16_t u16TransacID,
														uint8_t u8UnitId,
														uint8_t *pu8SerIpAddr,
														uint16_t u16Port,
														long lPriority,
														void* pFunCallBack);

/// Read holding register API
MODBUS_STACK_EXPORT uint8_t Modbus_Read_Holding_Registers(uint16_t u16StartReg,
														  uint16_t u16NumberOfRegisters,
														  uint16_t u16TransacID,
														  uint8_t u8UnitId,
														  uint8_t *pu8SerIpAddr,
														  uint16_t u16Port,
														  long lPriority,
														  void* pFunCallBack);

/// Read input register API
MODBUS_STACK_EXPORT uint8_t Modbus_Read_Input_Registers(uint16_t u16StartReg,
													    uint16_t u16NumberOfRegisters,
														uint16_t u16TransacID,
														uint8_t u8UnitId,
														uint8_t *pu8SerIpAddr,
														uint16_t u16Port,
														long lPriority,
														void* pFunCallBack);

/// write single coil API
MODBUS_STACK_EXPORT uint8_t Modbus_Write_Single_Coil(uint16_t u16StartCoil,
													 uint16_t u16OutputVal,
													 uint16_t u16TransacID,
													 uint8_t u8UnitId,
													 uint8_t *pu8SerIpAddr,
													 uint16_t u16Port,
													 long lPriority,
													 void* pFunCallBack);

/// write single register API
MODBUS_STACK_EXPORT uint8_t Modbus_Write_Single_Register(uint16_t u16StartReg,
														 uint16_t u16RegOutputVal,
														 uint16_t u16TransacID,
														 uint8_t u8UnitId,
														 uint8_t *pu8SerIpAddr,
														 uint16_t u16Port,
														 long lPriority,
														 void* pFunCallBack);

/// write multiple coils API
MODBUS_STACK_EXPORT uint8_t Modbus_Write_Multiple_Coils(uint16_t u16Startcoil,
													   uint16_t u16NumOfCoil,
													   uint16_t u16TransacID,
													   uint8_t  *pu8OutputVal,
													   uint8_t  u8UnitId,
													   uint8_t  *pu8SerIpAddr,
													   uint16_t u16Port,
													   long lPriority,
													   void*    pFunCallBack);

/// write multiple registers API
MODBUS_STACK_EXPORT uint8_t Modbus_Write_Multiple_Register(uint16_t u16StartReg,
									   	   	   	   	   	   uint16_t u16NumOfReg,
														   uint16_t u16TransacID,
														   uint8_t  *pu8OutputVal,
														   uint8_t  u8UnitId,
														   uint8_t  *pu8SerIpAddr,
														   uint16_t u16Port,
														   long lPriority,
														   void*    pFunCallBack);

/// read file record API
MODBUS_STACK_EXPORT uint8_t Modbus_Read_File_Record(uint8_t u8byteCount,
													uint8_t u8FunCode,
													stMbusReadFileRecord_t *pstFileRecord,
													uint16_t u16TransacID,
													uint8_t u8UnitId,
													uint8_t *pu8SerIpAddr,
													uint16_t u16Port,
													long lPriority,
													void* pFunCallBack);

/// write file record API
MODBUS_STACK_EXPORT uint8_t Modbus_Write_File_Record(uint8_t u8ReqDataLen,
		 	 	 	 	 	 	 	 	 	 	 	uint8_t u8FunCode,
													stWrFileSubReq_t *pstFileRecord,
													uint16_t u16TransacID,
													uint8_t u8UnitId,
													uint8_t *pu8SerIpAddr,
													uint16_t u16Port,
													long lPriority,
													void* pFunCallBack);

/// read write multiple registers API
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
									void* pFunCallBack);

/// Read device identification
MODBUS_STACK_EXPORT uint8_t Modbus_Read_Device_Identification(uint8_t u8MEIType,
		uint8_t u8FunCode,
		uint8_t u8ReadDevIdCode,
		uint8_t u8ObjectId,
		uint16_t u16TransacID,
		uint8_t u8UnitId,
		uint8_t *pu8SerIpAddr,
		uint16_t u16Port,
		long lPriority,
		void* pFunCallBack);

//struct for Modbus_AppllicationCallbackHandler
typedef struct _stMbusAppCallbackParams
{
	/** Holds the received transaction ID*/
	uint16_t m_u16TransactionID;
	/** Holds the unit id  */
	uint8_t m_u8FunctionCode;
	/** Holds Data received from server */
	//MbusRXData_t m_stMbusRxData;
	uint8_t m_u8MbusRXDataLength;
#ifdef MODBUS_STACK_TCPIP_ENABLED
	/** Holds the unit id  */
	uint8_t  m_u8UnitID;
	/** Holds Ip address of salve/server device */
	uint8_t m_u8IpAddr[4];
	uint16_t u16Port;
#else
	/** Received destination address */
	uint8_t	m_u8ReceivedDestination;
#endif
	uint8_t m_au8MbusRXDataDataFields[ MODBUS_DATA_LENGTH ];

	/** Holds the start address  */
	uint16_t  m_u16StartAdd;
	/** Holds the Quantity  */
	uint16_t  m_u16Quantity;
	/** Holds the Msg Priority  */
	long m_lPriority;
	stTimeStamps m_objTimeStamps;
	/** exception if any from Modbus **/
	//stException_t  *m_stException;

	uint8_t		m_u8ExceptionExcStatus;
	uint8_t		m_u8ExceptionExcCode;


}stMbusAppCallbackParams_t;
//end of Modbus_ApplicationCallbackHandler struct


#ifndef MODBUS_STACK_TCPIP_ENABLED
MODBUS_STACK_EXPORT int initSerialPort(uint8_t *portName, uint32_t baudrate, eParity  parity);
#endif


#endif /* API_H_ */
