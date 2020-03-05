/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef MQTTHANDLER_HPP_
#define MQTTHANDLER_HPP_

#include <atomic>
#include "mqtt/async_client.h"
#include "mqtt/client.h"
#include "MQTTCallback.hpp"
#include "TopicMapper.hpp"
#include "EISMsgbusHandler.hpp"
#include <eis/msgbus/msgbus.h>
#include <queue>

using namespace std;

// Declarations used for MQTT
#define SUBSCRIBERID								"MQTT_SUBSCRIBER"
#define CLIENTID    							    "MQTT_EXPORT"
#define LWT_PAYLOAD	                                "MQTT Export - Last will and testament."
#define QOS         							    0
#define ON_DEMAND_WRITE_PRIORITY					1 	//Write-On Demand Priority set as highest(1)
#define ON_DEMAND_READ_PRIORITY						2 	//Read-On Demand Priority set as 2

typedef enum MQTT_CONFIG_STATE
{
	//	MQTT_CREATE_CLIENT_STATE,
	MQTT_PUBLISHER_CONNECT_STATE,
	MQTT_PUBLISHER_PUBLISH_STATE
}Mqtt_Config_state_t;

typedef enum MQTT_SUBSCRIBER_CONFIG_STATE
{
	MQTT_SUSCRIBER_CONNECT_STATE,
	MQTT_SUSCRIBER_SUBSCRIBE_STATE
}Mqtt_Sub_Config_state_t;

#ifdef QUEUE_FAILED_PUBLISH_MESSAGES
struct stMsgData
{
	std::string m_sMsg;
	std::string m_sTopic;
	int m_iQOS;

	stMsgData(std::string a_sMsg, std::string a_sTopic, int a_iQOS): m_sMsg(a_sMsg), m_sTopic(a_sTopic), m_iQOS(a_iQOS)
	{}

	stMsgData():m_iQOS(0) {}
};
#endif

struct stSubMsgData
{
	long m_lPriority;
	mqtt::const_message_ptr m_ptMsg;
};

/**
 * Operator overloading function to return response with max priority
 */
struct ComparePriority {
    bool operator()(stSubMsgData const& response1, stSubMsgData const& response2)
    {
        return response1.m_lPriority < response2.m_lPriority;
    }
};

class CMQTTHandler
{
	// Default constructor
	CMQTTHandler(std::string strPlBusUrl) ;

	// delete copy and move constructors and assign operators
	CMQTTHandler(const CMQTTHandler&) = delete;	 			// Copy construct
	CMQTTHandler& operator=(const CMQTTHandler&) = delete;	// Copy assign

	mqtt::client publisher;
	mqtt::async_client subscriber;

	mqtt::connect_options conopts;
	mqtt::connect_options syncConnOpts;
	mqtt::token_ptr conntok;
	mqtt::delivery_token_ptr pubtok;

	CMQTTCallback callback;
	CSyncCallback syncCallback;
	CMQTTActionListener listener;

	std::atomic<Mqtt_Config_state_t> ConfigState;
	std::atomic<Mqtt_Sub_Config_state_t> subConfigState;

	std::mutex mqttMutexLock;
	std::mutex m_mutexSubMsgQ;

#ifdef QUEUE_FAILED_PUBLISH_MESSAGES
	std::mutex m_mutexMsgQ;
	std::queue <stMsgData> m_qMsgData;

#endif
	std::priority_queue <stSubMsgData, vector<stSubMsgData>, ComparePriority> m_qSubMsgData;

 	bool initSem();
 	bool connectSubscriber();
 	bool subscribeToTopics();

	void setMQTTConfigState(Mqtt_Config_state_t tempConfigState);
	void setMQTTSubConfigState(Mqtt_Sub_Config_state_t tempConfigState);

	Mqtt_Config_state_t getMQTTConfigState();
 	Mqtt_Sub_Config_state_t getMQTTSubConfigState();

	friend class CMQTTCallback;
	friend class CSyncCallback;
	friend class CMQTTActionListener;

#ifdef PERFTESTING
	// For testing
	static std::atomic<uint32_t> m_ui32PublishReq;
	static std::atomic<uint32_t> m_ui32PublishReqErr;
	static std::atomic<uint32_t> m_ui32Published;
	static std::atomic<uint32_t> m_ui32PublishFailed;
	static std::atomic<uint32_t> m_ui32ConnectionLost;
	static std::atomic<uint32_t> m_ui32Connection;
	static std::atomic<uint32_t> m_ui32PublishSkipped;
	static std::atomic<uint32_t> m_ui32PublishExcep;
	static std::atomic<uint32_t> m_ui32PublishReqTimeOut;
	static std::atomic<uint32_t> m_ui32Disconnected;
	static std::atomic<uint32_t> m_ui32PublishStrReq;
	static std::atomic<uint32_t> m_ui32PublishStrReqErr;
	static std::atomic<uint32_t> m_ui32PublishStrExcep;
	static std::atomic<uint32_t> m_ui32DelComplete;
	static std::atomic<uint32_t> m_ui32MessageArrived;
	static std::atomic<uint32_t> m_uiQReqTried;
	static std::atomic<uint32_t> m_uiSubscribeQReqTried;
	static std::atomic<uint32_t> m_ui32SubscribeSkipped;
#endif

#ifdef QUEUE_FAILED_PUBLISH_MESSAGES
	void postPendingMsgsThread();
	bool getMsgFromQ(stMsgData &a_msg);
	bool pushMsgInQ(const stMsgData &a_msg);
#endif
	bool publish(std::string &a_sMsg, std::string &a_sTopic, int &a_iQOS, bool a_bFromQ = false);
	bool addTimestampsToMsg(std::string &a_sMsg, struct timespec a_tsMsgRcvd);
	
public:

	~CMQTTHandler();
	static CMQTTHandler& instance(); //function to get single instance of this class
	bool publish(std::string a_sMsg, std::string a_sTopic, int qos, struct timespec a_tsMsgRcvd);
#ifdef QUEUE_FAILED_PUBLISH_MESSAGES
	void postPendingMsgs();
#endif
	bool connect();
 	bool getSubMsgFromQ(mqtt::const_message_ptr &msg);
 	bool pushSubMsgInQ(mqtt::const_message_ptr msg);

 	void cleanup();

#ifdef PERFTESTING
 	void incSubQTried() { m_uiSubscribeQReqTried++; }

 	void incSubQSkipped() { m_ui32SubscribeSkipped++; }

	static void printCounters();
#endif
 };

#endif
