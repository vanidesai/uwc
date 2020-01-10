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
#include "Logger.hpp"

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
			CLogger::getInstance().log(ERROR, LOGDETAILS("Message received from ZMQ could not be parsed in json format"));

			std::cout << __func__ << "Message received from ZMQ could not be parsed in json format" << std::endl;
			return topic_name;
		}

		if(! cJSON_HasObjectItem(root, "topic")) {
			CLogger::getInstance().log(INFO, LOGDETAILS(" Message received from ZMQ does not have key : topic"));
			std::cout << __func__ << "Message received from ZMQ does not have key : topic" << std::endl;
			if (NULL != root)
				free(root);

			return topic_name;
		}

		char *ctopic_name = cJSON_GetObjectItem(root, "topic")->valuestring;
		if (NULL == ctopic_name) {
			CLogger::getInstance().log(ERROR, LOGDETAILS("Key 'topic' could not be found in message received from ZMQ"));
			std::cout << "Key 'topic' could not be found in message received from ZMQ" << std::endl;

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
		CLogger::getInstance().log(FATAL, LOGDETAILS(ex.what()));

		std::cout << __func__ << "Exception : " << ex.what() << std::endl;
	}

	return topic_name;
}

//listens on EIS and sends data to MQTT
void listenOnEIS(string topic, stZmqContext context,
		stZmqSubContext subContext) {

	void *msgbus_ctx = context.m_pContext;
	recv_ctx_t *sub_ctx = subContext.m_pContext;

	CLogger::getInstance().log(INFO, LOGDETAILS("ZMQ listening for topic : " + topic));
	std::cout << __func__ << "ZMQ listening for topic : " << topic << std::endl;
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
					CLogger::getInstance().log(ERROR, LOGDETAILS( "received MSG_ERR_EINT"));
					std::cout << __func__ << "received MSG_ERR_EINT" << std::endl;
					break;
				}
				CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to receive message errno: " + std::to_string(ret)));
				std::cout << __func__ << "Failed to receive message errno: " << std::to_string(ret) << std::endl;
				continue;
			}

			num_parts = msgbus_msg_envelope_serialize(msg, &parts);
			if (num_parts <= 0) {
				CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to serialize message"));
				std::cout << __func__ << "Failed to serialize message" << endl;
				//continue;
			}
			else if(NULL != parts) {
				std::string revdTopic(parse_msg(parts[0].bytes));

				if (revdTopic == "") {
					string strTemp = "topic key not present in message: ";
					strTemp.append(parts[0].bytes);
					CLogger::getInstance().log(ERROR, LOGDETAILS(strTemp));

					std::cout << __func__ << strTemp << std::endl;
				} else {

					std::string mqttTopic = CTopicMapper::getInstance().GetMQTTopic(
							revdTopic);
					if (mqttTopic == "") {
						CLogger::getInstance().log(ERROR, LOGDETAILS("Could not find mapped MQTT topic for ZMQ topic : " + revdTopic));
						std::cout << __func__ << "Could not find mapped MQTT topic for ZMQ topic : " << revdTopic << std::endl;
					} else {
						string mqttMsg(parts[0].bytes);
						//publish data to MQTT
						std::cout << "ZMQ Message: Time: "
							<< std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()
							<< ", Msg: " << mqttMsg << std::endl;

						CLogger::getInstance().log(ERROR, LOGDETAILS("ZMQ Message: Time: "
							+ std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
							+ ", Msg: " + mqttMsg));

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
			string temp = ex.what();
			temp.append(" for topic : ");
			temp.append(topic);
			CLogger::getInstance().log(FATAL, LOGDETAILS(temp));
			std::cout << __func__ << "Exception : " << temp << endl;
		}
	}//while ends

	CLogger::getInstance().log(INFO, LOGDETAILS("exited !!"));
	std::cout << __func__ << "Exited" << endl;
}

//publish message to EIS
bool publishEISMsg(string eisMsg, stZmqContext &context,
		stZmqPubContext &pubContext) {

	bool retVal = false;
	// Creating message to be published
	msg_envelope_t *msg = msgbus_msg_envelope_new(CT_JSON);

	try {

		if(msg == NULL) {
			CLogger::getInstance().log(ERROR, LOGDETAILS("could not create new msg envelope"));
			return retVal;
		}

		//parse from root element
		cJSON *root = cJSON_Parse(eisMsg.c_str());
		if (NULL == root) {
			CLogger::getInstance().log(ERROR, LOGDETAILS("Could not parse value received from MQTT"));
			std::cout << __func__ << "Could not parse value received from MQTT" << endl;

			if(msg != NULL)
				msgbus_msg_envelope_destroy(msg);

			return retVal;
		}

		cJSON *device = root->child;
		while (device) {
			string temp(device->string);
			temp.append(" : ");
			temp.append(device->valuestring);
			CLogger::getInstance().log(DEBUG, LOGDETAILS(temp));

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
		CLogger::getInstance().log(FATAL, LOGDETAILS(ex.what()));
		cout << __func__ << "Exception : " << ex.what() << endl;

		if(msg != NULL)
			msgbus_msg_envelope_destroy(msg);
		retVal = false;
	}
	return retVal;
}

//reads data from queue filled up by MQTT and sends data to EIS
void postMsgsToEIS() {
	CLogger::getInstance().log(INFO, LOGDETAILS("Starting thread to send messages on EIS"));

	try {
		mqtt::const_message_ptr recvdMsg;
		while (false == g_shouldStop.load()) {

			sem_wait(&g_semaphoreRespProcess);

			if (false == CMQTTHandler::instance().getSubMsgFromQ(recvdMsg)) {
				CLogger::getInstance().log(DEBUG, LOGDETAILS("No message to send to EIS in queue"));
				continue;
			}

			//received msg from queue
			string rcvdTopic = recvdMsg->get_topic();
			string strMsg = recvdMsg->get_payload();

			//this should be present in each incoming request
			if (rcvdTopic == "") //will not be the case ever
					{
				CLogger::getInstance().log(DEBUG, LOGDETAILS("topic key not present in message: " + strMsg));
				continue;
			}

			std::string eisTopic = CTopicMapper::getInstance().GetZMQTopic(rcvdTopic);
			if (eisTopic == "")
			{
				CLogger::getInstance().log(ERROR, LOGDETAILS("Could not find mapped EIS topic for MQTT topic : "
						+ rcvdTopic));
				continue;
			}
			else
			{
				//publish data to EIS
				CLogger::getInstance().log(DEBUG, LOGDETAILS("Received mapped EIS topic : " + eisTopic));

				//get EIS msg bus context
				stZmqContext context;
				if (!CEISMsgbusHandler::Instance().getCTX(eisTopic, context)) {
					CLogger::getInstance().log(DEBUG, LOGDETAILS("cannot find msgbus context for topic : "
							+ eisTopic));
					continue;
				}

				//will give topic context
				stZmqPubContext pubContext;
				if (!CEISMsgbusHandler::Instance().getPubCTX(eisTopic,
						pubContext)) {
					CLogger::getInstance().log(DEBUG, LOGDETAILS("cannot find pub context for topic : "
							+ eisTopic));
					continue;
				}

				if(publishEISMsg(strMsg, context, pubContext)){
					CLogger::getInstance().log(DEBUG, LOGDETAILS("Published EIS message : "	+ strMsg + " on topic :" + eisTopic));
				}
				else
					CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to publish EIS message : "	+ strMsg + " on topic :" + eisTopic));
			}

#ifdef PERFTESTING
				CMQTTHandler::instance().incSubQTried();
#endif

		}
		CLogger::getInstance().log(DEBUG, LOGDETAILS("exiting from thread"));
	}
	catch (const std::exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
#ifdef PERFTESTING
		CMQTTHandler::instance().incSubQSkipped();
#endif
	}

	CLogger::getInstance().log(DEBUG, LOGDETAILS("Exited"));
}

//start listening to EIS and publishing msgs to MQTT
void postMsgstoMQTT() {

	CLogger::getInstance().log(INFO, LOGDETAILS("Entered"));

	//start listening on EIS msg bus
	vector<string> vFullTopics =
			CfgManager::Instance().getEnvConfig().get_topics_from_env("sub");

	for (auto topic : vFullTopics) {

		if(topic.empty()) {
			CLogger::getInstance().log(ERROR, LOGDETAILS("found empty MQTT subscriber topic"));
			continue;
		}

		std::size_t pos = topic.find('/');
		if (std::string::npos != pos) {
			std::string subTopic(topic.substr(pos + 1));
			stZmqContext context;

			if (!CEISMsgbusHandler::Instance().getCTX(subTopic, context)) {
				CLogger::getInstance().log(ERROR, LOGDETAILS("cannot find msgbus context for topic : " + subTopic));
				continue;		//go to next topic
			}

			//will give topic context
			stZmqSubContext subContext;
			if (!CEISMsgbusHandler::Instance().getSubCTX(subTopic,
					subContext)) {
				CLogger::getInstance().log(ERROR, LOGDETAILS("cannot find sub context context for topic : "
						+ subTopic));
				continue;		//go to next topic
			}

			CLogger::getInstance().log(DEBUG, LOGDETAILS("Full topic - " + topic + " AND listening on: " + subTopic));

			g_vThreads.push_back(
					std::thread(listenOnEIS, subTopic, context, subContext));
		} else {
			CLogger::getInstance().log(ERROR, LOGDETAILS("Incorrect topic name format: " + topic));
		}
	}

}

//create EIS msg bus context and topic context for publisher and subscriber both
bool initEISContext() {

	bool retVal = true;

	// Initializing all the pub/sub topic base context for ZMQ
	if (std::getenv("PubTopics") != NULL) {
		string temp = "List of topic configured for Pub are :: ";
		temp.append(getenv("PubTopics"));
		CLogger::getInstance().log(DEBUG, LOGDETAILS(temp));
		bool bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext(
				"pub");
		if (!bRetVal) {
			CLogger::getInstance().log(ERROR, LOGDETAILS("Context creation failed for sub topic "));
			retVal = false;
		}
	}
	else {
		CLogger::getInstance().log(INFO, LOGDETAILS("could not find PubTopics in environment variables"));
		retVal = false;
	}

	if (std::getenv("SubTopics") != NULL) {
		string temp = "List of topic configured for Sub are :: ";
		temp.append(getenv("SubTopics"));
		CLogger::getInstance().log(DEBUG, LOGDETAILS(temp));
		bool bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext("sub");
		if (!bRetVal) {
			CLogger::getInstance().log(ERROR, LOGDETAILS("Context creation failed for sub topic"));
			retVal = false;
		}
	}
	else {
		CLogger::getInstance().log(INFO, LOGDETAILS("could not find SubTopics in environment variables"));
		retVal = false;
	}

	return retVal;
}

void signalHandler( int signum ) {

   CLogger::getInstance().log(INFO, LOGDETAILS("Cleaning up application"));
   g_shouldStop = true;
   sem_post(&g_semaphoreRespProcess);
   CMQTTHandler::instance().cleanup();

   CEISMsgbusHandler::Instance().cleanup();

   CLogger::getInstance().log(WARN, LOGDETAILS("Application will restart to apply ETCD changes"));
}

int main(int argc, char *argv[]) {

#ifdef UNIT_TEST
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
#endif

	CLogger::getInstance().log(DEBUG, LOGDETAILS("Starting MQTT Export ..."));

	// register signal SIGINT and signal handler
	signal(SIGINT, signalHandler);
	signal(SIGUSR1, signalHandler);

	try {
		/// register callback for dynamic ETCD change
		const char* env_appname = std::getenv("AppName");
		if(env_appname == NULL) {
			CLogger::getInstance().log(ERROR, LOGDETAILS("AppName Environment Variable is not set"));
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

		std::cout << "** Total threads started from main function : " << g_vThreads.size() << std::endl;

		for (auto &th : g_vThreads) {
			if (th.joinable()) {
				th.join();
			}
		}

	} catch (std::exception &e) {
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		return -1;
	} catch (...) {
		CLogger::getInstance().log(FATAL, LOGDETAILS("Unknown Exception Occurred. Exiting"));
		return -1;
	}

	return 0;
}
