/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#include "ZmqHandler.hpp"
#include "Common.hpp"
#include "QueueMgr.hpp"
#include "MQTTSubscribeHandler.hpp"
#include "ConfigManager.hpp"
#include "Logger.hpp"
#include "MQTTPublishHandler.hpp"
#include "ConfigManager.hpp"
#include "EnvironmentVarHandler.hpp"

#ifdef UNIT_TEST
#include <gtest/gtest.h>
#endif

vector<std::thread> g_vThreads;

std::atomic<bool> g_shouldStop(false);

#define APP_VERSION "0.0.6.1"

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
				std::string mqttMsg(parts[0].bytes);
				mqttPublisher.createNPubMsg(mqttMsg, revdTopic);

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
void listenOnEIS(string topic, zmq_handler::stZmqContext context, zmq_handler::stZmqSubContext subContext, globalConfig::COperation operation)
{
	globalConfig::set_thread_sched_param(operation);
	globalConfig::display_thread_sched_attr(topic + " listenOnEIS");
	int qos = operation.getQos();

	if(context.m_pContext == NULL || subContext.sub_ctx == NULL)
	{
		std::cout << "Cannot start listening on EIS for topic : " << topic << endl;
		DO_LOG_ERROR("Cannot start listening on EIS for topic : " + topic);
		return;
	}

	void *msgbus_ctx = context.m_pContext;
	recv_ctx_t *sub_ctx = subContext.sub_ctx;

	//consider topic name as distinguishing factor for publisher
	CMQTTPublishHandler mqttPublisher(EnvironmentInfo::getInstance().getDataFromEnvMap("MQTT_URL_FOR_EXPORT").c_str(),
					topic, qos);
	mqttPublisher.connect();
	
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
 * @param a_oRcvdMsg  :[in] message to publish on EIS
 * @param a_sEisTopic :[in] EIS topic
 * @return true/false based on success/failure
 */
bool publishEISMsg(CMessageObject &a_oRcvdMsg, const std::string &a_sEisTopic)
{
	bool retVal = false;

	// Creating message to be published
	msg_envelope_t *msg = NULL;
	cJSON *root = NULL;

	try
	{
		msg = msgbus_msg_envelope_new(CT_JSON);
		if(msg == NULL)
		{
			DO_LOG_ERROR("could not create new msg envelope");
			return retVal;
		}
		std::string eisMsg = a_oRcvdMsg.getStrMsg();
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

		auto addField = [&msg](const std::string &a_sFieldName, const std::string &a_sValue) {
			DO_LOG_DEBUG(a_sFieldName + " : " + a_sValue);
			msg_envelope_elem_body_t *value = msgbus_msg_envelope_new_string(a_sValue.c_str());
			if((NULL != value) && (NULL != msg))
			{
				msgbus_msg_envelope_put(msg, a_sFieldName.c_str(), value);
			}
		};

		cJSON *device = root->child;
		while (device)
		{
			if(cJSON_IsString(device))
			{
				addField(device->string, device->valuestring);
			}
			else
			{
				throw std::string("Invalid JSON");
			}
			// get and print key
			device = device->next;
		}

		if (root)
		{
			cJSON_Delete(root);
		}
		root = NULL;

		//add time stamp before publishing msg on EIS
		//std::string strTsReceived;
		//CCommon::getInstance().getCurrentTimestampsInString(strTsReceived);

		addField("tsMsgRcvdFromMQTT", (std::to_string(CCommon::getInstance().get_micros(a_oRcvdMsg.getTimestamp()))).c_str());
		//addField("tsMsgPublishOnEIS", strTsReceived);
		addField("sourcetopic", a_oRcvdMsg.getTopic());

		
		std::string strTsReceived{""};
		bool bRet = true;
		if(true == zmq_handler::publishJson(strTsReceived, msg, a_sEisTopic, "tsMsgPublishOnEIS"))
		{
			bRet = true;
		}
		else
		{
			DO_LOG_ERROR("Failed to publish write msg on EIS: " + eisMsg);
			bRet = false;
		}

		msgbus_msg_envelope_destroy(msg);
		msg = NULL;

		return bRet;
	}
	catch(std::string& strException)
	{
		DO_LOG_ERROR(strException);
	}
	catch(exception &ex)
	{
		DO_LOG_ERROR(ex.what());
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
 * @param a_sEisTopic :[in] EIS topic
 * @return true/false based on success/failure
 */
void processMsgToSendOnEIS(CMessageObject &recvdMsg, const std::string a_sEisTopic)
{
	try
	{
		//received msg from queue
		std::string rcvdTopic = recvdMsg.getTopic();
		std::string strMsg = recvdMsg.getStrMsg();

		//this should be present in each incoming request
		if (rcvdTopic.empty()) //will not be the case ever
		{
			DO_LOG_ERROR("topic key not present in message: " + strMsg);
			return;
		}

		DO_LOG_DEBUG("Request received from MQTT for topic "+ rcvdTopic);

		if (a_sEisTopic.empty())
		{
			DO_LOG_ERROR("EIS topic is not set to publish on EIS"+ rcvdTopic);
			return;
		}
		else
		{
			//publish data to EIS
			DO_LOG_DEBUG("Received mapped EIS topic : " + a_sEisTopic);

			if(publishEISMsg(recvdMsg, a_sEisTopic))
			{
				DO_LOG_DEBUG("Published EIS message : "	+ strMsg + " on topic :" + a_sEisTopic);
			}
			else
			{
				DO_LOG_ERROR("Failed to publish EIS message : "	+ strMsg + " on topic :" + a_sEisTopic);
			}
		}
	}
	catch(exception &ex)
	{
		DO_LOG_ERROR(ex.what());
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

	std::string eisTopic = "";
	if(! isRead)//write request
	{
		if(isRealtime)
		{
			eisTopic.assign(EnvironmentInfo::getInstance().getDataFromEnvMap("WriteRequest_RT"));
		}
		else
		{
			eisTopic.assign(EnvironmentInfo::getInstance().getDataFromEnvMap("WriteRequest"));
		}
	}
	else//read request
	{
		if(isRealtime)
		{
			eisTopic.assign(EnvironmentInfo::getInstance().getDataFromEnvMap("ReadRequest_RT"));
		}
		else
		{
			eisTopic.assign(EnvironmentInfo::getInstance().getDataFromEnvMap("ReadRequest"));
		}
	}

	try
	{
		while (false == g_shouldStop.load())
		{
			CMessageObject oTemp;
			if(true == qMgr.isMsgArrived(oTemp))
			{
				processMsgToSendOnEIS(oTemp, eisTopic);
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
	vector<string> vFullTopics = CcommonEnvManager::Instance().getTopicList();

	for (auto &topic : vFullTopics)
	{
		if(topic.empty())
		{
			DO_LOG_ERROR("found empty MQTT subscriber topic");
			continue;
		}

		zmq_handler::stZmqContext& context = zmq_handler::getCTX(topic);

		//will give topic context
		zmq_handler::stZmqSubContext& subContext = zmq_handler::getSubCTX(topic);

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
		if (true != zmq_handler::prepareCommonContext("pub"))
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
		if (true != zmq_handler::prepareCommonContext("sub"))
		{
			DO_LOG_ERROR("Context creation failed for sub topic");
			std::cout << __func__ << ":" << __LINE__ << " Error : Context creation failed for sub topic" <<  std::endl;
			retVal = false;
		}
		else
		{
			cout << "***** Sub Topics :: " << static_cast<std::string>(env_subTopics) << endl;
			CcommonEnvManager::Instance().splitString(static_cast<std::string>(env_subTopics),',');
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
		CLogger::initLogger(std::getenv("Log4cppPropsFile"));
		DO_LOG_DEBUG("Starting MQTT Export ...");

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
