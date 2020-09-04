/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "InternalMQTTSubscriber.hpp"
#include "Common.hpp"
#include "ConfigManager.hpp"

#define SUBSCRIBER_ID "INTERNAL_MQTT_SUBSCRIBER"

int iMqttExpQOS = 0;

/**
 * Constructor Initializes MQTT publisher
 * @param strPlBusUrl :[in] MQTT broker URL
 * @param strClientID :[in] client ID with which to subscribe (this is topic name)
 * @param iQOS :[in] QOS value with which publisher will publish messages
 * @return None
 */
CIntMqttHandler::CIntMqttHandler(std::string strPlBusUrl, int iQOS):
		m_subscriber(strPlBusUrl, SUBSCRIBER_ID)
{
	try
	{
		m_QOS = iQOS;
		//connect options for sync publisher/client
		m_connOpts.set_keep_alive_interval(20);
		m_connOpts.set_clean_session(true);
		m_connOpts.set_automatic_reconnect(1, 10);

		// set the certificates if dev mode is false
		if(false == CCommon::getInstance().isDevMode())
		{
			mqtt::ssl_options sslopts;
			sslopts.set_trust_store("/run/secrets/ca_broker");
			sslopts.set_key_store("/run/secrets/client_cert");
			sslopts.set_private_key("/run/secrets/client_key");
			sslopts.set_enable_server_cert_auth(true);
			m_connOpts.set_ssl(sslopts);
		}

		m_subscriber.set_callback(m_mqttSubscriberCB);

		connect();

		DO_LOG_DEBUG("MQTT initialized successfully");
	}
	catch (const std::exception &e)
	{
		DO_LOG_FATAL(e.what());
	}
}

/**
 * Maintain single instance of this class
 * @param None
 * @return Reference of this instance of this class, if successful;
 * 			Application exits in case of failure
 */
CIntMqttHandler& CIntMqttHandler::instance()
{
	static string strPlBusUrl = CCommon::getInstance().getIntMqttURL();

	if(strPlBusUrl.empty())
	{
		DO_LOG_ERROR("MQTT_URL Environment variable is not set");
		std::cout << __func__ << ":" << __LINE__ << " Error : MQTT_URL Environment variable is not set" <<  std::endl;
		exit(EXIT_FAILURE);
	}

	static CIntMqttHandler handler(strPlBusUrl.c_str(), iMqttExpQOS);
	return handler;
}


/**
 * MQTT publisher connects with MQTT broker
 * @param None
 * @return true/false based on success/failure
 */
bool CIntMqttHandler::connect()
{

	bool bFlag = true;
	try
	{
		m_subscriber.connect(m_connOpts, nullptr, m_listener);

	    std::cout << __func__ << ":" << __LINE__ << " MQTT publisher & m_subscriber connected with MQTT broker" << std::endl;
	    DO_LOG_DEBUG("MQTT publisher & m_subscriber connected with MQTT broker");
	}
	catch (const std::exception &e)
	{
		DO_LOG_FATAL(e.what());
		std::cout << __func__ << ":" << __LINE__ << " Excception : MQTT publisher/ m_subscriber failed to connect with MQTT broker: " << e.what() << std::endl;

		bFlag = false;
	}
	return bFlag;
}

/**
 * Subscribe with MQTT broker for topics for on-demand operations
 * @return true/false based on success/failure
 */
bool CIntMqttHandler::subscribeToTopics()
{
	//get list of topics from topic mapper
	std::vector<std::string> vMqttTopics;

	try
	{
		vMqttTopics.push_back("/+/+/+/update");
		vMqttTopics.push_back("BIRTH/#");
		vMqttTopics.push_back("DATA/#");
		vMqttTopics.push_back("DEATH/#");

		for (auto &topic : vMqttTopics)
		{
			if(! topic.empty())
			{
				DO_LOG_DEBUG("MQTT handler subscribing topic : " + topic);
				std::cout << __func__ << ":" << __LINE__ << "MQTT handler subscribing topic : " << topic << endl;
				m_subscriber.subscribe(topic, m_QOS, nullptr, m_listener);
			}
		}

		std::cout << __func__ << ":" << __LINE__ << "MQTT handler subscribed topics with MQTT broker" << std::endl;
	}
	catch(exception &ex)
	{
		DO_LOG_FATAL(ex.what());
		std::cout << __func__ << ":" << __LINE__ << "CIntMqttHandler Exception : " << ex.what() << std::endl;
		return false;
	}

	DO_LOG_DEBUG("MQTT handler subscribed topics with MQTT broker");

	return true;
}

/**
 * Push message in message queue to send on EIS
 * @param msg :[in] reference of message to push in queue
 * @return true/false based on success/failure
 */
bool CIntMqttHandler::pushMsgInQ(mqtt::const_message_ptr msg)
{
	bool bRet = true;
	try
	{
		cout << "Message received from internal MQTT " << endl;
		QMgr::getDatapointsQ().pushMsg(msg);

		DO_LOG_DEBUG("Pushed MQTT message in queue");
		bRet = true;
	}
	catch (const std::exception &e)
	{
		DO_LOG_FATAL(e.what());
		bRet = false;
	}
	return bRet;
}

/**
 * Clean up, destroy semaphores, disables callback, disconnect from MQTT broker
 * @param None
 * @return None
 */
void CIntMqttHandler::cleanup()
{
	DO_LOG_DEBUG("Destroying CIntMqttHandler instance ...");

	if(m_subscriber.is_connected())
	{
		m_subscriber.disconnect();
	}
	DO_LOG_DEBUG("Destroyed CIntMqttHandler instance");
}

/**
 * Checks if external MQTT subscriber has been connected with the  MQTT broker
 * @param none
 * @return true/false as per the connection status of the external MQTT subscriber
 */
bool CIntMqttHandler::isIntMqttSubConnected()
{
	return m_subscriber.is_connected();
}

/**
 * Destructor
 */
CIntMqttHandler::~CIntMqttHandler()
{
}
