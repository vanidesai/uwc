/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

/*** InternalMQTTSubsciber.hpp is for internal mqtt */

#ifndef MQTT_HANDLER_HPP_
#define MQTT_HANDLER_HPP_

#include <semaphore.h>
#include "mqtt/async_client.h"
#include "MQTTPubSubClient.hpp"
#include "Logger.hpp"
#include "QueueMgr.hpp"
#include "SparkPlugDevMgr.hpp"

/** enumerator specifying mqtt connection status*/
enum eIntMQTTConStatus
{
	enCON_NONE, enCON_UP, enCON_DOWN
};

/** Handler class for internal mqtt operation*/
class CIntMqttHandler : public CMQTTBaseHandler
{
	int m_appSeqNo; /**app sequence number*/

	sem_t m_semConnSuccess; /** semaphore for connection success*/
	sem_t m_semConnLost; /** semphore for connection lost*/
	sem_t m_semConnSuccessToTimeOut; /** semaphore for connection success to timeout*/

	std::atomic<eIntMQTTConStatus> m_enLastConStatus; /** last connection status*/
	std::atomic<bool> m_bIsInTimeoutState; /** Timeout status*/

	/** constructor*/
	CIntMqttHandler(const std::string &strPlBusUrl, int iQOS);

	/** delete copy and move constructors and assign operators*/
	CIntMqttHandler(const CIntMqttHandler&) = delete;	 			/** Copy construct*/
	CIntMqttHandler& operator=(const CIntMqttHandler&) = delete;	/** Copy assign*/

	int getAppSeqNo();

	void subscribeTopics();
	void connected(const std::string &a_sCause) override;
	void disconnected(const std::string &a_sCause) override;
	void msgRcvd(mqtt::const_message_ptr a_pMsg) override;
	bool init();

	void handleConnMonitoringThread();
	void handleConnSuccessThread();

	/**function to set last connection status*/
	void setLastConStatus(eIntMQTTConStatus a_ConsStatus)
	{
		m_enLastConStatus.store(a_ConsStatus);
	}

	/**Function to get last connection status*/
	eIntMQTTConStatus getLastConStatus()
	{
		return m_enLastConStatus.load();
	}

	/**To set connection timeout status*/
	void setConTimeoutState(bool a_bFlag)
	{
		m_bIsInTimeoutState.store(a_bFlag);
	}

	/** to get connection timeout status*/
	bool getConTimeoutState()
	{
		return m_bIsInTimeoutState.load();
	}

public:
	~CIntMqttHandler();/** destructor*/
	static CIntMqttHandler& instance(); /**function to get single instance of this class*/
	bool prepareCJSONMsg(std::vector<stRefForSparkPlugAction>& a_stRefActionVec);
	bool prepareCMDMsg(std::reference_wrapper<CSparkPlugDev>& a_refSparkPlugDev,
						metricMap_t& a_mapChangedMetrics);
	bool prepareWriteMsg(std::reference_wrapper<CSparkPlugDev>& a_refSparkPlugDev,
							metricMap_t& a_mapChangedMetrics);
};

#endif
