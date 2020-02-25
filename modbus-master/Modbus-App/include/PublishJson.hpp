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

	/**
	 * Retrieve polled data topic
	 * @return polled data topic
	 */
	const std::string& getPolledDataTopic() const {
		return m_sPolledDataTopic;
	}

	/**
	 * set polled data topic
	 * @param sCommomPubTopic :[in] topic to set
	 */
	void setPolledDataTopic(const std::string &sCommomPubTopic) {
		m_sPolledDataTopic = sCommomPubTopic;
	}

	/**
	 * get read request topic
	 * @return read request topic
	 */
	const std::string& getSReadRequestTopic() const {
		return m_sReadRequestTopic;
	}

	/**
	 * set read request topic
	 * @param sReadRequestTopic :[in] topic to set
	 */
	void setSReadRequestTopic(const std::string &sReadRequestTopic) {
		m_sReadRequestTopic = sReadRequestTopic;
	}

	/**
	 * get read response topic
	 * @return read response topic
	 */
	const std::string& getSReadResponseTopic() const {
		return m_sReadResponseTopic;
	}

	/**
	 * set read response topic
	 * @param sReadResponseTopic :[in] topic to set for read response
	 */
	void setSReadResponseTopic(const std::string &sReadResponseTopic) {
		m_sReadResponseTopic = sReadResponseTopic;
	}

	/**
	 * get write request topic
	 * @return write request topic
	 */
	const std::string& getSWriteRequestTopic() const {
		return m_sWriteRequestTopic;
	}

	/**
	 * set write request topic
	 * @param sWriteRequestTopic :[in] write request topic to set
	 */
	void setSWriteRequestTopic(const std::string &sWriteRequestTopic) {
		m_sWriteRequestTopic = sWriteRequestTopic;
	}

	/**
	 * get write response topic
	 * @return write response topic
	 */
	const std::string& getSWriteResponseTopic() const {
		return m_sWriteResponseTopic;
	}

	/**
	 * set write response topic
	 * @param sWriteResponseTopic :[in] write response topic to set
	 */
	void setSWriteResponseTopic(const std::string &sWriteResponseTopic) {
		m_sWriteResponseTopic = sWriteResponseTopic;
	}

	/**
	 * get app name
	 * @return app name
	 */
	const std::string& getAppName() const {
		return m_sAppName;
	}

	/**
	 * set app name
	 * @param appName :[in] app name to set
	 */
	void setAppName(const std::string &appName) {
		m_sAppName = appName;
	}

	/**
	 * get site list from file name
	 * @return site list
	 */
	const std::string& getSiteListFileName() const {
		return m_siteListFileName;
	}

	/**
	 * set site list from file name
	 * @param siteListFileName	:[in] site list to set
	 */
	void setSiteListFileName(const std::string &siteListFileName) {
		m_siteListFileName = siteListFileName;
	}

	/**
	 * get transaction id
	 */
	unsigned short getTxId()
	{
		m_u16TxId++;
		return m_u16TxId;
	}

	/**
	 * check if it is dev mode
	 * @return whether dev mode
	 */
	bool isDevMode() const {
		return m_devMode;
	}

	/**
	 * set dev mode
	 * @param devMode :[in] dev mode to set
	 */
	void setDevMode(bool devMode) {
		m_devMode = devMode;
	}

	/**
	 * get sub topic list
	 * @return sub topic list
	 */
	const std::vector<std::string>& getSubTopicList() const {
		return subTopicList;
	}

	/**
	 *insert sub topic in list
	 * @param a_sTopic :[in] topic to insert in list
	 */
	void insertSubTopicInList(const std::string &a_sTopic) {
		subTopicList.push_back(a_sTopic);
	}

#ifdef REALTIME_THREAD_PRIORITY
	/**
	 * set thread priority
	 * @param strThreadPriority :[in] priority to set
	 */
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

	/**
	 * get thread priority
	 * @return thread priority
	 */
	const int& getIntThreadPriority() const {
		return m_intThreadPriority;
	}

	/**
	 * set thread policy
	 * @param strThreadPolicy :[in] thread policy to set
	 */
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

	/**
	 * get thread policy to set
	 * @return thread policy to set
	 */
	const int& getIntThreadPolicy() const {
		return m_intThreadPolicy;
	}

	void set_thread_priority();
#endif
};


#endif /* INCLUDE_PUBLISHJSON_HPP_ */
