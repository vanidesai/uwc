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

/// Session control thread function
void* SessionControlThread(void* threadArg);

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
#else
	uint8_t m_u8ReceivedDestination;
#endif
	void *m_pNextElm;
}stLiveSerSessionList_t;

#endif /* INC_SESSIONCONTROL_H_ */
