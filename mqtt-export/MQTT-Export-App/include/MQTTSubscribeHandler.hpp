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
#include <mqtt/async_client.h>
#include <mqtt/client.h>
#include "MQTTCallback.hpp"
#include "Common.hpp"
#include "ZmqHandler.hpp"
#include <eis/msgbus/msgbus.h>
#include "QueueMgr.hpp"

using namespace std;

// Declarations used for MQTT
#define SUBSCRIBERID								"MQTT_SUBSCRIBER"
#define CLIENTID    							    "MQTT_EXPORT"
#define LWT_PAYLOAD	                                "MQTT Export - Last will and testament."
#define QOS         							    0
#define ON_DEMAND_WRITE_PRIORITY					1 	//Write-On Demand Priority set as highest(1)
#define ON_DEMAND_READ_PRIORITY						2 	//Read-On Demand Priority set as 2

typedef enum MQTT_SUBSCRIBER_CONFIG_STATE
{
	MQTT_SUSCRIBER_CONNECT_STATE,
	MQTT_SUSCRIBER_SUBSCRIBE_STATE
}Mqtt_Sub_Config_state_t;

class CMQTTHandler
{
	// Default constructor
	CMQTTHandler(std::string strPlBusUrl) ;

	// delete copy and move constructors and assign operators
	CMQTTHandler(const CMQTTHandler&) = delete;	 			// Copy construct
	CMQTTHandler& operator=(const CMQTTHandler&) = delete;	// Copy assign

	mqtt::async_client subscriber;

	mqtt::connect_options conopts;
	mqtt::token_ptr conntok;
	mqtt::delivery_token_ptr pubtok;

	CMQTTCallback callback;
	CMQTTActionListener listener;

	std::atomic<Mqtt_Sub_Config_state_t> subConfigState;

 	bool connectSubscriber();
 	bool subscribeToTopics();

	void setMQTTSubConfigState(Mqtt_Sub_Config_state_t tempConfigState);
 	Mqtt_Sub_Config_state_t getMQTTSubConfigState();

	friend class CMQTTCallback;
	friend class CMQTTActionListener;

	/**
	* fill the default realtime from global config
	* @param isWrite	:[in] opearation type
	* @return realtime value
	*/
	bool fillDefaultRealtime(const bool isWrite);

#ifdef PERFTESTING
	// For testing
	static std::atomic<uint32_t> m_ui32ConnectionLost;
	static std::atomic<uint32_t> m_ui32Connection;
	static std::atomic<uint32_t> m_ui32Disconnected;
	static std::atomic<uint32_t> m_ui32MessageArrived;
	static std::atomic<uint32_t> m_uiQReqTried;
	static std::atomic<uint32_t> m_uiSubscribeQReqTried;
	static std::atomic<uint32_t> m_ui32SubscribeSkipped;
#endif
	
public:

	~CMQTTHandler();
	static CMQTTHandler& instance(); //function to get single instance of this class

	bool getOperation(string &topic, bool &isWrite);
 	bool parseMQTTMsg(const char *json, bool &isRealtime, const bool isWrite);

	bool pushSubMsgInQ(mqtt::const_message_ptr msg);
 	void cleanup();

#ifdef PERFTESTING
 	void incSubQTried() { m_uiSubscribeQReqTried++; }
 	void incSubQSkipped() { m_ui32SubscribeSkipped++; }
	static void printCounters();
#endif
 };

#endif
