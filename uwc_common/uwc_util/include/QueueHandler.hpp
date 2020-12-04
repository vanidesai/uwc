/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/
/*** QueueHandler.hpp is used to handle mqtt message queue.*/

#ifndef QHANDLER_HPP_
#define QHANDLER_HPP_

#include <atomic>
#include <map>
#include <semaphore.h>
#include "mqtt/async_client.h"
#include <queue>
#include <string>

	/**
	 * Message Object class which handles mqtt message, time & topic related operations
	 */
	class CMessageObject
	{
		mqtt::const_message_ptr m_mqttMsg; /** mqtt-export message*/
		struct timespec m_stTs;

		public:
		CMessageObject() : m_mqttMsg{}, m_stTs{}
		{
			timespec_get(&m_stTs, TIME_UTC);
		}

		CMessageObject(const std::string &a_sTopic, const std::string &a_sMsg) 
			: m_stTs{}
		{
			m_mqttMsg = mqtt::make_message(a_sTopic, a_sMsg);
			timespec_get(&m_stTs, TIME_UTC);
		}
		
		CMessageObject(mqtt::const_message_ptr a_mqttMsg) 
			: m_mqttMsg{a_mqttMsg}, m_stTs{}
		{
			timespec_get(&m_stTs, TIME_UTC);
		}

		
		CMessageObject(const CMessageObject& a_obj)
		: m_mqttMsg{a_obj.m_mqttMsg}, m_stTs{a_obj.m_stTs}
		{}

		CMessageObject& operator=(const CMessageObject &a_obj)
	    { 
	    	m_mqttMsg = a_obj.m_mqttMsg;
	    	m_stTs = a_obj.m_stTs;
	        return *this; 
	    }

		/** function to get topic*/
		std::string getTopic() 
		{
			if (NULL == m_mqttMsg)
			{
				return "";
			}
			return m_mqttMsg->get_topic();
		}
		/** function to string msg*/
		std::string getStrMsg() 
		{
			if (NULL == m_mqttMsg)
			{
				return "";
			}
			return m_mqttMsg->get_payload();
		}
		mqtt::const_message_ptr& getMqttMsg() {return m_mqttMsg;}
		struct timespec getTimestamp() {return m_stTs;}
	};
	/**
	 * Queue handler class which implements queue operations to be used across modules
	 */
	class CQueueHandler
	{
		bool initSem();

		std::mutex m_queueMutex; /** queue mutex*/
		std::queue<CMessageObject> m_msgQ; /** message queue*/
		sem_t m_semaphore;/** semaphore*/

		// delete copy and move constructors and assign operators
		CQueueHandler& operator=(const CQueueHandler&)=delete;	// Copy assign
		CQueueHandler(const CQueueHandler&)=delete;	 			// Copy construct

	public:
		CQueueHandler();//default constructor
		virtual ~CQueueHandler();

		bool pushMsg(CMessageObject msg);
		bool isMsgArrived(CMessageObject& msg);
		bool getSubMsgFromQ(CMessageObject& msg);

		bool breakWaitOnQ();

		void cleanup();
		void clear();
	};
#endif
