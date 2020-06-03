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

struct stReqManager g_objReqManager;

// global variable to store stack configurations
extern stDevConfig_t g_stModbusDevConfig;

#ifndef MODBUS_STACK_TCPIP_ENABLED

extern int fd;

#endif

//Linux message queue ID
int32_t i32MsgQueIdSC = 0;
//flag to check whether to exit the current thread or continue
extern bool g_bThreadExit;

//if Modbus stack communicates with Modbus slave device using TCP mode
#ifdef MODBUS_STACK_TCPIP_ENABLED

//Mutex for synchronization of session list
extern Mutex_H LivSerSesslist_Mutex;

//array of structures containing socket descriptors the have been registered with epoll
//and data to be read from
stTcpRecvData_t m_clientAccepted[MAX_DEVICE_PER_SITE] = {{0}};

//initialization of epoll descriptors
int m_epollFd = 0;

//pointer to structure holding events for registered socket descriptors
struct epoll_event *m_events = NULL;

//handle of thread receiving data from socket descriptors continuously using epoll mechanism
Thread_H EpollRecv_ThreadId = 0;

//handle of mutex to synchronize epoll data
Mutex_H EPollMutex;

//pointer to session list
stLiveSerSessionList_t *pstSesCtlThdLstHead = NULL;

//structure to tract timeout requests
struct stTimeOutTracker g_oTimeOutTracker = {0};
#endif

//structure to hold data for message queue
struct stIntDataForQ
{
	long m_lType;       /* message type, must be > 0 */
	unsigned short m_iID;
};

/**
 *
 * Description
 * This function looks for an available node in request array.
 * It reserves the available node for using this node.
 *
 * @param none
 * @return long [out] non-zero node index in long format;
 * 					   -1 in case of error
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
 * This function resets the request node to default values.
 *
 * @param a_pObjReqNode [in] stMbusPacketVariables_t* pointer to structure of node to reset
 *
 * @return none
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
 * This function initiates the request manager and data structures within it. The request
 * manager keeps track of requests send to Modbus slave and responses received for those requests.
 *
 * @param none
 *
 * @return bool [out] true if function succeeds in initializing the request manager;
 * 					   false if function fails to initialize the request manager;
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
 * This function sets data structure to process new request. It gets the available node
 * from the request manager's list and updates new request at the empty node.
 *
 * @param tsReqRcvd [in] const struct timespec time-stamp when request was received
 * @return [out] stMbusPacketVariables_t* pointer to emplaced request
 * 				 NULL in case of error
 *
 */
stMbusPacketVariables_t* emplaceNewRequest(const struct timespec tsReqRcvd)
{
	stMbusPacketVariables_t* ptr = NULL;
	// Get index of available request node in request array
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
 * This function frees a request node and resets it for use by next requests. This function
 * also releases the request from tracking for timeout.
 *
 * @param a_pobjReq [in] stMbusPacketVariables_t* request node to mark as free
 *
 * @return none
 *
 */
void freeReqNode(stMbusPacketVariables_t* a_pobjReq)
{
	// 1. remove the node from timeout tracker in case of TCP
	// 2. reset the request node elements
	// 3. mark the index as available
#ifdef MODBUS_STACK_TCPIP_ENABLED
	releaseFromTracker(a_pobjReq);
#endif
	// reset the structure
	resetReqNode(a_pobjReq);
}

#ifdef MODBUS_STACK_TCPIP_ENABLED
/**
 *
 * Description
 * This function is a thread routine for session control thread
 *
 * @param threadArg [in] void* thread arguments to start thread
 *
 * @return void 	[out] nothing
 */
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
						// Wait for next request
						//continue;
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
						// addressed review comment
						if(0 != Osal_Release_Mutex (LivSerSesslist_Mutex))
						{
							// fail to unlock mutex
							// Wait for next request
							//continue;
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
						// Wait for next request
						///continue;
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
					// addressed review comment
					if(0 != Osal_Release_Mutex (LivSerSesslist_Mutex))
					{
						// fail to unlock mutex
						// Wait for next request
						//continue;
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
					// addressed review comment
					if(0 != Osal_Release_Mutex (LivSerSesslist_Mutex))
					{
						// fail to unlock mutex
						// Wait for next request
						//continue;
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
					// addressed review comment
					if(0 != Osal_Release_Mutex (LivSerSesslist_Mutex))
					{
						// fail to unlock mutex
						// Wait for next request
						//continue;
					}
					freeReqNode(pstMBusReqPact);
					continue;
				}
			}
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
 * This function gets client socket id from already established client's list
 *
 * @param socketID [in] int socket id to retrieve from the list
 *
 * @return int [out] client index from list if client socket id already exists in the list;
 * 					 -1 if client socket id is not in the list
 *
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
 * This function resets client structure after data is read from the socket.
 *
 * @param clientAccepted [in] stTcpRecvData_t* pointer that holds socket's data received information
 *
 * @return [out] none
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
 * This function initializes data structures needed for epoll operation for receiving TCP data,
 * then creates epoll descriptor and starts a thread to receive data from registered socket
 * descriptors using epoll mechanism.
 *
 * @param None
 *
 * @return bool [out] true if function succeeds;
 * 					  false if function fails or in case of errors
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

	return true;
}

/**
 *
 * Description
 * This function de-initializes data structures related to epoll operation for receiving TCP data.
 * It resets all the socket descriptors registered for epoll.
 *
 * @param None
 *
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
 * This function removes a socket descriptor registered with with epoll and
 * resets associated data structures.
 * The calling function should have the lock for epoll data structure.
 *
 * @param a_iIndex [in] int reference to epoll data structure
 *
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
 * This function removes a socket descriptor from epoll descriptors and
 * resets associated data structures.
 * This function acquires lock for epoll data structure.
 *
 * @param a_iIndex [in] int Reference to epoll data structure
 *
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
 * This function sets the socket's failure state. It checks if the referenced socket ID and
 * socket ID from the function argument are same or not. If they are different, it removes the
 * socket descriptor from the epoll and resets the structure for next use. If socket IDs are same
 * and getting used for different operation, close the connection. If there is no socket ID with
 * the reference given, reset the socket descriptor.
 *
 * @param stIPConnect [in] IP_Connect_t* pointer to struct of type IP_Connect_t
 *
 * @return [out] none
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
 * This function gets the current epoch time
 *
 * @param none
 *
 * @return struct timespec [out] epoch time struct
 */
struct timespec getEpochTime() {

	struct timespec ts;
    timespec_get(&ts, TIME_UTC);

    return ts;
}

/**
 *
 * Description
 * This function is a thread routine to start polling on sockets registered with epoll to receive data.
 * This function sets the thread priority as per the configuration. It then allocates the number of polling events
 * the can occur and thus initializes epoll descriptors event structure.
 * Function then waits for events to occur on registered socket descriptors. If any socket descriptor notifies
 * of incoming response, epoll notifies it to the function. Function then iterates through all the socket descriptors
 * and reads the response data. In order to read the data, the function first reads only 6 bytes from the response
 * and parses the length of the response. Then calculates the number of bytes to be read from that socket descriptor.
 * It keeps on reading till a complete response is not received or if it has read 0 bytes, it goes to read from the
 * next socket descriptor.
 * Once the complete response is received, function adds it to the request manager's queue to process further
 * and resets the socket descriptor to be used for next request.
 *
 * @param  none
 * @return [out] none
 *
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
 * This function is a thread routine to send Modbus RTU messages to Modbus slave device
 * which are available in the Linux message queue. Thread continuously checks for a new message in
 * Linux message queue; once the message is available it sends the request to the Modbus slave device.
 * The Modbus_SendPacket function receives the response from Modbus slave device, after this session
 * control thread function invokes the ModbusApp callback function. After invoking ModbusApp callback
 * function, this function frees up with request node from request manager's list.
 *
 * @param threadArg [in] void* thread argument
 *
 * @return [out] none
 *
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

			pstMBusReqPact->m_u8ProcessReturn = u8ReturnType;
			if(STACK_NO_ERROR == u8ReturnType)
			{
				//pstMBusReqPact->m_state = RESP_RCVD_FROM_NETWORK;
			}
			else
			{
				addToRespQ(pstMBusReqPact);
			}
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

//structure to store response for processsing
struct stResProcessData g_stRespProcess;

//when Modbus communication mode is TCP
#ifdef MODBUS_STACK_TCPIP_ENABLED
/**
 *
 * Description
 * This function gets current time stamp in nano-seconds.
 *
 * @param none
 *
 * @return [out] unsigned long time in nano-seconds
 */
unsigned long get_nanos(void) {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (unsigned long)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

/**
 *
 * Description
 * This function removes request from timeout tracker node.
 * Assumed is calling function has acquired lock.
 *
 * @param pstMBusRequesPacket [in] stMbusPacketVariables_t* pointer to structure holding request
 * 									sent on Modbus slave device
 *
 * @param a_pstTracker [in] stTimeOutTrackerNode* reference to timeout tracker list from which
 * 							node needs to be removed
 *
 * @return [out] none
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
 * This function removes the request from timeout tracker.
 * Identifies tracker node from the tracker list and then removes the node from that tracker
 *
 * @param pstMBusRequesPacket [in] stMbusPacketVariables_t* pointer to structure holding information
 * 									about the request
 *
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

#endif
/**
 *
 * Description
 * This function adds Modbus request (sent to Modbus slave device) in
 * response queue
 *
 * @param a_pstReq [in] stMbusPacketVariables_t* pointer to structure holding information
 * 						about request to add in response queue
 *
 * @return [out] none
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
 * This function is thread routine which posts response to application.
 * It listens on a Linux message queue to receive response of requests to post to ModbusApp.
 * Once the response is sent to ModbusApp, function frees the response node so that other
 * requests can reuse it. The thread keeps working till the flag is not set to terminate
 * the thread.
 *
 * @param threadArg [in] void* pointer
 *
 * @return [out] none
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
				ApplicationCallBackHandler(pstMBusRequesPacket, pstMBusRequesPacket->m_u8ProcessReturn);
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

#ifdef MODBUS_STACK_TCPIP_ENABLED
/**
 *
 * Description
 * This function returns current timeout counter.
 * Timeout is implemented in the form of counter.
 *
 * @param none
 *
 * @return [out] int the timeout counter
 */
int getTimeoutTrackerCount()
{
	return g_oTimeOutTracker.m_iCounter;
}

/**
 *
 * Description
 * This function is a thread routine which implements a timer to measure timeout for
 * initiated requests. If the timeout occurs for a request, it notifies the processing
 * thread about the same.
 *
 * @param [in] void* thread argument
 *
 * @return none
 */
void* timeoutTimerThread(void* threadArg)
{
	// set thread priority
	set_thread_sched_param();
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
 * The function is a thread routine which identifies timed out requests and
 * initiates a response accordingly.
 *
 * @param [in] void* thread argument
 *
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
		if(i32RetVal <= 0)
		{
			//i32RetVal = -1;
			perror("Unable to get data from timeout q");
			continue;
		}

		// get list corresponding to the index

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

		// Obtain the lock
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
			// Change the state of request node
			if(true ==
					atomic_compare_exchange_strong(&pstCur->m_state, &expected, RESP_TIMEDOUT))
			{
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
 * This function initialize timeout tracker data structure and threads.
 *
 * @param none
 *
 * @return [out] int 0 if function succeeds in the initialization of timeout tracker;
 * 					-1 if function fails
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
		perror("Timeout tracker: semaphore creation error: ");
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

	// Resets the timeout tracker lists
	int iCount = 0;
	for(; iCount < g_oTimeOutTracker.m_iSize; ++iCount)
	{
		g_oTimeOutTracker.m_pstArray[iCount].m_pstStart = NULL;
		g_oTimeOutTracker.m_pstArray[iCount].m_pstLast = NULL;
		g_oTimeOutTracker.m_pstArray[iCount].m_iIsLocked = 0;
	}

	// Initiate timeout timer thread. This thread measures timeout tracker counter
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
#endif
/**
 *
 * Description
 * This function initializes the request list data and data structures
 * needed for epoll mechanism.
 *
 * @param none
 *
 * @return [out] int  0 if function succeeds in initialization;
 * 					  -1 if function fails to initialize
 */
int initRespStructs()
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
#ifdef MODBUS_STACK_TCPIP_ENABLED
	if(0 > initTimeoutTrackerArray())
	{
		printf("Timeout tracker array init failed\n");
		return -1;
	}
#endif

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

#ifdef MODBUS_STACK_TCPIP_ENABLED
	initEPollData();
#endif
	return 0;
}

/**
 *
 * Description
 * This function de-initializes data structures related to response processing.
 *
 * @param none
 *
 * @return Nothing
 */
void deinitRespStructs()
{
	// 3 steps:
	// Deinit response timeout mechanism
	// Deinit epoll mechanism
	// Deinit thread which posts responses to app
#ifdef MODBUS_STACK_TCPIP_ENABLED
	deinitTimeoutTrackerArray();
	deinitEPollData();
#endif

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

#ifdef MODBUS_STACK_TCPIP_ENABLED
/**
 *
 * Description
 * This function de-initializes the timeout tracker data structure and threads.
 *
 * @param none
 *
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
 * This function adds request to list for tracking timeout.
 *
 * @param pstMBusRequesPacket [in] stMbusPacketVariables_t* pointer to structure holding
 * 									 information about the request sent on Modbus slave device
 *
 * @return [out] int 0 if function succeeds;
 *  				 -1 if function fails
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
 * This function searches a request with specific unit id (Modbus slave device id) and
 * transaction id (request id that was sent on Modbus slave device) in request list and
 * mark as response is received.
 *
 * @param a_u8UnitID 				[in] uint8_t Modbus slave device ID
 * @param a_u16TransactionID  		[in] uint16_t request id that was sent on Modbus slave device
 * @return stMbusPacketVariables_t 	[out] stMbusPacketVariables_t* pointer to structure holding
 * 										  information
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
 * This function is a thread routine for TCP and callback functions. This function
 * sets the thread priority, receives a request from message queue and then sends it
 * to the Modbus slave device. After the request is sent a response is received in global
 * message queue, function adds it in the response queue for further processing.
 * The thread continues till the flag to terminate the thread is not set.
 *
 * @param threadArg [in] void* thread argument
 *
 * @return [out] void pointer
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
 * This function adds a response to handle in response queue.
 *
 * @param a_pstReq [in] stTcpRecvData_t* response data received from Modbus slave device
 *
 * @return [out] none
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
/**
 *
 * Description
 * This function sets thread parameters. All threads call this function.
 * At present, all stack threads are configured with same parameters.
 *
 * @param none
 *
 * @return [out] none
 */
void set_thread_sched_param()
{
	// Set thread priority and scheduler
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
