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
	std::cout << "Error::MQTT Connection lost \n";
	if (!cause.empty())
	{
		std::cout << "Error::MQTT Connection lost cause:  "<< cause << std::endl;
	}
#ifdef PERFTESTING
	std::cout << "CMQTTCallback:connection_lost:" << cause << endl;
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
	std::cout << "Error::MQTT action (connect/message sending) failed \n";
#ifdef PERFTESTING
	CMQTTHandler::m_ui32PublishFailed++;
	std::cout << "CMQTTActionListener::on_failure:" << tok.get_message_id() << endl;
	CMQTTHandler::printCounters();
#endif
}

void CMQTTActionListener::on_success(const mqtt::token& tok)
{
#ifdef PERFTESTING
	CMQTTHandler::m_ui32Published++;
#endif
	//std::cout << "Info::MQTT action (connect/message sending) success \n";
}
