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
/*
 * TopicMapper Class to map MQTT topics with ZMQ topics
 */

class CTopicMapper {
private:
	// Private constructor so that no objects can be created.
	CTopicMapper();
	CTopicMapper(const CTopicMapper & obj){}
	CTopicMapper& operator=(CTopicMapper const&);

	std::string m_strReadRequest;
	std::string m_strWriteRequest;
	std::string m_strAppName;
	std::string m_strAppVersion;
	std::string m_strMqttExportURL;

public:
	virtual ~CTopicMapper();

	bool readCommonEnvVariables();
	bool readEnvVariable(const char *pEnvVarName, std::string &storeVal);


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

	void setStrAppName(const std::string &strAppName) {
		m_strAppName = strAppName;
	}

	const std::string& getStrAppName() const {
		return m_strAppName;
	}

	void setStrAppVersion(const std::string &strAppVersion) {
		m_strAppVersion = strAppVersion;
	}

	const std::string& getStrAppVersion() const {
		return m_strAppVersion;
	}

	void setStrMqttExportURL(const std::string &strMqttExportURL) {
		m_strMqttExportURL = strMqttExportURL;
	}

	const std::string& getStrMqttExportURL() const {
		return m_strMqttExportURL;
	}
	static CTopicMapper& getInstance() {
		static CTopicMapper _self;
			return _self;
	}
};


#endif /* INCLUDE_TOPICMAPPER_HPP_ */
