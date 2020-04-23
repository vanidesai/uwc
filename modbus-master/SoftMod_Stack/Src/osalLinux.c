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
#include <sched.h>

/**
 *
 * DESCRIPTION
 * The OSAL API will allocate memory from the heap.
 *
 * @param a_i32MemorySize [in] Memory size to be allocated.
 * @return Success: Pointer to the allocated memory.
 *         Failed : NULL pointer.
 *
 */
void *OSAL_Malloc(int32_t a_i32MemorySize)
{
    void *pvData = NULL;

    /// Allocate memory of requested size
    pvData = malloc(a_i32MemorySize);

	return pvData;
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
    {
        return;
    }

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

    // null check
    if(NULL == pThreadParam)
    {
    	printf("NULL pointer received in Osal_Thread_Create\n");
    	return -1;
    }
    /// Set up thread attributes
    pthread_attr_init(&attr);

    // set stack size for thread
    if(pThreadParam->dwStackSize > 0)
    {
    	retcode = pthread_attr_setstacksize(&attr, pThreadParam->dwStackSize);
    	if (retcode != 0)
    	{
    		perror("pthread_attr_setstacksize failed :: ");
    	}
    }

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
    	perror("pthread_create failed::");
    	return -1;
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
    {
        return true;
    }
    else
    {
    	return false;
    }
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
	{
		return true;
	}
	else
	{
		// addressed review comment
		perror("Error in msgsnd:: ");
		return false;
	}
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
	do
	{
		i32RetVal = msgrcv(msqid, pstQueueMsg, MsgSize, MAX_RECV_PRIORITY, 0);
		if(i32RetVal < 0 && errno == EINTR)
		{
			// addressed review comment
			perror("msgrcv error :; ");
			continue;
		}
		else
		{
			// success
			break;
		}

	}while(i32RetVal > 0);

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
    if(errno == ENOMSG && (-1 == i32RetVal) && EINTR == errno)
    {
    	// addressed review comment
    	perror("failed to recv msg from queue ::");
    	i32RetVal = -1;
    }

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

    if(msqid < 0)
    {
    	perror("failed to create message queue:: ");
    	return -1;
    }

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
	int iStatus = 0;
	iStatus = msgctl(MsgQId, IPC_RMID, NULL);

    if(iStatus < 0)
    {
    	perror("failed to delete message queue:: ");
    	return false;
    }
    return true;
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
		printf("failed to create mutex..\n");
		return NULL;
	}

	int iRetVal = pthread_mutex_init(pstTpmPtr, NULL);
	if(EAGAIN == iRetVal || ENOMEM == iRetVal || EPERM == iRetVal)
	{
		/// destroy the mutex
		perror("mutex creation failed :: ");
		OSAL_Free(pstTpmPtr);
		return NULL;
	}
	else if (0 == iRetVal)
	{
		/// SUCCESS
	}
	else
	{
		// other cases free up the memory
		perror("mutex creation failed:: ");
		OSAL_Free(pstTpmPtr);
		return NULL;
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
int32_t Osal_Wait_Mutex(Mutex_H pMtxHandle)
{
	if (NULL == pMtxHandle)
	{
		// error
		printf("Invalid mutex handle received while acquiring the mutex\n");
		return -1;
	}
	else if(!(pthread_mutex_lock(pMtxHandle)))
	{
		// success
    	return 0;
	}
    else
    {
    	// return pthread error code
    	// addressed review comment
    	perror("Failed to lock mutex, error ::");
    	return errno;
    }
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
	if(NULL == pMtxHandle)
	{
		// error
		printf("Invalid mutex handle received while releasing the mutex\n");
		return -1;
	}
	else if(!(pthread_mutex_unlock(pMtxHandle)))
	{
		// success
    	return 0;
	}
	else
	{
    	// return pthread error code
    	// addressed review comment
    	perror("Failed to unlock mutex, error ::");
    	return errno;
	}
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
   if (NULL == pMtxHandle)
   {
      return -1;
   }

   if (pthread_mutex_destroy(pMtxHandle) == 0)
   {
	   if(!pMtxHandle)
	   {
		   OSAL_Free(pMtxHandle);
	   }
       return 0;
   }
   else
   {
		// return pthread error code
		// addressed review comment
		perror("Failed to destroy mutex, error ::");
		return errno;
   }
}

