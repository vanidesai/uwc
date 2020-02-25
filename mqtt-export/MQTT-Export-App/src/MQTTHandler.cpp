/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "MQTTHandler.hpp"
#include "TopicMapper.hpp"
#include <vector>
#include "cjson/cJSON.h"
#include <semaphore.h>


sem_t g_semaphoreRespProcess;
std::mutex g_mqttSubMutexLock;
std::mutex g_mqttPublishMutexLock;

/**
 * constructor
 * @param strPlBusUrl :[in] MQTT broker URL
 */
CMQTTHandler::CMQTTHandler(std::string strPlBusUrl) :
		publisher(strPlBusUrl, CLIENTID), subscriber(strPlBusUrl, SUBSCRIBERID), ConfigState(MQTT_PUBLISHER_CONNECT_STATE), subConfigState(
				MQTT_SUSCRIBER_CONNECT_STATE) {
	try {

		//connect options for async subscriber
		mqtt::message willmsg("MQTTConfiguration", LWT_PAYLOAD, QOS, true);
		mqtt::will_options will(willmsg);
		conopts.set_will(will);
		conopts.set_keep_alive_interval(20);
		conopts.set_clean_session(true);
		conopts.set_automatic_reconnect(1, 10);

		//connect options for sync publisher/client
		syncConnOpts.set_keep_alive_interval(20);
		syncConnOpts.set_clean_session(true);
		syncConnOpts.set_automatic_reconnect(1, 10);

#ifdef QUEUE_FAILED_PUBLISH_MESSAGES
		initSem();
#endif

		publisher.set_callback(syncCallback);
		subscriber.set_callback(callback);

		connectSubscriber();

		CLogger::getInstance().log(DEBUG, LOGDETAILS("MQTT initialized successfully"));


	} catch (const std::exception &e) {
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
	}
}

/**
 * function to get single instance of this class
 * @return Handle to single instance of this class, exits in case of failure
 */
CMQTTHandler& CMQTTHandler::instance() {

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

/**
 * MQTT publisher connects with MQTT broker
 * @return 	true : on success,
 * 			false : on error
 */
bool CMQTTHandler::connect() {

	bool bFlag = true;
	try {

		publisher.connect(syncConnOpts);
	    std::cout << __func__ << ":" << __LINE__ << " MQTT publisher connected with MQTT broker" << std::endl;

	} catch (const std::exception &e) {
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;

		bFlag = false;
	}
	return bFlag;
}

#ifdef QUEUE_FAILED_PUBLISH_MESSAGES
/**
 * Retrieves a message from message queue
 * @param a_msg :[in] reference to message to retrieve
 * @return 	true : on success,
 * 			false : on error
 */
bool CMQTTHandler::getMsgFromQ(stMsgData &a_msg) {
	CLogger::getInstance().log(DEBUG, "Pre-publish Q msgs count: " + m_qMsgData.size());

	bool bRet = true;
	try {
		std::lock_guard<std::mutex> lock(m_mutexMsgQ);
		/// Ensure that only on thread can execute at a time
		if (false == m_qMsgData.empty()) {
			a_msg = m_qMsgData.front();
			m_qMsgData.pop();
		} else {
			bRet = false;
		}
	} catch (const std::exception &e) {
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));

		bRet = false;
	}
	return bRet;
}

/**
 * Push a message in message queue
 * @param a_msg :[in] reference to message to push in queue
 * @return 	true : on success,
 * 			false : on error
 */
bool CMQTTHandler::pushMsgInQ(const stMsgData &a_msg) {

	bool bRet = true;
	try {
		/// Ensure that only on thread can execute at a time
		std::lock_guard<std::mutex> lock(m_mutexMsgQ);
		m_qMsgData.push(a_msg);

		CLogger::getInstance().log(DEBUG, "Total msgs pushed in publish Q : " + m_qMsgData.size());
	} catch (const std::exception &e) {
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		bRet = false;
	}
	return bRet;

}

/**
 * Thread function to post message from message queue
 */
void CMQTTHandler::postPendingMsgsThread() {
	CLogger::getInstance().log(DEBUG, "Starting thread to publish pending data");

#ifdef REALTIME_THREAD_PRIORITY
	//set priority
	CTopicMapper::getInstance().set_thread_priority();
#endif

	bool bDoRun = false;
	try {
		stMsgData msg;
		do {
				if (false == publisher.is_connected()) {
					bDoRun = false;
					break;
				}
				if (false == getMsgFromQ(msg)) {
					CLogger::getInstance().log(DEBUG, "No msgs to send, Q is empty, stopping thread");
					bDoRun = false;
					break;
				}

				bDoRun = true;
				publish(msg.m_sMsg, msg.m_sTopic, msg.m_iQOS, true);
#ifdef PERFTESTING
				m_uiQReqTried++;
#endif
		} while (true == bDoRun);
	} catch (const std::exception &e) {
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		
#ifdef PERFTESTING
		CMQTTHandler::m_ui32PublishSkipped++;
#endif
	}
	CLogger::getInstance().log(DEBUG, "--Stopping thread to publish pending data");
}

/**
 * Post pending messages from message queue
 */
void CMQTTHandler::postPendingMsgs() {
	// Create a new thread to post messages to MQTT
	std::thread { std::bind(&CMQTTHandler::postPendingMsgsThread,
			std::ref(*this)) }.detach();
}
#endif

/**
 * Get publisher current state
 * @return publisher state
 */
Mqtt_Config_state_t CMQTTHandler::getMQTTConfigState() {
	return ConfigState;
}

/**
 * Set publisher state to given
 * @param tempConfigState :[in] publisher state to set
 */
void CMQTTHandler::setMQTTConfigState(Mqtt_Config_state_t tempConfigState) {
	ConfigState = tempConfigState;
}

/**
 * Gets current time in nano seconds
 * @param ts :[in] structure of current time
 * @return current time in nano seconds
 */
static unsigned long get_nanos(struct timespec ts) {
    return (unsigned long)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

/**
 * Add current time stamp in message payload
 * @param a_sMsg 		:[in] message in which to add time
 * @param a_tsMsgRcvd	:[in] time stamp in nano seconds
 * @return 	true : on success,
 * 			false : on error
 */
bool CMQTTHandler::addTimestampsToMsg(std::string &a_sMsg, struct timespec a_tsMsgRcvd)
{
	cJSON *root = NULL;
	try {

		root = cJSON_Parse(a_sMsg.c_str());
		if (NULL == root) {
			CLogger::getInstance().log(ERROR, 
					LOGDETAILS("ZMQ Message could not be parsed in json format"));
			return false;
		}

		struct timespec tsMsgPublish;
		timespec_get(&tsMsgPublish, TIME_UTC);
		std::string strTsRcvd = std::to_string(get_nanos(a_tsMsgRcvd));
		std::string strTsPublish = std::to_string(get_nanos(tsMsgPublish));
		cJSON_AddStringToObject(root, "tsMsgReadyForPublish", strTsPublish.c_str());
		cJSON_AddStringToObject(root, "tsMsgRcvdForProcessing", strTsRcvd.c_str());

		a_sMsg.clear();
		char *psNewJson = cJSON_Print(root);
		if(NULL != psNewJson)
		{
			a_sMsg.assign(psNewJson);
			free(psNewJson);
			psNewJson = NULL;
		}

		if(root != NULL)
			cJSON_Delete(root);

		CLogger::getInstance().log(DEBUG, LOGDETAILS("Added timestamp in payload for MQTT"));
		return true;

	} catch (exception &ex) {

		CLogger::getInstance().log(DEBUG, LOGDETAILS("Failed to add timestamp in payload for MQTT"));
		
		if(root != NULL)
			cJSON_Delete(root);

		return false;
	}
}

/**
 * Publish message to MQTT broker without message queue
 * @param a_sMsg 		:[in] message to publish
 * @param a_sTopic		:[in] topic on which to publish
 * @param qos			:[in] QOS with which to publish
 * @param a_tsMsgRcvd	:[in] time when message is received
 * @return 	true : on success,
 * 			false : on error
 */
bool CMQTTHandler::publish(std::string a_sMsg, std::string a_sTopic, int qos, struct timespec a_tsMsgRcvd) {

	static bool bIsFirst = true;
	if (true == bIsFirst) {
		connect();
		bIsFirst = false;
	}

	try {
		// Add timestamp to message
		addTimestampsToMsg(a_sMsg, a_tsMsgRcvd);
		publish(a_sMsg, a_sTopic, qos);
	} catch (const mqtt::exception &exc) {
#ifdef PERFTESTING
		m_ui32PublishExcep++;
#endif
		CLogger::getInstance().log(FATAL, LOGDETAILS(exc.what()));
	}
	return false;
}

/**
 * Publish message on MQTT broker
 * @param a_sMsg 	:[in] message to publish
 * @param a_sTopic	:[in] topic on which to publish message
 * @param a_iQOS	:[in] QOS with which to publish message
 * @param a_bFromQ	:[in] is message from queue
 * @return 	true : on success,
 * 			false : on error
 */
bool CMQTTHandler::publish(std::string &a_sMsg, std::string &a_sTopic, int &a_iQOS,
		bool a_bFromQ) {

	try {
		std::lock_guard<std::mutex> lock(mqttMutexLock);

		// Check if topic is blank
		if (true == a_sTopic.empty()) {
			if (true == a_sMsg.empty()) {
				CLogger::getInstance().log(ERROR, LOGDETAILS("Blank topic and blank Message"));
			} else {
				CLogger::getInstance().log(ERROR, LOGDETAILS("Blank topic. Message not posted"));
			}
			return false;
		}

#ifdef PERFTESTING
		CMQTTHandler::m_ui32PublishReq++;
#endif

		if(false == publisher.is_connected()) {
			CLogger::getInstance().log(ERROR, LOGDETAILS("MQTT publisher is not connected with MQTT broker" + std::to_string(a_iQOS)));
			CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to publish msg on MQTT : " + a_sMsg));
#ifdef QUEUE_FAILED_PUBLISH_MESSAGES
			pushMsgInQ(stMsgData(a_sMsg, a_sTopic, a_iQOS));
#endif
#ifdef PERFTESTING
			m_ui32ConnectionLost++;
			m_ui32PublishFailed++;
#endif
			return false;
		}

		publisher.publish(mqtt::message(a_sTopic, a_sMsg, a_iQOS, false));
		CLogger::getInstance().log(DEBUG, LOGDETAILS("Published message on MQTT broker successfully with QOS:"
									+ std::to_string(a_iQOS)));
		return true;

	} catch (const mqtt::exception &exc) {
		if (false == a_bFromQ) {
#ifdef PERFTESTING
			m_ui32PublishStrExcep++;
#endif
#ifdef QUEUE_FAILED_PUBLISH_MESSAGES
			pushMsgInQ(stMsgData(a_sMsg, a_sTopic, a_iQOS));
#endif
		} else {
#ifdef PERFTESTING
			m_ui32PublishExcep++;
#endif
		}

		CLogger::getInstance().log(FATAL, LOGDETAILS(exc.what()));
	}
	return false;
}

#ifdef PERFTESTING
std::atomic<uint32_t> CMQTTHandler::m_ui32PublishReq(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32PublishReqErr(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32Published(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32PublishFailed(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32ConnectionLost(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32Connection(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32PublishSkipped(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32SubscribeSkipped(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32PublishExcep(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32PublishReqTimeOut(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32Disconnected(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32PublishStrReq(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32PublishStrReqErr(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32PublishStrExcep(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32DelComplete(0);
std::atomic<uint32_t> CMQTTHandler::m_ui32MessageArrived(0);
std::atomic<uint32_t> CMQTTHandler::m_uiQReqTried(0);
std::atomic<uint32_t> CMQTTHandler::m_uiSubscribeQReqTried(0);

/**
 * Print counters
 * @return
 */
void CMQTTHandler::printCounters()
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Req rcvd: " + std::to_string(m_ui32PublishReq)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Req err: "  + std::to_string(m_ui32PublishReqErr)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Req sendmsg excep: "  + std::to_string(m_ui32PublishSkipped)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Req publish excep: "  + std::to_string(m_ui32PublishExcep)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Req published: "  + std::to_string(m_ui32Published)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Req publish failed: "  + std::to_string(m_ui32PublishFailed)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Req publish timeout: "  + std::to_string(m_ui32PublishReqTimeOut)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Req during no connection: "  + std::to_string(m_ui32Disconnected)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Req conn lost: " + std::to_string(m_ui32ConnectionLost)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Req conn done: " + std::to_string(m_ui32Connection)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("*****Str Req: " + std::to_string(m_ui32PublishStrReq)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("*****Str Req err: " + std::to_string(m_ui32PublishStrReqErr)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("*****Str Req excep: " + std::to_string(m_ui32PublishStrExcep)));
#ifdef QUEUE_FAILED_PUBLISH_MESSAGES
	CLogger::getInstance().log(DEBUG, LOGDETAILS("----Pending Q Size: " + std::to_string(instance().m_qMsgData.size())));
#endif
	CLogger::getInstance().log(DEBUG, LOGDETAILS("++++Req posted from Q: " + std::to_string(m_uiQReqTried)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("$$$$Delivery completed: " + std::to_string(m_ui32DelComplete)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Subscriber tried to publish message:" + std::to_string(m_uiSubscribeQReqTried)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Subscriber skipped publishing message:" + std::to_string(m_ui32SubscribeSkipped)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Subscriber received messages:" + std::to_string(m_ui32MessageArrived)));
}
#endif

/**
 * Get MQTT subscriber current state
 * @return subscriber current state
 */
Mqtt_Sub_Config_state_t CMQTTHandler::getMQTTSubConfigState() {
	return subConfigState;
}

/**
 * Set MQTT subscriber state to given
 * @param tempConfigState :[in] subscriber state to set
 */
void CMQTTHandler::setMQTTSubConfigState(Mqtt_Sub_Config_state_t tempConfigState) {
	subConfigState = tempConfigState;
}

/**
 * Initialize semaphores
 * @return 	true : on success,
 * 			false : on error
 */
bool CMQTTHandler::initSem()
{
	int ok = sem_init(&g_semaphoreRespProcess, 0, 0 /* Initial value of zero*/);
	if (ok == -1) {
	   CLogger::getInstance().log(ERROR, LOGDETAILS("could not create unnamed semaphore, exiting"));
	   std::cout << __func__ << ":" << __LINE__ << " Error : could not create unnamed semaphore, exiting" <<  std::endl;
	   exit(0);
	}
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Sempaphores initialized successfully"));

	return true;
}

/**
 * Subscribe with MQTT broker for topics
 * @return 	true : on success,
 * 			false : on error
 */
bool CMQTTHandler::subscribeToTopics() {

	//get list of topics from topic mapper
	std::vector<std::string> vMqttEnvTopics;
	vMqttEnvTopics.push_back("mqtt_SubReadTopic");
	vMqttEnvTopics.push_back("mqtt_SubWriteTopic");

	std::vector<std::string> vMqttTopics;

	try
	{
		for (auto &envTopic : vMqttEnvTopics) {
			const char* env_pubWriteTopic = std::getenv(envTopic.c_str());
			if(env_pubWriteTopic == NULL) {
				CLogger::getInstance().log(ERROR, LOGDETAILS(envTopic + " Environment Variable is not set"));
				std::cout << __func__ << ":" << __LINE__ << " Error : " + envTopic + " Environment Variable is not set" <<  std::endl;
				continue;
			}
			vMqttTopics.push_back(env_pubWriteTopic);
		}

		for (auto &topic : vMqttTopics) {
			if(! topic.empty()) {
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
bool CMQTTHandler::connectSubscriber() {

	bool bFlag = true;
	try {
		std::lock_guard<std::mutex> lock(g_mqttSubMutexLock);

		subscriber.connect(conopts, nullptr, listener);

		// Wait for 2 seconds to get connected
		/*if (false == conntok->wait_for(2000))
		 {
		 CLogger::getInstance().log(DEBUG, LOGDETAILS("Error::Failed to connect to the platform bus ";
		 bFlag = false;
		 }*/
		std::cout << __func__ << ":" << __LINE__ << " Subscriber connected successfully with MQTT broker" << std::endl;
	} catch (const std::exception &e) {
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
		bFlag = false;
	}

	CLogger::getInstance().log(DEBUG, LOGDETAILS("Subscriber connected successfully with MQTT broker"));

	return bFlag;
}

/**
 * Retrieve message from message queue to publish on EIS
 * @param msg :[in] reference to message to retrieve from queue
 * @return 	true : on success,
 * 			false : on error
 */
bool CMQTTHandler::getSubMsgFromQ(mqtt::const_message_ptr &msg) {

	bool bRet = true;
	try {
		std::lock_guard<std::mutex> lock(m_mutexSubMsgQ);
		/// Ensure that only on thread can execute at a time
		if (false == m_qSubMsgData.empty()) {
			msg = m_qSubMsgData.front();
			m_qSubMsgData.pop();
		} else {
			bRet = false;
		}
	} catch (const std::exception &e) {
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		bRet = false;
	}
	return bRet;
}

/**
 * Push message in message queue to send on EIS
 * @param msg :[in] reference of message to push in queue
 * @return 	true : on success,
 * 			false : on error
 */
bool CMQTTHandler::pushSubMsgInQ(mqtt::const_message_ptr msg) {

	bool bRet = true;
	try {
		/// Ensure that only on thread can execute at a time
		std::lock_guard<std::mutex> lock(m_mutexSubMsgQ);
		m_qSubMsgData.push(msg);
		CLogger::getInstance().log(DEBUG, LOGDETAILS("Pushed MQTT message in queue"));

		// Signal response process thread
		sem_post(&g_semaphoreRespProcess);

	} catch (const std::exception &e) {
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
	sem_destroy(&g_semaphoreRespProcess);
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Destroying CMQTTHandler instance ..."));

	conopts.set_automatic_reconnect(0);

	subscriber.disable_callbacks();

	if(publisher.is_connected())
		publisher.disconnect();

	if(subscriber.is_connected())
		subscriber.disconnect();

#ifdef QUEUE_FAILED_PUBLISH_MESSAGES
	m_qMsgData = {};
#endif

	m_qSubMsgData = {};

	CLogger::getInstance().log(DEBUG, LOGDETAILS("Destroyed CMQTTHandler instance"));
}

/**
 * Destructor
 */
CMQTTHandler::~CMQTTHandler()
{
}

/////////////////////////////////////////////
