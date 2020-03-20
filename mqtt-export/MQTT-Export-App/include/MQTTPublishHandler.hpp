/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef MQTT_PUBLISH_HANDLER_HPP_
#define MQTT_PUBLISH_HANDLER_HPP_

#include <atomic>
#include "mqtt/client.h"
#include "MQTTCallback.hpp"
#include "Logger.hpp"
#include <queue>

using namespace std;

// Declarations used for MQTT
//#define CLIENTID    							    "MQTT_EXPORT"
//#define LWT_PAYLOAD	                                "MQTT Export - Last will and testament."
#define QOS         							    0
#define ON_DEMAND_WRITE_PRIORITY					1 	//Write-On Demand Priority set as highest(1)
#define ON_DEMAND_READ_PRIORITY						2 	//Read-On Demand Priority set as 2

typedef enum MQTT_CONFIG_STATE
{
	MQTT_PUBLISHER_CONNECT_STATE,
	MQTT_PUBLISHER_PUBLISH_STATE
}Mqtt_Config_state_t;

class CMQTTPublishHandler
{
	mqtt::client publisher;
	bool m_bIsFirst;
	mqtt::connect_options syncConnOpts;
	mqtt::token_ptr conntok;
	mqtt::delivery_token_ptr pubtok;

	CSyncCallback syncCallback;
	CMQTTActionListener listener;

	std::atomic<Mqtt_Config_state_t> ConfigState;

	std::mutex mqttMutexLock;

	std::queue<mqtt::const_message_ptr> m_qSubMsgData;

	void setMQTTConfigState(Mqtt_Config_state_t tempConfigState);
	Mqtt_Config_state_t getMQTTConfigState();

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
	static std::atomic<uint32_t> m_uiQReqTried;
#endif

	bool publish(std::string &a_sMsg, std::string &a_sTopic, int &a_iQOS, bool a_bFromQ = false);
	bool addTimestampsToMsg(std::string &a_sMsg, struct timespec a_tsMsgRcvd);
	
public:
	CMQTTPublishHandler(std::string strPlBusUrl, std::string strClientID);
	~CMQTTPublishHandler();

	bool publish(std::string a_sMsg, std::string a_sTopic, int qos, struct timespec a_tsMsgRcvd);

	bool connect();

 	void cleanup();

#ifdef PERFTESTING
	static void printCounters();
#endif
 };

#endif
