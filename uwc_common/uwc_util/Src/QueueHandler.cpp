/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "QueueHandler.hpp"
#include "Logger.hpp"

/**
 * Clean up, destroy semaphores, disables callback, disconnect from MQTT broker
 * @param None
 * @return None
 */
void CQueueHandler::cleanup()
{
	sem_destroy(&m_semaphore);
}

/**
 * Clear queue
 * @param None
 * @return None
 */
void CQueueHandler::clear()
{
	std::lock_guard<std::mutex> lock(m_queueMutex);
	m_msgQ = {};
}

/**
 * Constructor of queue manager
 * @param isRead :[in] set if operational instance is for read operation
 * 					or write (toggle between read and write)
 * @param isRealTime :[in] set if operational instance is for real-time operation
 * 					or non-RT (toggle between RT and non-RT)
 * @return None
 */
CQueueHandler::CQueueHandler()
{
	initSem();
}

/**
 * Destructor
 */
CQueueHandler::~CQueueHandler()
{
	cleanup();
}

/**
 * Initialize semaphores
 * @param None
 * @return true on success; application exits if fails to initialize semaphore
 */
bool CQueueHandler::initSem()
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
bool CQueueHandler::pushMsg(CMessageObject msg)
{
	try
	{
		std::lock_guard<std::mutex> lock(m_queueMutex);
		m_msgQ.push(msg);

		sem_post(&m_semaphore);
	}
	catch(std::exception &ex)
	{
		DO_LOG_ERROR(ex.what());
		return false;
	}
	return true;
}

/**
 * Push message in operational queue
 * @param msg :[in] MQTT message to push in message queue
 * @return true/false based on success/failure
 */
bool CQueueHandler::breakWaitOnQ()
{
	try
	{
		sem_post(&m_semaphore);
	}
	catch(std::exception &ex)
	{
		DO_LOG_ERROR(ex.what());
		return false;
	}
	return true;
}

/**
 * Retrieve non-RT read message from message queue to publish on EIS
 * @param msg :[in] reference to message to retrieve from queue
 * @return true/false based on success/failure
 */
bool CQueueHandler::getSubMsgFromQ(CMessageObject& msg)
{
	try
	{
		if(m_msgQ.empty())
		{
			return false;
		}

		std::lock_guard<std::mutex> lock(m_queueMutex);
		msg = m_msgQ.front();
		m_msgQ.pop();
	}
	catch (const std::exception &e)
	{
		DO_LOG_ERROR(e.what());
		return false;
	}
	return true;
}

/**
 * Checks if a new message has arrived and retrieves the message
 * @param msg :[out] reference to new message
 * @return	true/false based on success/failure
 */
bool CQueueHandler::isMsgArrived(CMessageObject& msg)
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
	catch(std::exception &e)
	{
		DO_LOG_ERROR(e.what());
		return false;
	}
	return true;
}

