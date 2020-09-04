/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

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

#define APP_VERSION "0.0.5.5"

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
 * Add sourcetopic key in message payload to publish on EIS
 * @param json	:[in] json string in which to add topic name
 * @param topic	:[in] topic name to add
 * @return true/false based on success/failure
 */
bool addSrTopic(string &json, string& topic)
{
	cJSON *root = NULL;
	try
	{
		root = cJSON_Parse(json.c_str());
		if (NULL == root)
		{
			DO_LOG_ERROR("Message received from ZMQ could not be parsed in json format");
			return false;
		}

		if(NULL == cJSON_AddStringToObject(root, "sourcetopic", topic.c_str()))
		{
			DO_LOG_ERROR("Could not add sourcetopic in message");
			if(root != NULL)
			{
				cJSON_Delete(root);
			}
			return false;
		}

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

		DO_LOG_DEBUG("Added sourcetopic " + topic + " in payload for EIS");
		return true;

	}
	catch (exception &ex)
	{
		DO_LOG_DEBUG("Failed to add sourcetopic " + topic + " in payload for EIS: " + ex.what());

		if(root != NULL)
		{
			cJSON_Delete(root);
		}
		return false;
	}
}

/**
 * Process message received from EIS and send for publishing on MQTT
 * @param msg	:[in] actual message
 * @param mqttPublisher :[in] mqtt publisher instance from which to publish this message
 * returns true/false based on success/failure
 */
bool processMsg(msg_envelope_t *msg, CMQTTPublishHandler &mqttPublisher)
{
	int num_parts = 0;
	msg_envelope_serialized_part_t *parts = NULL;
	bool bRetVal = false;

	if(msg == NULL)
	{
		DO_LOG_ERROR("Received NULL msg in msgbus_recv_wait");
		return bRetVal;
	}

	struct timespec tsMsgRcvd;
	timespec_get(&tsMsgRcvd, TIME_UTC);

	std::string revdTopic;
	msg_envelope_elem_body_t* data;
	msgbus_ret_t msgRet = msgbus_msg_envelope_get(msg, "data_topic", &data);
	if(msgRet != MSG_SUCCESS)
	{
		DO_LOG_ERROR("topic key not present in zmq message");
		bRetVal = false;
	}
	else
	{
		revdTopic = data->body.string;

		std::string strTsRcvd = std::to_string(CCommon::getInstance().get_micros(tsMsgRcvd));
		msg_envelope_elem_body_t* tsMsgRcvdPut = msgbus_msg_envelope_new_string(strTsRcvd.c_str());
		msgbus_msg_envelope_put(msg, "tsMsgRcvdForProcessing", tsMsgRcvdPut);

		num_parts = msgbus_msg_envelope_serialize(msg, &parts);
		if (num_parts <= 0)
		{
			DO_LOG_ERROR("Failed to serialize message");
		}
		else if(NULL != parts)
		{
			if(NULL != parts[0].bytes)
			{
				string mqttMsg(parts[0].bytes);
				mqttPublisher.publish(mqttMsg, revdTopic);

				bRetVal = true;
			}
		}
		else
		{
			DO_LOG_ERROR("NULL pointer received");
			bRetVal = false;
		}
	}

	if(parts != NULL)
	{
		msgbus_msg_envelope_serialize_destroy(parts, num_parts);
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
 * Get operation info from global config depending on the topic name
 * @param topic	:[in] topic for which to retrieve operation info
 * @param operation	:[out] operation info
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
 * @param topic :[in] topic to listen onto
 * @param context :[in] msg bus context
 * @param subContext :[in] sub context
 * @param operation :[in] operation type this thread needs to perform
 * @return None
 */
void listenOnEIS(string topic, stZmqContext context, stZmqSubContext subContext, globalConfig::COperation operation)
{
	globalConfig::set_thread_sched_param(operation);
	globalConfig::display_thread_sched_attr(topic + " listenOnEIS");
	int qos = operation.getQos();

	if(context.m_pContext == NULL || subContext.m_pContext == NULL)
	{
		std::cout << "Cannot start listening on EIS for topic : " << topic << endl;
		DO_LOG_ERROR("Cannot start listening on EIS for topic : " + topic);
		return;
	}

	void *msgbus_ctx = context.m_pContext;
	recv_ctx_t *sub_ctx = subContext.m_pContext;

	//consider topic name as distinguishing factor for publisher
	CMQTTPublishHandler mqttPublisher(CCommon::getInstance().getStrMqttExportURL().c_str(), topic, qos);

	DO_LOG_INFO("ZMQ listening for topic : " + topic);

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
					DO_LOG_ERROR("received MSG_ERR_EINT");
					//break;
				}
				DO_LOG_ERROR("Failed to receive message errno: " + std::to_string(ret));
				continue;
			}
			
			// process ZMQ message and publish to MQTT
			processMsg(msg, mqttPublisher);

		}
		catch (exception &ex)
		{
			DO_LOG_FATAL((std::string)ex.what()+" for topic : "+topic);
		}
	}//while ends

	DO_LOG_DEBUG("exited !!");
}

/**
 * publish message to EIS
 * @param eisMsg :[in] message to publish on EIS
 * @param context :[in] msg bus context
 * @param pubContext :[in] pub context
 * @return true/false based on success/failure
 */
bool publishEISMsg(string eisMsg, stZmqContext &context,
		stZmqPubContext &pubContext)
{
	bool retVal = false;

	if(context.m_pContext == NULL || pubContext.m_pContext == NULL)
	{
		DO_LOG_ERROR("Cannot publish on EIS as topic context or msgbus context is/are NULL");
		return retVal;
	}

	// Creating message to be published
	msg_envelope_t *msg = msgbus_msg_envelope_new(CT_JSON);
	if(msg == NULL)
	{
		DO_LOG_ERROR("could not create new msg envelope");
		return retVal;
	}

	cJSON *root = NULL;

	try
	{
		//parse from root element
		root = cJSON_Parse(eisMsg.c_str());
		if (NULL == root)
		{
			DO_LOG_ERROR("Could not parse value received from MQTT");

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
				DO_LOG_DEBUG((std::string)(device->string)+" : "+device->valuestring);

				msg_envelope_elem_body_t *value = msgbus_msg_envelope_new_string(
						device->valuestring);
				if(value != NULL)
				{
					msgbus_msg_envelope_put(msg, device->string, value);
				}
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
		DO_LOG_FATAL(strException);
	}
	catch(exception &ex)
	{
		DO_LOG_FATAL(ex.what());
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
 * Process message received from MQTT and send it on EIS
 * @param recvdMsg :[in] message to publish on EIS
 * @param isWrite :[in] is write request or read
 * @param isRealtime :[in] is real-time request or non-real-time
 * @return true/false based on success/failure
 */
void processMsgToSendOnEIS(mqtt::const_message_ptr &recvdMsg, bool &isRead, bool &isRealtime)
{
	try
	{
		//received msg from queue
		string rcvdTopic = recvdMsg->get_topic();
		string strMsg = recvdMsg->get_payload();

		//this should be present in each incoming request
		if (rcvdTopic.empty()) //will not be the case ever
		{
			DO_LOG_ERROR("topic key not present in message: " + strMsg);
			return;
		}

		DO_LOG_DEBUG("Request received from MQTT for topic "+ rcvdTopic);

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
			DO_LOG_ERROR("EIS topic is not set to publish on EIS"+ rcvdTopic);
			return;
		}
		else
		{
			//publish data to EIS
			DO_LOG_DEBUG("Received mapped EIS topic : " + eisTopic);

			//add source topic name in payload
			bool bRes = addSrTopic(strMsg, rcvdTopic);
			if(bRes == false)
			{
				DO_LOG_ERROR("Failed to add sourcetopic " + rcvdTopic + " in payload for EIS");
				return;
			}

			//get EIS msg bus context
			stZmqContext context;
			if (!CEISMsgbusHandler::Instance().getCTX(eisTopic, context)) {
				DO_LOG_ERROR("cannot find msgbus context for topic : "+ eisTopic);
				return;
			}

			//will give topic context
			stZmqPubContext pubContext;
			if (!CEISMsgbusHandler::Instance().getPubCTX(eisTopic,
					pubContext))
			{
				DO_LOG_ERROR("cannot find pub context for topic : "+ eisTopic)
				return;
			}

			if(publishEISMsg(strMsg, context, pubContext))
			{
				DO_LOG_DEBUG("Published EIS message : "	+ strMsg + " on topic :" + eisTopic);
				//std::cout << "Published EIS message : "	<< strMsg << " on topic :" << eisTopic << std::endl;
			}
			else
			{
				DO_LOG_ERROR("Failed to publish EIS message : "	+ strMsg + " on topic :" + eisTopic);

#ifdef PERFTESTING
			CMQTTHandler::instance().incSubQTried();
#endif
			}
		}
	}
	catch(exception &ex)
	{
		DO_LOG_FATAL(ex.what());
#ifdef PERFTESTING
		CMQTTHandler::instance().incSubQSkipped();
#endif
	}

	return;
}

/**
 * Set thread priority for threads that send messages from MQTT-Export to EIS
 * depending on read and real-time parameters
 * @param isRealtime :[in] is operation real-time or not
 * @param isRead :[in] is it read or write operation
 * @return None
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
		DO_LOG_DEBUG("Set thread priorities");
		std::cout << "** Set thread priorities for isRead: " << isRead << ", isRealtime : " << isRealtime << endl;
	}
	catch(exception &e)
	{
		DO_LOG_FATAL(e.what());
	}
}

/**
 * Thread function to read requests from queue filled up by MQTT and send data to EIS
 * @param qMgr 	:[in] pointer to respective queue manager
 * @return None
 */
void postMsgsToEIS(QMgr::CQueueMgr& qMgr)
{
	DO_LOG_DEBUG("Starting thread to send messages on EIS");

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
		DO_LOG_FATAL(e.what());
	}
}

/**
 * Get EIS topic list, get corresponding message bus and topic contexts.
 * Spawn threads to listen to EIS messages, receive messages from EIS and publish them to MQTT
 * @param None
 * @return None
 */
void postMsgstoMQTT()
{
	DO_LOG_DEBUG("Initializing threads to start listening on EIS topics...");

	// get sub topic list
	vector<string> vFullTopics = CEISMsgbusHandler::Instance().getSubTopicList();

	for (auto &topic : vFullTopics)
	{
		if(topic.empty())
		{
			DO_LOG_ERROR("found empty MQTT subscriber topic");
			continue;
		}

		stZmqContext context;

		if (!CEISMsgbusHandler::Instance().getCTX(topic, context))
		{
			DO_LOG_ERROR("cannot find msgbus context for topic : " + topic);
			continue;		//go to next topic
		}

		//will give topic context
		stZmqSubContext subContext;
		if (!CEISMsgbusHandler::Instance().getSubCTX(topic,
				subContext))
		{
			DO_LOG_ERROR("cannot find sub context context for topic : "+ topic);
			continue;		//go to next topic
		}

		DO_LOG_DEBUG("Full topic - " + topic + " AND listening on: " + topic);

		//get operation depending on the topic
		globalConfig::COperation objOperation;
		getOperation(topic, objOperation);

		g_vThreads.push_back(
				std::thread(listenOnEIS, topic, context, subContext, objOperation));
	}
}

/**
 * Create EIS msg bus context and topic context for publisher and subscriber both
 * @param None
 * @return true/false based on success/failure
 */
bool initEISContext()
{
	bool retVal = true;

	// Initializing all the pub/sub topic base context for ZMQ
	const char* env_pubTopics = std::getenv("PubTopics");
	if (env_pubTopics != NULL)
	{
		DO_LOG_DEBUG("List of topic configured for Pub are :: "+ (std::string)(env_pubTopics));
		bool bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext(
				"pub");
		if (!bRetVal)
		{
			DO_LOG_ERROR("Context creation failed for sub topic ");
			std::cout << __func__ << ":" << __LINE__ << " Error : Context creation failed for sub topic" <<  std::endl;
			retVal = false;
		}
	}
	else
	{
		DO_LOG_ERROR("could not find PubTopics in environment variables");
		std::cout << __func__ << ":" << __LINE__ << " Error : could not find PubTopics in environment variables" <<  std::endl;
		retVal = false;
	}

	const char* env_subTopics = std::getenv("SubTopics");
	if(env_subTopics != NULL)
	{
		DO_LOG_DEBUG("List of topic configured for Sub are :: "+ (std::string)(env_subTopics));
		bool bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext("sub");
		if (!bRetVal)
		{
			DO_LOG_ERROR("Context creation failed for sub topic");
			std::cout << __func__ << ":" << __LINE__ << " Error : Context creation failed for sub topic" <<  std::endl;
			retVal = false;
		}
	}
	else
	{
		DO_LOG_ERROR("could not find SubTopics in environment variables");
		std::cout << __func__ << ":" << __LINE__ << " Error : could not find SubTopics in environment variables" <<  std::endl;
		retVal = false;
	}

	return retVal;
}

/**
 * Main function of application
 * @param argc :[in] number of input parameters
 * @param argv :[in] input parameters
 * @return 	0/-1 based on success/failure
 */
int main(int argc, char *argv[])
{
	DO_LOG_DEBUG("Starting MQTT Export ...");
	std::cout << __func__ << ":" << __LINE__ << " ------------- Starting MQTT Export Container -------------" << std::endl;

	try
	{
		//initialize CCommon class to get common variables
		string AppName = CCommon::getInstance().getStrAppName();
		if(AppName.empty())
		{
			DO_LOG_ERROR("AppName Environment Variable is not set");
			std::cout << __func__ << ":" << __LINE__ << " Error : AppName Environment Variable is not set" <<  std::endl;
			return -1;
		}

		DO_LOG_INFO("MQTT-Expprt container app version is set to :: "+  std::string(APP_VERSION));
		cout << "MQTT-Expprt container app version is set to :: "+  std::string(APP_VERSION) << endl;

		// load global configuration for container real-time setting
		bool bRetVal = globalConfig::loadGlobalConfigurations();
		if(!bRetVal)
		{
			DO_LOG_INFO("Global configuration is set with some default parameters");
			cout << "\nGlobal configuration is set with some default parameters\n\n";
		}
		else
		{
			DO_LOG_INFO("Global configuration is set successfully");
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
		DO_LOG_FATAL(e.what());
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
		return -1;
	}
	catch (...)
	{
		DO_LOG_FATAL("Unknown Exception Occurred. Exiting");
		std::cout << __func__ << ":" << __LINE__ << "Exception : Unknown Exception Occurred. Exiting" << std::endl;
		return -1;
	}

	std::cout << __func__ << ":" << __LINE__ << " ------------- Exiting MQTT Export Container -------------" << std::endl;
	return 0;
}
