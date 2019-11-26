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

#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <safe_lib.h>
#include "SessionControl.h"

extern Mutex_H LivSerSesslist_Mutex;

int32_t i32MsgQueIdSC = 0;
stLiveSerSessionList_t *pstSesCtlThdLstHead = NULL;


extern void* ServerSessTcpAndCbThread(void* threadArg);
extern stDevConfig_t ModbusMasterConfig;

/**
 *
 * Description
 * Session control thread function
 *
 * @param threadArg [in] thread argument
 * @return void 	[out] nothing
 */
void* SessionControlThread(void* threadArg)
{
	int32_t i32MsgQueIdSC = 0;
	Linux_Msg_t stScMsgQue = { 0 };
	uint8_t u8NewDevEntryFalg = 0;
	stLiveSerSessionList_t *pstLivSerSesslist = NULL;
	stLiveSerSessionList_t *pstTempLivSerSesslist = NULL;
	stMbusPacketVariables_t *pstMBusReqPact = NULL;
	Post_Thread_Msg_t stPostThreadMsg = { 0 };
	thread_Create_t stThreadParam = { 0 };

	i32MsgQueIdSC = *((int32_t *)threadArg);


	while (1)
	{
		memset(&stScMsgQue,00,sizeof(stScMsgQue));
		memset(&stPostThreadMsg,00,sizeof(stPostThreadMsg));
		if(OSAL_Get_Message(&stScMsgQue, i32MsgQueIdSC))
		{
			u8NewDevEntryFalg = 0;
			pstMBusReqPact = stScMsgQue.wParam;
			Osal_Wait_Mutex (LivSerSesslist_Mutex,0);
			pstLivSerSesslist = pstSesCtlThdLstHead;
			if(NULL == pstLivSerSesslist)
			{
				pstSesCtlThdLstHead = OSAL_Malloc(sizeof(stLiveSerSessionList_t));
				if(NULL == pstSesCtlThdLstHead)
				{
					ApplicationCallBackHandler(pstMBusReqPact,
							STACK_ERROR_MALLOC_FAILED);
					Osal_Release_Mutex (LivSerSesslist_Mutex);
					continue;
				}
				else
				{
					pstLivSerSesslist = pstSesCtlThdLstHead;
					pstLivSerSesslist->m_pNextElm = NULL;
					u8NewDevEntryFalg = 1;
				}
			}
			else
			{
				while(NULL != pstLivSerSesslist)
				{
					pstTempLivSerSesslist = pstLivSerSesslist;

					if(NULL != pstLivSerSesslist)
					{
						if(pstMBusReqPact->m_u8IpAddr[0] == pstLivSerSesslist->m_u8IpAddr[0] &&
							pstMBusReqPact->m_u8IpAddr[1] == pstLivSerSesslist->m_u8IpAddr[1] &&
							pstMBusReqPact->m_u8IpAddr[2] == pstLivSerSesslist->m_u8IpAddr[2] &&
							pstMBusReqPact->m_u8IpAddr[3] == pstLivSerSesslist->m_u8IpAddr[3])
						{
							break;
						}
						else
						{
							pstLivSerSesslist = pstLivSerSesslist->m_pNextElm;
						}
					}
				}

				if(NULL == pstLivSerSesslist)
				{
					pstTempLivSerSesslist->m_pNextElm = OSAL_Malloc(sizeof(stLiveSerSessionList_t));
					if(NULL == pstTempLivSerSesslist->m_pNextElm )
					{
						ApplicationCallBackHandler(pstMBusReqPact, STACK_ERROR_MALLOC_FAILED);
						Osal_Release_Mutex (LivSerSesslist_Mutex);
						continue;
					}
					pstLivSerSesslist = pstTempLivSerSesslist->m_pNextElm;
					pstLivSerSesslist->m_pNextElm = NULL;
					u8NewDevEntryFalg = 1;
				}
			}

			if(u8NewDevEntryFalg)
			{
				memcpy_s(pstLivSerSesslist->m_u8IpAddr,sizeof(pstLivSerSesslist->m_u8IpAddr),
						pstMBusReqPact->m_u8IpAddr,sizeof(pstMBusReqPact->m_u8IpAddr));
				pstLivSerSesslist->MsgQId = OSAL_Init_Message_Queue();

				stPostThreadMsg.idThread = pstLivSerSesslist->MsgQId;
				stPostThreadMsg.lParam = pstMBusReqPact;
				stPostThreadMsg.wParam = pstLivSerSesslist;
				stPostThreadMsg.MsgType = 1;

				if(!OSAL_Post_Message(&stPostThreadMsg))
				{
					ApplicationCallBackHandler(pstMBusReqPact, STACK_ERROR_QUEUE_SEND);
					Osal_Release_Mutex (LivSerSesslist_Mutex);
					continue;
				}

				stThreadParam.dwStackSize = 0;
				stThreadParam.lpStartAddress = ServerSessTcpAndCbThread;
				stThreadParam.lpParameter = &pstLivSerSesslist->MsgQId;
				stThreadParam.lpThreadId = &pstLivSerSesslist->m_ThreadId;

				pstLivSerSesslist->m_ThreadId = Osal_Thread_Create(&stThreadParam);
				if(0 == pstLivSerSesslist->m_ThreadId)
				{
					ApplicationCallBackHandler(pstMBusReqPact, STACK_ERROR_THREAD_CREATE);
					Osal_Release_Mutex (LivSerSesslist_Mutex);
					continue;
				}
			}
			else
			{
				stPostThreadMsg.idThread = pstLivSerSesslist->MsgQId;
				stPostThreadMsg.lParam = pstMBusReqPact;
				stPostThreadMsg.wParam = pstLivSerSesslist;
				stPostThreadMsg.MsgType = 1;

				if(!OSAL_Post_Message(&stPostThreadMsg))
				{
					ApplicationCallBackHandler(pstMBusReqPact, STACK_ERROR_QUEUE_SEND);
					Osal_Release_Mutex (LivSerSesslist_Mutex);
					continue;
				}
			}
			Osal_Release_Mutex (LivSerSesslist_Mutex);
		}
		else
		{
			ApplicationCallBackHandler(pstMBusReqPact, STACK_ERROR_QUEUE_RECIVE);
			continue;
		}
		fflush(stdin);
	}
}

/**
 *
 * Description
 * TCP and callback thread function
 *
 * @param threadArg [in] thread argument
 * @return void pointer
 *
 */
void* ServerSessTcpAndCbThread(void* threadArg)
{

	Linux_Msg_t stScMsgQue = { 0 };
	uint32_t u32TimeCount = 0;
	uint8_t u8ReturnType = 0;
	stLiveSerSessionList_t *pstLivSerSesslist = NULL;
	stLiveSerSessionList_t *pstTempLivSerSesslist = NULL;
	stMbusPacketVariables_t *pstMBusRequesPacket = NULL;
	int32_t i32sockfd = 0;
	int32_t i32MsgQueIdSSTC = 0;
	int32_t i32RetVal = 0;

	i32MsgQueIdSSTC = *((int32_t *)threadArg);

	while (1)
	{
		memset(&stScMsgQue,00,sizeof(stScMsgQue));
		i32RetVal = 0;
		i32RetVal = OSAL_Get_NonBlocking_Message(&stScMsgQue, i32MsgQueIdSSTC);

		if(i32RetVal > 0)
		{
			pstLivSerSesslist = stScMsgQue.wParam;
			pstMBusRequesPacket = stScMsgQue.lParam;
			u8ReturnType = Modbus_SendPacket(pstMBusRequesPacket, &i32sockfd);
			ApplicationCallBackHandler(pstMBusRequesPacket,u8ReturnType);
			OSAL_Free(pstMBusRequesPacket);
			u32TimeCount = 0;
		}
		else
		{
			usleep(100000);
			u32TimeCount++;

			if(u32TimeCount >= (ModbusMasterConfig.m_u16TcpSessionTimeout * 10))
			{
				close(i32sockfd);
				Osal_Wait_Mutex (LivSerSesslist_Mutex,0);
				if(NULL != pstLivSerSesslist)
				{
					OSAL_Delete_Message_Queue(pstLivSerSesslist->MsgQId);
					if(pstSesCtlThdLstHead == pstLivSerSesslist)
					{
						pstSesCtlThdLstHead = NULL;
						OSAL_Free(pstLivSerSesslist);
						pstLivSerSesslist = NULL;
					}
					else
					{
						pstTempLivSerSesslist = pstSesCtlThdLstHead;
						if(NULL != pstTempLivSerSesslist)
						{
							while(pstTempLivSerSesslist->m_pNextElm != pstLivSerSesslist
									&& NULL != pstTempLivSerSesslist->m_pNextElm)
								pstTempLivSerSesslist = pstTempLivSerSesslist->m_pNextElm;

							if(NULL != pstTempLivSerSesslist->m_pNextElm)
								pstTempLivSerSesslist->m_pNextElm = pstLivSerSesslist->m_pNextElm;
						}

						if(NULL != pstLivSerSesslist)
							OSAL_Free(pstLivSerSesslist);
					}
				}
				Osal_Release_Mutex (LivSerSesslist_Mutex);
				break;
			}
		}
	}
}
