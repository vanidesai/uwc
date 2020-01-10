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
	ParseJson();
}

void CTopicMapper::ParseJson() {
	try {

		CLogger::getInstance().log(DEBUG, LOGDETAILS("Parsing json for topic mapping"));

		if(!CfgManager::Instance().IsClientCreated())
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("ETCD client is not created .."));
			return;
		}
		char *cEtcdValue  = CfgManager::Instance().getETCDValuebyKey("config");
		if(NULL == cEtcdValue)
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("No value received from ETCD"));
			return;
		}

		//parse from root element
		cJSON *root = cJSON_Parse(cEtcdValue);
		if(NULL == root)
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("Could not parse value received from ETCD."));
			return;
		}

		if(! cJSON_HasObjectItem(root, "Mapping")) {
			CLogger::getInstance().log(ERROR, LOGDETAILS("Topic json does not have 'mappings' key"));
			if (NULL != root)
				free(root);

			return;
		}

		// Now let's iterate through the Mapping array
		cJSON *mappings = cJSON_GetObjectItem(root, "Mapping");
		if(NULL == mappings)
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("Could not get mapping from JSON config present in ETCD."));
			cJSON_Delete(root);
			return;
		}

		// Get the count
		int mappings_count = cJSON_GetArraySize(mappings);

		for (int i = 0; i < mappings_count; i++) {

			// Get the JSON element and then get the values as before
			cJSON *msgSrc = cJSON_GetArrayItem(mappings, i);
			if(NULL == msgSrc)
			{
				CLogger::getInstance().log(ERROR, LOGDETAILS("Could not get JSON object for iteration " + std::to_string(i)));
				continue;
			}

			char *srcName =  cJSON_GetObjectItem(msgSrc, "MsgSource")->valuestring;
			if(NULL == srcName)
			{
				CLogger::getInstance().log(ERROR, LOGDETAILS("Wrong configuration: MsgSource key not found for iteration " + std::to_string(i)));
				continue;
			}

			cJSON *topics = cJSON_GetObjectItem(msgSrc, "Topics");
			if(NULL == topics)
			{
				CLogger::getInstance().log(ERROR, LOGDETAILS("Wrong configuration: Topic mapper does not have Topics"));
				continue;
			}
			std::string sourceName(srcName);

			cJSON *topic = NULL;
			cJSON_ArrayForEach(topic, topics)
			{
				if(NULL == topic)
				{
					// This should ideally not happen
					continue;
				}
				if(NULL == topic->child->string)
				{
					// Again unlikely scenario
					CLogger::getInstance().log(ERROR, LOGDETAILS("Key is not found"));
					continue;
				}

				std::string strTopicName{topic->child->string};

				if(NULL == topic->child->valuestring)
				{
					// Again unlikely scenario
					CLogger::getInstance().log(ERROR, LOGDETAILS("Value is not found for key: " + strTopicName));
					continue;
				}

				std::string strTopicValue{topic->child->valuestring};

				if(sourceName == "MQTT") {
					m_MQTTopics.insert(std::make_pair(strTopicName, strTopicValue));
				}
				else if(sourceName == "ZeroMQ") {
					m_ZMQTopics.insert(std::make_pair(strTopicName, strTopicValue));
				}
			}
		}

		if(NULL != root)
			cJSON_Delete(root);

	} catch (std::exception &e) {
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}
}

CTopicMapper::~CTopicMapper() {
	// TODO Auto-generated destructor stub
}

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



