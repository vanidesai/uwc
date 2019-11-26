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
	uint8_t m_u8IpAddr[4];
	void *m_pNextElm;
}stLiveSerSessionList_t;

#endif /* INC_SESSIONCONTROL_H_ */
