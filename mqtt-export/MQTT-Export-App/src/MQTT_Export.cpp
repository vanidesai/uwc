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
#include <semaphore.h>
#include <csignal>

#include "EISMsgbusHandler.hpp"
#include "TopicMapper.hpp"
#include "MQTTHandler.hpp"
#include "ConfigManager.hpp"

#ifdef UNIT_TEST
#include <gtest/gtest.h>
#endif

vector<std::thread> g_vThreads;
extern sem_t g_semaphoreRespProcess;

std::atomic<bool> g_shouldStop(false);

std::string parse_msg(const char *json) {

	std::string topic_name = "";
	try {
		cJSON *root = cJSON_Parse(json);
		if (NULL == root) {
			std::cout << __func__
					<< "Message received from ZMQ could not be parsed in json format\n";
			return topic_name;
		}

		if(! cJSON_HasObjectItem(root, "topic")) {
			std::cout << __func__ << " Message received from ZMQ does not have key : topic\n";
			if (NULL != root)
				free(root);

			return topic_name;
		}

		char *ctopic_name = cJSON_GetObjectItem(root, "topic")->valuestring;
		if (NULL == ctopic_name) {
			std::cout << __func__
					<< "Key 'topic' could not be found in message received from ZMQ\n";

			if (NULL != root)
				free(root);

			return topic_name;
		}

		topic_name.assign(ctopic_name);

		if (NULL != ctopic_name)
			free(ctopic_name);
		if (NULL != root)
			free(root);

	} catch (std::exception &ex) {
		std::cout << __func__ << "Exception received: " << ex.what()
				<< std::endl;
	}

	return topic_name;
}

//listens on EIS and sends data to MQTT
void listenOnEIS(string topic, stZmqContext context,
		stZmqSubContext subContext) {

	void *msgbus_ctx = context.m_pContext;
	recv_ctx_t *sub_ctx = subContext.m_pContext;

	std::cout << __func__ << ": ZMQ listening for topic : " << topic
			<< std::endl;
	while ((false == g_shouldStop.load()) && (msgbus_ctx != NULL)) {

		try {
			int num_parts = 0;
			msg_envelope_t *msg = NULL;
			msg_envelope_serialized_part_t *parts = NULL;
			msgbus_ret_t ret;

			ret = msgbus_recv_wait(msgbus_ctx, sub_ctx, &msg);
			if (ret != MSG_SUCCESS) {
				// Interrupt is an acceptable error
				if (ret == MSG_ERR_EINTR) {
					std::cout << __func__ << " received MSG_ERR_EINT" << std::endl;
					break;
				}
				std::cout << __func__
						<< ": Error: Failed to receive message errno: " << ret
						<< std::endl;
				continue;
			}

			num_parts = msgbus_msg_envelope_serialize(msg, &parts);
			if (num_parts <= 0) {
				std::cout << __func__
						<< ": Error: Failed to serialize message\n";
				//continue;
			}
			else if(NULL != parts) {
				std::string revdTopic(parse_msg(parts[0].bytes));

				if (revdTopic == "") {
					std::cout << __func__ << " topic key not present in message: "
							<< parts[0].bytes;
				} else {
					//std::cout << "received topic : " << revdTopic << "\n";
					std::string mqttTopic = CTopicMapper::getInstance().GetMQTTopic(
							revdTopic);
					if (mqttTopic == "") {
						std::cout << __func__
								<< ": Error: Could not find mapped MQTT topic for ZMQ topic : "
								<< revdTopic << std::endl;
					} else {
						string mqttMsg(parts[0].bytes);
						//publish data to MQTT
						std::cout << "ZMQ Message: Time: "
							<< std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()
							<< ", Msg: " << mqttMsg << std::endl;

						CMQTTHandler::instance().publish(mqttMsg,
								mqttTopic.c_str());
					}
				}
				msgbus_msg_envelope_serialize_destroy(parts, num_parts);
			}
			msgbus_msg_envelope_destroy(msg);
			msg = NULL;
			parts = NULL;
		} catch (exception &ex) {
			std::cout << __func__ << "Exception " << ex.what()
					<< " for topic : " << topic << std::endl;
		}
	}//while ends

	std::cout << __func__ << " exited !!\n";
}

//publish message to EIS
bool publishEISMsg(string eisMsg, stZmqContext &context,
		stZmqPubContext &pubContext) {

	bool retVal = false;
	// Creating message to be published
	msg_envelope_t *msg = msgbus_msg_envelope_new(CT_JSON);

	try {

		if(msg == NULL) {
			std::cout << __func__ << " could not create new msg envelope" << std::endl;
			return retVal;
		}

		//parse from root element
		cJSON *root = cJSON_Parse(eisMsg.c_str());
		if (NULL == root) {
			std::cout << __func__
					<< " Could not parse value received from MQTT\n";

			if(msg != NULL)
				msgbus_msg_envelope_destroy(msg);

			return retVal;
		}

		cJSON *device = root->child;
		while (device) {
			std::cout << device->string << " : " << device->valuestring
					<< std::endl;

			msg_envelope_elem_body_t *value = msgbus_msg_envelope_new_string(
					device->valuestring);
			msgbus_msg_envelope_put(msg, device->string, value);

			// get and print key
			device = device->next;
		}
		if (device)
			delete (device);

		if (root)
			delete (root);

		msgbus_publisher_publish(context.m_pContext,
				(publisher_ctx_t*) pubContext.m_pContext, msg);
		if(msg != NULL)
			msgbus_msg_envelope_destroy(msg);

		retVal = true;

	} catch (exception &ex) {
		std::cout << __func__ << " Exception : " << ex.what() << std::endl;
		if(msg != NULL)
			msgbus_msg_envelope_destroy(msg);
		retVal = false;
	}
	return retVal;
}

//reads data from queue filled up by MQTT and sends data to EIS
void postMsgsToEIS() {
	std::cout << __func__ << " Starting thread to send messages on EIS\n";

	try {
		mqtt::const_message_ptr recvdMsg;
		while (false == g_shouldStop.load()) {

			sem_wait(&g_semaphoreRespProcess);

			if (false == CMQTTHandler::instance().getSubMsgFromQ(recvdMsg)) {
				std::cout << "\nNo message to send to EIS in queue\n";
				continue;
			}

			//received msg from queue
			string rcvdTopic = recvdMsg->get_topic();
			string strMsg = recvdMsg->get_payload();

			//this should be present in each incoming request
			if (rcvdTopic == "") //will not be the case ever
					{
				std::cout << __func__ << " topic key not present in message: "
						<< strMsg;
				continue;
			}

			std::string eisTopic = CTopicMapper::getInstance().GetZMQTopic(rcvdTopic);
			if (eisTopic == "")
			{
				std::cout << __func__
						<< ": Error: Could not find mapped EIS topic for MQTT topic : "
						<< rcvdTopic << std::endl;
				continue;
			}
			else
			{
				//publish data to EIS
				std::cout << __func__ << " Received mapped EIS topic : " << eisTopic << "\n";

				//get EIS msg bus context
				stZmqContext context;
				if (!CEISMsgbusHandler::Instance().getCTX(eisTopic, context)) {
					std::cout << __func__
							<< " cannot find msgbus context for topic : "
							<< eisTopic << std::endl;
					continue;
				}

				//will give topic context
				stZmqPubContext pubContext;
				if (!CEISMsgbusHandler::Instance().getPubCTX(eisTopic,
						pubContext)) {
					std::cout << __func__
							<< " cannot find pub context for topic : "
							<< eisTopic << std::endl;
					continue;
				}

				if(publishEISMsg(strMsg, context, pubContext))
					std::cout << __func__ << " Published EIS message : " << strMsg
							<< " on topic :" << eisTopic << "\n";
				else
					std::cout << __func__ << " Failed to publish EIS message : " << strMsg
							<< " on topic :" << eisTopic << "\n";
			}

#ifdef PERFTESTING
				CMQTTHandler::instance().incSubQTried();
#endif

		}
		std::cout << __func__ << " exiting from thread\n";
	}
	catch (const std::exception &e)
	{
		std::cout << __func__ << " Exception : " << e.what();
#ifdef PERFTESTING
		CMQTTHandler::instance().incSubQSkipped();
#endif
	}

	std::cout << __func__ << " Exited !! \n";
}

//start listening to EIS and publishing msgs to MQTT
void postMsgstoMQTT() {
	//start listening on EIS msg bus
	vector<string> vFullTopics =
			CfgManager::Instance().getEnvConfig().get_topics_from_env("sub");

	for (auto topic : vFullTopics) {

		if(topic.empty()) {
			std::cout << __func__ << " found empty MQTT subscriber topic" << std::endl;
			continue;
		}

		std::size_t pos = topic.find('/');
		if (std::string::npos != pos) {
			std::string subTopic(topic.substr(pos + 1));
			stZmqContext context;

			if (!CEISMsgbusHandler::Instance().getCTX(subTopic, context)) {
				std::cout << __func__
						<< " cannot find msgbus context for topic : "
						<< subTopic << std::endl;
				continue;		//go to next topic
			}

			//will give topic context
			stZmqSubContext subContext;
			if (!CEISMsgbusHandler::Instance().getSubCTX(subTopic,
					subContext)) {
				std::cout << __func__
						<< " cannot find sub context context for topic : "
						<< subTopic << std::endl;
				continue;		//go to next topic
			}

			std::cout << __func__ << "Info: Full topic - " << topic
					<< " AND listening on: " << subTopic << std::endl;

			g_vThreads.push_back(
					std::thread(listenOnEIS, subTopic, context, subContext));
		} else {
			std::cout << __func__ << ": Error: Incorrect topic name format: "
					<< topic << std::endl;
		}
	}

}

//create EIS msg bus context and topic context for publisher and subscriber both
bool initEISContext() {

	bool retVal = true;

	// Initializing all the pub/sub topic base context for ZMQ
	if (std::getenv("PubTopics") != NULL) {
		std::cout << __func__ << " List of topic configured for Pub are :: "
				<< getenv("PubTopics") << endl;
		bool bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext(
				"pub");
		if (!bRetVal) {
			std::cout << __func__
					<< " Context creation failed for sub topic \n";
			retVal = false;
		}
	}
	else {
		std::cout << __func__ << " could not find PubTopics in environment variables\n";
		retVal = false;
	}

	if (std::getenv("SubTopics") != NULL) {
		std::cout << __func__ << " List of topic configured for Sub are :: "
				<< getenv("SubTopics") << endl;
		bool bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext(
				"sub");
		if (!bRetVal) {
			std::cout << __func__
					<< " Context creation failed for sub topic \n";
			retVal = false;
		}
	}
	else {
		std::cout << __func__ << " could not find SubTopics in environment variables\n";
		retVal = false;
	}

	return retVal;
}

void signalHandler( int signum ) {

	std::cout << __func__ << " Cleaning up application\n";
   g_shouldStop = true;
   sem_post(&g_semaphoreRespProcess);
   CMQTTHandler::instance().cleanup();

   CEISMsgbusHandler::Instance().cleanup();

   std::cout << __func__ << " Application will restart to apply ETCD changes\n";
}

int main(int argc, char *argv[]) {

#ifdef UNIT_TEST
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
#endif

	// register signal SIGINT and signal handler
	signal(SIGINT, signalHandler);
	signal(SIGUSR1, signalHandler);

	try {
		/// register callback for dynamic ETCD change
		const char* env_appname = std::getenv("AppName");
		if(env_appname == NULL) {
			std::cout << __func__ << " AppName Environment Variable is not set";
			return -1;
		}
		std::string AppName(env_appname);

		string keyToMonitor = "/" + AppName + "/";
		CfgManager::Instance().registerCallbackOnChangeDir(const_cast<char *>(keyToMonitor.c_str()));
		CfgManager::Instance().registerCallbackOnChangeKey(const_cast<char *>(keyToMonitor.c_str()));

		//Create topic mapping
		CTopicMapper::getInstance();

		//Prepare MQTT for publishing & subscribing
		//subscribing to topics happens in callback of connect()
		CMQTTHandler::instance();

		//Prepare ZMQ contexts for publishing & subscribing data
		initEISContext();

		//Start listening on EIS & publishing to MQTT
		postMsgstoMQTT();

		//Start listening on MQTT & publishing to EIS
		g_vThreads.push_back(std::thread(postMsgsToEIS));

		for (auto &th : g_vThreads) {
			if (th.joinable()) {
				th.join();
			}
		}

	} catch (std::exception &e) {
		std::cout << __func__ << " Error::Exception is ::" << e.what()
				<< std::endl << "Exiting\n";
		return -1;
	} catch (...) {
		std::cout << __func__
				<< " Error::Unknown Exception Occurred. Exiting\n";
		return -1;
	}

	return 0;
}
