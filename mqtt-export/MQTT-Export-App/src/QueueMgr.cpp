/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include <vector>
#include "cjson/cJSON.h"
#include "Common.hpp"
#include "QueueMgr.hpp"
#include "Logger.hpp"

using namespace QMgr;

//initialize queue manager instances for on-demand read/write and real-time read/write
CQueueMgr ng_qRTRead(true, true);
CQueueMgr ng_qRTWrite(false, true);
CQueueMgr ng_qRead(true, false);
CQueueMgr ng_qWrite(false, false);

/**
 * Returns reference of on-demand read operation instance
 * @return reference of on-demand read operation instance
 */
CQueueMgr& QMgr::getRead()
{
	return ng_qRead;
}

/**
 * Returns reference of on-demand write operation instance
 * @return reference of on-demand write operation instance
 */
CQueueMgr& QMgr::getWrite()
{
	return ng_qWrite;
}

/**
 * Returns reference of on-demand real-time read operation instance
 * @return reference of on-demand real-time read operation instance
 */
CQueueMgr& QMgr::getRTRead()
{
	return ng_qRTRead;
}

/**
 * Returns reference of on-demand real-time write operation instance
 * @return reference of on-demand real-time write operation instance
 */
CQueueMgr& QMgr::getRTWrite()
{
	return ng_qRTWrite;
}

/**
 * Retrieve non-RT read message from message queue to publish on EIS
 * @param msg :[in] reference to message to retrieve from queue
 * @return 	true : on success,
 * 			false : on error
 */
bool QMgr::CQueueMgr::getSubMsgFromQ(mqtt::const_message_ptr &msg)
{
	try
	{
		if(m_msgQueue.empty())
		{
			return false;
		}

		std::lock_guard<std::mutex> lock(m_queueMutex);
		msg = m_msgQueue.front();
		m_msgQueue.pop();
	}
	catch (const std::exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		return false;
	}
	return true;
}

/**
 * Clean up, destroy semaphores, disables callback, disconnect from MQTT broker
 */
void QMgr::CQueueMgr::cleanup()
{
	sem_destroy(&m_semaphore);
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Destroyed CMQTTHandler instance"));
}

/**
 * Constructor of queue manager
 * @param isRead 	:[in] set if operational instance is for read operation
 * 					or write (toggle between read and write)
 * @param isRealTime :[in] set if operational instance is for real-time operation
 * 					or non-RT (toggle between RT and non-RT)
 */
QMgr::CQueueMgr::CQueueMgr(bool isRead, bool isRealTime)
{
	initSem();

	m_bIsRead = isRead;
	m_bIsRealTime = isRealTime;
}

/**
 * Destructor
 */
QMgr::CQueueMgr::~CQueueMgr()
{
	cleanup();
}

/**
 * Initialize semaphores
 * @return 	true : on success,
 * 			false : on error
 */
bool QMgr::CQueueMgr::initSem()
{
	/* Initial value of zero*/
	if(-1 == sem_init(&m_semaphore, 0, 0))
	{
	   CLogger::getInstance().log(ERROR, LOGDETAILS("could not create semaphore, exiting"));
	   std::cout << __func__ << ":" << __LINE__ << " Error : could not create semaphore, exiting" <<  std::endl;
	   exit(0);
	}

	CLogger::getInstance().log(DEBUG, LOGDETAILS("Semaphore initialized successfully"));

	return true;
}

/**
 * Push message in operational queue
 * @param msg
 * @return 	true if success
 * 			false if failure
 */
bool QMgr::CQueueMgr::pushMsg(mqtt::const_message_ptr &msg)
{
	try
	{
		std::lock_guard<std::mutex> lock(m_queueMutex);
		m_msgQueue.push(msg);

		sem_post(&m_semaphore);
	}
	catch(exception &ex)
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS(
				"Exception received while pushing msg in queue :: "+ std::string(ex.what())));

		return false;
	}
	return true;
}

/**
 * Checks if a new message has arrived and retrieves the message
 * @param msg :[out] reference to new message
 * @return	true if a new message has arrived
 * 			false if error
 */
bool QMgr::CQueueMgr::isMsgArrived(mqtt::const_message_ptr& msg)
{
	try
	{
		if((sem_wait(&m_semaphore)) == -1 && errno == EINTR)
		{
			// Continue if interrupted by handler
			return false;
		}
		else
		{
			if (false == getSubMsgFromQ(msg))
			{
				CLogger::getInstance().log(INFO, LOGDETAILS("No message to send to EIS in queue"));
				return false;
			}
		}
	}
	catch(exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
		return false;
	}
	return true;
}
