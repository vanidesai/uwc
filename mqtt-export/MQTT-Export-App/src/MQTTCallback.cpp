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

void CMQTTCallback::connection_lost(const std::string& cause)
{
	CMQTTHandler::instance().setMQTTConfigState(MQTT_CLIENT_CONNECT_STATE);
	CMQTTHandler::instance().setMQTTSubConfigState(MQTT_SUSCRIBE_CONNECT_STATE);
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

void CMQTTCallback::connected(const std::string& cause)
{
	CMQTTHandler::instance().setMQTTConfigState(MQTT_PUBLISH_STATE);
	CMQTTHandler::instance().setMQTTSubConfigState(MQTT_SUBSCRIBE_STATE);

#ifdef PERFTESTING
	CMQTTHandler::m_ui32Connection++;
	CMQTTHandler::printCounters();
#endif


	CMQTTHandler::instance().postPendingMsgs();
	CMQTTHandler::instance().subscribeToTopics();
}

void CMQTTCallback::delivery_complete(mqtt::delivery_token_ptr tok)
{
#ifdef PERFTESTING
	CMQTTHandler::m_ui32DelComplete++;
#endif
}

void CMQTTCallback::message_arrived(mqtt::const_message_ptr msg)
{
	//add this message to queue - call a function
	CMQTTHandler::instance().pushSubMsgInQ(msg);

#ifdef PERFTESTING
	CMQTTHandler::m_ui32MessageArrived++;
#endif
}

void CMQTTActionListener::on_failure(const mqtt::token& tok)
{
	CLogger::getInstance().log(ERROR, LOGDETAILS("MQTT action (connect/message sending) failed"));
#ifdef PERFTESTING
	CMQTTHandler::m_ui32PublishFailed++;
	CLogger::getInstance().log(ERROR, LOGDETAILS("CMQTTActionListener::on_failure:" + std::to_string(tok.get_message_id())));
	CMQTTHandler::printCounters();
#endif
}

void CMQTTActionListener::on_success(const mqtt::token& tok)
{
#ifdef PERFTESTING
	CMQTTHandler::m_ui32Published++;
#endif
}
