/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "MQTTPublishHandler.hpp"
#include "cjson/cJSON.h"
//#include <semaphore.h>
#include "Common.hpp"
#include "ConfigManager.hpp"

/**
 * Constructor Initializes MQTT publisher
 * @param strPlBusUrl :[in] MQTT broker URL
 * @param strClientID :[in] client ID with which to subscribe (this is topic name)
 * @param iQOS :[in] QOS value with which publisher will publish messages
 * @return None
 */
CMQTTPublishHandler::CMQTTPublishHandler(std::string strPlBusUrl, std::string strClientID, int iQOS):
		publisher(strPlBusUrl, strClientID), ConfigState(MQTT_PUBLISHER_CONNECT_STATE)
{
	try
	{
		m_QOS = iQOS;
		//connect options for sync publisher/client
		syncConnOpts.set_keep_alive_interval(20);
		syncConnOpts.set_clean_session(true);
		syncConnOpts.set_automatic_reconnect(1, 10);

		// set the certificates if dev mode is false
		if(false == CCommon::getInstance().isDevMode())
		{
			mqtt::ssl_options sslopts;
			sslopts.set_trust_store("/run/secrets/ca_broker");
			sslopts.set_key_store("/run/secrets/client_cert");
			sslopts.set_private_key("/run/secrets/client_key");
			sslopts.set_enable_server_cert_auth(true);
			syncConnOpts.set_ssl(sslopts);
		}

		publisher.set_callback(syncCallback);

		if(connect())
		{
			m_bIsFirst = false;
		}
		else
		{
			m_bIsFirst = true;
		}
	}
	catch (const std::exception &e)
	{
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
	}
}

/**
 * MQTT publisher connects with MQTT broker
 * @param None
 * @return true/false based on success/failure
 */
bool CMQTTPublishHandler::connect()
{

	bool bFlag = true;
	try
	{
		publisher.connect(syncConnOpts);
	    std::cout << __func__ << ":" << __LINE__ << " MQTT publisher connected with MQTT broker" << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cout << __func__ << ":" << __LINE__ << " MQTT publisher failed to connect with MQTT broker: exception : " << e.what() << std::endl;

		bFlag = false;
	}
	return bFlag;
}

/**
 * Get publisher current state
 * @param None
 * @return publisher state
 */
Mqtt_Config_state_t CMQTTPublishHandler::getMQTTConfigState()
{
	return ConfigState;
}

/**
 * Set publisher state to given
 * @param tempConfigState :[in] publisher state to set
 * @return None
 */
void CMQTTPublishHandler::setMQTTConfigState(Mqtt_Config_state_t tempConfigState)
{
	ConfigState = tempConfigState;
}

/**
 * Publish message on MQTT broker
 * @param a_sMsg :[in] message to publish
 * @param a_sTopic :[in] topic on which to publish message
 * @param a_tsMsgRcvd :[in] time stamp to add while publishing
 * @return true/false based on success/failure
 */
bool CMQTTPublishHandler::publish(std::string &a_sMsg, std::string &a_sTopic)
{
	try
	{
		if (true == m_bIsFirst)
		{
			connect();
			m_bIsFirst = false;
		}

		std::lock_guard<std::mutex> lock(mqttMutexLock);

		//publish data to MQTT
#ifdef INSTRUMENTATION_LOG
		DO_LOG_DEBUG("ZMQ Message: Time: "
				+ std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
		+ ", Msg: " + msgWithTimeStamp);
#endif


		// Check if topic is blank
		if (true == a_sTopic.empty())
		{
			if (true == a_sMsg.empty())
			{
				cout << __func__ << ": " << "Blank topic and blank Message" << endl;
			}
			else
			{
				cout << __func__ << ": " << "Blank topic. Message not posted" << endl;
			}
			return false;
		}

		if(false == publisher.is_connected())
		{
			if(false == connect())
			{
				cout << "MQTT publisher is not connected with MQTT broker" << endl;

				m_bIsFirst = true;

				return false;
			}
		}

		mqtt::message_ptr pubmsg = mqtt::make_message(a_sTopic, a_sMsg, m_QOS, false);

		DO_LOG_INFOCL("Message:: " + a_sMsg);
		publisher.publish(pubmsg, nullptr, listener);

		return true;
	}
	catch (const mqtt::exception &exc)
	{
		cout << __func__ << ": " << exc.what() << endl;
	}
	return false;
}


/**
 * Clean up, destroy semaphores, disables callback, disconnect from MQTT broker
 * @param None
 * @return None
 */
void CMQTTPublishHandler::cleanup()
{
	if(publisher.is_connected())
	{
		publisher.disconnect();
	}
}

/**
 * Destructor
 */
CMQTTPublishHandler::~CMQTTPublishHandler()
{
}
