/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#include "TopicMapper.hpp"

#include "ConfigManager.hpp"

CTopicMapper::CTopicMapper() {
	// TODO Auto-generated constructor stub
	//ParseJson();
	readCommonEnvVariables();
}

/** This function is used to read environment variable
 *
 * @sEnvVarName : environment variable to be read
 * @storeVal : variable to store env variable value
 * @return: true/false based on success or error
 */
bool CTopicMapper::readEnvVariable(const char *pEnvVarName, string &storeVal)
{
	bool bRetVal = false;
	char *cEvar = getenv(pEnvVarName);
	if (NULL != cEvar)
	{
		bRetVal = true;
		std::string tmp (cEvar);
		storeVal = tmp;
		CLogger::getInstance().log(INFO, LOGDETAILS(std::string(pEnvVarName) + " environment variable is set to ::" + storeVal));
	}
	else
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS(std::string(pEnvVarName) + " environment variable is not found"));

	}
	return bRetVal;
}

/** This function is used to read common environment variables
 *
 * @return: true/false based on success or error
 */
bool CTopicMapper::readCommonEnvVariables()
{
	bool bRetVal = false;

	bRetVal = readEnvVariable("ReadRequest", m_strReadRequest);
	if(!bRetVal)
	{
		return false;
	}

	bRetVal = readEnvVariable("WriteRequest", m_strWriteRequest);
	if(!bRetVal)
	{
		return false;
	}
	return bRetVal;
}

CTopicMapper::~CTopicMapper() {
	// TODO Auto-generated destructor stub
}

/*
std::vector<std::string> CTopicMapper::GetMqttTopics()
{
	std::vector<std::string> allMQTTTopics;
	for(auto topic : m_MQTTopics) {
		if(! topic.first.empty())
			allMQTTTopics.push_back(topic.first);
	}
	return allMQTTTopics;
}

//return MQTT topic that is mapped with ZMQ
std::string CTopicMapper::GetMQTTopic(std::string topic) {

	//std::cout << "Topic to find : " << topic << "";
	std::string strMqttTopic = "";

	try{
		if(m_ZMQTopics.empty()) {
			CLogger::getInstance().log(DEBUG, LOGDETAILS("ZMQ map is empty"));
		}
		else
		{
			//get topic from map
			auto itrZMQTopic = m_ZMQTopics.find(topic);

			if(itrZMQTopic != m_ZMQTopics.end())
				strMqttTopic = itrZMQTopic->second;
		}
	} catch(std::exception &e) {
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}
	//std::cout << "Found matching MQTT topic : " << strMqttTopic << "";
	return strMqttTopic;
}

//return ZMQ topic that is mapped with MQTT
std::string CTopicMapper::GetZMQTopic(std::string topic) {

	std::string strZmqTopic = "";

	try
	{
	if(m_MQTTopics.empty()){
		CLogger::getInstance().log(ERROR, LOGDETAILS("MQTT map is empty"));
	}
	else
	{
		//get topic from map
		auto itrMQTTTopic = m_MQTTopics.find(topic);

		if(itrMQTTTopic != m_MQTTopics.end())
			strZmqTopic = itrMQTTTopic->second;
	}
	}catch(std::exception &e) {
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}
	return strZmqTopic;
}

*/

