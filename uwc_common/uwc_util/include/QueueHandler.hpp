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
#include <queue>
#include <string>

	class CMessageObject
	{
		std::string m_sTopic;
		std::string m_Msg;
		struct timespec m_stTs;

		public:
		CMessageObject() : m_sTopic{""}, m_Msg{""}, m_stTs{}
		{
			timespec_get(&m_stTs, TIME_UTC);
		}

		CMessageObject(const std::string &a_sTopic, const std::string &a_Msg) 
			: m_sTopic{a_sTopic}, m_Msg{a_Msg}, m_stTs{}
		{
			timespec_get(&m_stTs, TIME_UTC);
		}

		CMessageObject(const std::string &a_sTopic, const std::string &a_Msg, struct timespec a_stRcvdMsgTs) 
			: m_sTopic{a_sTopic}, m_Msg{a_Msg}, m_stTs{m_stTs} 
		{}

		CMessageObject(const CMessageObject& a_obj)
		: m_sTopic{a_obj.m_sTopic}, m_Msg{a_obj.m_Msg}, m_stTs{a_obj.m_stTs}
		{}

		std::string getTopic() {return m_sTopic;}
		std::string getMsg() {return m_Msg;}
		struct timespec getTimestamp() {return m_stTs;}
	};
	/**
	 * Queue handler class which implements queue operations to be used across modules
	 */
	class CQueueHandler
	{
		bool initSem();

		std::mutex m_queueMutex;
		std::queue<mqtt::const_message_ptr> m_msgQueue;
		std::queue<CMessageObject> m_msgQ;
		sem_t m_semaphore;

		// delete copy and move constructors and assign operators
		CQueueHandler& operator=(const CQueueHandler&)=delete;	// Copy assign
		CQueueHandler(const CQueueHandler&)=delete;	 			// Copy construct

	public:
		CQueueHandler();
		virtual ~CQueueHandler();

		bool pushMsg(mqtt::const_message_ptr msg);
		bool isMsgArrived(mqtt::const_message_ptr& msg);
		bool getSubMsgFromQ(mqtt::const_message_ptr& msg);

		bool pushMsg(CMessageObject msg);
		bool isMsgArrived(CMessageObject& msg);
		bool getSubMsgFromQ(CMessageObject& msg);

		bool breakWaitOnQ();

		void cleanup();
		void clear();
	};
#endif
