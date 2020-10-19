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

#include <vector>
#include <atomic>
#include "ZmqHandler.hpp"

#define READ_REQ	 	"RdReq"
#define READ_REQ_RT 	"_RdReq_RT"

#define READ_RES	"_RdResp"
#define READ_RES_RT	"_RdResp_RT"

#define WRITE_REQ 		"_WrReq"
#define WRITE_REQ_RT 	"_WrReq_RT"

#define WRITE_RES 	"_WrResp"
#define WRITE_RES_RT "_WrResp_RT"

#define POLLDATA	"PolledData"
#define POLLDATA_RT	"PolledData_RT"

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

	uint32_t u32CutoffIntervalPercentage;

	std::string m_sAppName;
	std::atomic<unsigned short> m_u16TxId;

	std::mutex m_mutexPublish;

public:
	// function to get single instance of this class
	static PublishJsonHandler& instance();
	bool setTopicForOperation(std::string a_sTopic);

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
	 * get transaction id
	 */
	unsigned short getTxId()
	{
		return ++m_u16TxId;
		//return m_u16TxId.load();
	}

	uint32_t getCutoffIntervalPercentage() const {
		return u32CutoffIntervalPercentage;
	}

	void setCutoffIntervalPercentage(uint32_t cutoffIntervalPercentage) {
		u32CutoffIntervalPercentage = cutoffIntervalPercentage;
	}
};


#endif /* INCLUDE_PUBLISHJSON_HPP_ */
