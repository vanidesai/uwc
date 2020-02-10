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

#define APP_VERSION "0.0.0.7"

//add sourcetopic key in payload to publish on EIS
bool addSrTopic(string &json, string& topic) {

	cJSON *root = NULL;
	try {

		root = cJSON_Parse(json.c_str());
		if (NULL == root) {
			CLogger::getInstance().log(ERROR,
					LOGDETAILS(
							"Message received from ZMQ could not be parsed in json format"));
			std::cout << __func__ << ":" << __LINE__ << " Error : Message received from ZMQ could not be parsed in json format" <<  std::endl;
			return false;
		}

		cJSON_AddStringToObject(root, "sourcetopic", topic.c_str());

		json.clear();
		json = cJSON_Print(root);

		if(root != NULL)
			cJSON_Delete(root);

		CLogger::getInstance().log(DEBUG, LOGDETAILS("Added sourcetopic " + topic + " in payload for EIS"));
		return true;

	} catch (exception &ex) {

		CLogger::getInstance().log(DEBUG, LOGDETAILS("Failed to add sourcetopic " + topic + " in payload for EIS"));
		std::cout << __func__ << ":" << __LINE__ << "Failed to add sourcetopic "  + topic + " in payload for EIS" << std::endl;
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << ex.what() << std::endl;

		if(root != NULL)
			cJSON_Delete(root);

		return false;
	}
}

std::string parse_msg(const char *json, int& qos) {

	std::string topic_name = "";
	try {
		cJSON *root = cJSON_Parse(json);
		if (NULL == root) {
			CLogger::getInstance().log(ERROR, LOGDETAILS("Message received from ZMQ could not be parsed in json format"));
			std::cout << __func__ << ":" << __LINE__ << " Error : Message received from ZMQ could not be parsed in json format" <<  std::endl;
			return topic_name;
		}

		if(! cJSON_HasObjectItem(root, "topic")) {
			CLogger::getInstance().log(ERROR, LOGDETAILS("Message received from ZMQ does not have key : topic"));
			std::cout << __func__ << ":" << __LINE__ << " Error : Message received from ZMQ does not have key : topic" <<  std::endl;
			if (NULL != root)
				cJSON_Delete(root);

			return topic_name;
		}

		char *ctopic_name = cJSON_GetObjectItem(root, "topic")->valuestring;
		if (NULL == ctopic_name) {
			CLogger::getInstance().log(ERROR, LOGDETAILS("Key 'topic' could not be found in message received from ZMQ"));
			std::cout << __func__ << ":" << __LINE__ << " Error : Key 'topic' could not be found in message received from ZMQ" <<  std::endl;
			if (NULL != root)
				cJSON_Delete(root);

			return topic_name;
		}

		topic_name.assign(ctopic_name);

		if(! cJSON_HasObjectItem(root, "qos")) {
			CLogger::getInstance().log(INFO, LOGDETAILS("Message received from ZMQ does not have key : qos, using default QOS = 0"));

			qos = 0;
		}
		else
		{
			char *c_qos = cJSON_GetObjectItem(root, "qos")->valuestring;
			if (NULL == c_qos) {
				CLogger::getInstance().log(ERROR, LOGDETAILS("Key 'qos' could not be found in message received from ZMQ, using default QOS = 0"));
			}
			else {
				if((strcmp(c_qos, "0") == 0) || (strcmp(c_qos, "1") == 0) || (strcmp(c_qos, "2") == 0)) {
					std::string::size_type sz;   // alias of size_t
					qos = std::stoi(c_qos, &sz);
				}
				else
				{
					CLogger::getInstance().log(ERROR, LOGDETAILS("Received invalid value for QOS, using default QOS = 0"));
					qos = 0;
				}

				CLogger::getInstance().log(DEBUG, LOGDETAILS("Using 'QOS' for message received from ZMQ : " + std::string(c_qos)));
			}
		}

/*		if (NULL != ctopic_name)
			free(ctopic_name);*/
		if (NULL != root)
			cJSON_Delete(root);

	} catch (std::exception &ex) {
		CLogger::getInstance().log(FATAL, LOGDETAILS(ex.what()));
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << ex.what() << std::endl;
	}

	return topic_name;
}

//listens on EIS and sends data to MQTT
void listenOnEIS(string topic, stZmqContext context,
		stZmqSubContext subContext) {

	void *msgbus_ctx = context.m_pContext;
	recv_ctx_t *sub_ctx = subContext.m_pContext;

	CLogger::getInstance().log(INFO, LOGDETAILS("ZMQ listening for topic : " + topic));
	while ((false == g_shouldStop.load()) && (msgbus_ctx != NULL) && (sub_ctx != NULL)) {

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
					std::cout << __func__ << ":" << __LINE__ << " Error : received MSG_ERR_EINT" <<  std::endl;
					break;
				}
				CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to receive message errno: " + std::to_string(ret)));
				std::cout << __func__ << ":" << __LINE__ << " Error : Failed to receive message errno: " + std::to_string(ret) <<  std::endl;
				continue;
			}

			if(msg == NULL) {
				std::cout << " Received NULL msg in msgbus_recv_wait" << std::endl;
				continue;
			}
			
			struct timespec tsMsgRcvd;
			timespec_get(&tsMsgRcvd, TIME_UTC);

			num_parts = msgbus_msg_envelope_serialize(msg, &parts);
			if (num_parts <= 0) {
				CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to serialize message"));
				std::cout << __func__ << ":" << __LINE__ << " Error : Failed to serialize message" <<  std::endl;
				//continue;
			}
			else if(NULL != parts) {
				int iQOS = 0;
				std::string revdTopic(parse_msg(parts[0].bytes, iQOS));

				if (revdTopic == "") {
					string strTemp = "topic key not present in message: ";
					strTemp.append(parts[0].bytes);
					CLogger::getInstance().log(ERROR, LOGDETAILS(strTemp));
					//std::cout << __func__ << ":" << __LINE__ << " Error : " << strTemp <<  std::endl;

				} else {
						string mqttMsg(parts[0].bytes);
						//publish data to MQTT
/*
						std::cout << "ZMQ Message: Time: "
							<< std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()
							<< ", Msg: " << mqttMsg << std::endl;
*/

						CLogger::getInstance().log(INFO, LOGDETAILS("ZMQ Message: Time: "
							+ std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
							+ ", Msg: " + mqttMsg));

						CMQTTHandler::instance().publish(mqttMsg,
								revdTopic.c_str(), iQOS, tsMsgRcvd);
				}

				if(parts != NULL)
					msgbus_msg_envelope_serialize_destroy(parts, num_parts);
			}

			if(msg != NULL) {
				msgbus_msg_envelope_destroy(msg);
				msg = NULL;
			}
			parts = NULL;
		} catch (exception &ex) {
			string temp = ex.what();
			temp.append(" for topic : ");
			temp.append(topic);
			CLogger::getInstance().log(FATAL, LOGDETAILS(temp));
			std::cout << __func__ << ":" << __LINE__ << " Exception : " << temp << std::endl;
		}
	}//while ends

	CLogger::getInstance().log(DEBUG, LOGDETAILS("exited !!"));
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
			std::cout << __func__ << ":" << __LINE__ << " Error : could not create new msg envelope" <<  std::endl;
			return retVal;
		}

		//parse from root element
		cJSON *root = cJSON_Parse(eisMsg.c_str());
		if (NULL == root) {
			CLogger::getInstance().log(ERROR, LOGDETAILS("Could not parse value received from MQTT"));
			std::cout << __func__ << ":" << __LINE__ << " Error : Could not parse value received from MQTT" <<  std::endl;

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

		if (root)
			cJSON_Delete(root);

		msgbus_publisher_publish(context.m_pContext,
				(publisher_ctx_t*) pubContext.m_pContext, msg);
		if(msg != NULL)
			msgbus_msg_envelope_destroy(msg);

		retVal = true;

	} catch (exception &ex) {
		CLogger::getInstance().log(FATAL, LOGDETAILS(ex.what()));

		std::cout << __func__ << ":" << __LINE__ << " Exception : " << ex.what() << std::endl;
		if(msg != NULL)
			msgbus_msg_envelope_destroy(msg);
		retVal = false;
	}
	return retVal;
}

//reads data from queue filled up by MQTT and sends data to EIS
void postMsgsToEIS() {
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Starting thread to send messages on EIS"));

	try {
		mqtt::const_message_ptr recvdMsg;
		while (false == g_shouldStop.load()) {

			sem_wait(&g_semaphoreRespProcess);

			if (false == CMQTTHandler::instance().getSubMsgFromQ(recvdMsg)) {
				CLogger::getInstance().log(INFO, LOGDETAILS("No message to send to EIS in queue"));
				continue;
			}

			//received msg from queue
			string rcvdTopic = recvdMsg->get_topic();
			string strMsg = recvdMsg->get_payload();

			//this should be present in each incoming request
			if (rcvdTopic == "") //will not be the case ever
					{
				CLogger::getInstance().log(ERROR, LOGDETAILS("topic key not present in message: " + strMsg));
				std::cout << __func__ << ":" << __LINE__ << " Error : topic key not present in message: " + strMsg <<  std::endl;
				continue;
			}

			std::string eisTopic = "";

			CLogger::getInstance().log(DEBUG, LOGDETAILS("Request received from MQTT for topic "
					+ rcvdTopic));

			//compare if request received for write or read
			if(rcvdTopic.find("write") != std::string::npos) {
				//write's getter'
				eisTopic.assign(CTopicMapper::getInstance().getStrWriteRequest());

			}else if(rcvdTopic.find("read") != std::string::npos) {
				//read getter
				eisTopic.assign(CTopicMapper::getInstance().getStrReadRequest());
			}

			if (eisTopic == "")
			{
				CLogger::getInstance().log(ERROR, LOGDETAILS("EIS topic is not set to publish on EIS"
						+ rcvdTopic));
				std::cout << __func__ << ":" << __LINE__ << " Error : EIS topic is not set to publish on EIS"
						+ rcvdTopic <<  std::endl;
				continue;
			}
			else
			{
				//publish data to EIS
				CLogger::getInstance().log(DEBUG, LOGDETAILS("Received mapped EIS topic : " + eisTopic));

				//add source topic name in payload
				bool bRes = addSrTopic(strMsg, rcvdTopic);
				if(bRes == false) {
					CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to add sourcetopic " + rcvdTopic + " in payload for EIS"));
					std::cout << __func__ << ":" << __LINE__ << " Error : Failed to add sourcetopic " + rcvdTopic + " in payload for EIS" <<  std::endl;
					continue;
				}

				//get EIS msg bus context
				stZmqContext context;
				if (!CEISMsgbusHandler::Instance().getCTX(eisTopic, context)) {
					CLogger::getInstance().log(ERROR, LOGDETAILS("cannot find msgbus context for topic : "
							+ eisTopic));
					std::cout << __func__ << ":" << __LINE__ << " Error : cannot find msgbus context for topic : "
							+ eisTopic <<  std::endl;
					continue;
				}

				//will give topic context
				stZmqPubContext pubContext;
				if (!CEISMsgbusHandler::Instance().getPubCTX(eisTopic,
						pubContext)) {
					CLogger::getInstance().log(ERROR, LOGDETAILS("cannot find pub context for topic : "
							+ eisTopic));
					std::cout << __func__ << ":" << __LINE__ << " Error : cannot find pub context for topic : "
							+ eisTopic <<  std::endl;
					continue;
				}

				if(publishEISMsg(strMsg, context, pubContext)){
					CLogger::getInstance().log(DEBUG, LOGDETAILS("Published EIS message : "	+ strMsg + " on topic :" + eisTopic));
				}
				else {
					CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to publish EIS message : "	+ strMsg + " on topic :" + eisTopic));
					std::cout << __func__ << ":" << __LINE__ << " Error : Failed to publish EIS message : "	+ strMsg + " on topic :" + eisTopic <<  std::endl;
				}
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
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
#ifdef PERFTESTING
		CMQTTHandler::instance().incSubQSkipped();
#endif
	}

	CLogger::getInstance().log(DEBUG, LOGDETAILS("Exited"));
}

//start listening to EIS and publishing msgs to MQTT
void postMsgstoMQTT() {

	CLogger::getInstance().log(DEBUG, LOGDETAILS("Entered"));

	//start listening on EIS msg bus

	/// get sub topic list
	vector<string> vFullTopics = CEISMsgbusHandler::Instance().getSubTopicList();

	for (auto &topic : vFullTopics) {

		if(topic.empty()) {
			CLogger::getInstance().log(ERROR, LOGDETAILS("found empty MQTT subscriber topic"));
			std::cout << __func__ << ":" << __LINE__ << " Error : found empty MQTT subscriber topic" <<  std::endl;
			continue;
		}

		stZmqContext context;

		if (!CEISMsgbusHandler::Instance().getCTX(topic, context)) {
			CLogger::getInstance().log(ERROR, LOGDETAILS("cannot find msgbus context for topic : " + topic));
			std::cout << __func__ << ":" << __LINE__ << " Error : cannot find msgbus context for topic : " + topic <<  std::endl;
			continue;		//go to next topic
		}

		//will give topic context
		stZmqSubContext subContext;
		if (!CEISMsgbusHandler::Instance().getSubCTX(topic,
				subContext)) {
			CLogger::getInstance().log(ERROR, LOGDETAILS("cannot find sub context context for topic : "
					+ topic));
			std::cout << __func__ << ":" << __LINE__ << " Error : cannot find sub context context for topic : "
					+ topic <<  std::endl;
			continue;		//go to next topic
		}

		CLogger::getInstance().log(DEBUG, LOGDETAILS("Full topic - " + topic + " AND listening on: " + topic));

		g_vThreads.push_back(
				std::thread(listenOnEIS, topic, context, subContext));
	}
}

//create EIS msg bus context and topic context for publisher and subscriber both
bool initEISContext() {

	bool retVal = true;

	// Initializing all the pub/sub topic base context for ZMQ
	const char* env_pubTopics = std::getenv("PubTopics");
	if (env_pubTopics != NULL) {
		string temp = "List of topic configured for Pub are :: ";
		temp.append(env_pubTopics);
		CLogger::getInstance().log(DEBUG, LOGDETAILS(temp));
		bool bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext(
				"pub");
		if (!bRetVal) {
			CLogger::getInstance().log(ERROR, LOGDETAILS("Context creation failed for sub topic "));
			std::cout << __func__ << ":" << __LINE__ << " Error : Context creation failed for sub topic" <<  std::endl;
			retVal = false;
		}
	}
	else {
		CLogger::getInstance().log(ERROR, LOGDETAILS("could not find PubTopics in environment variables"));
		std::cout << __func__ << ":" << __LINE__ << " Error : could not find PubTopics in environment variables" <<  std::endl;
		retVal = false;
	}

	const char* env_subTopics = std::getenv("SubTopics");
	if(env_subTopics != NULL) {
		string temp = "List of topic configured for Sub are :: ";
		temp.append(env_subTopics);
		CLogger::getInstance().log(DEBUG, LOGDETAILS(temp));
		bool bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext("sub");
		if (!bRetVal) {
			CLogger::getInstance().log(ERROR, LOGDETAILS("Context creation failed for sub topic"));
			std::cout << __func__ << ":" << __LINE__ << " Error : Context creation failed for sub topic" <<  std::endl;
			retVal = false;
		}
	}
	else {
		CLogger::getInstance().log(ERROR, LOGDETAILS("could not find SubTopics in environment variables"));
		std::cout << __func__ << ":" << __LINE__ << " Error : could not find SubTopics in environment variables" <<  std::endl;
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

   CLogger::getInstance().log(WARN, LOGDETAILS("MQTT-Export is restarting..."));
}

int main(int argc, char *argv[]) {

#ifdef UNIT_TEST
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
#endif

	CLogger::getInstance().log(DEBUG, LOGDETAILS("Starting MQTT Export ..."));
	std::cout << __func__ << ":" << __LINE__ << " ------------- Starting MQTT Export Container -------------" << std::endl;

	// register signal SIGINT and signal handler
	signal(SIGINT, signalHandler);
	signal(SIGUSR1, signalHandler);

	try {
		/// register callback for dynamic ETCD change
		string AppName = CTopicMapper::getInstance().getStrAppName();
		if(AppName.empty()) {
			CLogger::getInstance().log(ERROR, LOGDETAILS("AppName Environment Variable is not set"));
			std::cout << __func__ << ":" << __LINE__ << " Error : AppName Environment Variable is not set" <<  std::endl;
			return -1;
		}

		CLogger::getInstance().log(INFO, LOGDETAILS("MQTT-Expprt container app version is set to :: "+  std::string(APP_VERSION)));
		cout << "MQTT-Expprt container app version is set to :: "+  std::string(APP_VERSION) << endl;

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
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
		return -1;
	} catch (...) {
		CLogger::getInstance().log(FATAL, LOGDETAILS("Unknown Exception Occurred. Exiting"));
		std::cout << __func__ << ":" << __LINE__ << "Exception : Unknown Exception Occurred. Exiting" << std::endl;
		return -1;
	}

	std::cout << __func__ << ":" << __LINE__ << " ------------- Exiting MQTT Export Container -------------" << std::endl;
	return 0;
}
