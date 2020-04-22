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

#ifndef INC_SESSIONCONTROL_H_
#define INC_SESSIONCONTROL_H_
#include "StackConfig.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h> // for epoll_create1(), epoll_ctl(), struct epoll_event

/**
 enum eClientSessionStatus
 @brief
    This enum defines status of client request
*/
typedef enum
{
	CLIENTSESSION_REQ_SENT,
	CLIENTSESSION_RES_REC,
	CLIENTSESSION_REQ_TIMEOUT,
	CLIENTSESSION_CALLBACK,
	CLIENTSESSION_FREE
}eClientSessionStatus;

/**
 @struct LiveSerSessionList
 @brief
    This structure defines list of session
*/
typedef struct LiveSerSessionList
{
	uint8_t m_Index;
	Thread_H m_ThreadId;
	int32_t MsgQId;
	eClientSessionStatus m_eCltSesStatus;
#ifdef MODBUS_STACK_TCPIP_ENABLED
	uint8_t m_u8IpAddr[4];
	uint16_t m_u16Port;
	int32_t m_i32sockfd;
	uint8_t m_u8UnitId;
	int m_iLastConnectStatus;
	uint8_t m_u8ConnectAttempts;
	uint16_t m_u16TxID;
#else
	uint8_t m_u8ReceivedDestination;
#endif
	void *m_pNextElm;
}stLiveSerSessionList_t;


typedef struct EpollTcpRecv
{
	uint8_t m_Index;
	Thread_H m_ThreadId;
	int32_t MsgQId;
	eClientSessionStatus m_eCltSesStatus;
#ifdef MODBUS_STACK_TCPIP_ENABLED
	uint8_t m_u8IpAddr[4];
	uint16_t m_u16Port;
#else
	uint8_t m_u8ReceivedDestination;
#endif
	void *m_pNextElm;
}stEpollTcpRecv_t;

#ifdef MODBUS_STACK_TCPIP_ENABLED
typedef struct TcpRecvData
{
	IP_Connect_t *m_pstConRef;
	int m_len;
	int m_bytesRead;
	int m_bytesToBeRead;
	unsigned char m_readBuffer[MODBUS_DATA_LENGTH];
}stTcpRecvData_t;

typedef struct mesg_data
{
	long mesg_type;
	/// data buffer
	unsigned char m_readBuffer[MODBUS_DATA_LENGTH];
}mesg_data_t;

//// Function declarations

/**
 *
 * Description
 * Initialize request list data
 *
 * @param none
 * @return int [out] 0 (if success)
 * 					-1 (if failure)
 */
int initTCPRespStructs();

/**
 *
 * Description
 * De-initialize data structures related to TCP response processing.
 *
 * @param none
 * @return Nothing
 */
void deinitTCPRespStructs();

/**
 *
 * Description
 * Initializes data structures needed for epoll operation for receiving TCP data
 *
 * @param None
 * @return true/false based on status
 */
bool initEPollData();

/**
 *
 * Description
 * Add request to list
 *
 * @param pstMBusRequesPacket [in] pointer to struct of type stMbusPacketVariables_t
 * @return int 0 for success, -1 for error
 */
int addReqToList(stMbusPacketVariables_t *pstMBusRequesPacket);

//void removeReqFromListWithLock(stMbusPacketVariables_t *pstMBusRequesPacket);

/**
 *
 * Description
 * Add msg to response queue
 *
 * @param a_pstReq [in] pointer to struct of type stMbusPacketVariables_t
 * @return void [out] none
 */
void addToRespQ(stMbusPacketVariables_t *a_pstReq);

/**
 *
 * Description
 * Add response to handle in a queue
 *
 * @param a_u8UnitID [in] uint8_t unit id from modbus header
 * @param a_u16TransactionID  [in] uint16_t transction id from modbus header
 * @return void [out] none
 */
void addToHandleRespQ(stTcpRecvData_t *a_pstReq);


//stMbusPacketVariables_t* searchReqList(uint8_t a_u8UnitID, uint16_t a_u16TransactionID);
/**
 *
 * Description
 * get time stamp in nano-seconds
 *
 * @param - none
 * @return unsigned long [out] time in nano-seconds
 */
unsigned long get_nanos(void);

/**
 * Description
 * Close the socket connection and reset the structure
 *
 * @param stIPConnect [in] pointer to struct of type IP_Connect_t
 *
 * @return void [out] none
 *
 */
void closeConnection(IP_Connect_t *a_pstIPConnect);

/**
 * Description
 * Set socket failure state
 *
 * @param stIPConnect [in] pointer to struct of type IP_Connect_t
 *
 * @return void [out] none
 *
 */
void Mark_Sock_Fail(IP_Connect_t *stIPConnect);

/**
 *
 * Description
 * TCP and callback thread function
 *
 * @param threadArg [in] thread argument
 * @return void pointer
 *
 */
void* ServerSessTcpAndCbThread(void* threadArg);

#endif

/**
 *
 * Description
 * Session control thread function
 *
 * @param threadArg [in] thread argument
 * @return void 	[out] nothing
 */
void* SessionControlThread(void* threadArg);

/**
 *
 * Description
 * thread to start polling on sockets o receive data
 *
 * @param - none
 * @return void [out] none
 */
void* EpollRecvThread();

/**
 * Description
 * Function to decode received modbus data
 *
 * @param ServerReplyBuff [in] Input buffer
 * @param pstMBusRequesPacket [in] Request packet
 * @return uint8_t [out] respective error codes
 *
 */
uint8_t DecodeRxPacket(uint8_t *ServerReplyBuff,stMbusPacketVariables_t *pstMBusRequesPacket);
#endif /* INC_SESSIONCONTROL_H_ */
