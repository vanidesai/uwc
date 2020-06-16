/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "Common.hpp"
#include "QueueMgr.hpp"
#include "Logger.hpp"

using namespace QMgr;

//initialize queue manager instances for on-demand read/write and real-time read/write
CQueueMgr ng_qMqtt;
CQueueMgr ng_qScada;

/**
 * Get reference of on-demand real-time read operation instance
 * @param None
 * @return reference of on-demand real-time read operation instance
 */
CQueueMgr& QMgr::getMqtt()
{
	return ng_qMqtt;
}

/**
 * Get reference of on-demand real-time write operation instance
 * @param None
 * @return reference of on-demand real-time write operation instance
 */
CQueueMgr& QMgr::getScada()
{
	return ng_qScada;
}

/**
 * Retrieve non-RT read message from message queue to publish on EIS
 * @param msg :[in] reference to message to retrieve from queue
 * @return true/false based on success/failure
 */
bool QMgr::CQueueMgr::getSubMsgFromQ(stMqttMsg &msg)
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
		DO_LOG_FATAL(e.what());
		return false;
	}
	return true;
}

/**
 * Clean up, destroy semaphores, disables callback, disconnect from MQTT broker
 * @param None
 * @return None
 */
void QMgr::CQueueMgr::cleanup()
{
	sem_destroy(&m_semaphore);
	DO_LOG_DEBUG("Destroyed CMQTTHandler instance");
}

/**
 * Constructor of queue manager
 * @param isRead :[in] set if operational instance is for read operation
 * 					or write (toggle between read and write)
 * @param isRealTime :[in] set if operational instance is for real-time operation
 * 					or non-RT (toggle between RT and non-RT)
 * @return None
 */
QMgr::CQueueMgr::CQueueMgr()
{
	initSem();
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
 * @param None
 * @return true on success; application exits if fails to initialize semaphore
 */
bool QMgr::CQueueMgr::initSem()
{
	/* Initial value of zero*/
	if(-1 == sem_init(&m_semaphore, 0, 0))
	{
	   DO_LOG_ERROR("could not create semaphore, exiting");
	   std::cout << __func__ << ":" << __LINE__ << " Error : could not create semaphore, exiting" <<  std::endl;
	   exit(0);
	}

	DO_LOG_DEBUG("Semaphore initialized successfully");

	return true;
}

/**
 * Push message in operational queue
 * @param msg :[in] MQTT message to push in message queue
 * @return true/false based on success/failure
 */
bool QMgr::CQueueMgr::pushMsg(stMqttMsg &msg)
{
	try
	{
		std::lock_guard<std::mutex> lock(m_queueMutex);
		m_msgQueue.push(msg);

		sem_post(&m_semaphore);
	}
	catch(exception &ex)
	{
		DO_LOG_ERROR(
				"Exception received while pushing msg in queue :: "+ std::string(ex.what()));

		return false;
	}
	return true;
}

/**
 * Checks if a new message has arrived and retrieves the message
 * @param msg :[out] reference to new message
 * @return	true/false based on success/failure
 */
bool QMgr::CQueueMgr::isMsgArrived(stMqttMsg& msg)
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
				DO_LOG_INFO("No message to send to EIS in queue");
				return false;
			}
		}
	}
	catch(exception &e)
	{
		DO_LOG_FATAL(e.what());
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
		return false;
	}
	return true;
}
