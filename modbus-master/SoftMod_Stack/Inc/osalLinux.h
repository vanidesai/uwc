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

#ifndef INC_OSALLINUX_H_
#define INC_OSALLINUX_H_

#include <pthread.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include "API.h"

#define WAIT_OBJECT_0 0
#define WAIT_OBJECT_1 1
#define WAIT_OBJECT_2 2
#define WAIT_TIMEOUT -1
#define INFINITE      0xFFFFFFFF  // Infinite timeout
#define MAX_RECV_PRIORITY -100000

typedef pthread_t  Thread_H;
typedef pthread_mutex_t*  Mutex_H;

/**
 @struct thread_Create_t
 @brief
   The OSAL Create Thread parameters for various OS
*/
typedef struct
{

    //LPSECURITY_ATTRIBUTES lpThreadAttributes;
    size_t dwStackSize;
    void* lpStartAddress;
    void* lpParameter;
    //DWORD dwCreationFlags;
    Thread_H * lpThreadId;

}thread_Create_t;

/**
 @struct Linux_Msg_t
 @brief
   The OSAL Create msg for various OS
*/
typedef struct Linux_Msg
{
	long mtype;       /* message type, must be > 0 */
	void* wParam;
	void* lParam;
}Linux_Msg_t;

/**
 @struct Post_Thread_Msg
 @brief
   The OSAL Post Thread msg parameters for various OS
*/
typedef struct Post_Thread_Msg
{
	int idThread;		/* Message queue ID */
	long  MsgType;       /* message type, must be > 0 */
	void* wParam;
	void* lParam;
}Post_Thread_Msg_t;

/// The OSAL API will allocate memory from the heap.
void *OSAL_Malloc(int32_t i32MemorySize);
void OSAL_Free(void *pvPointer);

/// The OSAL Thread create API will generate Thread for various OS
Thread_H Osal_Thread_Create(thread_Create_t *pThreadParam);

/// The OSAL Thread Terminate API will terminate Thread.
bool Osal_Thread_Terminate(Thread_H pThreadTerminate);

/// The OSAL Mutex create API will generate Mutex
Mutex_H Osal_Mutex(void);
/// The OSAL API Releases the Mutex acquired by process
int32_t Osal_Wait_Mutex(Mutex_H pMtxHandle);
/// The OSAL API Releases the Mutex acquired by process
int32_t Osal_Release_Mutex(Mutex_H pMtxHandle);

/// close the mutex
int32_t Osal_Close_Mutex( Mutex_H pMtxHandle);

/// Copies a message to message queue
int32_t OSAL_Init_Message_Queue();
/// Copies a message to message queue
bool OSAL_Post_Message(Post_Thread_Msg_t *pstPostThreadMsg);
/// Copies a message from message queue
bool OSAL_Get_Message(Linux_Msg_t *pstQueueMsg, int   msqid);
/// Copies a message from message queue
int32_t OSAL_Get_NonBlocking_Message(Linux_Msg_t *pstQueueMsg, int   msqid);
/// Delete a message from message queue.
bool OSAL_Delete_Message_Queue(int MsgQId);

#endif /* INC_OSALLINUX_H_ */
