/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

//#include <vector>
#include "cjson/cJSON.h"
#include "Common.hpp"
#include "QueueMgr.hpp"
#include "Logger.hpp"

using namespace QMgr;

//initialize queue manager instances for on-demand read/write and real-time read/write
CEISMsgQMgr g_qPollMsgs(true);
CEISMsgQMgr g_qWrRespMsgs(false);

CEISMsgQMgr& QMgr::PollMsgQ()
{
	return g_qPollMsgs;
}

CEISMsgQMgr& QMgr::WriteRespMsgQ()
{
	return g_qWrRespMsgs;
}

/**
 * Constructor of queue manager
 * @param isRead :[in] set if operational instance is for read operation
 * 					or write (toggle between read and write)
 * @param isRealTime :[in] set if operational instance is for real-time operation
 * 					or non-RT (toggle between RT and non-RT)
 * @return None
 */
QMgr::CEISMsgQMgr::CEISMsgQMgr(bool a_bIsPollQ): m_bIsPollQ{a_bIsPollQ}
{
	initSem();
}

/**
 * Destructor
 */
QMgr::CEISMsgQMgr::~CEISMsgQMgr()
{
	sem_destroy(&m_semaphore);
	std::lock_guard<std::mutex> lock(m_queueMutex);
	while(false == m_msgQueue.empty())
	{
		struct stEISMsg msg = m_msgQueue.front();
		m_msgQueue.pop();
		if(msg.m_pEISMsg != NULL)
		{
			msgbus_msg_envelope_destroy(msg.m_pEISMsg);
			msg.m_pEISMsg = NULL;
		}
	}
}

/**
 * Initialize semaphores
 * @param None
 * @return true on success; application exits if fails to initialize semaphore
 */
bool QMgr::CEISMsgQMgr::initSem()
{
	/* Initial value of zero*/
	if(-1 == sem_init(&m_semaphore, 0, 0))
	{
	   std::cout << __func__ << ":" << __LINE__ << " Error : could not create semaphore, exiting" <<  std::endl;
	   exit(0);
	}

	return true;
}

/**
 * Push message in operational queue
 * @param msg :[in] MQTT message to push in message queue
 * @return true/false based on success/failure
 */
bool QMgr::CEISMsgQMgr::pushMsg(msg_envelope_t *a_pMsg, struct timespec& a_stTs)
{
	bool bRet = false;
	try
	{
		if(NULL != a_pMsg)
		{
			struct stEISMsg msg{a_pMsg, a_stTs};
			{
				std::lock_guard<std::mutex> lock(m_queueMutex);
				m_msgQueue.push(msg);
			}

			{
				std::string revdTopic;
				msg_envelope_elem_body_t* data;
				msgbus_ret_t msgRet = msgbus_msg_envelope_get(a_pMsg, "data_topic", &data);
				if(msgRet != MSG_SUCCESS)
				{
					cout << __func__ << ": " << "topic key not present in zmq message" << endl;
				}
				else
				{
					revdTopic = data->body.string;
					std::cout << CCommon::getInstance().get_micros(a_stTs) << ": new msg pushed with topic: " << revdTopic << std::endl;
				}
			}

			sem_post(&m_semaphore);
			bRet = true;
		}
	}
	catch(exception &ex)
	{
		cout << __func__ << ": " << "Exception received while pushing msg in queue :: " << std::string(ex.what()) << endl;
	}
	return bRet;
}

/**
 * Checks if a new message has arrived and retrieves the message
 * @param msg :[out] reference to new message
 * @return	true/false based on success/failure
 */
bool QMgr::CEISMsgQMgr::waitForMsg(struct stEISMsg &a_stEISMsg)
{
	bool bRet = false;
	try
	{
		if((sem_wait(&m_semaphore)) == -1 && errno == EINTR)
		{
			// Continue if interrupted by handler
		}
		else
		{
			if(false == m_msgQueue.empty())
			{
				std::lock_guard<std::mutex> lock(m_queueMutex);
				a_stEISMsg = m_msgQueue.front();
				m_msgQueue.pop();

				bRet = true;
			}
		}
	}
	catch(exception &e)
	{
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
	}
	return bRet;
}
