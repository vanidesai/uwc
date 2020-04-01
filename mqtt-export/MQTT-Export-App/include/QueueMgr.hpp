/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef QMGR_HPP_
#define QMGR_HPP_

#include <atomic>
#include <vector>
#include <semaphore.h>
#include "mqtt/async_client.h"
#include "Common.hpp"
#include <queue>

using namespace std;
/**
 * namespace for Queue manager
 */
namespace QMgr
{
/**
 * Queue manager class to manage instances of on-demand operations for msg handling
 */
class CQueueMgr
	{
		bool initSem();

		std::mutex m_queueMutex;
		std::queue <mqtt::const_message_ptr> m_msgQueue;
		bool m_bIsRead;
		bool m_bIsRealTime;

		// delete copy and move constructors and assign operators
		CQueueMgr& operator=(const CQueueMgr&)=delete;	// Copy assign
		CQueueMgr(const CQueueMgr&)=delete;	 			// Copy construct

	public:
		sem_t m_semaphore;

		bool isRead() {return m_bIsRead;};
		bool isRealTime() {return m_bIsRealTime;};

		bool pushMsg(mqtt::const_message_ptr &msg);
		CQueueMgr(bool isRead, bool isRealTime);
		bool isMsgArrived(mqtt::const_message_ptr& msg);

		~CQueueMgr();

		bool getSubMsgFromQ(mqtt::const_message_ptr &msg);

		void cleanup();
	};

	//functions to get on-demand operation instances
	CQueueMgr& getRead();
	CQueueMgr& getWrite();
	CQueueMgr& getRTRead();
	CQueueMgr& getRTWrite();
}
#endif
