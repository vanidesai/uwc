/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#include <iostream>
#include <thread>
#include <vector>
#include <iterator>
#include <assert.h>

#include "TopicMapper.hpp"
#include "ZmqToMqtt.hpp"
#include "MQTTHandler.hpp"
#include "ConfigManager.hpp"

//using namespace std;

std::string parse_msg(const char *json) {

	std::string topic_name = "";
	try {
		cJSON *root = cJSON_Parse(json);

		char *ctopic_name = cJSON_GetObjectItem(root, "topic")->valuestring;
		topic_name = ctopic_name;

		if(NULL != ctopic_name)
			free(ctopic_name);
		if(NULL != root)
			free(root);

	} catch (std::exception &ex) {
		std::cout << __func__ << "Exception received: " << ex.what() << std::endl;
	}

	return topic_name;
}

void listenOnEIS(string topic, stZmqContext context, stZmqSubContext subContext) {

	void* msgbus_ctx = context.m_pContext;
	recv_ctx_t* sub_ctx = subContext.m_pContext;

	std::cout << __func__ << ": ZMQ listening for topic : " << topic << std::endl;
	while (msgbus_ctx != NULL)
	{
		try
		{
			int num_parts = 0;
			msg_envelope_t *msg = NULL;
			msg_envelope_serialized_part_t *parts = NULL;
			msgbus_ret_t ret;

			ret = msgbus_recv_wait(msgbus_ctx, sub_ctx, &msg);
			if (ret != MSG_SUCCESS) {
				// Interrupt is an acceptable error
				if (ret == MSG_ERR_EINTR)
					break;
				std::cout << __func__ << ": Error: Failed to receive message errno: " << ret << std::endl;
				continue;
			}

			num_parts = msgbus_msg_envelope_serialize(msg, &parts);
			if (num_parts <= 0) {
				std::cout << __func__ << ": Error: Failed to serialize message\n";
				continue;
			}

			std::string revdTopic = parse_msg(parts[0].bytes);

			if(revdTopic == "")
			{
				std::cout << __func__ << " topic key not present in message: " << parts[0].bytes;
			}
			else
			{
				std::string mqttTopic = CTopicMapper::getInstance().GetMQTTopic(revdTopic);
				if(mqttTopic == "")
				{
					std::cout << __func__ << ": Error: Could not find mapped MQTT topic for ZMQ topic : " << revdTopic << std::endl;
				}
				else
				{
					//std::cout << __func__ << " Received mapped MQTT topic : " << mqttTopic << "\n";
					//publish data to MQTT
					CMQTTHandler::instance().publish(parts[0].bytes, mqttTopic.c_str());
					//end of publish on MQTT
				}
			}
			msgbus_msg_envelope_serialize_destroy(parts, num_parts);
			msgbus_msg_envelope_destroy(msg);
			msg = NULL;
			parts = NULL;
		}
		catch (exception &ex)
		{
			std::cout << __func__ << "Exception " << ex.what() << " for topic : " << topic << std::endl;
		}
	}//while ends
}

bool initEISContext()
{

	CZmqToMqtt objZmqToMqtt;

	// Initializing all the pub/sub topic base context for ZMQ
	if(std::getenv("PubTopics") != NULL)
	{
		// TODO used in future
	}
	if(std::getenv("SubTopics") != NULL)
	{
		std::cout << __func__ << " List of topic configured for Sub are :: " << getenv("SubTopics")<<endl;
		bool bRetVal = objZmqToMqtt.prepareCommonContext("sub");
		if(!bRetVal)
		{
			std::cout << __func__ << " Context creation failed for sub topic \n";
		}
	}

	//start thread while loop through all the topics
	vector<string> vFullTopics = CfgManager::Instance().getEnvConfig().get_topics_from_env("sub");

	vector<std::thread> vThreads;

	for(auto topic : vFullTopics)
	{
		std::size_t pos = topic.find('/');
		if(std::string::npos != pos)
		{
			std::string subTopic{topic.substr(pos+1)};
			stZmqContext context = objZmqToMqtt.getCTX(subTopic);
			//will give topic context
			stZmqSubContext subContext = objZmqToMqtt.getSubCTX(subTopic);

			std::cout << __func__ << "Info: Full topic - " << topic << " AND listening on: " << subTopic << std::endl;

			vThreads.push_back(std::thread(listenOnEIS, subTopic, context, subContext));
		}
		else
		{
			std::cout << __func__ << ": Error: Incorrect topic name format: " << topic << std::endl;
		}
	}
	for(auto &th : vThreads) {
		if(th.joinable())
			th.join();
	}

	return true;
}

int main(int argc, char *argv[]) {

// 1. Form topic mapper
// 2. Prepare MQTT connection
// 3. Prepare ZMQ contexts
// 4. Once all things needed for subscripton is done,
//		=> Prepare ZMQ listeners
//		=> Prepare MQTT listeners

	try
	{
		CTopicMapper::getInstance();
		CMQTTHandler::instance();
		initEISContext();
	}
	catch(std::exception &e)
	{
		std::cout << __func__ << " Error::Exception is ::" << e.what() << std::endl << "Exiting\n";
	}
	catch(...)
	{
		std::cout << __func__ << " Error::Unknown Exception Occurred. Exiting\n";
	}

	return 0;
}
