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
#include "MQTTPubSubClient.hpp"
#include "Logger.hpp"
#include "QueueMgr.hpp"
#include "SparkPlugDevMgr.hpp"

using namespace std;

enum eIntMQTTConStatus
{
	enCON_NONE, enCON_UP, enCON_DOWN
};

class CIntMqttHandler : public CMQTTBaseHandler
{
	int m_appSeqNo;

	sem_t m_semConnSuccess;
	sem_t m_semConnLost;
	sem_t m_semConnSuccessToTimeOut;

	std::atomic<eIntMQTTConStatus> m_enLastConStatus;
	std::atomic<bool> m_bIsInTimeoutState;

	CIntMqttHandler(const std::string &strPlBusUrl, int iQOS);

	// delete copy and move constructors and assign operators
	CIntMqttHandler(const CIntMqttHandler&) = delete;	 			// Copy construct
	CIntMqttHandler& operator=(const CIntMqttHandler&) = delete;	// Copy assign

	int getAppSeqNo();

	void subscribeTopics();
	void connected(const std::string &a_sCause) override;
	void disconnected(const std::string &a_sCause) override;
	void msgRcvd(mqtt::const_message_ptr a_pMsg) override;
	bool init();

	void handleConnMonitoringThread();
	void handleConnSuccessThread();

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
	bool prepareCJSONMsg(std::vector<stRefForSparkPlugAction>& a_stRefActionVec);
	bool prepareCMDMsg(std::reference_wrapper<CSparkPlugDev>& a_refSparkPlugDev,
						metricMap_t& a_mapChangedMetrics);
	bool prepareWriteMsg(std::reference_wrapper<CSparkPlugDev>& a_refSparkPlugDev,
							metricMap_t& a_mapChangedMetrics);
};

#endif
