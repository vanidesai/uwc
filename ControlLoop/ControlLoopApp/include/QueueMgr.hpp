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
#include <map>
#include <semaphore.h>
#include "mqtt/async_client.h"
#include "Common.hpp"
#include "EISMsgbusHandler.hpp"
#include <queue>
#include <time.h>

using namespace std;
/**
 * namespace for Queue manager
 */
namespace QMgr
{

	struct stEISMsg
	{
		msg_envelope_t *m_pEISMsg;
		struct timespec m_stRcvdMsgTs;

		stEISMsg(): m_pEISMsg{0}, m_stRcvdMsgTs{0} {;};
		stEISMsg(msg_envelope_t *a_pEISMsg, struct timespec a_stRcvdMsgTs)
				: m_pEISMsg{a_pEISMsg}, m_stRcvdMsgTs{a_stRcvdMsgTs} {;};
	};
	class CControlLoopMapper
	{
	private:
		std::map<std::string, struct stEISMsg> m_mapPollWrite;
		std::mutex m_mapMutex;

		CControlLoopMapper() {};
	public:
		static CControlLoopMapper& getInstace()
		{
			static CControlLoopMapper _self;
			return _self;
		}
		void insertForTracking(std::string a_sKey, struct stEISMsg &a_stEISMsg)
		{
			std::unique_lock<std::mutex> lck(m_mapMutex);
			m_mapPollWrite.insert(std::pair <std::string, struct stEISMsg> (a_sKey, a_stEISMsg));
		}
		void getForProcessing(std::string a_sKey, struct stEISMsg &a_stEISMsg)
		{
			std::unique_lock<std::mutex> lck(m_mapMutex);
			a_stEISMsg = m_mapPollWrite.at(a_sKey);
			m_mapPollWrite.erase(a_sKey);
		}
	};
	class CEISMsgQMgr
	{
		bool initSem();

		std::mutex m_queueMutex;
		std::queue <struct stEISMsg> m_msgQueue;
		bool m_bIsPollQ;
		// delete copy and move constructors and assign operators
		CEISMsgQMgr& operator=(const CEISMsgQMgr&)=delete;	// Copy assign
		CEISMsgQMgr(const CEISMsgQMgr&)=delete;	 			// Copy construct

		sem_t m_semaphore;
	public:
		CEISMsgQMgr(bool a_bIsPollQ);
		~CEISMsgQMgr();

		bool pushMsg(msg_envelope_t *a_pMsg, struct timespec& a_stTs);
		bool waitForMsg(struct stEISMsg &a_stEISMsg);
	};

	CEISMsgQMgr& PollMsgQ();
	CEISMsgQMgr& WriteRespMsgQ();
}
#endif
