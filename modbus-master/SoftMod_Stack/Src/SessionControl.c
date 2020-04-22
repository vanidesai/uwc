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

#include <stdio.h>
#include <safe_lib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdatomic.h>
#include <sys/epoll.h> // for epoll_create1(), epoll_ctl(), struct epoll_event
#include <time.h>
#include "SessionControl.h"

//#define MODBUS_STACK_TCPIP_ENABLED
struct stReqManager g_objReqManager;

// global variable to store stack configurations
extern stDevConfig_t g_stModbusDevConfig;

#define MODBUS_HEADER_LENGTH 6 //no. of bytes till length paramater in header
//array of accepted clients
#define MAX_DEVICE_PER_SITE 300

#ifndef MODBUS_STACK_TCPIP_ENABLED

extern int fd;

#endif

int32_t i32MsgQueIdSC = 0;
extern bool g_bThreadExit;

#define READ_SIZE 1024
#define MAXEVENTS 100
#define EPOLL_TIMEOUT 1000
#define MAXQUEUE 5

#ifdef MODBUS_STACK_TCPIP_ENABLED
extern Mutex_H LivSerSesslist_Mutex;
stTcpRecvData_t m_clientAccepted[MAX_DEVICE_PER_SITE] = {{0}};
int m_epollFd = 0;
struct epoll_event *m_events = NULL;
Thread_H EpollRecv_ThreadId = 0;
Mutex_H EPollMutex;
stLiveSerSessionList_t *pstSesCtlThdLstHead = NULL;
struct stTimeOutTracker g_oTimeOutTracker = {0};

struct stIntDataForQ
{
	long m_lType;       /* message type, must be > 0 */
	unsigned short m_iID;
};
#endif

/**
 *
 * Description
 * Looks for an availale node in request array.
 * It reserves the available node for using this node.
 *
 * @param - none
 * @returns none
 *
 */
long getAvailableReqNode()
{
	//Osal_Wait_Mutex(g_objReqManager.m_mutexReqArray);

	// addressed review comment
	if(0 != Osal_Wait_Mutex(g_objReqManager.m_mutexReqArray))
	{
		// fail to lock mutex
		return -1;
	}
	long index = -1;
	for (long iLoop = 0; iLoop < MAX_REQUESTS; iLoop++)
	{
		eTransactionState expected = IdleState;
		stMbusPacketVariables_t* ptr = &g_objReqManager.m_objReqArray[iLoop];
		if(true ==
			atomic_compare_exchange_strong(&ptr->m_state, &expected, RESERVED))
		{
			index = iLoop;
			break;
		}
	}
	Osal_Release_Mutex(g_objReqManager.m_mutexReqArray);
	return index;
}

/**
 *
 * Description
 * Resets the request node to default values.
 *
 * @param - node to reset
 * @returns none
 *
 */
void resetReqNode(stMbusPacketVariables_t* a_pObjReqNode)
{
	if(NULL != a_pObjReqNode)
	{
#ifdef MODBUS_STACK_TCPIP_ENABLED
		a_pObjReqNode->__next = NULL;
		a_pObjReqNode->__prev = NULL;
#endif
		// Init timestamps to 0
		a_pObjReqNode->m_objTimeStamps.tsReqRcvd = (struct timespec){0};
		a_pObjReqNode->m_objTimeStamps.tsReqSent = (struct timespec){0};
		a_pObjReqNode->m_objTimeStamps.tsRespRcvd = (struct timespec){0};
		a_pObjReqNode->m_objTimeStamps.tsRespSent = (struct timespec){0};

		a_pObjReqNode->m_iTimeOutIndex = -1;
		a_pObjReqNode->m_state = IdleState;
	}
	else
	{
		printf("Null pointer to reset\n");
	}
}

/**
 *
 * Description
 * Initiate request manager and data structures within it.
 *
 * @param - none
 * @returns true/false
 *
 */
bool initReqManager()
{
	unsigned int iCount = 0;
	for (; iCount < MAX_REQUESTS; iCount++)
	{
		g_objReqManager.m_objReqArray[iCount].m_ulMyId = iCount;
		resetReqNode(&(g_objReqManager.m_objReqArray[iCount]));
	}

	g_objReqManager.m_mutexReqArray = Osal_Mutex();
	if(NULL == g_objReqManager.m_mutexReqArray )
	{
		return false;
	}
	return true;
}

/**
 *
 * Description
 * emplace new request in request queue
 *
 * @param - tsReqRcvd - request received time-stamp
 * @returns pointer to stMbusPacketVariables_t
 *
 */
stMbusPacketVariables_t* emplaceNewRequest(const struct timespec tsReqRcvd)
{
	stMbusPacketVariables_t* ptr = NULL;
	long iCount = getAvailableReqNode();
	if ((iCount >= 0) && (iCount < MAX_REQUESTS))
	{
		eTransactionState expected = RESERVED;
		ptr = &g_objReqManager.m_objReqArray[iCount];
		if(true ==
				atomic_compare_exchange_strong(&ptr->m_state, &expected, REQ_RCVD_FROM_APP))
		{
#ifdef MODBUS_STACK_TCPIP_ENABLED
			ptr->__next = NULL;
			ptr->__prev = NULL;
#endif
			ptr->m_ulMyId = iCount;

			// copy the req recvd timestamp
			memcpy_s(&(ptr->m_objTimeStamps.tsReqRcvd), sizeof(struct timespec),
					&tsReqRcvd, sizeof(struct timespec));

			// Init other timestamps to 0
			ptr->m_objTimeStamps.tsReqSent = (struct timespec){0};
			ptr->m_objTimeStamps.tsRespRcvd = (struct timespec){0};
			ptr->m_objTimeStamps.tsRespSent = (struct timespec){0};

			ptr->m_iTimeOutIndex = -1;
		}
	}
	return ptr;
}

/**
 *
 * Description
 * emplace new request in request queue
 *
 * @param - a_pObjTempReq
 * @returns pointer to stMbusPacketVariables_t
 *
 */
void freeReqNode(stMbusPacketVariables_t* a_pobjReq)
{
#ifdef MODBUS_STACK_TCPIP_ENABLED
	//removeReqFromListWithLock(a_pobjReq);
	releaseFromTracker(a_pobjReq);
#endif
	resetReqNode(a_pobjReq);
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

	// set thread priority
	set_thread_sched_param();

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
			//Osal_Wait_Mutex (LivSerSesslist_Mutex);

			// addressed review comment
			if(0 != Osal_Wait_Mutex(LivSerSesslist_Mutex))
			{
				// fail to lock mutex
				continue;
			}
			pstLivSerSesslist = pstSesCtlThdLstHead;
			if(NULL == pstLivSerSesslist)
			{
				pstSesCtlThdLstHead = OSAL_Malloc(sizeof(stLiveSerSessionList_t));
				if(NULL == pstSesCtlThdLstHead)
				{
					ApplicationCallBackHandler(pstMBusReqPact,
							STACK_ERROR_MALLOC_FAILED);

					// addressed review comment
					if(0 != Osal_Release_Mutex (LivSerSesslist_Mutex))
					{
						// fail to unlock mutex
						continue;
					}

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
						//Osal_Release_Mutex (LivSerSesslist_Mutex);
						// addressed review comment
						if(0 != Osal_Release_Mutex (LivSerSesslist_Mutex))
						{
							// fail to unlock mutex
							continue;
						}
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

				if(-1 == pstLivSerSesslist->MsgQId)
				{
					printf("failed to create msg queue\n");
					ApplicationCallBackHandler(pstMBusReqPact, STACK_ERROR_QUEUE_CREATE);

					// addressed review comment
					if(0 != Osal_Release_Mutex (LivSerSesslist_Mutex))
					{
						// fail to unlock mutex
						continue;
					}
					freeReqNode(pstMBusReqPact);
					continue;
				}
				pstLivSerSesslist->m_i32sockfd = 0;

				pstLivSerSesslist->m_u16Port = pstMBusReqPact->u16Port;
				stPostThreadMsg.idThread = pstLivSerSesslist->MsgQId;
				stPostThreadMsg.lParam = pstMBusReqPact;
				stPostThreadMsg.wParam = pstLivSerSesslist;
				stPostThreadMsg.MsgType = pstMBusReqPact->m_lPriority;

				if(!OSAL_Post_Message(&stPostThreadMsg))
				{
					ApplicationCallBackHandler(pstMBusReqPact, STACK_ERROR_QUEUE_SEND);
					//Osal_Release_Mutex (LivSerSesslist_Mutex);
					// addressed review comment
					if(0 != Osal_Release_Mutex (LivSerSesslist_Mutex))
					{
						// fail to unlock mutex
						continue;
					}
					freeReqNode(pstMBusReqPact);
					continue;
				}

				stThreadParam.dwStackSize = 0;
				stThreadParam.lpStartAddress = ServerSessTcpAndCbThread;
				stThreadParam.lpParameter = &pstLivSerSesslist->MsgQId;
				stThreadParam.lpThreadId = &pstLivSerSesslist->m_ThreadId;

				pstLivSerSesslist->m_ThreadId = Osal_Thread_Create(&stThreadParam);
				if(-1 == pstLivSerSesslist->m_ThreadId)
				{
					ApplicationCallBackHandler(pstMBusReqPact, STACK_ERROR_THREAD_CREATE);
					//Osal_Release_Mutex (LivSerSesslist_Mutex);
					// addressed review comment
					if(0 != Osal_Release_Mutex (LivSerSesslist_Mutex))
					{
						// fail to unlock mutex
						continue;
					}
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
					//Osal_Release_Mutex (LivSerSesslist_Mutex);
					// addressed review comment
					if(0 != Osal_Release_Mutex (LivSerSesslist_Mutex))
					{
						// fail to unlock mutex
						continue;
					}
					freeReqNode(pstMBusReqPact);
					continue;
				}
			}
			//Osal_Release_Mutex (LivSerSesslist_Mutex);
			// addressed review comment
			if(0 != Osal_Release_Mutex (LivSerSesslist_Mutex))
			{
				// fail to unlock mutex
				continue;
			}
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

/**
 *
 * Description
 * get client socker id from already established client's list
 *
 * @param socketID [in] socket id
 * @return int 	[out] client index from list
 */
int getClientIdFromList(int socketID)
{
	int socketIndex = -1;

	for(int i = 0; i < MAX_DEVICE_PER_SITE; i++)
	{
		if(NULL != m_clientAccepted[i].m_pstConRef)
		{
			if(m_clientAccepted[i].m_pstConRef->m_sockfd == socketID)
			{
				return i;
			}
		}
	}

	return socketIndex;
}

/**
 *
 * Description
 * reset client struct after data is read from the socket
 *
 * @param clientAccepted [in] pointer to stTcpRecvData_t holding socket's data received information
 * @return void	[out] none
 */
void resetEPollClientDataStruct(stTcpRecvData_t* clientAccepted)
{
	if(NULL != clientAccepted)
	{
	clientAccepted->m_bytesRead = 0;
	clientAccepted->m_bytesToBeRead = 0;
	clientAccepted->m_len = 0;
	memset(&clientAccepted->m_readBuffer, 0,
			sizeof(clientAccepted->m_readBuffer));
	}
}

/**
 *
 * Description
 * Initializes data structures needed for epoll operation for receiving TCP data
 *
 * @param None
 * @return true/false based on status
 */
bool initEPollData()
{
	for(int i = 0; i < MAX_DEVICE_PER_SITE; i++)
	{
		resetEPollClientDataStruct(&m_clientAccepted[i]);
	}

	//init epoll
	m_epollFd = epoll_create1(0);
	if (m_epollFd == -1) {
		perror("Failed to create epoll file descriptor :: \n");
		return false;
	}

	thread_Create_t stEpollRecvThreadParam = { 0 };
	stEpollRecvThreadParam.dwStackSize = 0;
	stEpollRecvThreadParam.lpStartAddress = EpollRecvThread;
	stEpollRecvThreadParam.lpThreadId = &EpollRecv_ThreadId;

	EpollRecv_ThreadId = Osal_Thread_Create(&stEpollRecvThreadParam);
	if(-1 == EpollRecv_ThreadId)
	{
		return false;
	}
	EPollMutex = Osal_Mutex();
	if(NULL == EPollMutex)
	{
		return false;
	}
}

/**
 *
 * Description
 * De-initializes data structures related to epoll operation for receiving TCP data
 *
 * @param None
 * @return None
 */
void deinitEPollData()
{
	Osal_Thread_Terminate(EpollRecv_ThreadId);

	for(int i = 0; i < MAX_DEVICE_PER_SITE; i++)
	{
		resetEPollClientDataStruct(&m_clientAccepted[i]);
		m_clientAccepted[i].m_pstConRef = NULL;
	}

	Osal_Close_Mutex(EPollMutex);
}

/**
 *
 * Description
 * Stops listening on a socket with epoll.
 * Resets associated data structures.
 * The calling function should have the lock for epoll data structure.
 *
 * @param Reference to epoll data structure
 * @return None
 */
void removeEPollRefNoLock(int a_iIndex)
{
	if((a_iIndex >= 0) && (a_iIndex < MAX_DEVICE_PER_SITE))
	{
		// Obtain lock
		if(NULL != m_clientAccepted[a_iIndex].m_pstConRef)
		{
			// remove from epoll
			struct epoll_event m_event;
			m_event.events = EPOLLIN;
			m_event.data.fd = m_clientAccepted[a_iIndex].m_pstConRef->m_sockfd;
			if (epoll_ctl(m_epollFd, EPOLL_CTL_DEL, m_clientAccepted[a_iIndex].m_pstConRef->m_sockfd, &m_event))
			{
				perror("Failed to delete file descriptor from epoll:");
			}
			m_clientAccepted[a_iIndex].m_pstConRef = NULL;
		}
		resetEPollClientDataStruct(&m_clientAccepted[a_iIndex]);
		m_clientAccepted[a_iIndex].m_pstConRef = NULL;
	}
}

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
void removeEPollRef(int a_iIndex)
{
	if((a_iIndex >= 0) && (a_iIndex < MAX_DEVICE_PER_SITE))
	{
		//Osal_Wait_Mutex(EPollMutex);
		// addressed review comment
		if(0 != Osal_Wait_Mutex(EPollMutex))
		{
			// fail to lock mutex
			return;
		}
		removeEPollRefNoLock(a_iIndex);
		//Osal_Release_Mutex(EPollMutex);
		// addressed review comment
		if(0 != Osal_Release_Mutex (EPollMutex))
		{
			// fail to unlock mutex
			return;
		}
		//printf("Removing ref %d\n", a_iIndex);
	}
}

/**
 * Description
 * Set socket failure state
 *
 * @param stIPConnect [in] pointer to struct of type IP_Connect_t
 *
 * @return void [out] none
 *
 */
int addtoEPollList(IP_Connect_t *a_pstIPConnect)
{
	if(NULL != a_pstIPConnect)
	{
		int ret = -1;
		//Osal_Wait_Mutex(EPollMutex);

		// addressed review comment
		if(0 != Osal_Wait_Mutex(EPollMutex))
		{
			// fail to lock mutex
			return ret;
		}
		int index = -1;
		for(int i = 0; i < MAX_DEVICE_PER_SITE; i++)
		{
			// Check if this connection ref is already in list
			if(m_clientAccepted[i].m_pstConRef == a_pstIPConnect)
			{
				if(m_clientAccepted[i].m_pstConRef->m_sockfd == a_pstIPConnect->m_sockfd)
				{
					// Ref and socket are same. No action
				}
				else
				{
					// Sockets are not same. Remove earlier connection
					removeEPollRefNoLock(i);
				}
				index = i;
				break;
			}
			else if(m_clientAccepted[i].m_pstConRef == NULL)
			{
				// This is to find first empty node in array
				if(-1 == index)
				{
					index = i;
				}
			}
			else
			{
				// Node is not NULL
				if(m_clientAccepted[i].m_pstConRef->m_sockfd == a_pstIPConnect->m_sockfd)
				{
					// Sockets are same but connection references are different.
					// This should ideally not occur.
					printf("Error: Socket for EPOLL-ADD is already used for some other connection");
					removeEPollRefNoLock(i);
					closeConnection(m_clientAccepted[i].m_pstConRef);
					ret = -2;
					break;
				}
			}
		}

		// This fd is not yet added to list
		if (-1 != index)
		{
			resetEPollClientDataStruct(&m_clientAccepted[index]);
			m_clientAccepted[index].m_pstConRef = NULL;
			// add to epoll events
			struct epoll_event m_event;
			m_event.events = EPOLLIN;
			m_event.data.fd = a_pstIPConnect->m_sockfd;
			ret = 0;
			if (epoll_ctl(m_epollFd, EPOLL_CTL_ADD, a_pstIPConnect->m_sockfd, &m_event))
			{
				if(EEXIST != errno)
				{
					perror("Failed to add file descriptor to epoll:");
					ret = -1;
				}
			}
			if(ret == 0)
			{
				// Set 2 way-references
				m_clientAccepted[index].m_pstConRef = a_pstIPConnect;
				a_pstIPConnect->m_iRcvConRef = index;
				ret = index;
				//printf("adding ref: %d", ret);
			}
		}
		//Osal_Release_Mutex(EPollMutex);
		if(0 != Osal_Release_Mutex (EPollMutex))
		{
			// fail to unlock mutex
			return -1;
		}
		return ret;
	}
	return -1;
}

/**
 *
 * Description
 * reset client struct after data is read from the socket
 *
 * @param - none
 * @return struct [out] epoch time struct
 */
struct timespec getEpochTime() {

	struct timespec ts;
    timespec_get(&ts, TIME_UTC);

    return ts;
}

/**
 *
 * Description
 * thread to start polling on sockets o receive data
 *
 * @param - none
 * @return void [out] none
 */
void* EpollRecvThread()
{
	int event_count = 0;
	size_t bytes_read = 0;
	size_t bytes_to_read = 0;

	// set thread priority
	set_thread_sched_param();

	//allocate no of polling events
	m_events = (struct epoll_event*) calloc(MAXEVENTS, sizeof(struct epoll_event));

	if(NULL == m_events)
	{
		return NULL;
	}

	while (true != g_bThreadExit)
	{
		event_count = 0;

		event_count = epoll_wait(m_epollFd, m_events, MAXEVENTS, EPOLL_TIMEOUT);
		//Osal_Wait_Mutex(EPollMutex);

		// addressed review comment
		if(0 != Osal_Wait_Mutex(EPollMutex))
		{
			// fail to lock mutex
			continue;
		}

		for (int i = 0; i < event_count; i++)
		{
			int clientID = getClientIdFromList(m_events[i].data.fd);

			if (clientID == -1)
			{
				continue;
			}
			if(NULL == m_clientAccepted[clientID].m_pstConRef)
			{
				continue;
			}

			//if there are still bytes remaining to be read for this socket, adjust read size accordingly
			if (m_clientAccepted[clientID].m_bytesToBeRead > 0)
			{
				bytes_to_read = m_clientAccepted[clientID].m_bytesToBeRead;
			}
			else
			{
				bytes_to_read = MODBUS_HEADER_LENGTH; //read only header till length
			}

			//receive data from socket
			bytes_read = recv(m_clientAccepted[clientID].m_pstConRef->m_sockfd,
					m_clientAccepted[clientID].m_readBuffer+m_clientAccepted[clientID].m_bytesRead, bytes_to_read, 0);


			if (bytes_read == 0)
			{
				// No data received, check for new socket
				continue;
			}

			//parse length if fd has 0 bytes read, otherwise rest packets are with actual data and not header
			if (m_clientAccepted[clientID].m_bytesRead == 0)
			{
				m_clientAccepted[clientID].m_len = (m_clientAccepted[clientID].m_readBuffer[4] << 8
						| m_clientAccepted[clientID].m_readBuffer[5]);
			}

			if (bytes_to_read == bytes_read)
			{
				m_clientAccepted[clientID].m_bytesRead += bytes_read;

				m_clientAccepted[clientID].m_bytesToBeRead =
						(m_clientAccepted[clientID].m_len + MODBUS_HEADER_LENGTH)
								- m_clientAccepted[clientID].m_bytesRead;
			}

			//add one more recv to read response completely
			if (m_clientAccepted[clientID].m_bytesToBeRead > 0)
			{
				while (m_clientAccepted[clientID].m_bytesToBeRead != 0)
					{
					bytes_to_read = m_clientAccepted[clientID].m_bytesToBeRead;

						//receive data from socket
					bytes_read = recv(m_clientAccepted[clientID].m_pstConRef->m_sockfd,
								m_clientAccepted[clientID].m_readBuffer+m_clientAccepted[clientID].m_bytesRead,
								bytes_to_read, 0);

					if(bytes_read < 0)
					{
						//recv failed with error
						perror("Recv() failed : ");
						//Close the connection and mark socket invalid
						removeEPollRefNoLock(clientID);
						closeConnection(m_clientAccepted[clientID].m_pstConRef);

						break;
					}
					else
					{
						if (bytes_read == 0)
						{
							// there is no data read. Let's check new thing
							break;
						}

						m_clientAccepted[clientID].m_bytesRead += bytes_read;

						m_clientAccepted[clientID].m_bytesToBeRead =
								(m_clientAccepted[clientID].m_len + MODBUS_HEADER_LENGTH)
										- m_clientAccepted[clientID].m_bytesRead;
					}
				}
				//should have read response completely

				if ((m_clientAccepted[clientID].m_len + MODBUS_HEADER_LENGTH)
						== m_clientAccepted[clientID].m_bytesRead)
				{
					addToHandleRespQ(&m_clientAccepted[clientID]);

					//clear socket struct for next iteration
					resetEPollClientDataStruct(&m_clientAccepted[clientID]);

				}
				else
				{
					//this case should never happen ideally
					//continue to read for the same socket & epolling
					continue;
				}

			}
		}//for loop for sockets ends
		//Osal_Release_Mutex(EPollMutex);
		if(0 != Osal_Release_Mutex (EPollMutex))
		{
			// fail to unlock mutex
			continue;
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
struct stResProcessData g_stRespProcess;

/**
 *
 * Description
 * get time stamp in nano-seconds
 *
 * @param - none
 * @return unsigned long [out] time in nano-seconds
 */
unsigned long get_nanos(void) {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (unsigned long)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

/**
 *
 * Description
 * Remove request from list with no locks
 *
 * @param pstMBusRequesPacket [in] pointer to struct of type stMbusPacketVariables_t
 * @return void [out] none
 */
void releaseFromTrackerNode(stMbusPacketVariables_t *a_pstNodeToRemove,
		struct stTimeOutTrackerNode *a_pstTracker)
{
	if((NULL == a_pstNodeToRemove) || (NULL == a_pstTracker))
	{
		return;
	}

	{
		stMbusPacketVariables_t *pstPrev = a_pstNodeToRemove->__prev;
		stMbusPacketVariables_t *pstNext = a_pstNodeToRemove->__next;

		if(NULL != pstPrev)
		{
			pstPrev->__next = pstNext;
		}
		if(NULL != pstNext)
		{
			pstNext->__prev = pstPrev;
		}
		if(a_pstNodeToRemove == a_pstTracker->m_pstStart)
		{
			a_pstTracker->m_pstStart = pstNext;
		}
		if(a_pstNodeToRemove == a_pstTracker->m_pstLast)
		{
			a_pstTracker->m_pstLast = pstPrev;
		}

		a_pstNodeToRemove->__prev = NULL;
		a_pstNodeToRemove->__next = NULL;
	}
}


/**
 *
 * Description
 * Remove request from list with locks
 *
 * @param pstMBusRequesPacket [in] pointer to struct of type stMbusPacketVariables_t
 * @return void [out] none
 */
void releaseFromTracker(stMbusPacketVariables_t *pstMBusRequesPacket)
{
	if(NULL == pstMBusRequesPacket)
	{
		return;
	}
	// Timeout index is out of bounds
	if((pstMBusRequesPacket->m_iTimeOutIndex < 0) ||
			(pstMBusRequesPacket->m_iTimeOutIndex > g_oTimeOutTracker.m_iSize))
	{
		return;
	}
	struct stTimeOutTrackerNode *pstTemp = &g_oTimeOutTracker.m_pstArray[pstMBusRequesPacket->m_iTimeOutIndex];
	if(NULL == pstTemp)
	{
		printf("Error: Null timeout tracker node\n");
		return;
	}

	// Obtain the lock
	int expected = 0;
	do
	{
		expected = 0;
	} while(!atomic_compare_exchange_weak(&(pstTemp->m_iIsLocked), &expected, 1));

	// Lock is obtained

	releaseFromTrackerNode(pstMBusRequesPacket, pstTemp);
	pstMBusRequesPacket->m_iTimeOutIndex = -1;
	// Done. Release the lock
	pstTemp->m_iIsLocked = 0;
}

/**
 *
 * Description
 * Add msg to response queue
 *
 * @param a_pstReq [in] pointer to struct of type stMbusPacketVariables_t
 * @return void [out] none
 */
void addToRespQ(stMbusPacketVariables_t *a_pstReq)
{
	if(NULL != a_pstReq)
	{
		Post_Thread_Msg_t stPostThreadMsg = { 0 };
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

/**
 *
 * Description
 * Post response to application
 *
 * @param threadArg [in] void pointer
 * @return void [out] none
 */
void* postResponseToApp(void* threadArg)
{
	Linux_Msg_t stScMsgQue = { 0 };
	stMbusPacketVariables_t *pstMBusRequesPacket = NULL;
	int32_t i32RetVal = 0;

	// set thread priority
	set_thread_sched_param();

	while(false == g_bThreadExit)
	{
		//sem_wait(&g_stRespProcess.m_semaphoreResp);
		if((sem_wait(&g_stRespProcess.m_semaphoreResp)) == -1 && errno == EINTR)
		{
			// Continue if interrupted by handler
			continue;
		}
		memset(&stScMsgQue,00,sizeof(stScMsgQue));
		i32RetVal = 0;
		i32RetVal = OSAL_Get_NonBlocking_Message(&stScMsgQue, g_stRespProcess.m_i32RespMsgQueId);
		if(i32RetVal > 0)
		{
			pstMBusRequesPacket = stScMsgQue.lParam;
			if(NULL != pstMBusRequesPacket)
			{
				if(pstMBusRequesPacket->m_state == RESP_RCVD_FROM_NETWORK)
				{
					pstMBusRequesPacket->m_u8ProcessReturn = DecodeRxPacket(pstMBusRequesPacket->m_u8RawResp, pstMBusRequesPacket);
				}
				//pstMBusRequesPacket->m_ulRespSentTimebyStack = get_nanos();
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

/**
 *
 * Description
 * Timeout is implemented in the form of counter.
 * This function returns current counter.
 *
 * @param thread argument - none
 * @return none
 */
int getTimeoutTrackerCount()
{
	return g_oTimeOutTracker.m_iCounter;
}

/**
 *
 * Description
 * Thead function: Implements a timer to measure timeout for
 * initiated requests.
 *
 * @param thread argument - none
 * @return none
 */
void* timeoutTimerThread(void* threadArg)
{
	// set thread priority
	set_thread_sched_param();
	printf("In timeoutTimerThread\n");
	g_oTimeOutTracker.m_iCounter = 0;

	struct timespec ts;
	int rc = clock_getres(CLOCK_MONOTONIC, &ts);
	if(0 != rc)
	{
		perror("Unable to get clock resolution: ");
	}
	else
	{
		printf("Monotonic Clock resolution: %10ld.%09ld\n", (long) ts.tv_sec, ts.tv_nsec);
	}

	rc = clock_gettime(CLOCK_MONOTONIC, &ts);
	if(0 != rc)
	{
		perror("Stack fatal error: Response timeout: clock_gettime failed: ");
		return NULL;
	}
	while(false == g_bThreadExit)
	{
		unsigned long next_tick = (ts.tv_sec * 1000000000L + ts.tv_nsec) + 1*1000*1000;
		ts.tv_sec = next_tick / 1000000000L;
		ts.tv_nsec = next_tick % 1000000000L;
		do
		{
			rc = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL);
		} while(EINTR == rc);

		if(0 == rc)
		{
			g_oTimeOutTracker.m_iCounter = (g_oTimeOutTracker.m_iCounter + 1) % g_oTimeOutTracker.m_iSize;

			struct stIntDataForQ objSt;
			objSt.m_iID = g_oTimeOutTracker.m_iCounter;
			objSt.m_lType = 1;

			if(-1 != msgsnd(g_oTimeOutTracker.m_iTimeoutActionQ, &objSt, sizeof(objSt) - sizeof(long), 0))
			{
				sem_post(&g_oTimeOutTracker.m_semTimeout);
				//printf("to post %d - ", g_oTimeOutTracker.m_iCounter);
			}
			else
			{
				perror("Unable to post timeout timer action:");
			}
		}
		else
		{
			printf("Stack: Error in Response timeout function: %d\n", rc);
		}
	}

	return NULL;
}

/**
 *
 * Description
 * Thead function: Identifies timed out requests and 
 * initiates a response accordingly.
 *
 * @param thread argument - none
 * @return none
 */
void* timeoutActionThread(void* threadArg)
{
	printf("in timeoutActionThread thread\n");

	// set thread priority
	set_thread_sched_param();

	const int iArrayToTimeoutDiff = g_oTimeOutTracker.m_iSize - g_stModbusDevConfig.m_lResponseTimeout/1000;
	const size_t MsgSize = sizeof(struct stIntDataForQ) - sizeof(long);

	while(false == g_bThreadExit)
	{
		if((sem_wait(&g_oTimeOutTracker.m_semTimeout)) == -1 && errno == EINTR)
		{
			// Continue if interrupted by handler
			continue;
		}
		struct stIntDataForQ objSt = {0};
		int i32RetVal = msgrcv(g_oTimeOutTracker.m_iTimeoutActionQ, &objSt, MsgSize, 0, MSG_NOERROR | IPC_NOWAIT);
		//if(errno == ENOMSG && (-1 == i32RetVal))
		if(i32RetVal <= 0)
		{
			//i32RetVal = -1;
			perror("Unable to get data from timeout q");
			continue;
		}

		// get list corresponding to the index
		//printf("q index: %d\n", objSt.m_iID);
		// This is the current index
		// Find timed out index using following formula
		// timedOutIndex = [current index + (timeout-array-size - timeout)] % timeout-array-size
		int iTimedOutIndex = (objSt.m_iID + iArrayToTimeoutDiff) % g_oTimeOutTracker.m_iSize;

		// identify index into q
		if(iTimedOutIndex > g_oTimeOutTracker.m_iSize)
		{
			printf("Error: Timed out Index is greater than array size\n");
			continue;
		}
		int expected = 0;
		struct stTimeOutTrackerNode *pstTemp = &g_oTimeOutTracker.m_pstArray[iTimedOutIndex];
		if(NULL == pstTemp)
		{
			printf("Error: Null timeout tracker node\n");
			continue;
		}

		do
		{
			expected = 0;
		} while(!atomic_compare_exchange_weak(&(pstTemp->m_iIsLocked), &expected, 1));

		// Lock is obtained
		stMbusPacketVariables_t *pstNextNode = pstTemp->m_pstStart;
		while(NULL != pstNextNode)
		{
			stMbusPacketVariables_t *pstCur = pstNextNode;
			pstNextNode = pstNextNode->__next;
			eTransactionState expected = REQ_SENT_ON_NETWORK;
			if(true ==
					atomic_compare_exchange_strong(&pstCur->m_state, &expected, RESP_TIMEDOUT))
			{
				/*printf("Cur cnt:%d, Timedout cnt:%d, Node for timeout is found: %d, TxID: %d, %lu\n",
	                        objSt.m_iID, iTimedOutIndex,
	                        pstCur->m_iTimeOutIndex, pstCur->m_u16TransactionID,
	                        get_nanos());
				 */
				pstCur->m_u8ProcessReturn = STACK_ERROR_RECV_TIMEOUT;
				pstCur->m_stMbusRxData.m_u8Length = 0;
				// Init resp received timestamp
				timespec_get(&(pstCur->m_objTimeStamps.tsRespRcvd), TIME_UTC);
				addToRespQ(pstCur);
			}
		}
		// Done. Release the lock
		pstTemp->m_iIsLocked = 0;
	}
	return NULL;
}

/**
 *
 * Description
 * Initialize timeout tracker data structure and threads
 *
 * @param none
 * @return int [out] 0 (if success)
 * 					-1 (if failure)
 */
int initTimeoutTrackerArray()
{
	g_oTimeOutTracker.m_iTimeoutActionQ = OSAL_Init_Message_Queue();
	if(-1 == g_oTimeOutTracker.m_iTimeoutActionQ)
	{
		return -1;
	}

	if(-1 == sem_init(&g_oTimeOutTracker.m_semTimeout, 0, 0 /* Initial value of zero*/))
	{
		perror("Timeout tracker: semaphore creaton error: ");
		return -1;
	}

	// determine size of timeout tracker array
	g_oTimeOutTracker.m_iSize = g_stModbusDevConfig.m_lResponseTimeout/1000 + ADDITIONAL_RECORDS_TIMEOUT_TRACKER;
	if(g_oTimeOutTracker.m_iSize % REQ_ARRAY_MULTIPLIER)
	{
		g_oTimeOutTracker.m_iSize =
				g_oTimeOutTracker.m_iSize + (REQ_ARRAY_MULTIPLIER - g_oTimeOutTracker.m_iSize % REQ_ARRAY_MULTIPLIER);
	}
	printf("Timeout tracker array size: %d\n", g_oTimeOutTracker.m_iSize);

	if(g_oTimeOutTracker.m_iSize > 0)
	{
		g_oTimeOutTracker.m_pstArray =
			(struct stTimeOutTrackerNode*)calloc(g_oTimeOutTracker.m_iSize, sizeof(struct stTimeOutTrackerNode));
	}

	if(NULL == g_oTimeOutTracker.m_pstArray)
	{
		printf("Unable to allocate memory for timeout tracker of size: %d\n", g_oTimeOutTracker.m_iSize);
		return -1;
	}

	int iCount = 0;
	for(; iCount < g_oTimeOutTracker.m_iSize; ++iCount)
	{
		g_oTimeOutTracker.m_pstArray[iCount].m_pstStart = NULL;
		g_oTimeOutTracker.m_pstArray[iCount].m_pstLast = NULL;
		g_oTimeOutTracker.m_pstArray[iCount].m_iIsLocked = 0;
	}

	// Initiate timeout timer thread
	{
		thread_Create_t stThreadParam = { 0 };
		stThreadParam.dwStackSize = 0;
		stThreadParam.lpStartAddress = timeoutTimerThread;
		stThreadParam.lpParameter = NULL;
		stThreadParam.lpThreadId = &g_oTimeOutTracker.m_threadIdTimeoutTimer;

		g_oTimeOutTracker.m_threadIdTimeoutTimer = Osal_Thread_Create(&stThreadParam);

		if(-1 == g_oTimeOutTracker.m_threadIdTimeoutTimer)
		{
			return -1;
		}
	}

	// Initiate timeout action thread
	{
		thread_Create_t stThreadParam = { 0 };
		stThreadParam.dwStackSize = 0;
		stThreadParam.lpStartAddress = timeoutActionThread;
		stThreadParam.lpParameter = NULL;
		stThreadParam.lpThreadId = &g_oTimeOutTracker.m_threadTimeoutAction;

		g_oTimeOutTracker.m_threadTimeoutAction = Osal_Thread_Create(&stThreadParam);

		if(-1 == g_oTimeOutTracker.m_threadTimeoutAction)
		{
			return -1;
		}
	}
	printf("Timeout tracker is configured\n");
	return 0;
}

/**
 *
 * Description
 * Initialize request list data
 *
 * @param none
 * @return int [out] 0 (if success)
 * 					-1 (if failure)
 */
int initTCPRespStructs()
{
	g_stRespProcess.m_i32RespMsgQueId = OSAL_Init_Message_Queue();

	if(-1 == g_stRespProcess.m_i32RespMsgQueId)
	{
		 return -1;
	}

	if(-1 == sem_init(&g_stRespProcess.m_semaphoreResp, 0, 0 /* Initial value of zero*/))
	{
	   //printf("initTCPRespStructs::Could not create unnamed semaphore\n");
	   return -1;
	}

	if(0 > initTimeoutTrackerArray())
	{
		printf("Timeout tracker array init failed\n");
		return -1;
	}

	// Initiate response thread
	{
		thread_Create_t stThreadParam1 = { 0 };
		stThreadParam1.dwStackSize = 0;
		stThreadParam1.lpStartAddress = postResponseToApp;
		stThreadParam1.lpParameter = &g_stRespProcess.m_i32RespMsgQueId;
		stThreadParam1.lpThreadId = &g_stRespProcess.m_threadIdRespToApp;

		g_stRespProcess.m_threadIdRespToApp = Osal_Thread_Create(&stThreadParam1);

		if(-1 == g_stRespProcess.m_threadIdRespToApp)
		{
			// error
			return -1;
		}
	}

	initEPollData();
	return 0;
}

/**
 *
 * Description
 * De-initialize timeout tracker data structure and threads
 *
 * @param none
 * @return none
 */
void deinitTimeoutTrackerArray()
{
	Osal_Thread_Terminate(g_oTimeOutTracker.m_threadIdTimeoutTimer);
	Osal_Thread_Terminate(g_oTimeOutTracker.m_threadTimeoutAction);
	sem_destroy(&g_oTimeOutTracker.m_semTimeout);
	if(g_oTimeOutTracker.m_iTimeoutActionQ)
	{
		OSAL_Delete_Message_Queue(g_stRespProcess.m_i32RespMsgQueId);
	}

	if(NULL != g_oTimeOutTracker.m_pstArray)
	{
		free(g_oTimeOutTracker.m_pstArray);
	}
	g_oTimeOutTracker.m_iSize = 0;
}

/**
 *
 * Description
 * De-initialize data structures related to TCP response processing.
 *
 * @param none
 * @return int [out] 0 (if success)
 * 					-1 (if failure)
 */
void deinitTCPRespStructs()
{
	// 3 steps:
	// Deinit response timeout mechanism
	// Deinit epoll mechanism
	// Deinit thread which posts responses to app
	deinitTimeoutTrackerArray();

	deinitEPollData();

	// De-Initiate response thread
	{
		Osal_Thread_Terminate(g_stRespProcess.m_threadIdRespToApp);
		sem_destroy(&g_stRespProcess.m_semaphoreResp);
		if(g_stRespProcess.m_i32RespMsgQueId)
		{
			OSAL_Delete_Message_Queue(g_stRespProcess.m_i32RespMsgQueId);
		}
	}
}

/**
 *
 * Description
 * Add request to list
 *
 * @param pstMBusRequesPacket [in] pointer to struct of type stMbusPacketVariables_t
 * @return int 0 for success, -1 for error
 */
int addReqToList(stMbusPacketVariables_t *pstMBusRequesPacket)
{
	if(NULL == pstMBusRequesPacket)
	{
		return -1;
	}
	pstMBusRequesPacket->__next = NULL;
	int iTimeoutTracker = getTimeoutTrackerCount();

	if((0 > iTimeoutTracker)
		|| (iTimeoutTracker > g_oTimeOutTracker.m_iSize))
	{
		printf("Error:addReqToList: %d index is greater than size %d",
				iTimeoutTracker,
				g_oTimeOutTracker.m_iSize);

		return -1;
	}

	struct stTimeOutTrackerNode *pstTemp = &g_oTimeOutTracker.m_pstArray[iTimeoutTracker];
	if(NULL == pstTemp)
	{
		printf("Error: Null timeout tracker node\n");
		return -1;
	}

	// Obtain the lock
	int expected = 0;
	do
	{
		expected = 0;
	} while(!atomic_compare_exchange_weak(&(pstTemp->m_iIsLocked), &expected, 1));

	// Lock is obtained
	pstMBusRequesPacket->__prev = pstTemp->m_pstLast;
	if(NULL == pstTemp->m_pstStart)
	{
		pstTemp->m_pstStart = pstMBusRequesPacket;
		pstTemp->m_pstLast = pstMBusRequesPacket;
	}
	else
	{
		pstTemp->m_pstLast->__next = pstMBusRequesPacket;
		pstTemp->m_pstLast = pstMBusRequesPacket;
	}
	pstMBusRequesPacket->m_iTimeOutIndex = iTimeoutTracker;
	// Done. Release the lock
	pstTemp->m_iIsLocked = 0;

	return 0;
}

/**
 *
 * Description
 * Search request with specific unit id and transaction id in request list and mark as response is received
 *
 * @param a_u8UnitID [in] uint8_t
 * @param a_u16TransactionID  [in] uint16_t
 * @return stMbusPacketVariables_t [out] pointer to struct of type stMbusPacketVariables_t
 */
stMbusPacketVariables_t* markRespRcvd(uint8_t a_u8UnitID, uint16_t a_u16TransactionID)
{
	//return NULL;
	stMbusPacketVariables_t *pstTemp = NULL;
	// Get a lock to list
	if(a_u16TransactionID < MAX_REQUESTS)
	{
		eTransactionState expected = REQ_SENT_ON_NETWORK;
		stMbusPacketVariables_t *pstTemp1 = &g_objReqManager.m_objReqArray[a_u16TransactionID];
		if(a_u8UnitID == pstTemp1->m_u8UnitID && a_u16TransactionID == pstTemp1->m_u16TransactionID)
		{
			if(true ==
					atomic_compare_exchange_strong(&pstTemp1->m_state, &expected, RESP_RCVD_FROM_NETWORK))
			{
				pstTemp = pstTemp1;//&g_objReqManager.m_objReqArray[a_u16TransactionID];
			}
		}
	}

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
	//stLiveSerSessionList_t *pstLivSerSesslist = NULL;
	stMbusPacketVariables_t *pstMBusRequesPacket = NULL;
	int32_t i32MsgQueIdSSTC = 0;
	int32_t i32RetVal = 0;
	//int32_t i32sockfd = 0;
	IP_Connect_t stIPConnect;

	stIPConnect.m_bIsAddedToEPoll = false;
	stIPConnect.m_sockfd = 0;
	stIPConnect.m_retryCount = 0;
	stIPConnect.m_lastConnectStatus = SOCK_NOT_CONNECTED;

	i32MsgQueIdSSTC = *((int32_t *)threadArg);

	// set thread priority
	set_thread_sched_param();

	//while (NULL != threadArg)
	while(false == g_bThreadExit)
	{
		memset(&stScMsgQue,00,sizeof(stScMsgQue));
		i32RetVal = 0;
		i32RetVal = OSAL_Get_Message(&stScMsgQue, i32MsgQueIdSSTC);

		if(i32RetVal > 0)
		{
			//pstLivSerSesslist = stScMsgQue.wParam;
			pstMBusRequesPacket = stScMsgQue.lParam;
			if(NULL != pstMBusRequesPacket)
			{
				u8ReturnType = Modbus_SendPacket(pstMBusRequesPacket, &stIPConnect);
				pstMBusRequesPacket->m_u8ProcessReturn = u8ReturnType;
				if(STACK_NO_ERROR == u8ReturnType)
				{
					pstMBusRequesPacket->m_state = REQ_SENT_ON_NETWORK;
				}
				else
				{
					pstMBusRequesPacket->m_state = REQ_PROCESS_ERROR;
					addToRespQ(pstMBusRequesPacket);
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

/**
 *
 * Description
 * Add response to handle in a queue
 *
 * @param a_u8UnitID [in] uint8_t unit id from modbus header
 * @param a_u16TransactionID  [in] uint16_t transction id from modbus header
 * @return void [out] none
 */
void addToHandleRespQ(stTcpRecvData_t *a_pstReq)
{
	if(NULL != a_pstReq)
	{
		// TCP IP message format
		// 2 bytes = TxID, 2 bytes = Protocol ID, 2 bytes = length, 1 byte = unit id
		// Holds the unit id
		uint8_t  u8UnitID = a_pstReq->m_readBuffer[6];

		// Get TxID
		uByteOrder_t ustByteOrder = {0};
		ustByteOrder.u16Word = 0;
		ustByteOrder.TwoByte.u8ByteTwo = a_pstReq->m_readBuffer[0];
		ustByteOrder.TwoByte.u8ByteOne = a_pstReq->m_readBuffer[1];

		stMbusPacketVariables_t *pstMBusRequesPacket = markRespRcvd(u8UnitID, ustByteOrder.u16Word);
		if(NULL != pstMBusRequesPacket)
		{
			memcpy_s(pstMBusRequesPacket->m_u8RawResp, sizeof(pstMBusRequesPacket->m_u8RawResp),
							a_pstReq->m_readBuffer,sizeof(a_pstReq->m_readBuffer));
			// Init resp received timestamp
			timespec_get(&(pstMBusRequesPacket->m_objTimeStamps.tsRespRcvd), TIME_UTC);
			addToRespQ(pstMBusRequesPacket);
		}
		else
		{
			//printf("addToHandleRespQ: not found: %d %d\n", u8UnitID, u16TransactionID);
		}
	}

	return;
}
#endif

// Function to set thread parameters
void set_thread_sched_param()
{
	int iThreadPriority = 0;
	eThreadScheduler threadPolicy;

	iThreadPriority = THREAD_PRIORITY;
	threadPolicy = THREAD_SCHEDULER;

	//set priority
	struct sched_param param;

	param.sched_priority = iThreadPriority;

	int result;
	result = pthread_setschedparam(pthread_self(), threadPolicy, &param);
	if(0 != result)
	{
		handle_error_en(result, "pthread_setschedparam");
		printf(" Cannot set thread priority, result : %d\n",result);
	}
	else
	{
		//printf("Modbus stack :: thread parameters: %d\n", iThreadPriority);
		// success
	}

	//end of set priority for current thread
}
