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

/// Session control thread function
void* SessionControlThread(void* threadArg);
void* EpollRecvThread();

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
	uint8_t m_u8IpAddr[4];
	uint16_t m_u16Port;
	int m_clientFD;
	int m_len;
	int m_bytesRead;
	int m_bytesToBeRead;
	unsigned char m_readBuffer[1024];
}stTcpRecvData_t;

struct stHandleRespData {
	sem_t m_semaphoreHandleResp;

	int32_t m_i32HandleRespMsgQueId;
	Thread_H m_threadIdHandleResp;
};

typedef struct mesg_data
{
	/// array
	unsigned char m_readBuffer[1024];
}mesg_data_t;

int initHandleResponseContext();

void removeReqFromListWithLock(stMbusPacketVariables_t *pstMBusRequesPacket);
void addToRespQ(stMbusPacketVariables_t *a_pstReq);
void addToHandleRespQ(stTcpRecvData_t a_pstReq);
stMbusPacketVariables_t* searchReqList(uint8_t a_u8UnitID, uint16_t a_u16TransactionID);
unsigned long get_nanos(void);
void Mark_Sock_Fail(IP_Connect_t *stIPConnect);
#endif
uint8_t DecodeRxPacket(uint8_t *ServerReplyBuff,stMbusPacketVariables_t *pstMBusRequesPacket);
#endif /* INC_SESSIONCONTROL_H_ */
