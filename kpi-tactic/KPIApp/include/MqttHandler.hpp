/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

/*** MqttHandler.hpp is handler for MQTT operations*/

#ifndef MQTT_PUBLISH_HANDLER_HPP_
#define MQTT_PUBLISH_HANDLER_HPP_

#include <semaphore.h>
#include "MQTTPubSubClient.hpp"
#include "QueueMgr.hpp"

/** Handler class for mqtt*/
class CMqttHandler : public CMQTTBaseHandler
{
	sem_t m_semConnSuccess; /** semaphore for connection success*/

	CMqttHandler(const std::string &strPlBusUrl, int iQOS);//constructor

	/** delete copy and move constructors and assign operators*/
	CMqttHandler(const CMqttHandler&) = delete;	 			/** Copy construct*/
	CMqttHandler& operator=(const CMqttHandler&) = delete;	/** Copy assign*/

	void subscribeTopics();
	void connected(const std::string &a_sCause) override;
	void msgRcvd(mqtt::const_message_ptr a_pMsg) override;
	bool init();

	void handleConnSuccessThread();

	bool pushMsgInQ(mqtt::const_message_ptr& a_msgMQTT);

public:
	~CMqttHandler();/** destructor*/
	static CMqttHandler& instance(); /** function to get single instance of this class*/
};

#endif
