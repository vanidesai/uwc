/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include <vector>
#include "cjson/cJSON.h"
#include "Common.hpp"
#include "MQTTSubscribeHandler.hpp"

/**
 * constructor
 * @param strPlBusUrl :[in] MQTT broker URL
 */
CMQTTHandler::CMQTTHandler(std::string strPlBusUrl) :
		subscriber(strPlBusUrl, SUBSCRIBERID), subConfigState(MQTT_SUSCRIBER_CONNECT_STATE)
{
	try
	{
		//connect options for async subscriber
		mqtt::message willmsg("MQTTConfiguration", LWT_PAYLOAD, QOS, true);
		mqtt::will_options will(willmsg);
		conopts.set_will(will);
		conopts.set_keep_alive_interval(20);
		conopts.set_clean_session(true);
		conopts.set_automatic_reconnect(1, 10);

		subscriber.set_callback(callback);

		connectSubscriber();

		CLogger::getInstance().log(DEBUG, LOGDETAILS("MQTT initialized successfully"));
	}
	catch (const std::exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
	}
}

/**
 * function to get single instance of this class
 * @return Handle to single instance of this class, exits in case of failure
 */
CMQTTHandler& CMQTTHandler::instance()
{
	static string strPlBusUrl = CTopicMapper::getInstance().getStrMqttExportURL();

	if(strPlBusUrl.empty())
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS(":MQTT_URL_FOR_EXPORT Environment variable is not set"));
		std::cout << __func__ << ":" << __LINE__ << " Error : MQTT_URL_FOR_EXPORT Environment variable is not set" <<  std::endl;
		exit(EXIT_FAILURE);
	}

	static CMQTTHandler handler(strPlBusUrl.c_str());
	return handler;
}

#ifdef PERFTESTING
std::atomic<uint32_t> CMQTTHandler::m_ui32ConnectionLost(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32Connection(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32SubscribeSkipped(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32Disconnected(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32MessageArrived(0);
std::atomic<uint32_t> CMQTTHandler::m_uiQReqTried(0);
std::atomic<uint32_t> CMQTTHandler::m_uiSubscribeQReqTried(0);

/**
 * Print counters
 * @return
 */
void CMQTTHandler::printCounters()
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Req conn lost: " + std::to_string(m_ui32ConnectionLost)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Req conn done: " + std::to_string(m_ui32Connection)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("++++Req posted from Q: " + std::to_string(m_uiQReqTried)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Subscriber tried to publish message:" + std::to_string(m_uiSubscribeQReqTried)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Subscriber skipped publishing message:" + std::to_string(m_ui32SubscribeSkipped)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Subscriber received messages:" + std::to_string(m_ui32MessageArrived)));
}
#endif

/**
 * Get MQTT subscriber current state
 * @return subscriber current state
 */
Mqtt_Sub_Config_state_t CMQTTHandler::getMQTTSubConfigState()
{
	return subConfigState;
}

/**
 * Set MQTT subscriber state to given
 * @param tempConfigState :[in] subscriber state to set
 */
void CMQTTHandler::setMQTTSubConfigState(Mqtt_Sub_Config_state_t tempConfigState)
{
	subConfigState = tempConfigState;
}

/**
 * Subscribe with MQTT broker for topics
 * @return 	true : on success,
 * 			false : on error
 */
bool CMQTTHandler::subscribeToTopics()
{
	//get list of topics from topic mapper
	std::vector<std::string> vMqttEnvTopics;
	vMqttEnvTopics.push_back("mqtt_SubReadTopic");
	vMqttEnvTopics.push_back("mqtt_SubWriteTopic");

	std::vector<std::string> vMqttTopics;

	try
	{
		for (auto &envTopic : vMqttEnvTopics)
		{
			const char* env_pubWriteTopic = std::getenv(envTopic.c_str());
			if(env_pubWriteTopic == NULL)
			{
				CLogger::getInstance().log(ERROR, LOGDETAILS(envTopic + " Environment Variable is not set"));
				std::cout << __func__ << ":" << __LINE__ << " Error : " + envTopic + " Environment Variable is not set" <<  std::endl;
				continue;
			}
			vMqttTopics.push_back(env_pubWriteTopic);
		}

		for (auto &topic : vMqttTopics)
		{
			if(! topic.empty())
			{
				CLogger::getInstance().log(DEBUG, LOGDETAILS("Subscribing topic : " + topic));
				subscriber.subscribe(topic, QOS, nullptr, listener);
			}
		}
		std::cout << __func__ << ":" << __LINE__ << "Subscribed topics with MQTT broker" << std::endl;
	}
	catch(exception &ex)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(ex.what()));
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << ex.what() << std::endl;
		return false;
	}

	CLogger::getInstance().log(DEBUG, LOGDETAILS("Subscribed topics with MQTT broker"));

	return true;
}

/**
 * Connect subscriber with MQTT broker
 * @return 	true : on success,
 * 			false : on error
 */
bool CMQTTHandler::connectSubscriber()
{
	bool bFlag = true;
	try
	{
		//std::lock_guard<std::mutex> lock(g_mqttSubMutexLock);

		subscriber.connect(conopts, nullptr, listener);

		// Wait for 2 seconds to get connected
		/*if (false == conntok->wait_for(2000))
		 {
		 CLogger::getInstance().log(DEBUG, LOGDETAILS("Error::Failed to connect to the platform bus ";
		 bFlag = false;
		 }*/
		std::cout << __func__ << ":" << __LINE__ << " Subscriber connected successfully with MQTT broker" << std::endl;
	}
	catch (const std::exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
		bFlag = false;
	}

	CLogger::getInstance().log(DEBUG, LOGDETAILS("Subscriber connected successfully with MQTT broker"));

	return bFlag;
}

/**
 * Retrieve non-RT read message from message queue to publish on EIS
 * @param msg :[in] reference to message to retrieve from queue
 * @return 	true : on success,
 * 			false : on error
 */
bool CQueueMgr::getSubMsgFromQ(int msgRequestType, mqtt::const_message_ptr &msg)
{
	try
	{
		if(msgVector[msgRequestType].msgQueue.empty())
		{
			return false;
		}

		std::lock_guard<std::mutex> lock(msgVector[msgRequestType].queueMutex);
		msg = msgVector[msgRequestType].msgQueue.front();
		msgVector[msgRequestType].msgQueue.pop();
	}
	catch (const std::exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		return false;
	}
	return true;
}

/**
* parse message to check if topic is of read or write
* @param json	:[in] message from which to check read/write operation
* @return bool
*/
bool CQueueMgr::getOperation(string &topic, bool &isWrite)
{
	//compare if request received for write or read
	if(topic.find("write") != std::string::npos)
	{
		isWrite = true;
	}
	else if(topic.find("read") != std::string::npos)
	{
		isWrite = false;
	}
	else
	{
		CLogger::getInstance().log(DEBUG, LOGDETAILS("Invalid topic received on MQTT export from MQTT"));
		return false;
	}

	return true;
}

/**
* parse message to retrieve QOS and topic names
* @param json	:[in] message from which to retrieve real-time
* @return bool
*/
bool CQueueMgr::parseMQTTMsg(const char *json, bool &isRealtime)
{
	bool bRetVal = false;

	try
	{
		cJSON *root = cJSON_Parse(json);
		if (NULL == root)
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("Message received from MQTT could not be parsed in json format"));
			return bRetVal;
		}

		if(! cJSON_HasObjectItem(root, "realtime"))
		{
			CLogger::getInstance().log(DEBUG, LOGDETAILS("Message received from MQTT does not have key \"realtime\", request will be sent as non-RT"));
			if (NULL != root)
				cJSON_Delete(root);

			isRealtime = false;
			return true;
		}

		char *crealtime = cJSON_GetObjectItem(root, "realtime")->valuestring;
		if (NULL == crealtime)
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("Key 'realtime' could not be found in message received from ZMQ"));
			if (NULL != root)
				cJSON_Delete(root);

			return false;
		}
		else
		{
			if(strcmp(crealtime, "0") == 0)
			{
				isRealtime = false;
			}
			else if(strcmp(crealtime, "1") == 0)
			{
				isRealtime = true;
			}
			else
			{
				isRealtime = false;
			}
		}

		if (NULL != root)
			cJSON_Delete(root);

		bRetVal = true;
	}
	catch (std::exception &ex)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(ex.what()));
		bRetVal = false;
	}

	return bRetVal;
}

/**
 * Push message in message queue to send on EIS
 * @param msg :[in] reference of message to push in queue
 * @return 	true : on success,
 * 			false : on error
 */
bool CMQTTHandler::pushSubMsgInQ(mqtt::const_message_ptr msg)
{
	bool bRet = true;
	try
	{
		string topic = msg->get_topic();
		string payload = msg->get_payload();

		//this should be present in each incoming request
		if (payload == "") //will not be the case ever
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("No payload with request"));
			return false;
		}

		bool isWrite = false;//toggle between read and write operation
		if(! queueMgr.getOperation(topic, isWrite))
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("Invalid topic received from MQTT on MQTT-Export"));
			return false;
		}

		//parse payload to check if realtime is true or false
		bool isRealTime = false;
		if( ! queueMgr.parseMQTTMsg(payload.c_str(), isRealTime))
		{
			CLogger::getInstance().log(DEBUG, LOGDETAILS("Could not parse MQTT msg"));
			return false;
		}

		int requestType = -1;
		//if payload contains realtime flag, push message to real time queue
		if(isRealTime)
		{
			if(isWrite)
			{
				requestType = 3;
			}
			else
			{
				requestType = 1;
			}
		}
		else //non-RT
		{
			if(isWrite)
			{
				requestType = 2;
			}
			else
			{
				requestType = 0;
			}
		}
		queueMgr.pushMsg(requestType, msg);

		CLogger::getInstance().log(DEBUG, LOGDETAILS("Pushed MQTT message in queue"));
		bRet = true;
	}
	catch (const std::exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		bRet = false;
	}
	return bRet;
}

/**
 * Clean up, destroy semaphores, disables callback, disconnect from MQTT broker
 */
void CMQTTHandler::cleanup()
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Destroying CMQTTHandler instance ..."));

	conopts.set_automatic_reconnect(0);

	subscriber.disable_callbacks();

	if(subscriber.is_connected())
		subscriber.disconnect();

	CLogger::getInstance().log(DEBUG, LOGDETAILS("Destroyed CMQTTHandler instance"));
}

/**
 * Clean up, destroy semaphores, disables callback, disconnect from MQTT broker
 */
void CQueueMgr::cleanup()
{
	sem_destroy(&g_semReadMsg);
	sem_destroy(&g_semRTReadMsg);

	sem_destroy(&g_semWriteMsg);
	sem_destroy(&g_semRTWriteMsg);
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Destroyed CMQTTHandler instance"));
}

/**
 * Destructor
 */
CMQTTHandler::~CMQTTHandler()
{
}

/////////////////////////////////////////////
//CQueueMgr
////////////////////////////////////////////
CQueueMgr::CQueueMgr()
{

	//initialize struct for all 4 operations
	for(int i = 0 ; i < 4; i++)
	{
		msgVector[i].type = i;
	}

	initSem();
}

CQueueMgr::~CQueueMgr()
{
	cleanup();
}

/**
 * Initialize semaphores
 * @return 	true : on success,
 * 			false : on error
 */
bool CQueueMgr::initSem()
{
	/* Initial value of zero*/
	if(-1 == sem_init(&g_semRTReadMsg, 0, 0))
	{
	   CLogger::getInstance().log(ERROR, LOGDETAILS("could not create semaphore for real-time msgs, exiting"));
	   std::cout << __func__ << ":" << __LINE__ << " Error : could not create semaphore for real-time msgs, exiting" <<  std::endl;
	   exit(0);
	}

	if( -1 == sem_init(&g_semRTWriteMsg, 0, 0))
	{
	   CLogger::getInstance().log(ERROR, LOGDETAILS("could not create semaphore for real-time msgs, exiting"));
	   std::cout << __func__ << ":" << __LINE__ << " Error : could not create semaphore for real-time msgs, exiting" <<  std::endl;
	   exit(0);
	}

	if( -1 == sem_init(&g_semReadMsg, 0, 0))
	{
	   CLogger::getInstance().log(ERROR, LOGDETAILS("could not create semaphore for non-real-time msgs, exiting"));
	   std::cout << __func__ << ":" << __LINE__ << " Error : could not create semaphore for non-real-time msgs, exiting" <<  std::endl;
	   exit(0);
	}

	if( -1 == sem_init(&g_semWriteMsg, 0, 0))
	{
	   CLogger::getInstance().log(ERROR, LOGDETAILS("could not create semaphore for non-real-time msgs, exiting"));
	   std::cout << __func__ << ":" << __LINE__ << " Error : could not create semaphore for non-real-time msgs, exiting" <<  std::endl;
	   exit(0);
	}

	CLogger::getInstance().log(DEBUG, LOGDETAILS("Semaphores initialized successfully"));

	return true;
}

bool CQueueMgr::pushMsg(int requestType, mqtt::const_message_ptr &msg)
{
	try
	{
		std::lock_guard<std::mutex> lock(msgVector[requestType].queueMutex);
		msgVector[requestType].msgQueue.push(msg);

		switch(requestType)
		{
			case 0://read
			{
				sem_post(&g_semReadMsg);
			}
			break;
			case 1://RT-read
			{
				sem_post(&g_semRTReadMsg);
			}
			break;
			case 2://write
			{
				sem_post(&g_semWriteMsg);
			}
			break;
			case 3://RT-write
			{
				sem_post(&g_semRTWriteMsg);
			}
			break;
			default:
				CLogger::getInstance().log(DEBUG, LOGDETAILS("Invalid msg type"));
			break;
		}
	}
	catch(exception &ex)
	{
		return false;
	}
	return true;
}
