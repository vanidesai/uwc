/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#ifndef INCLUDE_PUBLISHJSON_HPP_
#define INCLUDE_PUBLISHJSON_HPP_

#include "Common.hpp"
#include <eis/utils/thread_safe_queue.h>
#include <eis/utils/config.h>
#include <eis/utils/json_config.h>
#include <eis/msgbus/msgbus.h>
#include <eis/config_manager/env_config.h>
#include <eis/config_manager/config_manager.h>

/**
 * Structure to contain state for a publisher thread
 */
typedef struct {
    msg_envelope_t* msg;
    publisher_ctx_t* pub_ctx;
} pub_thread_ctx_t;

class PublishJsonHandler
{
	/// delete copy and move constructors and assign operators
	PublishJsonHandler(const PublishJsonHandler&) = delete;	 			/// Copy construct
	PublishJsonHandler& operator=(const PublishJsonHandler&) = delete;	/// Copy assign

	/// constructor
	PublishJsonHandler();

	// topic for publish
	std::string m_sPolledDataTopic;
	std::string m_sReadResponseTopic;
	std::string m_sWriteResponseTopic;

	// topic for subscription
	std::string m_sReadRequestTopic;
	std::string m_sWriteRequestTopic;
	std::string m_siteListFileName;
#ifdef REALTIME_THREAD_PRIORITY
	int m_intThreadPolicy;
	int m_intThreadPriority;
#endif
	std::vector<std::string> subTopicList;

	std::string m_sAppName;
	bool m_devMode;
	std::atomic<unsigned short> m_u16TxId;

public:
	// function to get single instance of this class
	static PublishJsonHandler& instance();
	BOOLEAN publishJson(msg_envelope_t* msg, void* msgbus_ctx, void* pub_ctx, const std::string str_Topic);

	const std::string& getPolledDataTopic() const {
		return m_sPolledDataTopic;
	}

	void setPolledDataTopic(const std::string &sCommomPubTopic) {
		m_sPolledDataTopic = sCommomPubTopic;
	}

	const std::string& getSReadRequestTopic() const {
		return m_sReadRequestTopic;
	}

	void setSReadRequestTopic(const std::string &sReadRequestTopic) {
		m_sReadRequestTopic = sReadRequestTopic;
	}

	const std::string& getSReadResponseTopic() const {
		return m_sReadResponseTopic;
	}

	void setSReadResponseTopic(const std::string &sReadResponseTopic) {
		m_sReadResponseTopic = sReadResponseTopic;
	}

	const std::string& getSWriteRequestTopic() const {
		return m_sWriteRequestTopic;
	}

	void setSWriteRequestTopic(const std::string &sWriteRequestTopic) {
		m_sWriteRequestTopic = sWriteRequestTopic;
	}

	const std::string& getSWriteResponseTopic() const {
		return m_sWriteResponseTopic;
	}

	void setSWriteResponseTopic(const std::string &sWriteResponseTopic) {
		m_sWriteResponseTopic = sWriteResponseTopic;
	}

	const std::string& getAppName() const {
		return m_sAppName;
	}

	void setAppName(const std::string &appName) {
		m_sAppName = appName;
	}

	const std::string& getSiteListFileName() const {
		return m_siteListFileName;
	}

	void setSiteListFileName(const std::string &siteListFileName) {
		m_siteListFileName = siteListFileName;
	}

	unsigned short getTxId()
	{
		m_u16TxId++;
		return m_u16TxId;
	}

	bool isDevMode() const {
		return m_devMode;
	}

	void setDevMode(bool devMode) {
		m_devMode = devMode;
	}

	const std::vector<std::string>& getSubTopicList() const {
		return subTopicList;
	}

	void insertSubTopicInList(const std::string &a_sTopic) {
		subTopicList.push_back(a_sTopic);
	}

#ifdef REALTIME_THREAD_PRIORITY
	void setStrThreadPriority(const std::string &strThreadPriority) {
		try {
			std::string::size_type sz;   // alias of size_t
			m_intThreadPriority = std::stoi(strThreadPriority, &sz);

			/*Processes scheduled under one of the real-time policies (SCHED_FIFO,
			       SCHED_RR) have a sched_priority value in the range 1 (low) to 99
			       (high)*/
			if(m_intThreadPriority < 1 || m_intThreadPriority > 99) {
				m_intThreadPolicy = 50;//medium priority
			}
		}catch(exception &ex) {
			m_intThreadPriority = -1;
		}
	}

	const int& getIntThreadPriority() const {
		return m_intThreadPriority;
	}

	void setStrThreadPolicy(const std::string &strThreadPolicy) {
		try {
			std::string::size_type sz;   // alias of size_t
			m_intThreadPolicy = std::stoi(strThreadPolicy, &sz);

			if(m_intThreadPolicy < 0 || m_intThreadPolicy > 2) {
				m_intThreadPolicy = SCHED_RR;
			}
		}catch(exception &ex) {
			m_intThreadPolicy = -1;
		}
	}

	const int& getIntThreadPolicy() const {
		return m_intThreadPolicy;
	}

	void set_thread_priority();
#endif
};


#endif /* INCLUDE_PUBLISHJSON_HPP_ */
