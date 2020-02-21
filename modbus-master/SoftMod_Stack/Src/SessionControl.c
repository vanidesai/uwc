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

#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <safe_lib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include <sys/epoll.h> // for epoll_create1(), epoll_ctl(), struct epoll_event

#include "SessionControl.h"

//#define MODBUS_STACK_TCPIP_ENABLED
int m_clientFd = 0, m_epollFd = 0;
struct epoll_event m_event, *m_events = NULL;
struct sockaddr_in m_clientAddr;

struct stReqManager g_objReqManager;

#define MODBUS_HEADER_LENGTH 6 //no. of bytes till length paramater in header
//array of accepted clients
#define MAX_DEVICE_PER_SITE 300

#ifndef MODBUS_STACK_TCPIP_ENABLED

extern int fd;

#endif

int32_t i32MsgQueIdSC = 0;
extern bool g_bThreadExit;
extern long g_lResponseTimeout;

#define READ_SIZE 1024
#define MAXEVENTS 100
#define EPOLL_TIMEOUT 1000
#define MAXQUEUE 5

#ifdef MODBUS_STACK_TCPIP_ENABLED
extern Mutex_H LivSerSesslist_Mutex;
stTcpRecvData_t m_clientAccepted[MAX_DEVICE_PER_SITE];
stLiveSerSessionList_t *pstSesCtlThdLstHead = NULL;
extern stDevConfig_t ModbusMasterConfig;
extern void* ServerSessTcpAndCbThread(void* threadArg);
extern Mutex_H LivSerSesslist_Mutex;
#endif

void initReqManager()
{
	//printf("initReqManager start\n");
	unsigned int iCount = 0;
	for (; iCount < MAX_REQUESTS; iCount++)
	{
		g_objReqManager.m_objReqArray[iCount].m_ulMyId = iCount;
		g_objReqManager.m_objReqArray[iCount].m_bIsAvailable = true;
#ifdef MODBUS_STACK_TCPIP_ENABLED
		g_objReqManager.m_objReqArray[iCount].__next = NULL;
		g_objReqManager.m_objReqArray[iCount].__prev = NULL;
#endif
		g_objReqManager.m_objReqArray[iCount].m_state = IdleState;

		// Init timestamps to 0
		g_objReqManager.m_objReqArray[iCount].m_objTimeStamps.tsReqRcvd = (struct timespec){0};
		g_objReqManager.m_objReqArray[iCount].m_objTimeStamps.tsReqSent = (struct timespec){0};
		g_objReqManager.m_objReqArray[iCount].m_objTimeStamps.tsRespRcvd = (struct timespec){0};
		g_objReqManager.m_objReqArray[iCount].m_objTimeStamps.tsRespSent = (struct timespec){0};
	}
	//printReqListNoLock("initReqManager");
	g_objReqManager.m_mutexReqArray = Osal_Mutex();
	//printf("initReqManager end\n");
}

stMbusPacketVariables_t* emplaceNewRequest(stMbusPacketVariables_t* a_pObjTempReq)
{
	if(NULL == a_pObjTempReq)
	{
		return NULL;
	}
	////printf("getNodeForNewRequest: start\n");
	stMbusPacketVariables_t* ptr = NULL;
	long iCount = 0;
	Osal_Wait_Mutex(g_objReqManager.m_mutexReqArray, 0);
	for (; iCount < MAX_REQUESTS; iCount++)
	{
		if(true == g_objReqManager.m_objReqArray[iCount].m_bIsAvailable)
		{
			ptr = &g_objReqManager.m_objReqArray[iCount];

			// Init the object from temp object
			memcpy_s(ptr, sizeof(stMbusPacketVariables_t),
						a_pObjTempReq, sizeof(stMbusPacketVariables_t));

			ptr->m_bIsAvailable = false;
#ifdef MODBUS_STACK_TCPIP_ENABLED
			ptr->__next = NULL;
			ptr->__prev = NULL;
#endif
			ptr->m_state = REQ_RCVD_FROM_APP;
			ptr->m_ulMyId = iCount;

			// Init req rcvd timestamp
			timespec_get(&(ptr->m_objTimeStamps.tsReqRcvd), TIME_UTC);
			// Init other timestamps to 0
			ptr->m_objTimeStamps.tsReqSent = (struct timespec){0};
			ptr->m_objTimeStamps.tsRespRcvd = (struct timespec){0};
			ptr->m_objTimeStamps.tsRespSent = (struct timespec){0};

			//printf("getNodeForNewRequest: %u, time: %lu\n", ptr->m_ulMyId, get_nanos());
			break;
		}
	}
	//printReqListNoLock("getNodeForNewRequest");
	Osal_Release_Mutex(g_objReqManager.m_mutexReqArray);
	if(NULL == ptr)
	{
		//printf("getNodeForNewRequest: No empty node !\n");
	}
	//printf("getNodeForNewRequest: end\n");
	return ptr;
}

void freeReqNode(stMbusPacketVariables_t* a_pobjReq)
{
	Osal_Wait_Mutex(g_objReqManager.m_mutexReqArray, 0);
	if(NULL != a_pobjReq)
	{
#ifdef MODBUS_STACK_TCPIP_ENABLED
		a_pobjReq->__next = NULL;
		a_pobjReq->__prev = NULL;
#endif
		a_pobjReq->m_state = IdleState;
		a_pobjReq->m_bIsAvailable = true;

		// Init timestamps to 0
		a_pobjReq->m_objTimeStamps.tsReqRcvd = (struct timespec){0};
		a_pobjReq->m_objTimeStamps.tsReqSent = (struct timespec){0};
		a_pobjReq->m_objTimeStamps.tsRespRcvd = (struct timespec){0};
		a_pobjReq->m_objTimeStamps.tsRespSent = (struct timespec){0};
		//printf("freeReqNode after: %u, time: %lu\n", a_pobjReq->m_ulMyId, get_nanos());
	}
	//printReqListNoLock("freeReqNode");
	Osal_Release_Mutex(g_objReqManager.m_mutexReqArray);
}

/**
 *
 * Description
 * Session control thread function
 *
 * @param threadArg [in] thread argument
 * @return void 	[out] nothing
 */
#ifdef MODBUS_STACK_TCPIP_ENABLED
void* SessionControlThread(void* threadArg)
{
	int32_t i32MsgQueIdSC = 0;
	Linux_Msg_t stScMsgQue = { 0 };
	uint8_t u8NewDevEntryFalg=0;
	stLiveSerSessionList_t *pstLivSerSesslist = NULL;
	stLiveSerSessionList_t *pstTempLivSerSesslist = NULL;
	stMbusPacketVariables_t *pstMBusReqPact = NULL;
	Post_Thread_Msg_t stPostThreadMsg = { 0 };
	thread_Create_t stThreadParam = { 0 };

	i32MsgQueIdSC = *((int32_t *)threadArg);


	//while (NULL != threadArg)
	while(false == g_bThreadExit)
	{
		memset(&stScMsgQue,00,sizeof(stScMsgQue));
		memset(&stPostThreadMsg,00,sizeof(stPostThreadMsg));
		if(OSAL_Get_Message(&stScMsgQue, i32MsgQueIdSC))
		{
			u8NewDevEntryFalg = 0;
			pstMBusReqPact = stScMsgQue.wParam;
			pstMBusReqPact->m_lPriority = stScMsgQue.mtype;
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
					freeReqNode(pstMBusReqPact);
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
								pstMBusReqPact->m_u8IpAddr[3] == pstLivSerSesslist->m_u8IpAddr[3] &&
								pstMBusReqPact->u16Port == pstLivSerSesslist->m_u16Port)
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
						freeReqNode(pstMBusReqPact);
						continue;
					}
					pstLivSerSesslist = pstTempLivSerSesslist->m_pNextElm;
					pstLivSerSesslist->m_pNextElm = NULL;
					u8NewDevEntryFalg = 1;
				}
			}

			if(u8NewDevEntryFalg )
			{

				memcpy_s(pstLivSerSesslist->m_u8IpAddr,sizeof(pstLivSerSesslist->m_u8IpAddr),
										pstMBusReqPact->m_u8IpAddr,sizeof(pstMBusReqPact->m_u8IpAddr));
				pstLivSerSesslist->MsgQId = OSAL_Init_Message_Queue();
				pstLivSerSesslist->m_i32sockfd = 0;

				pstLivSerSesslist->m_u16Port = pstMBusReqPact->u16Port;
				stPostThreadMsg.idThread = pstLivSerSesslist->MsgQId;
				stPostThreadMsg.lParam = pstMBusReqPact;
				stPostThreadMsg.wParam = pstLivSerSesslist;
				stPostThreadMsg.MsgType = pstMBusReqPact->m_lPriority;

				if(!OSAL_Post_Message(&stPostThreadMsg))
				{
					ApplicationCallBackHandler(pstMBusReqPact, STACK_ERROR_QUEUE_SEND);
					Osal_Release_Mutex (LivSerSesslist_Mutex);
					freeReqNode(pstMBusReqPact);
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
					freeReqNode(pstMBusReqPact);
					continue;
				}
			}
			else
			{
				stPostThreadMsg.idThread = pstLivSerSesslist->MsgQId;
				stPostThreadMsg.lParam = pstMBusReqPact;
				stPostThreadMsg.wParam = pstLivSerSesslist;
				stPostThreadMsg.MsgType = pstMBusReqPact->m_lPriority;

				if(!OSAL_Post_Message(&stPostThreadMsg))
				{
					ApplicationCallBackHandler(pstMBusReqPact, STACK_ERROR_QUEUE_SEND);
					Osal_Release_Mutex (LivSerSesslist_Mutex);
					freeReqNode(pstMBusReqPact);
					continue;
				}
			}
			Osal_Release_Mutex (LivSerSesslist_Mutex);
		}
		else
		{
			ApplicationCallBackHandler(pstMBusReqPact, STACK_ERROR_QUEUE_RECIVE);
			freeReqNode(pstMBusReqPact);
			continue;
		}
		fflush(stdin);

		/// check for thread exit
		if(g_bThreadExit)
		{
			break;
		}
	}
	return NULL;
}

int getClientIdFromList(int socketID) {
	int socketIndex = -1;

	for(int i = 0; i < 256; i++) {
		if(m_clientAccepted[i].m_clientFD == socketID) {
			return i;
		}
	}

	return socketIndex;
}

void resetClientStruct(stTcpRecvData_t* clientAccepted) {

	clientAccepted->m_bytesRead = 0;
	clientAccepted->m_bytesToBeRead = 0;
	clientAccepted->m_len = 0;
	memset(&clientAccepted->m_readBuffer, 0,
			sizeof(clientAccepted->m_readBuffer));

}

struct timespec getEpochTime() {

	struct timespec ts;
    timespec_get(&ts, TIME_UTC);

    return ts;
}

//epoll thread
void* EpollRecvThread()
{

	int event_count = 0;
	size_t bytes_read = 0;
	size_t bytes_to_read = 0;

	//allocate no of polling events
	m_events = (struct epoll_event*) calloc(MAXEVENTS, sizeof(m_event));

	if(NULL == m_events)
	{
		return NULL;
	}

	int iClientCount = 0; //for array

	while (true != g_bThreadExit)
	{
		event_count = 0;

		event_count = epoll_wait(m_epollFd, m_events, MAXEVENTS, EPOLL_TIMEOUT);
		for (int i = 0; i < event_count; i++) {

			int clientID = getClientIdFromList(m_events[i].data.fd);

			if (clientID == -1) {
				//this is a new client socket - add it to the list
				clientID = ++iClientCount;

				//resetClientStruct(&m_clientAccepted[clientID]);
				m_clientAccepted[clientID].m_bytesRead = 0;
				m_clientAccepted[clientID].m_bytesToBeRead = 0;
				m_clientAccepted[clientID].m_len = 0;
				memset(&m_clientAccepted[clientID].m_readBuffer, 0,
						sizeof(m_clientAccepted[clientID].m_readBuffer));

				//set client id = received socket descriptor
				m_clientAccepted[clientID].m_clientFD = m_events[i].data.fd;
			}

			//if there are still bytes remaining to be read for this socket, adjust read size accordingly
			if (m_clientAccepted[clientID].m_bytesToBeRead > 0) {
				bytes_to_read = m_clientAccepted[clientID].m_bytesToBeRead;
			} else {
				bytes_read = 0;
				bytes_to_read = MODBUS_HEADER_LENGTH; //read only header till length
			}

			//receive data from socket
			bytes_read = recv(m_clientAccepted[clientID].m_clientFD,
					m_clientAccepted[clientID].m_readBuffer+m_clientAccepted[clientID].m_bytesRead, bytes_to_read, 0);


			if (bytes_read == 0)
				continue;

			//parse length if fd has 0 bytes read, otherwise rest packets are with actual data and not header
			if (m_clientAccepted[clientID].m_bytesRead == 0) {
				m_clientAccepted[clientID].m_len = (m_clientAccepted[clientID].m_readBuffer[4] << 8
						| m_clientAccepted[clientID].m_readBuffer[5]);
			}

			if (bytes_to_read == bytes_read) {
				m_clientAccepted[clientID].m_bytesRead += bytes_read;

				m_clientAccepted[clientID].m_bytesToBeRead =
						(m_clientAccepted[clientID].m_len + MODBUS_HEADER_LENGTH)
								- m_clientAccepted[clientID].m_bytesRead;
			}

			//add one more recv to read response completely
			if (m_clientAccepted[clientID].m_bytesToBeRead > 0) {

				while (m_clientAccepted[clientID].m_bytesToBeRead != 0) {

					//check if we have data on socket
					int bytesPresentOnSocket = 0;
					if(-1 == ioctl(m_clientAccepted[clientID].m_clientFD, FIONREAD,
							&bytesPresentOnSocket))
						//printf("Error in ioctl\n");

					if (bytesPresentOnSocket > 0) {
						bytes_to_read =
								m_clientAccepted[clientID].m_bytesToBeRead;

						//receive data from socket
						bytes_read = recv(m_clientAccepted[clientID].m_clientFD,
								m_clientAccepted[clientID].m_readBuffer+m_clientAccepted[clientID].m_bytesRead,
								bytes_to_read, 0);

						if (bytes_read == 0)
							continue;

						m_clientAccepted[clientID].m_bytesRead += bytes_read;

						m_clientAccepted[clientID].m_bytesToBeRead =
								(m_clientAccepted[clientID].m_len + MODBUS_HEADER_LENGTH)
										- m_clientAccepted[clientID].m_bytesRead;
					}
				}
				//should have read response completely

				if ((m_clientAccepted[clientID].m_len + MODBUS_HEADER_LENGTH)
						== m_clientAccepted[clientID].m_bytesRead) {

					addToHandleRespQ(&m_clientAccepted[clientID]);

					//clear socket struct for next iteration
					resetClientStruct(&m_clientAccepted[clientID]);

				} else {
					//this case should never happen ideally
					//continue to read for the same socket & epolling
					continue;
				}

			}
		}//for loop for sockets ends

		/// check for thread exit
		if(g_bThreadExit)
		{
			break;
		}

	} //while ends

	//close client socket or push all the client sockets in a vector and close all
	close(m_epollFd);

	if(m_events != NULL)
		free(m_events);

	return NULL;
}

#else

/**
 *
 * Description
 * Thread sends Modbus RTU messages which are available in the message queue to the Modbus RTU device.
 * Thread gets executed whenever message is available in the queue.
 * @param threadArg [in] thread argument
 * @return void [out] nothing
 */
void* SessionControlThread(void* threadArg)
{
	int32_t i32MsgQueIdSC = 0;
	Linux_Msg_t stScMsgQue = { 0 };
	stMbusPacketVariables_t *pstMBusReqPact = NULL;
	uint8_t u8ReturnType = 0;
	i32MsgQueIdSC = *((int32_t *)threadArg);


	//while (NULL != threadArg)
	while(false == g_bThreadExit)
	{
		memset(&stScMsgQue,00,sizeof(stScMsgQue));
		if(OSAL_Get_Message(&stScMsgQue, i32MsgQueIdSC))
		{
			pstMBusReqPact = stScMsgQue.wParam;
			u8ReturnType = Modbus_SendPacket(pstMBusReqPact, &fd);
			ApplicationCallBackHandler(pstMBusReqPact, u8ReturnType);
			//OSAL_Free(pstMBusReqPact);
			freeReqNode(pstMBusReqPact);
		}
		fflush(stdin);

		/// check for thread exit
		if(g_bThreadExit)
		{
			break;
		}
	}
	return NULL;
}
#endif

#ifdef MODBUS_STACK_TCPIP_ENABLED
struct stHandleRespData g_stHandleResp;
struct stReqList g_stReqList;
struct stResProcessData g_stRespProcess;

unsigned long get_nanos(void) {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (unsigned long)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

void removeReqFromListNoLock(stMbusPacketVariables_t *pstMBusRequesPacket)
{
	if(NULL == pstMBusRequesPacket)
	{
		return;
	}

	// Get a lock to list
	//Osal_Wait_Mutex(g_stReqList.m_mutexReqList, 0);
	{
		stMbusPacketVariables_t *pstPrev = pstMBusRequesPacket->__prev;
		stMbusPacketVariables_t *pstNext = pstMBusRequesPacket->__next;

		if(NULL != pstPrev)
		{
			pstPrev->__next = pstNext;
		}
		if(NULL != pstNext)
		{
			pstNext->__prev = pstPrev;
		}
		if(pstMBusRequesPacket == g_stReqList.m_pstStart)
		{
			g_stReqList.m_pstStart = pstNext;
		}
		if(pstMBusRequesPacket == g_stReqList.m_pstLast)
		{
			g_stReqList.m_pstLast = pstPrev;
		}

		pstMBusRequesPacket->__prev = NULL;
		pstMBusRequesPacket->__next = NULL;
	}
	//Osal_Release_Mutex(g_stReqList.m_mutexReqList);
}

void removeReqFromListWithLock(stMbusPacketVariables_t *pstMBusRequesPacket)
{
	if(NULL == pstMBusRequesPacket)
	{
		return;
	}

	// Get a lock to list
	Osal_Wait_Mutex(g_stReqList.m_mutexReqList, 0);
	removeReqFromListNoLock(pstMBusRequesPacket);
	Osal_Release_Mutex(g_stReqList.m_mutexReqList);
}


void addToRespQ(stMbusPacketVariables_t *a_pstReq)
{
	if(NULL != a_pstReq)
	{
		Post_Thread_Msg_t stPostThreadMsg = { 0 };
		a_pstReq->m_ulRespRcvdTimebyStack = get_nanos();
		// Add to queue
		stPostThreadMsg.idThread = g_stRespProcess.m_i32RespMsgQueId;
		stPostThreadMsg.wParam = NULL;
		stPostThreadMsg.lParam = a_pstReq;
		stPostThreadMsg.MsgType = a_pstReq->m_lPriority;

		if(!OSAL_Post_Message(&stPostThreadMsg))
		{
			//OSAL_Free(a_pstReq);
			freeReqNode(a_pstReq);
		}
		else
		{
			// Signal response timeout thread
			sem_post(&g_stRespProcess.m_semaphoreResp);
		}
	}
}

void* resquestTimeOutThreadFunction(void* threadArg)
{
	stMbusPacketVariables_t *pstTemp = NULL;
	stMbusPacketVariables_t *pstCur = NULL;

	while(false == g_bThreadExit)
	{
		//unsigned long tm1, tm2;
		//tm1 = get_nanos();
		//printf("\n");
		// Get a lock to list
		Osal_Wait_Mutex(g_stReqList.m_mutexReqList, 0);
		pstTemp = g_stReqList.m_pstStart;
		while(NULL != pstTemp)
		{
			pstCur = pstTemp;
			pstTemp = pstTemp->__next;

			if(REQ_SENT_ON_NETWORK == pstCur->m_state)
			{
				--pstCur->m_u32MsTimeout;
				if(pstCur->m_u32MsTimeout == 0)
				{
					removeReqFromListNoLock(pstCur);
					pstCur->m_state = RESP_TIMEDOUT;
					pstCur->m_u8ProcessReturn = STACK_ERROR_RECV_TIMEOUT;
					pstCur->m_stMbusRxData.m_u8Length = 0;
					addToRespQ(pstCur);
					pstCur->m_ulRespProcess = get_nanos();
				}
			}
		}
		Osal_Release_Mutex(g_stReqList.m_mutexReqList);
		//tm2 = get_nanos();
		// Sleep for 1 msec
		usleep(1000);

		/// check for thread exit
		if(g_bThreadExit)
		{
			break;
		}
	}

	return NULL;
}

void* postResponseToApp(void* threadArg)
{
	Linux_Msg_t stScMsgQue = { 0 };
	stMbusPacketVariables_t *pstMBusRequesPacket = NULL;
	int32_t i32RetVal = 0;
	while(false == g_bThreadExit)
	{
		sem_wait(&g_stRespProcess.m_semaphoreResp);
		memset(&stScMsgQue,00,sizeof(stScMsgQue));
		i32RetVal = 0;
		i32RetVal = OSAL_Get_NonBlocking_Message(&stScMsgQue, g_stRespProcess.m_i32RespMsgQueId);
		if(i32RetVal > 0)
		{
			pstMBusRequesPacket = stScMsgQue.lParam;
			if(NULL != pstMBusRequesPacket)
			{
				pstMBusRequesPacket->m_ulRespSentTimebyStack = get_nanos();
				ApplicationCallBackHandler(pstMBusRequesPacket, pstMBusRequesPacket->m_u8ProcessReturn);
				//OSAL_Free(pstMBusRequesPacket);
				freeReqNode(pstMBusRequesPacket);
			}
		}
		/// check for thread exit
		if(g_bThreadExit)
		{
			break;
		}
	}

	return NULL;
}

int initReqListData()
{
	g_stReqList.m_mutexReqList = Osal_Mutex();
	g_stReqList.m_pstStart = NULL;
	g_stReqList.m_pstLast = NULL;

	g_stRespProcess.m_i32RespMsgQueId = OSAL_Init_Message_Queue();

	if(-1 == sem_init(&g_stRespProcess.m_semaphoreResp, 0, 0 /* Initial value of zero*/))
	{
	   //printf("initReqListData::Could not create unnamed semaphore\n");
	   return -1;
	}

	// Initiate request thread
	{
		thread_Create_t stThreadParam = { 0 };
		stThreadParam.dwStackSize = 0;
		stThreadParam.lpStartAddress = resquestTimeOutThreadFunction;
		stThreadParam.lpParameter = &g_stRespProcess.m_i32RespMsgQueId;
		stThreadParam.lpThreadId = &g_stReqList.m_threadIdReqTimeout;

		g_stReqList.m_threadIdReqTimeout = Osal_Thread_Create(&stThreadParam);
	}

	// Initiate response thread
	{
		thread_Create_t stThreadParam1 = { 0 };
		stThreadParam1.dwStackSize = 0;
		stThreadParam1.lpStartAddress = postResponseToApp;
		stThreadParam1.lpParameter = &g_stRespProcess.m_i32RespMsgQueId;
		stThreadParam1.lpThreadId = &g_stRespProcess.m_threadIdRespToApp;

		g_stRespProcess.m_threadIdRespToApp = Osal_Thread_Create(&stThreadParam1);
	}
	return 0;
}

void addReqToList(stMbusPacketVariables_t *pstMBusRequesPacket)
{
	if(NULL == pstMBusRequesPacket)
	{
		return;
	}
	pstMBusRequesPacket->__next = NULL;

	// Get a lock to list
	Osal_Wait_Mutex(g_stReqList.m_mutexReqList, 0);
	pstMBusRequesPacket->__prev = g_stReqList.m_pstLast;
	if(NULL == g_stReqList.m_pstStart)
	{
		g_stReqList.m_pstStart = pstMBusRequesPacket;
		g_stReqList.m_pstLast = pstMBusRequesPacket;
	}
	else
	{
		g_stReqList.m_pstLast->__next = pstMBusRequesPacket;
		g_stReqList.m_pstLast = pstMBusRequesPacket;
	}
	Osal_Release_Mutex(g_stReqList.m_mutexReqList);
}

stMbusPacketVariables_t* searchReqList(uint8_t a_u8UnitID, uint16_t a_u16TransactionID)
{
	stMbusPacketVariables_t *pstTemp = NULL;
	// Get a lock to list
	Osal_Wait_Mutex(g_stReqList.m_mutexReqList, 0);
	pstTemp = g_stReqList.m_pstStart;
	while(NULL != pstTemp)
	{
		if(a_u8UnitID == pstTemp->m_u8UnitID && a_u16TransactionID == pstTemp->m_u16TransactionID)
		{
			// Request found
			break;
		}
		pstTemp = pstTemp->__next;
	}
	Osal_Release_Mutex(g_stReqList.m_mutexReqList);
	return pstTemp;
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
	uint8_t u8ReturnType = 0;
	stLiveSerSessionList_t *pstLivSerSesslist = NULL;
	stMbusPacketVariables_t *pstMBusRequesPacket = NULL;
	int32_t i32MsgQueIdSSTC = 0;
	int32_t i32RetVal = 0;
	//int32_t i32sockfd = 0;
	IP_Connect_t stIPConnect;

	stIPConnect.m_bIsAddedToEPoll = false;
	stIPConnect.m_sockfd = 0;
	stIPConnect.m_retryCount = 0;
	stIPConnect.m_timeOut = 200;
	stIPConnect.m_lastConnectStatus = SOCK_NOT_CONNECTED;

	i32MsgQueIdSSTC = *((int32_t *)threadArg);

	//while (NULL != threadArg)
	while(false == g_bThreadExit)
	{
		memset(&stScMsgQue,00,sizeof(stScMsgQue));
		i32RetVal = 0;
		i32RetVal = OSAL_Get_Message(&stScMsgQue, i32MsgQueIdSSTC);

		if(i32RetVal > 0)
		{
			pstLivSerSesslist = stScMsgQue.wParam;
			pstMBusRequesPacket = stScMsgQue.lParam;
			if(NULL != pstMBusRequesPacket)
			{
				if(NULL != pstLivSerSesslist)
				{
					pstMBusRequesPacket->m_u32MsTimeout = g_lResponseTimeout/1000; /// convert to millisecond
					addReqToList(pstMBusRequesPacket);
					pstMBusRequesPacket->m_ulReqSentTimeByStack = get_nanos();
					u8ReturnType = Modbus_SendPacket(pstMBusRequesPacket, &stIPConnect);
					pstMBusRequesPacket->m_ulReqProcess = get_nanos();
					pstMBusRequesPacket->m_u8ProcessReturn = u8ReturnType;
					if(STACK_NO_ERROR == u8ReturnType)
					{
						pstMBusRequesPacket->m_state = REQ_SENT_ON_NETWORK;
					}
					else
					{
						pstMBusRequesPacket->m_state = REQ_PROCESS_ERROR;
						removeReqFromListWithLock(pstMBusRequesPacket);
						addToRespQ(pstMBusRequesPacket);
					}

				}
			}
		}

		/// check for thread exit
		if(g_bThreadExit)
		{
			break;
		}
	}

	if(stIPConnect.m_sockfd)
	{
		close(stIPConnect.m_sockfd);
	}
	return NULL;
}

void addToHandleRespQ(stTcpRecvData_t *a_pstReq)
{
	if(NULL != a_pstReq)
	{
		mesg_data_t stLocalData;
		size_t MsgSize = 0;
		memcpy_s(stLocalData.m_readBuffer,sizeof(stLocalData.m_readBuffer),
				a_pstReq->m_readBuffer,sizeof(a_pstReq->m_readBuffer));

		stLocalData.mesg_type = 1;
		MsgSize = sizeof(stLocalData) - sizeof(long);
		int32_t iStatus = msgsnd(g_stHandleResp.m_i32HandleRespMsgQueId, &stLocalData, MsgSize, 0);

		if(iStatus == 0)
		{
			// Signal response timeout thread
			sem_post(&g_stHandleResp.m_semaphoreHandleResp);
		}
		else
		{
			static int count = 0;
			printf("Post failed error = %d,%d count = %d\n", iStatus, errno, count++);
		}
	}
	return;
}

void* handleClientReponseThreadFunction(void* threadArg)
{

	stMbusPacketVariables_t *pstMBusRequesPacket = NULL;

	// TCP IP message format
	// 2 bytes = TxID, 2 bytes = Protocol ID, 2 bytes = length, 1 byte = unit id
	// Get TxID and unit-id from response
	uByteOrder_t ustByteOrder = {0};

	// Holds the received transaction ID
	uint16_t u16TransactionID = 0;

	// Holds the unit id
	uint8_t  u8UnitID = 0;

	mesg_data_t *pstLocalData;
	mesg_data_t stLocalData;
	int32_t i32RetVal = 0;
	while(false == g_bThreadExit)
	{
		sem_wait(&g_stHandleResp.m_semaphoreHandleResp);
		memset(&stLocalData,00,sizeof(stLocalData));
		i32RetVal = 0;

		size_t MsgSize = 0;

		MsgSize = sizeof(stLocalData) - sizeof(long);

		i32RetVal = msgrcv(g_stHandleResp.m_i32HandleRespMsgQueId, &stLocalData, MsgSize, 0, MSG_NOERROR | IPC_NOWAIT);
		if(errno == ENOMSG && (-1 == i32RetVal))
		   	i32RetVal = -1;

		if(i32RetVal > 0)
		{
			///pstRecvData = stScMsgQue.lParam;
			pstLocalData = &stLocalData;
			if(NULL != pstLocalData)
			{
				// Transaction ID
				ustByteOrder.u16Word = 0;
				ustByteOrder.TwoByte.u8ByteTwo = pstLocalData->m_readBuffer[0];
				ustByteOrder.TwoByte.u8ByteOne = pstLocalData->m_readBuffer[1];
				u16TransactionID = ustByteOrder.u16Word;

				// Get unit id
				u8UnitID = pstLocalData->m_readBuffer[6];

				pstMBusRequesPacket = searchReqList(u8UnitID, u16TransactionID);
				if(NULL != pstMBusRequesPacket)
				{
					pstMBusRequesPacket->m_state = RESP_RCVD_FROM_NETWORK;
					removeReqFromListWithLock(pstMBusRequesPacket);
					pstMBusRequesPacket->m_u8ProcessReturn = DecodeRxPacket(pstLocalData->m_readBuffer, pstMBusRequesPacket);
					addToRespQ(pstMBusRequesPacket);
					pstMBusRequesPacket->m_ulRespProcess = get_nanos();
				}
				else
				{
					//printf("handleClientReponseThreadFunction: not found: %d %d\n", u8UnitID, u16TransactionID);
				}
			}
		}
		/// check for thread exit
		if(g_bThreadExit)
		{
			break;
		}
	}

	return NULL;
}

int initHandleResponseContext()
{
	g_stHandleResp.m_i32HandleRespMsgQueId = OSAL_Init_Message_Queue();

	if(-1 == sem_init(&g_stHandleResp.m_semaphoreHandleResp, 0, 0 /* Initial value of zero*/))
	{
	   //printf("initHandleResponseContext::Could not create unnamed semaphore\n");
	   return -1;
	}

	// Initiate response thread
	{
		thread_Create_t stThreadParam1 = { 0 };
		stThreadParam1.dwStackSize = 0;
		stThreadParam1.lpStartAddress = handleClientReponseThreadFunction;
		stThreadParam1.lpParameter = &g_stHandleResp.m_i32HandleRespMsgQueId;
		stThreadParam1.lpThreadId = &g_stHandleResp.m_threadIdHandleResp;

		g_stHandleResp.m_threadIdHandleResp = Osal_Thread_Create(&stThreadParam1);
	}
	return 0;
}
#endif
