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
#include "MQTTHandler.hpp"

/**
 * Callback function gets called when subscriber is disconnected from MQTT broker
 * @param cause :[in] cause of connection lost
 */
void CMQTTCallback::connection_lost(const std::string& cause)
{
	CMQTTHandler::instance().setMQTTSubConfigState(MQTT_SUSCRIBER_CONNECT_STATE);

	CLogger::getInstance().log(ERROR, LOGDETAILS("MQTT Connection lost"));

	if (!cause.empty())
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS("MQTT Connection lost cause:  " + cause));
	}
#ifdef PERFTESTING
	CLogger::getInstance().log(ERROR, LOGDETAILS("CMQTTCallback:connection_lost:" + cause));
	CMQTTHandler::m_ui32ConnectionLost++;
	CMQTTHandler::printCounters();
#endif
}

/**
 * Callback function gets called when subscriber is connected with MQTT broker
 * @param cause :[in]
 */
void CMQTTCallback::connected(const std::string& cause)
{
	CMQTTHandler::instance().setMQTTSubConfigState(MQTT_SUSCRIBER_SUBSCRIBE_STATE);

#ifdef PERFTESTING
	CMQTTHandler::m_ui32Connection++;
	CMQTTHandler::printCounters();
#endif
	CMQTTHandler::instance().subscribeToTopics();
}

/**
 * Callback function gets called when a message is arrived on subscriber
 * @param msg :[in] pointer to message arriving at subscriber
 */
void CMQTTCallback::message_arrived(mqtt::const_message_ptr msg)
{
	//add this message to queue - call a function
	CMQTTHandler::instance().pushSubMsgInQ(msg);

#ifdef PERFTESTING
	CMQTTHandler::m_ui32MessageArrived++;
#endif
}

/**
 * Callback function gets called when subscriber fails to send/receive
 * @param tok :[in] failed message token
 */
void CMQTTActionListener::on_failure(const mqtt::token& tok)
{
	CLogger::getInstance().log(ERROR, LOGDETAILS("MQTT action (connect/message sending) failed"));
#ifdef PERFTESTING
	CMQTTHandler::m_ui32PublishFailed++;
	CLogger::getInstance().log(ERROR, LOGDETAILS("CMQTTActionListener::on_failure:" + std::to_string(tok.get_message_id())));
	CMQTTHandler::printCounters();
#endif
}

/**
 * Callback function gets called when subscriber succeeds in send/receive
 * @param tok :[in] message token
 */
void CMQTTActionListener::on_success(const mqtt::token& tok)
{
#ifdef PERFTESTING
	CMQTTHandler::m_ui32Published++;
#endif
}

/**
 * Callback function gets called when publisher is disconnected from MQTT broker
 * @param cause :[in] cause of connection lost
 */
void CSyncCallback::connection_lost(const std::string& cause)
{
	CMQTTHandler::instance().setMQTTConfigState(MQTT_PUBLISHER_CONNECT_STATE);

	CLogger::getInstance().log(ERROR, LOGDETAILS("MQTT publisher lost MQTT Connection"));

	if (!cause.empty())
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS("MQTT publisher lost MQTT Connection : cause:  " + cause));
	}
#ifdef PERFTESTING
	CMQTTHandler::m_ui32ConnectionLost++;
	CMQTTHandler::printCounters();
#endif
}

/**
 * Callback function gets called when publisher is connected with MQTT broker
 * @param cause :[in]
 */
void CSyncCallback::connected(const std::string& cause)
{
	CMQTTHandler::instance().setMQTTConfigState(MQTT_PUBLISHER_PUBLISH_STATE);

#ifdef PERFTESTING
	CMQTTHandler::m_ui32Connection++;
	CMQTTHandler::printCounters();
#endif

#ifdef QUEUE_FAILED_PUBLISH_MESSAGES
	CMQTTHandler::instance().postPendingMsgs();
#endif
}

/**
 * Callback function gets called when publisher publishes message successfully
 * @param tok :[in] message token
 */
void CSyncCallback::delivery_complete(mqtt::delivery_token_ptr tok)
{
#ifdef PERFTESTING
	CMQTTHandler::m_ui32DelComplete++;
#endif
}
