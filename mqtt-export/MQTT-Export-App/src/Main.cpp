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
//#include <assert.h>
#include <semaphore.h>
//#include <csignal>

#include "Common.hpp"
#include "QueueMgr.hpp"
#include "MQTTSubscribeHandler.hpp"
#include "EISMsgbusHandler.hpp"
#include "ConfigManager.hpp"
#include "Logger.hpp"
#include "MQTTPublishHandler.hpp"
#include "ConfigManager.hpp"

#ifdef UNIT_TEST
#include <gtest/gtest.h>
#endif

vector<std::thread> g_vThreads;

std::atomic<bool> g_shouldStop(false);

#define APP_VERSION "0.0.2.8"

// patterns to be used to find on-demand topic strings
// topic syntax -
// for non-RT topic for polling - <topic_name>__PolledData
// for RT topic read RT - <topic_name>__RdReq_RT
#define POLLING			 		"_PolledData"
#define POLLING_RT 				"_PolledData_RT"
#define READ_RESPONSE 			"_RdResp"
#define READ_RESPONSE_RT		"_RdResp_RT"
#define WRITE_RESPONSE 			"_WrResp"
#define WRITE_RESPONSE_RT		"_WrResp_RT"
/**
 * add sourcetopic key in payload to publish on EIS
 * @param json	:[in] json string in which to add topic name
 * @param topic	:[in] topic name to add
 * @return 	true : on success,
 * 			false : on error
 */
bool addSrTopic(string &json, string& topic)
{
	cJSON *root = NULL;
	try
	{
		root = cJSON_Parse(json.c_str());
		if (NULL == root)
		{
			CLogger::getInstance().log(ERROR,
					LOGDETAILS(
							"Message received from ZMQ could not be parsed in json format"));
			return false;
		}

		cJSON_AddStringToObject(root, "sourcetopic", topic.c_str());

		json.clear();
		char *psNewJson = cJSON_Print(root);
		if(NULL != psNewJson)
		{
			json.assign(psNewJson);
			free(psNewJson);
			psNewJson = NULL;
		}

		if(root != NULL)
		{
			cJSON_Delete(root);
		}

		CLogger::getInstance().log(DEBUG, LOGDETAILS("Added sourcetopic " + topic + " in payload for EIS"));
		return true;

	}
	catch (exception &ex)
	{
		CLogger::getInstance().log(DEBUG, LOGDETAILS("Failed to add sourcetopic " + topic + " in payload for EIS: " + ex.what()));

		if(root != NULL)
		{
			cJSON_Delete(root);
		}
		return false;
	}
}

/**
 * parse message to retrieve QOS and topic names
 * @param json	:[in] message from which to retrieve QOS and topic name
 * @param qos	:[out] QOS
 * @return parsed topic name from message
 */
std::string parse_msg(const char *json)
{
	std::string topic_name = "";
	try
	{
		cJSON *root = cJSON_Parse(json);
		if (NULL == root)
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("Message received from ZMQ could not be parsed in json format"));
			return topic_name;
		}

		if(! cJSON_HasObjectItem(root, "topic"))
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("Message received from ZMQ does not have key : topic"));
			if (NULL != root)
			{
				cJSON_Delete(root);
			}
			return topic_name;
		}

		char *ctopic_name = cJSON_GetObjectItem(root, "topic")->valuestring;
		if (NULL == ctopic_name)
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("Key 'topic' could not be found in message received from ZMQ"));
			if (NULL != root)
			{
				cJSON_Delete(root);
			}
			return topic_name;
		}

		topic_name.assign(ctopic_name);

		if (NULL != root)
		{
			cJSON_Delete(root);
		}

	}
	catch (std::exception &ex)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(ex.what()));
	}

	return topic_name;
}

/**
 * Process ZMQ message
 * @param msg	:	[in] actual message
 * returns true on success
 * 			false on failure
 */
bool processMsg(msg_envelope_t *msg, CMQTTPublishHandler &mqttPublisher)
{
	int num_parts = 0;
	msg_envelope_serialized_part_t *parts = NULL;
	bool bRetVal = false;

	if(msg == NULL)
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS( "Received NULL msg in msgbus_recv_wait"));
		return bRetVal;
	}

	struct timespec tsMsgRcvd;
	timespec_get(&tsMsgRcvd, TIME_UTC);

	num_parts = msgbus_msg_envelope_serialize(msg, &parts);
	if (num_parts <= 0)
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to serialize message"));
	}
	else if(NULL != parts)
	{
		if(NULL != parts[0].bytes)
		{
			std::string revdTopic(parse_msg(parts[0].bytes));

			if (revdTopic == "")
			{
				string strTemp = "topic key not present in message: ";
				strTemp.append(parts[0].bytes);
				CLogger::getInstance().log(ERROR, LOGDETAILS(strTemp));

			}
			else
			{
				string mqttMsg(parts[0].bytes);
				//publish data to MQTT
	#ifdef INSTRUMENTATION_LOG
				CLogger::getInstance().log(DEBUG, LOGDETAILS("ZMQ Message: Time: "
						+ std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
				+ ", Msg: " + mqttMsg));
	#endif
				mqttPublisher.publish(mqttMsg, revdTopic, tsMsgRcvd);

				bRetVal = true;
			}
		}

		if(parts != NULL)
		{
			msgbus_msg_envelope_serialize_destroy(parts, num_parts);
		}
	}
	else
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS("NULL pointer received"));
		bRetVal = false;
	}

	if(msg != NULL)
	{
		msgbus_msg_envelope_destroy(msg);
		msg = NULL;
	}
	parts = NULL;

	return bRetVal;
}

/**
 * get operation info from global config depending on the topic name
 * @param topic			:[in] topic for which to retrieve operation info
 * @param operation		:[out] operation info
 * @return none
 */
void getOperation(string topic, globalConfig::COperation& operation)
{
	if(std::string::npos != topic.find(POLLING_RT,
			topic.length() - std::string(POLLING_RT).length(),
			std::string(POLLING_RT).length()))
	{
		operation = globalConfig::CGlobalConfig::getInstance().getOpPollingOpConfig().getRTConfig();
	}
	else if(std::string::npos != topic.find(POLLING,
			topic.length() - std::string(POLLING).length(),
			std::string(POLLING).length()))
	{
		operation = globalConfig::CGlobalConfig::getInstance().getOpPollingOpConfig().getNonRTConfig();
	}
	else if(std::string::npos != topic.find(READ_RESPONSE_RT,
			topic.length() - std::string(READ_RESPONSE_RT).length(),
			std::string(READ_RESPONSE_RT).length()))
	{
		operation = globalConfig::CGlobalConfig::getInstance().getOpOnDemandReadConfig().getRTConfig();
	}
	else if(std::string::npos != topic.find(WRITE_RESPONSE_RT,
			topic.length() - std::string(WRITE_RESPONSE_RT).length(),
			std::string(WRITE_RESPONSE_RT).length()))
	{
		operation = globalConfig::CGlobalConfig::getInstance().getOpOnDemandWriteConfig().getRTConfig();
	}
	else if(std::string::npos != topic.find(READ_RESPONSE,
			topic.length() - std::string(READ_RESPONSE).length(),
			std::string(READ_RESPONSE).length()))
	{
		operation = globalConfig::CGlobalConfig::getInstance().getOpOnDemandReadConfig().getNonRTConfig();
	}
	else if(std::string::npos != topic.find(WRITE_RESPONSE,
			topic.length() - std::string(WRITE_RESPONSE).length(),
			std::string(WRITE_RESPONSE).length()))
	{
		operation = globalConfig::CGlobalConfig::getInstance().getOpOnDemandWriteConfig().getNonRTConfig();
	}
}

/**
 * Thread function to listen on EIS and send data to MQTT
 * @param topic 	:[in] topic to listen onto
 * @param context	:[in] msg bus context
 * @param subContext:[in] sub context
 */
void listenOnEIS(string topic, stZmqContext context, stZmqSubContext subContext, globalConfig::COperation operation)
{
	globalConfig::set_thread_sched_param(operation);
	globalConfig::display_thread_sched_attr(topic + " listenOnEIS");
	int qos = operation.getQos();

	void *msgbus_ctx = context.m_pContext;
	recv_ctx_t *sub_ctx = subContext.m_pContext;

	//consider topic name as distinguishing factor for publisher
	CMQTTPublishHandler mqttPublisher(CCommon::getInstance().getStrMqttExportURL().c_str(), topic.c_str(), qos);

	CLogger::getInstance().log(INFO, LOGDETAILS("ZMQ listening for topic : " + topic));

	while ((false == g_shouldStop.load()) && (msgbus_ctx != NULL) && (sub_ctx != NULL))
	{
		try
		{
			msg_envelope_t *msg = NULL;
			msgbus_ret_t ret;

			ret = msgbus_recv_wait(msgbus_ctx, sub_ctx, &msg);
			if (ret != MSG_SUCCESS)
			{
				// Interrupt is an acceptable error
				if (ret == MSG_ERR_EINTR)
				{
					CLogger::getInstance().log(ERROR, LOGDETAILS( "received MSG_ERR_EINT"));
					//break;
				}
				CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to receive message errno: " + std::to_string(ret)));
				continue;
			}
			
			/// process ZMQ message and publish to MQTT
			processMsg(msg, mqttPublisher);

		}
		catch (exception &ex)
		{
			string temp = ex.what();
			temp.append(" for topic : ");
			temp.append(topic);
			CLogger::getInstance().log(FATAL, LOGDETAILS(temp));
		}
	}//while ends

	CLogger::getInstance().log(DEBUG, LOGDETAILS("exited !!"));
}

/**
 * publish message to EIS
 * @param eisMsg 	:[in] message to publish on EIS
 * @param context	:[in] msg bus context
 * @param pubContext:[in] pub context
 * @return 	true : on success,
 * 			false : on error
 */
bool publishEISMsg(string eisMsg, stZmqContext &context,
		stZmqPubContext &pubContext)
{
	bool retVal = false;

	if(context.m_pContext == NULL || pubContext.m_pContext == NULL)
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS("Cannot publish on EIS as topic context or msgbus context is/are NULL"));
		return retVal;
	}

	// Creating message to be published
	msg_envelope_t *msg = msgbus_msg_envelope_new(CT_JSON);
	cJSON *root = NULL;
	try
	{
		if(msg == NULL)
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("could not create new msg envelope"));
			return retVal;
		}

		//parse from root element
		root = cJSON_Parse(eisMsg.c_str());
		if (NULL == root)
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("Could not parse value received from MQTT"));

			if(msg != NULL)
			{
				msgbus_msg_envelope_destroy(msg);
			}

			return retVal;
		}

		cJSON *device = root->child;
		while (device)
		{

			if(cJSON_IsString(device))
			{
				string temp(device->string);
				temp.append(" : ");
				temp.append(device->valuestring);
				CLogger::getInstance().log(DEBUG, LOGDETAILS(temp));


				msg_envelope_elem_body_t *value = msgbus_msg_envelope_new_string(
						device->valuestring);
				msgbus_msg_envelope_put(msg, device->string, value);
			}
			else
			{
				throw string("Invalid JSON");
			}
			// get and print key
			device = device->next;
		}

		if (root)
		{
			cJSON_Delete(root);
		}

		//add time stamp before publishing msg on EIS
		std::string strTsReceived;
		CCommon::getInstance().getCurrentTimestampsInString(strTsReceived);

		msg_envelope_elem_body_t* msgPublishOnEIS = msgbus_msg_envelope_new_string(strTsReceived.c_str());
		if(msgPublishOnEIS != NULL)
		{
			msgbus_msg_envelope_put(msg, "tsMsgPublishOnEIS", msgPublishOnEIS);
		}

		msgbus_publisher_publish(context.m_pContext,
				(publisher_ctx_t*) pubContext.m_pContext, msg);
		if(msg != NULL)
			msgbus_msg_envelope_destroy(msg);

		return true;
	}
	catch(string& strException)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(strException));
	}
	catch(exception &ex)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(ex.what()));
	}

	if(msg != NULL)
	{
		msgbus_msg_envelope_destroy(msg);
	}
	if (root)
	{
		cJSON_Delete(root);
	}

	return false;
}

/**
 * @param recvdMsg 	:[in] message to publish on EIS
 * @param isWrite	:[in] is write request or read
 * @param isRealtime:[in] is real-time request or non-real-time
 * @return 	true : on success,
 * 			false : on error
 */
bool processMsgToSendOnEIS(mqtt::const_message_ptr &recvdMsg, bool &isRead, bool &isRealtime)
{
	bool bRetVal = false;

	try
	{
		//received msg from queue
		string rcvdTopic = recvdMsg->get_topic();
		string strMsg = recvdMsg->get_payload();

		//this should be present in each incoming request
		if (rcvdTopic.empty()) //will not be the case ever
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("topic key not present in message: " + strMsg));
			return false;
		}

		CLogger::getInstance().log(DEBUG, LOGDETAILS("Request received from MQTT for topic "
				+ rcvdTopic));

		std::string eisTopic = "";
		//compare if request received for write or read
		if(! isRead)//write request
		{
			if(isRealtime)
			{
				eisTopic.assign(CCommon::getInstance().getStrRTWriteRequest());
			}
			else
			{
				eisTopic.assign(CCommon::getInstance().getStrWriteRequest());
			}
		}
		else//read request
		{
			if(isRealtime)
			{
				eisTopic.assign(CCommon::getInstance().getStrRTReadRequest());
			}
			else
			{
				eisTopic.assign(CCommon::getInstance().getStrReadRequest());
			}
		}

		if (eisTopic.empty())
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("EIS topic is not set to publish on EIS"
					+ rcvdTopic));
			return false;
		}
		else
		{
			//publish data to EIS
			CLogger::getInstance().log(DEBUG, LOGDETAILS("Received mapped EIS topic : " + eisTopic));

			//add source topic name in payload
			bool bRes = addSrTopic(strMsg, rcvdTopic);
			if(bRes == false)
			{
				CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to add sourcetopic " + rcvdTopic + " in payload for EIS"));
				return false;
			}

			//get EIS msg bus context
			stZmqContext context;
			if (!CEISMsgbusHandler::Instance().getCTX(eisTopic, context)) {
				CLogger::getInstance().log(ERROR, LOGDETAILS("cannot find msgbus context for topic : "
						+ eisTopic));
				return false;
			}

			//will give topic context
			stZmqPubContext pubContext;
			if (!CEISMsgbusHandler::Instance().getPubCTX(eisTopic,
					pubContext))
			{
				CLogger::getInstance().log(ERROR, LOGDETAILS("cannot find pub context for topic : "
						+ eisTopic));
				return false;
			}

			if(publishEISMsg(strMsg, context, pubContext))
			{
				CLogger::getInstance().log(DEBUG, LOGDETAILS("Published EIS message : "	+ strMsg + " on topic :" + eisTopic));
				bRetVal = true;
				//std::cout << "Published EIS message : "	<< strMsg << " on topic :" << eisTopic << std::endl;
			}
			else
			{
				CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to publish EIS message : "	+ strMsg + " on topic :" + eisTopic));
				bRetVal = false;

#ifdef PERFTESTING
			CMQTTHandler::instance().incSubQTried();
#endif
			}
		}
	}
	catch(exception &ex)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(ex.what()));
		bRetVal = false;
#ifdef PERFTESTING
		CMQTTHandler::instance().incSubQSkipped();
#endif
	}

	return bRetVal;
}

/**
 * set thread priority for threads which send msgs from MQTT-Export to EIS
 * @param isRealtime
 * @param isRead
 */
void set_thread_priority_for_eis(bool& isRealtime, bool& isRead)
{
	globalConfig::COperation operation;

	try
	{
		if(isRealtime)
		{
			if(isRead)
			{
				operation = globalConfig::CGlobalConfig::getInstance().getOpOnDemandReadConfig().getRTConfig();
			}
			else
			{
				operation = globalConfig::CGlobalConfig::getInstance().getOpOnDemandWriteConfig().getRTConfig();
			}
		}
		else
		{
			if(isRead)
			{
				operation = globalConfig::CGlobalConfig::getInstance().getOpOnDemandReadConfig().getNonRTConfig();
			}
			else
			{
				operation = globalConfig::CGlobalConfig::getInstance().getOpOnDemandWriteConfig().getNonRTConfig();
			}
		}
		globalConfig::set_thread_sched_param(operation);
		CLogger::getInstance().log(DEBUG, LOGDETAILS("Set thread priorities"));
		std::cout << "** Set thread priorities for isRead: " << isRead << ", isRealtime : " << isRealtime << endl;
	}
	catch(exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}
}

/**
 * Thread function to read requests from queue filled up by MQTT and send data to EIS
 * @param qMgr 	:[in] pointer to respective queue manager
 */
void postMsgsToEIS(QMgr::CQueueMgr& qMgr)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Starting thread to send messages on EIS"));

	bool isRealtime = qMgr.isRealTime();
	bool isRead = qMgr.isRead();

	//set priority to send msgs on EIS from MQTT-export (on-demand)
	set_thread_priority_for_eis(isRealtime, isRead);

	globalConfig::display_thread_sched_attr("postMsgsToEIS");

	try
	{
		mqtt::const_message_ptr recvdMsg;

		while (false == g_shouldStop.load())
		{
			if(true == qMgr.isMsgArrived(recvdMsg))
			{
				processMsgToSendOnEIS(recvdMsg, isRead, isRealtime);
			}
		}
	}
	catch (const std::exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}
}

/**
 * start listening to EIS and publishing msgs to MQTT
 */
void postMsgstoMQTT() {

	CLogger::getInstance().log(DEBUG, LOGDETAILS("Entered"));

	/// get sub topic list
	vector<string> vFullTopics = CEISMsgbusHandler::Instance().getSubTopicList();

	for (auto &topic : vFullTopics)
	{
		if(topic.empty())
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("found empty MQTT subscriber topic"));
			continue;
		}

		stZmqContext context;

		if (!CEISMsgbusHandler::Instance().getCTX(topic, context))
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("cannot find msgbus context for topic : " + topic));
			continue;		//go to next topic
		}

		//will give topic context
		stZmqSubContext subContext;
		if (!CEISMsgbusHandler::Instance().getSubCTX(topic,
				subContext))
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("cannot find sub context context for topic : "
					+ topic));
			continue;		//go to next topic
		}

		CLogger::getInstance().log(DEBUG, LOGDETAILS("Full topic - " + topic + " AND listening on: " + topic));

		//get operation depending on the topic
		globalConfig::COperation objOperation;
		getOperation(topic, objOperation);

		g_vThreads.push_back(
				std::thread(listenOnEIS, topic, context, subContext, objOperation));
	}
}

/**
 * Create EIS msg bus context and topic context for publisher and subscriber both
 * @return 	true : on success,
 * 			false : on error
 */
bool initEISContext()
{
	bool retVal = true;

	// Initializing all the pub/sub topic base context for ZMQ
	const char* env_pubTopics = std::getenv("PubTopics");
	if (env_pubTopics != NULL)
	{
		string temp = "List of topic configured for Pub are :: ";
		temp.append(env_pubTopics);
		CLogger::getInstance().log(DEBUG, LOGDETAILS(temp));
		bool bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext(
				"pub");
		if (!bRetVal)
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("Context creation failed for sub topic "));
			std::cout << __func__ << ":" << __LINE__ << " Error : Context creation failed for sub topic" <<  std::endl;
			retVal = false;
		}
	}
	else
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS("could not find PubTopics in environment variables"));
		std::cout << __func__ << ":" << __LINE__ << " Error : could not find PubTopics in environment variables" <<  std::endl;
		retVal = false;
	}

	const char* env_subTopics = std::getenv("SubTopics");
	if(env_subTopics != NULL)
	{
		string temp = "List of topic configured for Sub are :: ";
		temp.append(env_subTopics);
		CLogger::getInstance().log(DEBUG, LOGDETAILS(temp));
		bool bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext("sub");
		if (!bRetVal)
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("Context creation failed for sub topic"));
			std::cout << __func__ << ":" << __LINE__ << " Error : Context creation failed for sub topic" <<  std::endl;
			retVal = false;
		}
	}
	else
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS("could not find SubTopics in environment variables"));
		std::cout << __func__ << ":" << __LINE__ << " Error : could not find SubTopics in environment variables" <<  std::endl;
		retVal = false;
	}

	return retVal;
}

/**
 * Main function of application
 * @param argc :[in]
 * @param argv :[in]
 * @return 	0 : on success,
 * 			-1 : on error
 */
int main(int argc, char *argv[])
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Starting MQTT Export ..."));
	std::cout << __func__ << ":" << __LINE__ << " ------------- Starting MQTT Export Container -------------" << std::endl;

	try
	{
		/// register callback for dynamic ETCD change
		string AppName = CCommon::getInstance().getStrAppName();
		if(AppName.empty())
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("AppName Environment Variable is not set"));
			std::cout << __func__ << ":" << __LINE__ << " Error : AppName Environment Variable is not set" <<  std::endl;
			return -1;
		}

		CLogger::getInstance().log(INFO, LOGDETAILS("MQTT-Expprt container app version is set to :: "+  std::string(APP_VERSION)));
		cout << "MQTT-Expprt container app version is set to :: "+  std::string(APP_VERSION) << endl;

		// load global configuration for container real-time setting
		bool bRetVal = globalConfig::loadGlobalConfigurations();
		if(!bRetVal)
		{
			CLogger::getInstance().log(INFO, LOGDETAILS("Global configuration is set with some default parameters"));
			cout << "\nGlobal configuration is set with some default parameters\n\n";
		}
		else
		{
			CLogger::getInstance().log(INFO, LOGDETAILS("Global configuration is set successfully"));
			cout << "\nGlobal configuration for container real-time is set successfully\n\n";
		}

		globalConfig::CPriorityMgr::getInstance();

		//read environment values from settings
		CCommon::getInstance();

		//Prepare MQTT for publishing & subscribing
		//subscribing to topics happens in callback of connect()
		CMQTTHandler::instance();

		//Prepare ZMQ contexts for publishing & subscribing data
		initEISContext();

#ifdef UNIT_TEST
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
#endif

		//Start listening on EIS & publishing to MQTT
		postMsgstoMQTT();

		//threads to send on-demand requests on EIS
		g_vThreads.push_back(std::thread(postMsgsToEIS, std::ref(QMgr::getRTRead())));
		g_vThreads.push_back(std::thread(postMsgsToEIS, std::ref(QMgr::getRTWrite())));
		g_vThreads.push_back(std::thread(postMsgsToEIS, std::ref(QMgr::getRead())));
		g_vThreads.push_back(std::thread(postMsgsToEIS, std::ref(QMgr::getWrite())));


		for (auto &th : g_vThreads)
		{
			if (th.joinable())
			{
				th.join();
			}
		}

	}
	catch (std::exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
		return -1;
	}
	catch (...)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS("Unknown Exception Occurred. Exiting"));
		std::cout << __func__ << ":" << __LINE__ << "Exception : Unknown Exception Occurred. Exiting" << std::endl;
		return -1;
	}

	std::cout << __func__ << ":" << __LINE__ << " ------------- Exiting MQTT Export Container -------------" << std::endl;
	return 0;
}
