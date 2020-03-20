/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#ifndef INCLUDE_TOPICMAPPER_HPP_
#define INCLUDE_TOPICMAPPER_HPP_

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <list>
/*
 * TopicMapper Class to map MQTT topics with ZMQ topics
 */

using namespace std;

class CTopicMapper {
private:
	// Private constructor so that no objects can be created.
	CTopicMapper();
	CTopicMapper(const CTopicMapper & obj){}
	CTopicMapper& operator=(CTopicMapper const&);

	std::string m_strReadRequest;
	std::string m_strRTReadRequest;
	std::string m_strWriteRequest;
	std::string m_strRTWriteRequest;
	std::string m_strAppName;
	std::string m_strMqttExportURL;
#ifdef REALTIME_THREAD_PRIORITY
	int m_intThreadPriority;
	int m_intThreadPolicy;
#endif
	bool m_devMode;

public:
	virtual ~CTopicMapper();

	bool readCommonEnvVariables();
	bool readEnvVariable(const char *pEnvVarName, std::string &storeVal);
	void set_thread_priority();

	const std::string& getStrReadRequest() const {
		return m_strReadRequest;
	}

	void setStrReadRequest(const std::string &strReadRequest) {
		m_strReadRequest = strReadRequest;
	}

	const std::string& getStrWriteRequest() const {
		return m_strWriteRequest;
	}

	void setStrWriteRequest(const std::string &strWriteRequest) {
		m_strWriteRequest = strWriteRequest;
	}

	//real time
	const std::string& getStrRTWriteRequest() const {
		return m_strRTWriteRequest;
	}

	void setStrRTWriteRequest(const std::string &strWriteRequest) {
		m_strRTWriteRequest = strWriteRequest;
	}

	const std::string& getStrRTReadRequest() const {
		return m_strRTReadRequest;
	}

	void setStrRTReadRequest(const std::string &strReadRequest) {
		m_strRTReadRequest = strReadRequest;
	}
	//

	void setStrAppName(const std::string &strAppName) {
		m_strAppName = strAppName;
	}

	const std::string& getStrAppName() const {
		return m_strAppName;
	}

	void setStrMqttExportURL(const std::string &strMqttExportURL) {
		m_strMqttExportURL = strMqttExportURL;
	}

	const std::string& getStrMqttExportURL() const {
		return m_strMqttExportURL;
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
#endif

	static CTopicMapper& getInstance() {
		static CTopicMapper _self;
			return _self;
	}

	bool isDevMode() const {
		return m_devMode;
	}

	void setDevMode(bool devMode) {
		m_devMode = devMode;
	}
};


#endif /* INCLUDE_TOPICMAPPER_HPP_ */
