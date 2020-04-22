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

#ifndef STACKCONFIG_H_
#define STACKCONFIG_H_

#include "osalLinux.h"
#include "semaphore.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h> // for epoll_create1(), epoll_ctl(), struct epoll_event

//#define MODBUS_STACK_TCPIP_ENABLED
#define MAX_REQUESTS 5000

#define THREAD_PRIORITY 30
#define THREAD_SCHEDULER SCHED_RR

#define handle_error_en(en, msg) do { errno = en; perror(msg); } while (0)

#ifdef MODBUS_STACK_TCPIP_ENABLED
	#define TCP_MODBUS_ADU_LENGTH 260
	#define MODBUS_DATA_LENGTH (260)
	#define SESSION_TIMEOUT_IN_SEC 10
	#define MODBUS_TCP_PORT 	502
	#define MODBUS_MASTER_CONNECT_TIMEOUT_IN_SEC 10
	#define MAXIMUM_TCP_CONNECTION 	32
	#define ADDITIONAL_RECORDS_TIMEOUT_TRACKER 100
	#define REQ_ARRAY_MULTIPLIER 100
#else
	#define TCP_MODBUS_ADU_LENGTH 256
	#define MODBUS_DATA_LENGTH (256)
#endif

#define FILE_RECORD_REFERENCE_TYPE 6
#define MAX_ALLOWED_SLAVES	(247)

/// Starting Addresses for coils and registers
#define SERIES_COIL_STATUS_ADDR		(00001)
#define SERIES_READ_INPUT_ADDR		(10001)
#define SERIES_INPUT_REG_ADDR		(30001)
#define SERIES_HOLDING_REG_ADDR		(40001)

#define APP_DATA_TIMEOUT	(250)

/// Validation limits for stack
#define MIN_VALUE			(0x0000)
#define MIN_COILS			(1)
#define MAX_COILS			(2000)
#define MAX_COIL_REGISTER	(65535)

#define MIN_INPUT_REGISTER	(1)
#define MAX_INPUT_REGISTER	(125)
#define MIN_MULTI_REGISTER	(1)
#define MAX_MULTI_REGISTER	(123)

#define MIN_MULTI_COIL		(1)
#define MAX_MULTI_COIL		(1968)
#define MAX_STARTING_ADDRESS (65535)
#define COIL_ON			(0xFF00)
#define COIL_OFF		(0x0000)
#define MAX_VALUE		(65535)
#define MAX_REGISTERS	(200)

#define MIN_HOLDING_REGISTERS (1)
#define MAX_HOLDING_REGISTERS (125)
#define MIN_INPUT			  (1)
#define MAX_INPUT			(65535)

#define MIN_MUL_WRITE_REG	(1)
#define MAX_MUL_WRITE_REG	(121)

#define MIN_FILE_BYTE_COUNT		(7)
#define MAX_FILE_BYTE_COUNT	(245)

/// Application Address
/// To Do : Need to update as per application requirements
#define MAX_COIL_ADDR			(65535)
#define MAX_INPUT_REGISTER_ADDR (65535)

#define MAX_BITS		(8)
#define VALUE_ZERO		(0x00)
#define MAX_BYTE	(0xFF)
#define MEI_TYPE	(14)


/**
 @struct MbusTXData_t
 @brief
    This structure defines Modbus transmit data
*/
typedef struct
{
	/// Holds the data field
	uint8_t m_au8DataFields[ MODBUS_DATA_LENGTH ];
	/// Holds the received length of packet
	uint16_t m_u16Length;
}MbusTXData_t;

/**
 @struct MbusRXData_t
 @brief
    This structure defines Modbus receive data
*/
typedef struct
{
	/// Holds the data field
	uint8_t m_au8DataFields[ MODBUS_DATA_LENGTH ];
	/// Holds the received length of packet
	uint8_t m_u8Length;

	void* m_pvAdditionalData;
}MbusRXData_t;

/**
 @enum eTransactionState
 @brief
    This structure defines modbus index
*/
typedef enum
{
	REQ_RCVD_FROM_APP,
	REQ_PROCESS_ERROR,
	REQ_SENT_ON_NETWORK,
	RESP_RCVD_FROM_NETWORK,
	RESP_TIMEDOUT,
	RESP_ERROR,
	RESP_SENT_TO_APP,
	IdleState,
	RESERVED,
}eTransactionState;

typedef enum
{
	SOCK_CONNECT_SUCCESS,
	SOCK_CONNECT_FAILED,
	SOCK_CONNECT_INPROGRESS,
	SOCK_CREATE_FAILED,
	SOCK_CREATE_SUCCESS,
	SOCK_SEND_FAILED,
	SOCK_NOT_CONNECTED
}eSockConnect_enum;

#define MAX_RETRY_COUNT 10

typedef struct IP_Connect
{
	int32_t m_retryCount;
	struct sockaddr_in m_servAddr;
	int32_t m_sockfd;
	eSockConnect_enum m_lastConnectStatus;
	bool m_bIsAddedToEPoll;
	int m_iRcvConRef;
}IP_Connect_t;

/**
*
* @struct  - stMbusPacketVariables_t
*
* DESCRIPTION
* This structure defines the Receive queue along with block
* size, fill & process index. It also defines the mutex
* lock required to synchronize the queue.
*
*/
typedef struct _stMbusPacketVariables
{
	/** Holds the received transaction ID*/
	uint16_t m_u16TransactionID;
	uint16_t m_u16AppTxID;
	/** Holds the unit id  */
	uint8_t  m_u8UnitID;
	_Atomic eTransactionState m_state;
	uint8_t  m_u8ProcessReturn;
#ifdef MODBUS_STACK_TCPIP_ENABLED
	/** Holds Ip address of salve/server device */
	uint8_t m_u8IpAddr[4];
	uint16_t u16Port;
	struct _stMbusPacketVariables *__next;
	struct _stMbusPacketVariables *__prev;
#else
	/** Received destination address */
	uint8_t	m_u8ReceivedDestination;	
#endif
	/** Holds the unit id  */
	uint8_t m_u8FunctionCode;
	/** Holds Data to be send to server */
	MbusTXData_t m_stMbusTxData;
	/** Holds Data received from server */
	MbusRXData_t m_stMbusRxData;
	/** Holds Status of command success or failure
	 * after ack received from server or timeout */
	uint8_t m_u8CommandStatus;
	/** Holds the start address  */
	uint16_t  m_u16StartAdd;
	/** Holds the Quantity  */
	uint16_t  m_u16Quantity;
	void *pFunc;
	/** Holds the Msg Priority  */
	long m_lPriority;

	//bool m_bIsAvailable;
	unsigned int m_ulMyId;
	int m_iTimeOutIndex;
	stTimeStamps m_objTimeStamps;
	unsigned char m_u8RawResp[MODBUS_DATA_LENGTH];
}stMbusPacketVariables_t;

struct stReqManager {
	stMbusPacketVariables_t m_objReqArray[MAX_REQUESTS];
	Mutex_H m_mutexReqArray;
};

typedef enum ThreadScheduler
{
	OTHER,
	FIFO,
	RR,
	UNKNOWN
}eThreadScheduler;

bool initReqManager();
stMbusPacketVariables_t* emplaceNewRequest(const struct timespec tsReqRcvd);
void freeReqNode(stMbusPacketVariables_t* a_pobjReq);

// Function to set thread parameters
void set_thread_sched_param();

#ifdef MODBUS_STACK_TCPIP_ENABLED

struct stTimeOutTrackerNode {
	stMbusPacketVariables_t *m_pstStart;
	stMbusPacketVariables_t *m_pstLast;
	int m_iIsLocked;
};

struct stTimeOutTracker {
	struct stTimeOutTrackerNode *m_pstArray;
	int m_iSize;

	_Atomic int m_iCounter;
	int32_t m_iTimeoutActionQ;
	sem_t m_semTimeout;

	Thread_H m_threadIdTimeoutTimer;
	Thread_H m_threadTimeoutAction;
};

/**
 *
 * Description
 * Timeout is implemented in the form of counter.
 * This function returns current counter.
 *
 * @param thread argument - none
 * @return return timer tracker count on success& -1 in case of error
 */
int getTimeoutTrackerCount();

/**
 *
 * Description
 * Remove request from list with locks
 *
 * @param pstMBusRequesPacket [in] pointer to struct of type stMbusPacketVariables_t
 * @return void [out] none
 */
void releaseFromTracker(stMbusPacketVariables_t *pstMBusRequesPacket);

struct stResProcessData {
	sem_t m_semaphoreResp;

	int32_t m_i32RespMsgQueId;
	Thread_H m_threadIdRespToApp;
};

/**
 *
 * Description
 * Stops listening on a socket with epoll.
 * Resets associated data structures.
 * This function acquires lock for epoll data structure.
 *
 * @param Reference to epoll data structure
 * @return None
 */
void removeEPollRef(int a_iIndex);

/**
 * Description
 * Set socket failure state
 *
 * @param stIPConnect [in] pointer to struct of type IP_Connect_t
 *
 * @return void [out] return status based on condition
 *
 */
int addtoEPollList(IP_Connect_t *a_pstIPConnect);

/**
 @struct IP_address
 @brief
    This structure defines IP address parameters
*/
typedef struct IP_address
{
	union
	{
		struct{
			unsigned char IP_1;
			unsigned char IP_2;
			unsigned char IP_3;
			unsigned char IP_4;
		}s_un_b;
		uint32_t s_addr;
	}s_un;
}IP_address_t;
#endif
/**
 @union uByteOrder
 @brief
    This structure defines byte structure
*/
typedef union uByteOrder
{
	struct
	{
		unsigned char u8ByteOne;
		unsigned char u8ByteTwo;
	}TwoByte;
	uint16_t u16Word;
}uByteOrder_t;

/**
 @union stEndianess
 @brief
    This structure defines endian ness structure
*/
typedef union stEndianess
{
	struct
	{
		uint8_t u8FirstByte;
		uint8_t u8SecondByte;
	}stByteOrder;
	uint16_t u16word;
}stEndianess_t;

/**
 @enum eMbusIndex_enum
 @brief
    This structure defines modbus index
*/
typedef enum
{
	MBUS_INDEX_0,
	MBUS_INDEX_1,
	MBUS_INDEX_2,
	MBUS_INDEX_3,
	MBUS_INDEX_4,
	MBUS_INDEX_5,
	MBUS_INDEX_6,
	MBUS_INDEX_7,
	MBUS_INDEX_8,
	MBUS_INDEX_9,
	MBUS_INDEX_10
}eMbusIndex_enum;

/**
 *
 * Description
 * Application callback handler
 *
 * @param pstMBusRequesPacket [in] Request packet
 * @param eMbusStackErr       [in] Stack error codes
 *
 */
void ApplicationCallBackHandler(stMbusPacketVariables_t *pstMBusRequesPacket,
		eStackErrorCode eMbusStackErr);

#ifdef MODBUS_STACK_TCPIP_ENABLED

/**
 * Description
 * Send modbus packet on network
 *
 * @param pstMBusRequesPacket [in] pointer to request packet struct of type stMbusPacketVariables_t
 * @param a_pstIPConnect [in] pointer to socket struct of type IP_Connect_t
 *
 * @return uint8_t [out] respective error codes
 *
 */
uint8_t Modbus_SendPacket(stMbusPacketVariables_t *pstMBusRequesPacket,
		IP_Connect_t *m_pstIPConnect);
#else
/**
 * Description
 * Send modbus packet on network
 *
 * @param pstMBusRequesPacket [in] pointer to request packet struct of type stMbusPacketVariables_t
 * @param a_pstIPConnect [in] pointer to socket struct of type IP_Connect_t
 *
 * @return uint8_t [out] respective error codes
 *
 */uint8_t Modbus_SendPacket(stMbusPacketVariables_t *pstMBusRequesPacket,
		int32_t *pi32sockfd);
#endif
#endif /* STACKCONFIG_H_ */
