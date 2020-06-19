/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/
#include "MQTTCallback.hpp"

#include "SCADAHandler.hpp"
#include "MQTTHandler.hpp"
/**
 * This is a callback function which gets called when subscriber is disconnected from MQTT broker
 * @param cause :[in] cause of connection lost
 * @return None
 */
void CSubscriberCallback::connection_lost(const std::string& cause)
{
	DO_LOG_ERROR("MQTT Connection lost");

	if (!cause.empty())
	{
		DO_LOG_ERROR("MQTT Connection lost cause:  " + cause);
	}
}

/**
 * This is a callback function which gets called when subscriber is connected with MQTT broker
 * @param cause :[in]
 * @return None
 */
void CSubscriberCallback::connected(const std::string& cause)
{
	CMQTTHandler::instance().subscribeToTopics();
}

/**
 * This is a callback function which gets called when a message is arrived on subscriber
 * @param msg :[in] pointer to message arriving at subscriber
 * @return None
 */
void CSubscriberCallback::message_arrived(mqtt::const_message_ptr msg)
{
	CMQTTHandler::instance().pushMsgInQ(msg);
}

/**
 * This is a callback function which gets called when subscriber fails to send/receive
 * @param tok :[in] failed message token
 * @return None
 */
void CMQTTActionListener::on_failure(const mqtt::token& tok)
{
	DO_LOG_ERROR("MQTT action (connect/message sending) failed");
}

/**
 * This is a callback function which gets called when subscriber succeeds in send/receive
 * @param tok :[in] message token
 * @return None
 */
void CMQTTActionListener::on_success(const mqtt::token& tok)
{
}

/**
 * This is a callback function which gets called when publisher is disconnected from MQTT broker
 * @param cause :[in] cause of connection lost
 * @return None
 */
void CPublisherCallback::connection_lost(const std::string& cause)
{
	DO_LOG_ERROR("MQTT publisher lost MQTT Connection");

	if (!cause.empty())
	{
		DO_LOG_ERROR("MQTT publisher lost MQTT Connection : cause:  " + cause);
	}
}

/**
 * This is a callback function which gets called when publisher is connected with MQTT broker
 * @param cause :[in]
 * @return None
 */
void CPublisherCallback::connected(const std::string& cause)
{
}

/**
 * This is a callback function which gets called when publisher publishes message successfully
 * @param tok :[in] message token
 * @return None
 */
void CPublisherCallback::delivery_complete(mqtt::delivery_token_ptr tok)
{
}

/**
 * This is a callback function which gets called when subscriber is disconnected from MQTT broker
 * @param cause :[in] cause of connection lost
 * @return None
 */
void CScadaCallback::connection_lost(const std::string& cause)
{
	DO_LOG_ERROR("SCADA Connection lost");

	if (!cause.empty())
	{
		DO_LOG_ERROR("SCADA Connection lost cause:  " + cause);
	}
}

/**
 * This is a callback function which gets called when subscriber is connected with MQTT broker
 * @param cause :[in]
 * @return None
 */
void CScadaCallback::connected(const std::string& cause)
{
	DO_LOG_DEBUG("SCADA-RTU Connected with the MQTT broker");
}

/**
 * This is a callback function which gets called when a message is arrived on subscriber
 * @param msg :[in] pointer to message arriving at subscriber
 * @return None
 */
void CScadaCallback::message_arrived(mqtt::const_message_ptr msg)
{
}
