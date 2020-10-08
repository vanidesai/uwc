/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef MQTT_HANDLER_HPP_
#define MQTT_HANDLER_HPP_

#include <semaphore.h>
#include "mqtt/async_client.h"
#include "MQTTCallback.hpp"
#include "Logger.hpp"
#include "QueueMgr.hpp"
#include "SparkPlugDevMgr.hpp"

using namespace std;

enum eIntMQTTConStatus
{
	enCON_NONE, enCON_UP, enCON_DOWN
};

class CIntMqttHandler
{
	CMQTTPubSubClient m_MQTTClient;
	//mqtt::async_client m_subscriber;
	//mqtt::connect_options m_connOpts;
	//mqtt::token_ptr m_conntok;
	int m_QOS;

	int m_appSeqNo;

	sem_t m_semConnSuccess;
	sem_t m_semConnLost;
	sem_t m_semConnSuccessToTimeOut;

	std::atomic<eIntMQTTConStatus> m_enLastConStatus;
	std::atomic<bool> m_bIsInTimeoutState;

	//CSubscriberCallback m_mqttSubscriberCB;
	//CMQTTActionListener m_listener;

	//friend class CSubscriberCallback;
	//friend class CMQTTActionListener;

	CIntMqttHandler(const std::string &strPlBusUrl, int iQOS);

	// delete copy and move constructors and assign operators
	CIntMqttHandler(const CIntMqttHandler&) = delete;	 			// Copy construct
	CIntMqttHandler& operator=(const CIntMqttHandler&) = delete;	// Copy assign

	bool subscribeToTopics();
	string getDatatypeInString(uint32_t a_uiDatatype);

	int getAppSeqNo();

	void subscribeTopics();
	static void connected(const std::string &a_sCause);
	static void disconnected(const std::string &a_sCause);
	static void msgRcvd(mqtt::const_message_ptr a_pMsg);
	bool init();

	void handleConnMonitoringThread();
	void handleConnSuccessThread();

	void signalIntMQTTConnLostThread();
	void signalIntMQTTConnDoneThread();

	void setLastConStatus(eIntMQTTConStatus a_ConsStatus)
	{
		m_enLastConStatus.store(a_ConsStatus);
	}

	eIntMQTTConStatus getLastConStatus()
	{
		return m_enLastConStatus.load();
	}

	void setConTimeoutState(bool a_bFlag)
	{
		m_bIsInTimeoutState.store(a_bFlag);
	}

	bool getConTimeoutState()
	{
		return m_bIsInTimeoutState.load();
	}

public:
	~CIntMqttHandler();
	static CIntMqttHandler& instance(); //function to get single instance of this class
	bool isConnected();
	bool pushMsgInQ(mqtt::const_message_ptr msg);
	bool prepareCJSONMsg(std::vector<stRefForSparkPlugAction>& a_stRefActionVec);
	void connect();
	void disconnect();
 	void cleanup();

	bool publishIntMqttMsg(const std::string &a_sMsg, const std::string &a_sTopic);
 };

#endif
