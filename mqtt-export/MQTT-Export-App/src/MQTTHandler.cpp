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
#include <vector>
#include "cjson/cJSON.h"
#include <semaphore.h>

sem_t g_semaphoreRespProcess;
std::mutex g_mqttSubMutexLock;

// constructor
CMQTTHandler::CMQTTHandler(std::string strPlBusUrl) :
		client(strPlBusUrl, CLIENTID), subscriber(strPlBusUrl, SUBSCRIBERID), ConfigState(MQTT_CLIENT_CONNECT_STATE), subConfigState(
				MQTT_SUSCRIBE_CONNECT_STATE) {
	try {
		mqtt::message willmsg("MQTTConfiguration", LWT_PAYLOAD, QOS, true);
		mqtt::will_options will(willmsg);
		conopts.set_will(will);

		conopts.set_keep_alive_interval(60);
		conopts.set_clean_session(true);
		conopts.set_automatic_reconnect(1, 10);

#ifdef TLSENABLED
		mqtt::ssl_options sslopts;
		char *RootCAPath = getenv("PLBUS_ROOTCAPATH");
		if(NULL == RootCAPath)
		{
			CLogger::getInstance().log(DEBUG, LOGDETAILS("in reading PLBUS_ROOTCAPATH"));
			return;
		}
		sslopts.set_trust_store(RootCAPath);

		char *ClientCertPath = getenv("BACNET_PLBUS_CLIENT_CERT");
		if(NULL == ClientCertPath)
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("Error in reading BACNET_PLBUS_CLIENT_CERT"));
			std::cout << __func__ << ":" << __LINE__ << " Error : Error in reading BACNET_PLBUS_CLIENT_CERT" <<  std::endl;
			return;
		}
		sslopts.set_key_store(ClientCertPath);
		char *ClientKeyPath = getenv("BACNET_PLBUS_CLIENT_KEY");
		if(NULL == ClientKeyPath)
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("Error in reading BACNET_PLBUS_CLIENT_KEY"));
			return;
		}
		sslopts.set_private_key(ClientKeyPath);
		sslopts.set_enable_server_cert_auth(true);
		conopts.set_ssl(sslopts);
#endif
		client.set_callback(callback);

		initSem();

		subscriber.set_callback(callback);

		connectSubscriber();

		CLogger::getInstance().log(DEBUG, LOGDETAILS("MQTT initialized successfully"));

	} catch (const std::exception &e) {
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
	}
}

// function to get single instance of this class
CMQTTHandler& CMQTTHandler::instance() {

	static bool bPlBusUrl = false;
	const char *mqttBrokerURL = NULL;

	string strPlBusUrl;

	if (!bPlBusUrl) {
		/// get the platform bus env variable
		mqttBrokerURL = std::getenv("MQTT_URL_FOR_EXPORT");
		/// check for null
		if (NULL == mqttBrokerURL) {
			CLogger::getInstance().log(ERROR, LOGDETAILS(":MQTT_URL_FOR_EXPORT Environment variable is not set"));
			std::cout << __func__ << ":" << __LINE__ << " Error : MQTT_URL_FOR_EXPORT Environment variable is not set" <<  std::endl;
			exit(EXIT_FAILURE);
		} else {
			strPlBusUrl.assign(mqttBrokerURL);
			CLogger::getInstance().log(DEBUG, LOGDETAILS(":MQTT_URL_FOR_EXPORT Environment variable is set to : "
					+ strPlBusUrl));
			bPlBusUrl = true;
		}
	}

	static CMQTTHandler handler(mqttBrokerURL);
	return handler;
}

bool CMQTTHandler::connect() {
	bool bFlag = true;
	try {
		std::lock_guard<std::mutex> lock(mqttMutexLock);
		conntok = client.connect(conopts, nullptr, listener);
		/// Wait for 2 seconds to get connected
		/*if (false == conntok->wait_for(2000))
		 {
		 CLogger::getInstance().log(DEBUG, LOGDETAILS("Error::Failed to connect to the platform bus ";
		 bFlag = false;
		 }*/
	} catch (const std::exception &e) {
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;

		bFlag = false;
	}
	return bFlag;
}

bool CMQTTHandler::getMsgFromQ(stMsgData &a_msg) {
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
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;

		bRet = false;
	}
	return bRet;
}

bool CMQTTHandler::pushMsgInQ(const stMsgData &a_msg) {
	bool bRet = true;
	try {
		/// Ensure that only on thread can execute at a time
		std::lock_guard<std::mutex> lock(m_mutexMsgQ);
		m_qMsgData.push(a_msg);
	} catch (const std::exception &e) {
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
		bRet = false;
	}
	return bRet;

}

void CMQTTHandler::postPendingMsgsThread() {
	bool bDoRun = false;
	try {
		stMsgData msg;
		do {
			{
				if (false == client.is_connected()) {
					bDoRun = false;
					break;
				}
				if (false == getMsgFromQ(msg)) {
					bDoRun = false;
					break;
				}

				bDoRun = true;
				publish(msg.m_sMsg, msg.m_sTopic, msg.m_iQOS, true);
#ifdef PERFTESTING
				m_uiQReqTried++;
#endif
			}
		} while (true == bDoRun);
	} catch (const std::exception &e) {
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
#ifdef PERFTESTING
		CMQTTHandler::m_ui32PublishSkipped++;
#endif
	}
}

void CMQTTHandler::postPendingMsgs() {
	// Create a new thread to post messages to MQTT
	std::thread { std::bind(&CMQTTHandler::postPendingMsgsThread,
			std::ref(*this)) }.detach();
}

Mqtt_Config_state_t CMQTTHandler::getMQTTConfigState() {
	return ConfigState;
}

void CMQTTHandler::setMQTTConfigState(Mqtt_Config_state_t tempConfigState) {
	ConfigState = tempConfigState;
}

bool CMQTTHandler::publish(std::string a_sMsg, const char *topic, int qos) {
	static bool bIsFirst = true;
	if (true == bIsFirst) {
		connect();
		bIsFirst = false;
	}

	std::string sTopic(topic);

	try {
		publish(a_sMsg, sTopic, qos);
	} catch (const mqtt::exception &exc) {
#ifdef PERFTESTING
		m_ui32PublishExcep++;
#endif
		CLogger::getInstance().log(FATAL, LOGDETAILS(exc.what()));
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << exc.what() << std::endl;
	}
	return false;
}

bool CMQTTHandler::publish(std::string &a_sMsg, std::string &a_sTopic, int &a_iQOS,
		bool a_bFromQ) {
	try {
		std::lock_guard<std::mutex> lock(mqttMutexLock);

		// Check if topic is blank
		if (true == a_sTopic.empty()) {
			if (true == a_sMsg.empty()) {
				CLogger::getInstance().log(ERROR, LOGDETAILS("Blank topic and blank Message"));
				std::cout << __func__ << ":" << __LINE__ << " Error : Blank topic and blank Message" <<  std::endl;
			} else {
				CLogger::getInstance().log(ERROR, LOGDETAILS("Blank topic. Message not posted"));
				std::cout << __func__ << ":" << __LINE__ << " Error : Blank topic. Message not posted" <<  std::endl;
			}
			return false;
		}

#ifdef PERFTESTING
		CMQTTHandler::m_ui32PublishReq++;
#endif
		if (true == client.is_connected()) {
			mqtt::message_ptr pubmsg = mqtt::make_message(a_sTopic, a_sMsg, a_iQOS,
					false);

			client.publish(pubmsg, nullptr, listener);
			CLogger::getInstance().log(DEBUG, LOGDETAILS("Published message on EIS successfully " + std::to_string(a_iQOS)));

			return true;
		} else {
			pushMsgInQ(stMsgData(a_sMsg, a_sTopic, a_iQOS));
#ifdef PERFTESTING
			CMQTTHandler::m_ui32Disconnected++;
#endif
		}
	} catch (const mqtt::exception &exc) {
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << exc.what() << std::endl;
		if (false == a_bFromQ) {
#ifdef PERFTESTING
			m_ui32PublishStrExcep++;
#endif
			pushMsgInQ(stMsgData(a_sMsg, a_sTopic, a_iQOS));
		} else {
			//g_uiStrMsgNotPublished++;
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
	CLogger::getInstance().log(DEBUG, LOGDETAILS("----Pending Q Size: " + std::to_string(instance().m_qMsgData.size())));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("++++Req posted from Q: " + std::to_string(m_uiQReqTried)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("$$$$Delivery completed: " + std::to_string(m_ui32DelComplete)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Subscriber tried to publish message:" + std::to_string(m_uiSubscribeQReqTried)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Subscriber skipped publishing message:" + std::to_string(m_ui32SubscribeSkipped)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Subscriber received messages:" + std::to_string(m_ui32MessageArrived)));
}
#endif

Mqtt_Sub_Config_state_t CMQTTHandler::getMQTTSubConfigState() {
	return subConfigState;
}

void CMQTTHandler::setMQTTSubConfigState(Mqtt_Sub_Config_state_t tempConfigState) {
	subConfigState = tempConfigState;
}


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

bool CMQTTHandler::subscribeToTopics() {

	//get list of topics from topic mapper
	std::vector<std::string> vMqttEnvTopics;
	vMqttEnvTopics.push_back("mqtt_SubReadTopic");
	vMqttEnvTopics.push_back("mqtt_SubWriteTopic");

	std::vector<std::string> vMqttTopics;

	try
	{
		for (auto envTopic : vMqttEnvTopics) {
			const char* env_pubWriteTopic = std::getenv(envTopic.c_str());
			if(env_pubWriteTopic == NULL) {
				CLogger::getInstance().log(ERROR, LOGDETAILS(envTopic + " Environment Variable is not set"));
				std::cout << __func__ << ":" << __LINE__ << " Error : " + envTopic + " Environment Variable is not set" <<  std::endl;
				continue;
			}
			vMqttTopics.push_back(env_pubWriteTopic);
		}

		for (auto topic : vMqttTopics) {
			if(! topic.empty()) {
				CLogger::getInstance().log(DEBUG, LOGDETAILS("Subscribing topic : " + topic));
				subscriber.subscribe(topic, QOS, nullptr, listener);
			}
		}
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
	} catch (const std::exception &e) {
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
		bFlag = false;
	}

	CLogger::getInstance().log(DEBUG, LOGDETAILS("Subscriber connected successfully with MQTT broker"));

	return bFlag;
}

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
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
		bRet = false;
	}
	return bRet;
}

//this should push message in queue
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
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
		bRet = false;
	}
	return bRet;
}

void CMQTTHandler::cleanup()
{
	sem_destroy(&g_semaphoreRespProcess);
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Destroying CMQTTHandler instance ..."));

	conopts.set_automatic_reconnect(0);
	client.disable_callbacks();

	subscriber.disable_callbacks();

	if(client.is_connected())
		client.disconnect();

	if(subscriber.is_connected())
		subscriber.disconnect();

	m_qMsgData = {};
	m_qSubMsgData = {};

	CLogger::getInstance().log(DEBUG, LOGDETAILS("Destroyed CMQTTHandler instance"));
}

CMQTTHandler::~CMQTTHandler()
{
}

/////////////////////////////////////////////
