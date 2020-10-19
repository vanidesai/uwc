/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef QHANDLER_HPP_
#define QHANDLER_HPP_

#include <atomic>
#include <map>
#include <semaphore.h>
#include "mqtt/async_client.h"
//#include "Common.hpp"
#include <queue>
#include <string>

//using namespace std;
/**
 * namespace for Queue manager
 */
//namespace QMgr
//{

	/**
	 * Queue manager class to manage instances of on-demand operations for msg handling
	 */
	class CQueueHandler
	{
		bool initSem();

		std::mutex m_queueMutex;
		std::queue<mqtt::const_message_ptr> m_msgQueue;
		sem_t m_semaphore;

		// delete copy and move constructors and assign operators
		CQueueHandler& operator=(const CQueueHandler&)=delete;	// Copy assign
		CQueueHandler(const CQueueHandler&)=delete;	 			// Copy construct

	public:
		CQueueHandler();
		~CQueueHandler();

		bool pushMsg(mqtt::const_message_ptr msg);
		bool isMsgArrived(mqtt::const_message_ptr& msg);
		bool getSubMsgFromQ(mqtt::const_message_ptr& msg);

		void cleanup();
		void clear();
	};
//}
#endif
