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

		std::cout << __func__ << " Parsing json for topic mapping\n";

		if(!CfgManager::Instance().IsClientCreated())
		{
			std::cout << __func__ << " ETCD client is not created ..\n";
			return;
		}
		char *cEtcdValue  = CfgManager::Instance().getETCDValuebyKey("config");
		if(NULL == cEtcdValue)
		{
			std::cout << __func__ << " No value received from ETCD\n";
			return;
		}

		//parse from root element
		cJSON *root = cJSON_Parse(cEtcdValue);
		if(NULL == root)
		{
			std::cout << __func__ << " Could not parse value received from ETCD.\n";
			return;
		}

		// Now let's iterate through the Mapping array
		cJSON *mappings = cJSON_GetObjectItem(root, "Mapping");
		if(NULL == mappings)
		{
			std::cout << __func__ << " Could not get mapping from JSON config present in ETCD.\n";
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
				std::cout << __func__ << " Could not get JSON object for iteration " << i << std::endl;
				continue;
			}

			char *srcName =  cJSON_GetObjectItem(msgSrc, "MsgSource")->valuestring;
			if(NULL == srcName)
			{
				std::cout << __func__ << " Wrong configuration: MsgSource key not found for iteration " << i << std::endl;
				continue;
			}

			cJSON *topics = cJSON_GetObjectItem(msgSrc, "Topics");
			if(NULL == topics)
			{
				std::cout << __func__ << " Wrong configuration: Topic mapping not listed for " << msgSrc << std::endl;
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
					std::cout << __func__ << " Key is not found \n";
					continue;
				}

				std::string strTopicName{topic->child->string};

				if(NULL == topic->child->valuestring)
				{
					// Again unlikely scenario
					std::cout << __func__ << " Value is not found for key: " << strTopicName << std::endl;
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
		std::cout << __func__ << ": Exception: " << e.what() << std::endl;
	}
}

CTopicMapper::~CTopicMapper() {
	// TODO Auto-generated destructor stub
}

std::vector<std::string> CTopicMapper::GetMqttTopics()
{
	std::vector<std::string> allMQTTTopics;
	for(auto topic : m_MQTTopics)
		allMQTTTopics.push_back(topic.first);

	return allMQTTTopics;
}

//return MQTT topic that is mapped with ZMQ
std::string CTopicMapper::GetMQTTopic(std::string topic) {

	//std::cout << "Topic to find : " << topic << "\n";
	std::string strMqttTopic = "";

	try{
		if(m_ZMQTopics.empty()) {
			std::cout << __func__ << ": ZMQ map is empty !\n";
		}
		else
		{
			//get topic from map
			auto itrZMQTopic = m_ZMQTopics.find(topic);

			if(itrZMQTopic != m_ZMQTopics.end())
				strMqttTopic = itrZMQTopic->second;
		}
	} catch(std::exception &e) {
		std::cout << __func__ << ": Exception : " << e.what() << " for topic " << topic << std::endl;
	}
	//std::cout << "Found matching MQTT topic : " << strMqttTopic << "\n";
	return strMqttTopic;
}

//return ZMQ topic that is mapped with MQTT
std::string CTopicMapper::GetZMQTopic(std::string topic) {

	std::string strZmqTopic = "";

	try
	{
	if(m_MQTTopics.empty()){
		std::cout << __func__ << " MQTT map is empty !\n";
	}
	else
	{
		//get topic from map
		auto itrMQTTTopic = m_MQTTopics.find(topic);

		if(itrMQTTTopic != m_MQTTopics.end())
			strZmqTopic = itrMQTTTopic->second;
	}
	}catch(std::exception &e) {
		std::cout << __func__ << ": Exception : " << e.what() << " for topic " << topic << std::endl;
	}
	return strZmqTopic;
}



