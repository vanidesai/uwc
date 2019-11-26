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

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#ifdef __cplusplus
extern "C" {
#include "API.h"
}
#endif
#include <map>

#ifdef __linux

typedef int             BOOL;
typedef bool            BOOLEAN;
typedef unsigned long   DWORD;
typedef unsigned long   ULONG;
typedef void            VOID;

#define TRUE true
#define FALSE false
#define SOCKET_ERROR -1

#endif

//#include "APIHandlerDef.hpp"

#define NO_OF_ARGUMENTS	2

#define REST_API_VERSION "/v1"

#define API_MODBUS_SPECIFIC 					"/api/modbus/readWriteCommon"
#define API_MODBUS_READ_WRITE_MUL_REG			"/api/modbus/readWriteMultipleRegister"
#define API_MODBUS_HELP 						"/api/modbus/help"
#define API_MODBUS_READ_FILE_REC 				"/api/modbus/readFileRecord"
#define API_MODBUS_WRITE_FILE_REC 				"/api/modbus/writeFileRecord"
#define API_MODBUS_READ_PERIODIC 				"/api/modbus/readPeriodic"
#define API_MODBUS_BUILD_VERSION 				"/api/modbus/buildVersion"
#define API_MODBUS_READ_DEVICE_IDENTIFICATION 	"/api/modbus/readDeviceIdentification"
#define API_MODBUS_CONFIG 						"/api/modbus/configuration"
#define API_MODBUS_GET_CONFIG 					"/api/modbus/getConfiguration"
#define API_MODBUS_AUTHETICATE_USER 			"/api/modbus/authenticateUser"

#define MODBUS_OPERATION_READ		(1)
#define MODBUS_OPERATION_WRITE		(2)

#define MODBUS_FRAME_TYPE_TAG		(1)
#define MODBUS_FRAME_TYPE_VALUE		(2)

#define	MODBUS_DISCREATE_OUTPUT		(1)
#define MODBUS_DISCREATE_INPUT 		(2)
#define MODBUS_INPUT_REGISTER 		(3)
#define MODBUS_HOLDING_REGISTER 	(4)
#define MIN_COILS					(1)
#define MAX_COILS					(2000)
#define MIN_REGISTER				(1)
#define MAX_REGISTER				(125)
#define MIN_MULTI_COIL				(1)
#define MAX_MULTI_COIL				(1968)

#define DEVICE_TAG_IS_STRING
#define MAX_DEVICE_ID 				(247)
#define MIN_DEVICE_ID 				(0)
#define MODBUS_SINGLE_REGISTER_LENGTH (2)

#define MAX_READ_FILE_RECORD 		(124)
#define MIN_READ_FILE_RECORD 		(1)

#define MAX_READ_FILE_RECORD_NUMBER (9999)
#define MIN_READ_FILE_RECORD_NUMBER (0)

#define  RFR_SUBREQ_BYTECOUNT 		(7)

#define  RFR_MIN_BYTECOUNT 			(7)
#define  RFR_MAX_BYTECOUNT 			(245)
#define  FR_REFERENCE_TYPE 			(6)
#define  FR_MIN_FILE_NUMBER 		(1)
#define  FR_MAX_FILE_NUMBER 		(65535)
#define  WFR_MIN_REQ_DATA_LEN 		(9)
#define  WFR_MAX_REQ_DATA_LEN 		(251)
#define  MAX_WRITE_FILE_RECORD 		(9999)
#define  MIN_WRITE_FILE_RECORD 		(0)

#define  MIN_UNIT_ID 				(0)
#define  MAX_UNIT_ID 				(247)
#define  TCP_MODBUS_UNIT_ID 		(255)
#define  IP_CHAR_ARRAY_SIZE 		(20)
#define MAX_CHARACTER_STRING_BYTES  (255)

#define  CONTENT_TYPE_STRING 		"text/plain"

#define  MAX_UINT16_RANG 	(65535)
#define	 MAX_UINT8_RANG 	(255)
#define  ZERO 				(0)
#define	 INDEX_0 			(0)
#define  INDEX_1 			(1)
#define	 INDEX_2 			(2)
#define  INDEX_3 			(3)
//#define MEMSET_TO_ZERO 0x00

using namespace std;

// enums for Read device ID code
typedef enum Read_Dev_Id_Code
{
	BASIC_DEVICE_ID = 1,
	REGULAR_DEVICE_ID,
	EXTENDED_DEVICE_ID,
	SPECIFIC_IDENTIFICATION_OBJECT
}Read_Dev_Id_Code_t;

// This structure defines Audit log messages parameters for modbus write operations
typedef struct AuditLogMsg
{
	unsigned char  m_u8FunCode;
	string  m_ipAddr;
	string 	m_strRemoteClientIP;
}AuditLogMsg_t;

// This structure defines request data parameters for modbus
typedef struct ReqData
{
	unsigned char  m_u8DataLen;
	unsigned char  *m_pu8Data;
}ReqData_t;

// This structure defines request data parameters for modbus
typedef struct RespData
{
	unsigned char m_u8DataLen;
	unsigned char  *m_pu8Data;
	stException_t  m_stExcStatus;
}RespData_t;

// This structure defines sub tag parameters for modbus request
typedef struct SubTagPart
{
	unsigned char m_u8NodeId;
	unsigned char  m_u8FunCode;
	unsigned char  m_u8Operation;
	unsigned short m_u16StartAddr;
	unsigned short m_u16EndAddr;
	unsigned short m_u16Quantity;
	ReqData_t 	   m_stReqData;
	RespData_t	   m_stRespData;
	struct SubTagPart  *m_pstNextSubTag;
}SubTagPart_t;

// This structure defines device tag parameters for modbus request
typedef struct VariablePart
{
	unsigned char m_u8IpAddr[4];
	unsigned char m_u8DevId;
	unsigned char m_u8SubTagCnt;
	unsigned char m_u8MbusTable;
	SubTagPart_t *m_pstSubTagPart;
	struct VariablePart *m_pstNextTag;
}VariablePart_t;

// This structure defines generic request parameters for modbus
typedef struct RestMbusReqGeneric
{
	unsigned short m_u16ReffId;
	unsigned char m_u8TagCount;
	unsigned char m_u8IpAddr[4];
	unsigned char m_u8DevId;
	unsigned char m_u8NodeId;
	unsigned char  m_u8FunCode;
	unsigned char  m_u8Operation;
	unsigned short m_u16StartAddr;
	unsigned short m_u16EndAddr;
	unsigned short m_u16Quantity;
	ReqData_t 	   m_stReqData;
	RespData_t	   m_stRespData;
}RestMbusReqGeneric_t;

// This structure defines generic response parameters for modbus
typedef struct MbusRespGeneric
{
	unsigned short m_u16ReffId;
//	unsigned char m_u8FrameType;
	unsigned char m_u8Count;
	struct RspVariPart
	{
		unsigned char m_u8NodeId;
		struct EncodeJsonPart
		{
			unsigned char  m_u8ReadWrite;
		}m_stEncJsonPart;
		unsigned char m_u8RespType; /// 0 - Valid Response, 1 - Mbus Exception, 2 - Stack Error
		unsigned short m_u16DataLen;
		unsigned char  *m_pu8Data;
		struct VariablePart *m_pstNextData;
	}m_stRspVariPart;
}MbusRespGeneric_t;

// This structure defines generic parameters for modbus common API
typedef struct MbusAPI
{
	unsigned char	m_u8DevId;
	unsigned char	m_u8IpAddr[4];
	unsigned short  m_u16TxId;
	unsigned short  m_u16Quantity;
	unsigned short  m_u16StartAddr;
	unsigned short  m_u16ByteCount;
	unsigned char  *m_pu8Data;
}MbusAPI_t;

// This structure defines request parameters for read file record
typedef struct RestReadFileRecord
{
	unsigned char 	m_u8FrameReqType;
	uint8_t 		m_u8FunCode;
	uint8_t			m_u8ByteCount;
	uint8_t 		m_u8UnitId;
	uint8_t			m_u8IpAddr[4];
	stMbusReadFileRecord_t SubRequest;
}stRestReadFileRecord_t;

// This structure defines request parameters for write file record
typedef struct RestWriteFileRecord
{
	unsigned char 	m_u8FrameReqType;
	uint8_t 		m_u8FunCode;
	uint8_t			m_u8ReqDataLen;
	uint8_t 		m_u8UnitId;
	uint8_t			m_u8IpAddr[4];
	unsigned char 	m_u8RemoteClientIP[4];
	stWrFileSubReq_t SubRequest;
}RestWriteFileRecord_t;

// This structure defines response parameters for read file record
typedef struct RestReadFileRecordResp
{
	stException_t m_stException;
	unsigned char  m_u8FunCode;
	stMbusRdFileRecdResp_t m_stMbusRdFileRecdResp;
}RestReadFileRecordResp_t;

// This structure defines response parameters for write file record
typedef struct RestWriteFileRecordResp
{
	stException_t m_stException;
	unsigned char  m_u8FunCode;
	stMbusWrFileRecdResp_t m_stMbusWrFileRecdResp;
}RestWriteFileRecordResp_t;

typedef enum HaystackDataType
{
	MBUS_DT_BINARY,
	MBUS_DT_UNSIGNED_INT,
	MBUS_DT_ENUM,
	MBUS_DT_FLOAT,
	MBUS_DT_SIGNED_SHORT,
	MBUS_DT_UNSIGNED_SHORT,
	MBUS_DT_SIGNED_INT,
	MBUS_DT_DOUBLE,
	MBUS_DT_STRING,
	MBUS_DT_SINGED_LONG,
	MBUS_DT_UNSIGNED_LONG,
	MBUS_DT_INVALID
}HaystackDataType_t;

/// This structure defines haystack request info,.
typedef struct HaystackInfo
{
	string	m_stHaystackId;
	string	m_stKind;
	HaystackDataType_t m_stDataType;
	string	m_stEnum;
	string m_sMqttTopic;
}HaystackInfo_t;
// This structure defines request parameters for read periodic
typedef struct RestRdPeriodicTagPart
{
    bool  IsSubscription;
    bool  bIsRespAwaited;
    unsigned char m_u8FrameReqType;
    unsigned char 	m_u8IpAddr[4];
    unsigned char 	m_u8UnitId;
//    unsigned char 	m_u8MbusTable;
    unsigned char  	m_u8FunCode;
    unsigned char  *m_pu8RespData;
    unsigned short 	m_u16StartAddr;
    unsigned short 	m_u16EndAddr;
    unsigned short 	m_u16Quantity;
    unsigned short 	m_u16ReqDataLen;
    unsigned short 	m_u16RespDataLen;
    unsigned short  m_u16TxId;
    unsigned int   	m_u32Interval;
    unsigned int	m_u32LocInterval;
	unsigned char 	m_stMQTT_Topic[200];
	HaystackInfo_t	m_stHaystackInfo;
    stException_t m_stException;
}RestRdPeriodicTagPart_t;

// This structure defines request parameters for read write multiple registers
typedef struct RestRdWrMulRegReq
{
	unsigned char	m_u8IpAddr[4];
	unsigned short 	m_u16RdStartAddr;
	unsigned short 	m_u16RdQuantity;
	unsigned short 	m_u16WrStartAddr;
	unsigned short 	m_u16WrQuantity;
	unsigned char	m_u8ByteCount;
	unsigned char	m_u8FunCode;
	unsigned char 	m_u8UnitId;
	unsigned char  *m_pu8ReqData;
	unsigned char 	m_u8RemoteClientIP[4];
	RespData_t	   	m_stRespData;
}RestRdWrMulRegReq_t;

// Function to get function code
unsigned char GetFunctionCode(uint8_t u8ReadWrite,
								  uint16_t u16Quantity,
								  uint8_t u8MbudTable);

// This structure defines device configuration parameters
typedef struct DeviceConfig
{
	string 		m_sthttpIP;
	uint32_t 	m_u32httpPortNo;
	const char* m_pPlatformBusURL;
	string 		m_stPLBusRootCACert;
	string 		m_stPLBusServiceCert;
	string 		m_stPLBusServiceKey;
	string 		m_stBMPUserName;
}DeviceConfig_t;

// This structure defines request parameters for read device identification
typedef struct RestRdDevIdInfo
{
	 stException_t m_stException;
	stRdDevIdReq_t RestRdDevIdRequest;
	stRdDevIdResp_t RestRdDevIdResponse;
}RestRdDevIdInfo_t;

// This enum defines environment variables
typedef enum MbusParseEnvVar
{
	MBUSPARSE_HTTP_PORT,
	MBUSPARSE_PLATFROMBUS_URL,
	MBUSPARSE_CLIENT_CET_ROOT_CA,
	MBUSPARSE_CLIENT_SERVICE_CRT,
	MBUSPARSE_CLIENT_SERVICE_KEY,
	MBUSPARSE_HAYSTACK_USER_NAME
}MbusParEnvVar_t;

// This enumerator defines MODBUS error codes

typedef enum MbusStackErrorCode
{
	MBUS_STACK_NO_ERROR,
	MBUS_STACK_TXNID_OR_UNITID_MISSMATCH,
	MBUS_STACK_ERROR_SOCKET_FAILED,
	MBUS_STACK_ERROR_CONNECT_FAILED,
	MBUS_STACK_ERROR_SEND_FAILED,
	MBUS_STACK_ERROR_RECV_FAILED,
	MBUS_STACK_ERROR_RECV_TIMEOUT,
	MBUS_STACK_ERROR_MALLOC_FAILED,
	MBUS_STACK_ERROR_QUEUE_SEND,
	MBUS_STACK_ERROR_QUEUE_RECIVE,
	MBUS_STACK_ERROR_THREAD_CREATE,
	MBUS_STACK_ERROR_INVALID_INPUT_PARAMETER,
	MBUS_STACK_ERROR_PACKET_LENGTH_EXCEEDED,
	MBUS_JSON_APP_ERROR_MALLOC_FAILED,
	MBUS_JSON_APP_ERROR_INVALID_INPUT_PARAMETER,
	MBUS_JSON_APP_ERROR_PACKET_LENGTH_EXCEEDED,
	MBUS_JSON_APP_ERROR_NULL_POINTER,
	MBUS_JSON_APP_ERROR_INVALID_FUN_CODE,
	MBUS_JSON_APP_ERROR_EXCEPTION_RISE,
	MBUS_APP_TAG_OR_SUBTAG_NOT_FOUND,
	MBUS_APP_SUBSCRIPTION_NOT_FOUND,
	MBUS_PERIODIC_FAILED_TO_FIND_TOPIC,
	MBUS_PERIODIC_INTERVAL_OUT_OF_RANG,
	MBUS_INVALID_ENV_VARIABLE,
	MBUS_APP_ERROR_UNKNOWN_SERVICE_REQUEST,
	MBUS_APP_ERROR_IMPROPER_METHOD,
	MBUS_APP_ERROR_CONTENT_TYPE_NOT_JSON,
	MBUS_APP_ERROR_AUTHENTICATION_FAILED,
}eMbusStackErrorCode;

// This enum defines decode states for read write multiple registers
typedef enum MbusAPIRdWrMulRegDecodeState
{
	MBUS_API_RDWR_MULREG_INIT_STATE,
	MBUS_API_RDWR_MULREG_IP_DECODE_STATE,
	MBUS_API_RDWR_MULREG_UNITID_DECODE_STATE,
	MBUS_API_RDWR_MULREG_FUNCTIONCODE_DECODE_STATE,
	MBUS_API_RDWR_MULREG_READSTART_ADDR_DECODE_STATE,
	MBUS_API_RDWR_MULREG_READQUANTITY_DECODE_STATE,
	MBUS_API_RDWR_MULREG_WRITESTART_ADDR_DECODE_STATE,
	MBUS_API_RDWR_MULREG_WRITEQUANTITY_DECODE_STATE,
	MBUS_API_RDWR_MULREG_BYTECOUNT_DECODE_STATE,
	MBUS_API_RDWR_MULREG_DATA_DECODE_STATE
}MbusAPIRdWrMulRegDecodeState_t;

// This enum defines decode states for read dev identification
typedef enum MbusAPIRdDevIDDecodeState
{
	MBUS_API_RD_DEV_ID_INIT_STATE,
	MBUS_API_RD_DEV_ID_IP_DECODE_STATE,
	MBUS_API_RD_DEV_ID_UNITID_DECODE_STATE,
	MBUS_API_RD_DEV_ID_MEITYPE_DECODE_STATE,
	MBUS_API_RD_DEV_ID_READDEVIDCODE_DECODE_STATE,
	MBUS_API_RD_DEV_ID_OBJECTID_DECODE_STATE,
	MBUS_API_RD_DEV_ID_FUNCTIONCODE_DECODE_STATE
}MbusAPIRdDevIDDecodeState_t;

// This enum defines decode states for read periodic
typedef enum MbusAPIReadPeriodicDecodeState
{
	MBUS_API_RD_PERIODIC_INIT_STATE,
	MBUS_API_RD_PERIODIC_IP_DEVTAG_DECODE_STATE,
	MBUS_API_RD_PERIODIC_UNIT_ID_DECODE_STATE,
	MBUS_API_RD_PERIODIC_STARTADDR_DECODE_STATE,
	MBUS_API_RD_PERIODIC_QUANTITY_DECODE_STATE,
	MBUS_API_RD_PERIODIC_DATALEN_DECODE_STATE,
	MBUS_API_RD_PERIODIC_INTERVAL_DECODE_STATE,
	MBUS_API_RD_PERIODIC_ISSUB_DECODE_STATE,
	MBUS_API_RD_PERIODIC_FUNCODE_DECODE_STATE,
}MbusAPIReadPeriodicDecodeState_t;

// This enum defines decode states for read write common API
typedef enum MbusSpecificAPIDecodeState
{
	MBUS_SPECIFIC_API_INIT_STATE,
	MBUS_SPECIFIC_API_IP_DECODE_STATE,
	MBUS_SPECIFIC_API_STARTADDR_DECODE_STATE,
	MBUS_SPECIFIC_API_BYTECOUNT_DECODE_STATE,
	MBUS_SPECIFIC_API_QUANTITY_DECODE_STATE,
	MBUS_SPECIFIC_API_FUNCODE_DECODE_STATE,
	MBUS_SPECIFIC_API_DATA_DECODE_STATE,
	MBUS_SPECIFIC_API_UNIT_ID_DECODE_STATE,
}MbusSpecificAPIDecodeState_t;

/**
 *
 * DESCRIPTION
 * This enum defines decode states for write file record
 *
 */
typedef enum MbusWrFileRecAPIReqDecodeState
{
	MBUS_WRFILE_REC_API_INIT_STATE,
	MBUS_WRFILE_REC_API_REQDATALEN_DECODE_STATE,
	MBUS_WRFILE_REC_API_FUNCTIONCODE_DECODE_STATE,
	MBUS_WRFILE_REC_API_UNIT_ID_DECODE_STATE,
	MBUS_WRFILE_REC_API_IP_DECODE_STATE,
	MBUS_WRFILE_REC_API_SUBREQUEST_DECODE_STATE
}MbusWrFileRecAPIReqDecodeState_t;

/**
 *
 * DESCRIPTION
 * This enum defines decode states for sub request of write file record
 *
 */
typedef enum MbusWrFileRecAPISubReqDecodeState
{
	MBUS_WRFILE_REC_API_SUBREQ_INIT_STATE,
	MBUS_WRFILE_REC_API_SUBREQ_REFTYPE_DECODE_STATE,
	MBUS_WRFILE_REC_API_SUBREQ_FILENUM_DECODE_STATE,
	MBUS_WRFILE_REC_API_SUBREQ_RECNUM_DECODE_STATE,
	MBUS_WRFILE_REC_API_SUBREQ_RECLEN_DECODE_STATE,
	MBUS_WRFILE_REC_API_SUBREQ_RECDATA_DECODE_STATE,
}MbusWrFileRecAPISubReqDecodeState_t;

/**
 *
 * DESCRIPTION
 * This enum defines decode states for read file record
 *
 */
typedef enum MbusRdFileRecAPIReqDecodeState
{
	MBUS_RDFILE_REC_API_INIT_STATE,
	MBUS_RDFILE_REC_API_BYTECOUNT_DECODE_STATE,
	MBUS_RDFILE_REC_API_FUNCTIONCODE_DECODE_STATE,
	MBUS_RDFILE_REC_API_IP_DECODE_STATE,
	MBUS_RDFILE_REC_API_UNIT_ID_DECODE_STATE,
	MBUS_RDFILE_REC_API_SUBREQUEST_DECODE_STATE
}MbusRdFileRecAPIReqDecodeState_t;

/**
 *
 * DESCRIPTION
 * This enum defines decode states for sub request of read file record
 *
 */
typedef enum MbusRdFileRecAPISubReqDecodeState
{
	MBUS_RDFILE_REC_API_SUBREQ_INIT_STATE,
	MBUS_RDFILE_REC_API_SUBREQ_REFTYPE_DECODE_STATE,
	MBUS_RDFILE_REC_API_SUBREQ_FILENUM_DECODE_STATE,
	MBUS_RDFILE_REC_API_SUBREQ_RECNUM_DECODE_STATE,
	MBUS_RDFILE_REC_API_SUBREQ_RECLEN_DECODE_STATE,
}MbusRdFileRecAPISubReqDecodeState_t;

typedef enum MbusAPIStackConfigDecodeState
{
	MBUS_API_STACK_CONFIG_INIT_STATE,
	MBUS_API_STACK_CONFIG_SESSION_TIMEOUT_DECODE_STATE,
	MBUS_API_STACK_CONFIG_CONNECTION_TIMEOUT_DECODE_STATE
}MbusAPIStackConfigDecodeState_t;

/// function to call Modbus stack APIs
uint8_t Modbus_Stack_API_Call(unsigned char u8FunCode,
								MbusAPI_t *pstMbusApiPram,void* vpCallBackFun);

/// API to verify unsigned int 16 range
uint8_t VerifyUint16Range(uint32_t u32Value);

/// API to verify unsigned int 8 range
uint8_t VerifyUint8Range(uint32_t u32Value);

/// This function is for application callback for common read write coils and registers function codes.
void ModbusMaster_AppCallback(uint8_t  u8UnitID,
		 	 	 	 	 	 uint16_t u16TransacID,
							 uint8_t* pu8IpAddr,
							 uint8_t  u8FunCode,
							 stException_t  *pstException,
							 uint8_t  u8numBytes,
							 uint8_t* pu8data,
							 uint16_t  u16StartAdd,
							 uint16_t  u16Quantity);

/// This function is for application callback for read file record function codes.
void AppReadFileRecord_CallbackFunction(uint8_t  u8UnitID,
		 	 	 	 	 	 	 	 	uint8_t* pu8IpAddr,
										uint16_t u16TransacID,
										uint8_t  u8FunCode,
										stException_t  *pstException,
										stMbusRdFileRecdResp_t* pstRdFileRecdResp);

/// This function is for application callback for write file record function codes.
void AppWriteFileRecord_CallbackFunction(uint8_t  u8UnitID,
		uint8_t* pu8IpAddr,
		uint16_t u16TransacID,
		uint8_t  u8FunCode,
		stException_t  *pstException,
		stMbusWrFileRecdResp_t* pstMbusWrFileRecdResp);

/// This function is for application callback for read write multiple registers
void ModbusMasterRdWrMulReg_AppCallback(uint8_t  u8UnitID,
		 	 	 	 	 	 uint16_t u16TransacID,
							 uint8_t* pu8IpAddr,
							 uint8_t  u8FunCode,
							 stException_t  *pstException,
							 uint8_t  u8numBytes,
							 uint8_t* pu8data);

/// This function is for application callback for read device identification
void ModbusMasterRdDevIdentification_AppCallback(uint8_t  u8UnitID,
							 uint8_t* pu8IpAddr,
							 uint16_t u16TransacID,
							 uint8_t  u8FunCode,
							 stException_t  *pstException,
							 stRdDevIdResp_t* pstRdDevIdResp);

class boostbeats_ModbusInterface
{
	public :
//		void *pstModbusRxPacket;
//		void *vpThisObj;

		/// function to process or call stack APIs
		uint8_t MbusApp_Process_Request(RestMbusReqGeneric_t *pstMbusReqGen);

		/// function to process ReadFileRecord request
		uint8_t MbusApp_ReadFileRecord_Request(stRestReadFileRecord_t *pstModbusRxPacket,uint16_t u16TransacID);

		/// function to process WriteFileRecord request
		uint8_t MbusApp_WriteFileRecord_Request(RestWriteFileRecord_t *pstModbusRxPacket,uint16_t u16TransacID);

		/// function to process ReadWriteMultipleRegister request
		uint8_t MbusApp_ReadWriteMulReg_Request(RestRdWrMulRegReq_t *pstModbusRxPacket,uint16_t u16TransacID);

		/// function to process ReadDevIdentification request
		uint8_t MbusApp_ReadDevIdentification_Request(RestRdDevIdInfo_t *pstModbusRxPacket,uint16_t u16TransacID);

		/// function to process stack interface
		uint8_t MdodbusStackInterafec(RestMbusReqGeneric_t *pstMbusReqGen);

		/// This function is used to validate input parameters in generic request.
		uint8_t GenericRequestInputDataVerifiaction(SubTagPart_t * pstSubTagPart);

//		boostbeats_ModbusInterface(void *pstMbusRxPac)
//		{
//			pstModbusRxPacket = pstMbusRxPac;
//			vpThisObj = this;
//		}
		boostbeats_ModbusInterface()
		{

		}

};


#endif /* INCLUDE_INC_COMMON_HPP_ */
