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
	PublishJsonHandler(const PublishJsonHandler&) = delete;	 			/// Copy constructor
	PublishJsonHandler& operator=(const PublishJsonHandler&) = delete;	/// assignmnet operator

	/// constructor
	PublishJsonHandler();

	// topic for publish
	std::string m_sPolledDataTopic;
	std::string m_sPolledDataTopic_RT;
	std::string m_sReadResponseTopic;
	std::string m_sReadResponseTopic_RT;
	std::string m_sWriteResponseTopic;
	std::string m_sWriteResponseTopic_RT;

	// topic for subscription
	std::string m_sReadRequestTopic;
	std::string m_sWriteRequestTopic;
	std::string m_sReadRequestTopic_RT;
	std::string m_sWriteRequestTopic_RT;
	std::string m_siteListFileName;
	uint32_t u32CutoffIntervalPercentage;
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
	 * Retrieve polled data topic for realtime
	 * @return polled data topic
	 */
	const std::string& getPolledDataTopicRT() const {
		return m_sPolledDataTopic_RT;
	}

	/**
	 * set polled data topic for realtime
	 * @param sCommomPubTopic :[in] topic to set
	 */
	void setPolledDataTopicRT(const std::string &sCommomPubTopic) {
		m_sPolledDataTopic_RT = sCommomPubTopic;
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
	 * get read request topic for realtime
	 * @return read request topic
	 */
	const std::string& getSReadRequestTopicRT() const {
		return m_sReadRequestTopic_RT;
	}

	/**
	 * set read request topic for realtime
	 * @param sReadRequestTopic :[in] topic to set
	 */
	void setSReadRequestTopicRT(const std::string &sReadRequestTopic) {
		m_sReadRequestTopic_RT = sReadRequestTopic;
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
	 * get read response topic
	 * @return read response topic
	 */
	const std::string& getSReadResponseTopicRT() const {
		return m_sReadResponseTopic_RT;
	}

	/**
	 * set read response topic
	 * @param sReadResponseTopic :[in] topic to set for read response
	 */
	void setSReadResponseTopicRT(const std::string &sReadResponseTopic) {
		m_sReadResponseTopic_RT = sReadResponseTopic;
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
	 * get write request topic
	 * @return write request topic
	 */
	const std::string& getSWriteRequestTopicRT() const {
		return m_sWriteRequestTopic_RT;
	}

	/**
	 * set write request topic
	 * @param sWriteRequestTopic :[in] write request topic to set
	 */
	void setSWriteRequestTopicRT(const std::string &sWriteRequestTopic) {
		m_sWriteRequestTopic_RT = sWriteRequestTopic;
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
	 * get write response topic
	 * @return write response topic
	 */
	const std::string& getSWriteResponseTopicRT() const {
		return m_sWriteResponseTopic_RT;
	}

	/**
	 * set write response topic
	 * @param sWriteResponseTopic :[in] write response topic to set
	 */
	void setSWriteResponseTopicRT(const std::string &sWriteResponseTopic) {
		m_sWriteResponseTopic_RT = sWriteResponseTopic;
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
		return ++m_u16TxId;
		//return m_u16TxId.load();
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

	uint32_t getCutoffIntervalPercentage() const {
		return u32CutoffIntervalPercentage;
	}

	void setCutoffIntervalPercentage(uint32_t cutoffIntervalPercentage) {
		u32CutoffIntervalPercentage = cutoffIntervalPercentage;
	}
};


#endif /* INCLUDE_PUBLISHJSON_HPP_ */
