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

#include <stdbool.h>
#include "osalLinux.h"
#include <stdio.h>

/**
 *
 * DESCRIPTION
 * The OSAL API will allocate memory from the heap.
 *
 * @param i32MemorySize [in] Memory size to be allocated.
 * @return Success: Pointer to the allocated memory.
 *         Failed : NULL pointer.
 *
 */
void *OSAL_Malloc(int32_t i32MemorySize)
{
    void *Allocated_Location;
    /// Allocate memory of requested size
	Allocated_Location = malloc(i32MemorySize);

	return Allocated_Location;
}

/**
 *
 * DESCRIPTION
 * The OSAL API will deallocate assigned memory pointed by the argument.
 *
 * @param pPointer [in] Pointer to the memory.
 */
void OSAL_Free(void *pvPointer)
{
    /// Check for NULL pointer free
    if(NULL == pvPointer)
        return;

    free(pvPointer);
	pvPointer = NULL;

}



/**
 *
 * DESCRIPTION
 * The OSAL Thread create API will generate Thread
 * for various OS.
 *
 * @param pThreadParam Pointer to structure holding
 *                  Thread creation parameters.
 * @return Returns Handle to created Thread
 *
 */
Thread_H Osal_Thread_Create(thread_Create_t *pThreadParam)
{
    pthread_attr_t attr;
    int retcode;

    /// Set up thread attributes
    pthread_attr_init(&attr);

    pthread_attr_setstacksize(&attr, pThreadParam->dwStackSize);

    /// Start the thread
    retcode = pthread_create(pThreadParam->lpThreadId, &attr, pThreadParam->lpStartAddress,
                            pThreadParam->lpParameter);

    /// clear up assigned attributes
    pthread_attr_destroy(&attr);

    if (retcode == 0)
    {
        return (*(pThreadParam->lpThreadId));
    }
    else
    {
    	return 0;
    }
}

/**
 *
 * DESCRIPTION
 * The OSAL Thread Terminate API will
 * terminate Thread for various OS.
 *
 * @param pThreadTerminate Pointer to structure holding
 *                  Thread Termination parameters.
 * @return True/False
 *
 */
bool Osal_Thread_Terminate(Thread_H pThreadTerminate)
{
    /// Cancel thread
    pthread_cancel(pThreadTerminate);

    /// Wait for thread to stop and reclaim thread resources
    if (0 == pthread_join(pThreadTerminate, NULL ) )
        return 1;
    else
        return 0;
}

/**
 *
 * DESCRIPTION
 * The OSAL Thread Terminate API will
 * terminate Thread for various OS.
 *
 * @param pThreadTerminate Pointer to structure holding
 *                  Thread Termination parameters.
 * @return True/False
 *
 */
bool OSAL_Set_Thread_Priority(Thread_H pvThreadHandle, uint8_t u8Priority)
{
    struct sched_param param;

    param.sched_priority = 255 - u8Priority;
    return(pthread_setschedparam( pvThreadHandle, 0, &param ));
}


/**
 *
 * DESCRIPTION
 * Resumes a suspended thread.
 *
 * @param hThread [in] Handle to the suspended thread.
 *
 * @return uint32_t Status Value.
 *
 */
uint32_t OSAL_ResumeThread(Thread_H hThread)
{

	return 0;
}

/**
 *
 * DESCRIPTION
 * Suspends an active thread.
 *
 * @param hThread [in] Handle to the thread that needs to be suspended.
 *
 * @return uint32_t Status Value.
 *
 */
uint32_t OSAL_SuspendThread(Thread_H hThread)
{
	return 0;
}


/**
 *
 * DESCRIPTION
 * Copies a message to message queue.
 *
 * @param pstPostThreadMsg [in] Pointer to struct to be copied.
 *
 * @return TRUE/FALSE
 *
 */
bool OSAL_Post_Message(Post_Thread_Msg_t *pstPostThreadMsg)
{
	int iStatus = 0;
	Linux_Msg_t stMsgData;
	size_t MsgSize = 0;

	stMsgData.mtype = pstPostThreadMsg->MsgType;
	stMsgData.wParam = pstPostThreadMsg->wParam;
	stMsgData.lParam = pstPostThreadMsg->lParam;

	MsgSize = sizeof(Linux_Msg_t) - sizeof(long);

	iStatus = msgsnd( pstPostThreadMsg->idThread, &stMsgData, MsgSize, 0);

	if(iStatus == 0)
		return true;
	else
		return false;

}


/**
 *
 * DESCRIPTION
 * Copies a message from message queue.
 *
 * @param pstQueueMsg [out] Pointer to struct where data is copied.
 * @param msqid [in]  messages id that can be received.
 *
 * @return TRUE/FALSE
 *
 */
bool OSAL_Get_Message(Linux_Msg_t *pstQueueMsg, int   msqid)
{
	int32_t i32RetVal = 0;

	size_t MsgSize = 0;

	MsgSize = sizeof(Linux_Msg_t) - sizeof(long);

	/// Wait Till Message received or Error other than Signal interrupt
	while( (i32RetVal = msgrcv(msqid, pstQueueMsg, MsgSize, MAX_RECV_PRIORITY, 0)) == -1 && errno == EINTR )
		continue;

	/// Returns Number of Bytes received or Error received
	return( i32RetVal);
}

/**
 * DESCRIPTION
 * Copies a message from message queue.
 *
 * @param pstQueueMsg [out] Pointer to struct where data is copied.
 * @param msqid [in] messages id that can be received.
 *
 * @return TRUE/FALSE
 *
 */
int32_t OSAL_Get_NonBlocking_Message(Linux_Msg_t *pstQueueMsg, int   msqid)
{
	int32_t i32RetVal = 0;

	size_t MsgSize = 0;

	MsgSize = sizeof(Linux_Msg_t) - sizeof(long);

    /// Wait Till Message received or Error other than Signal interrupt
    i32RetVal = msgrcv(msqid, pstQueueMsg, MsgSize, MAX_RECV_PRIORITY, MSG_NOERROR | IPC_NOWAIT);
    if(errno == ENOMSG && (-1 == i32RetVal))
    	i32RetVal = -1;

    /// Returns Number of Bytes received or Error received
	return( i32RetVal);
}


/**
 *
 * DESCRIPTION
 * Copies a message to message queue.
 *
 * @param Nothing
 *
 * @return int32_t value.
 *
 */
int32_t OSAL_Init_Message_Queue()
{
    int msqid = 0;

    msqid = msgget(IPC_PRIVATE, (IPC_CREAT | IPC_EXCL | 0666));

    return msqid;
}

/**
 *
 * DESCRIPTION
 * Delete a message to message queue.
 *
 * @param MsgQId [in] Message queue id
 *
 * @return bool value.
 *
 */
bool OSAL_Delete_Message_Queue(int MsgQId)
{
	return(msgctl(MsgQId, IPC_RMID, NULL));

}

/**
 *
 * DESCRIPTION
 * The OSAL Mutex create API will generate Mutex
 * for various OS.
 *
 * @return Returns Handle to created Mutex
 *
 */
Mutex_H Osal_Mutex(void)
{
	Mutex_H pstTpmPtr = NULL;

	/// Assign memory to mutex handle
	pstTpmPtr = (Mutex_H)OSAL_Malloc(sizeof(pthread_mutex_t));

	if (NULL == pstTpmPtr)
	{
		return NULL;
	}

	int iRetVal = pthread_mutex_init(pstTpmPtr, NULL);
	if(EAGAIN == iRetVal || ENOMEM == iRetVal || EPERM == iRetVal)
	{
		/// destroy the mutex
		printf("mutex creation failed !!\n");
		OSAL_Free(pstTpmPtr);
		pthread_mutex_destroy(pstTpmPtr);
	}
	else if (iRetVal == 0)
	{
		/// Success
		//printf("mutex creation success !!\n");
	}
	else
	{
		// other cases free up the memory
		printf("mutex creation failed !!\n");
		OSAL_Free(pstTpmPtr);
		pthread_mutex_destroy(pstTpmPtr);
	}

	return pstTpmPtr;
}

/**
 *
 * DESCRIPTION
 * The OSAL API Releases the Mutex acquired by process.
 *
 * @return int32_t returns 0 If the function succeeds, the return value is nonzero.
 *
 */
int32_t Osal_Wait_Mutex(Mutex_H pMtxHandle, int32_t i32Time)
{
	if (NULL == pMtxHandle)
	{
		return WAIT_TIMEOUT;
	}
	else if( !(pthread_mutex_lock(pMtxHandle)) )
    	return WAIT_OBJECT_0;
    else
    	return WAIT_TIMEOUT;
}

/**
 *
 * DESCRIPTION
 * The OSAL API Releases the Mutex acquired by process.
 *
 * @return returns 0 If the function succeeds, the return value is nonzero.
 */
int32_t Osal_Release_Mutex(Mutex_H pMtxHandle)
{
	if(NULL != pMtxHandle)
	{
		return(pthread_mutex_unlock(pMtxHandle));
	}
	else
		return -1;
}

/**
 *
 * DESCRIPTION
 * Delete a mutex.
 *
 * @param pMtxHandle       Handle of mutex to delete.
 *
 * @retval 0       if Error.
 * @retval 1       if Successful
 *
 */
int32_t Osal_Close_Mutex( Mutex_H pMtxHandle)
{
   if ( NULL == pMtxHandle )
      return 0;

   if ( pthread_mutex_destroy( pMtxHandle ) == 0 )
   {
	   OSAL_Free(pMtxHandle);
       return 1;
   }
   else
   {
       return 0;
   }
}

