/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#ifndef MQTTHANDLER_HPP_
#define MQTTHANDLER_HPP_

#include <atomic>
#include "mqtt/async_client.h"
#include "MQTTCallback.hpp"
#include <queue>

using namespace std;
//using namespace web;

// Declarations used for MQTT
#define CLIENTID    							    "MQTT_EXPORT"
#define LWT_PAYLOAD	                                "MQTT Export - Last will and testament."
#define QOS         							    0

typedef enum MQTT_CONFIG_STATE
{
	//	MQTT_CREATE_CLIENT_STATE,
	MQTT_CLIENT_CONNECT_STATE,
	MQTT_PUBLISH_STATE
}Mqtt_Config_state_t;

struct stMsgData
{
	std::string m_sMsg;
	std::string m_sTopic;

	stMsgData(std::string a_sMsg, std::string a_sTopic): m_sMsg(a_sMsg), m_sTopic(a_sTopic)
	{}

	stMsgData() {}
};

class CMQTTHandler
{
	// delete copy and move constructors and assign operators
	CMQTTHandler(const CMQTTHandler&) = delete;	 			// Copy construct
	CMQTTHandler& operator=(const CMQTTHandler&) = delete;	// Copy assign
	mqtt::async_client client;
	mqtt::token_ptr conntok;
	mqtt::connect_options conopts;
	CMQTTCallback callback;
	CMQTTActionListener listener;

	mqtt::delivery_token_ptr pubtok;
	std::atomic<Mqtt_Config_state_t> ConfigState;
	std::mutex mqttMutexLock;

	// Default constructor
	CMQTTHandler(std::string strPlBusUrl) ;
	void setMQTTConfigState(Mqtt_Config_state_t tempConfigState);
	Mqtt_Config_state_t getMQTTConfigState();
	friend class CMQTTCallback;
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
	static std::atomic<uint32_t> m_uiQReqTried;
#endif

	std::queue <stMsgData> m_qMsgData;
	std::mutex m_mutexMsgQ;

	void postPendingMsgsThread();
	bool getMsgFromQ(stMsgData &a_msg);
	bool pushMsgInQ(const stMsgData &a_msg);
public:
	// function to get single instance of this class
	static CMQTTHandler& instance();

#ifdef PERFTESTING
	static void printCounters();
#endif

	void postPendingMsgs();

	~CMQTTHandler()
	{
		;
	}

	bool publish(std::string a_sMsg, const char *topic);
	bool publish(std::string &a_sMsg, std::string &a_sTopic, bool a_bFromQ = false);

	bool connect();
};

#endif
