/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "Publisher.hpp"
#include "Common.hpp"
#include "ConfigManager.hpp"

#define EXT_PUBLISHER_ID "EXT_SCADARTU_PUBLISHER"
#define INT_PUBLISHER_ID "INT_SCADARTU_PUBLISHER"

/**
 * Constructor Initializes MQTT m_ExtPublisher
 * @param strPlBusUrl :[in] MQTT broker URL
 * @param strClientID :[in] client ID with which to subscribe (this is topic name)
 * @param iQOS :[in] QOS value with which m_ExtPublisher will publish messages
 * @return None
 */
CPublisher::CPublisher(std::string a_ExtMqttURL, std::string a_IntMqttURL, int a_QOS):
		m_ExtPublisher(a_ExtMqttURL, EXT_PUBLISHER_ID), m_IntPublisher(a_IntMqttURL, INT_PUBLISHER_ID)
{
	try
	{
		m_QOS = a_QOS;
		//connect options for m_ExtPublisher
		m_connOpts.set_keep_alive_interval(20);
		m_connOpts.set_clean_session(true);
		m_connOpts.set_automatic_reconnect(1, 10);
		m_ExtPublisher.set_callback(m_publisherCB);

		if(connect(m_ExtPublisher, m_connOpts))
		{
			cout << "External MQTT publisher connected" << endl;
		}
		else
		{
			cout << "External MQTT publisher failed to connect" << endl;
		}

		//connect options for m_ExtPublisher
		m_SSLConnOpts.set_keep_alive_interval(20);
		m_SSLConnOpts.set_clean_session(true);
		m_SSLConnOpts.set_automatic_reconnect(1, 10);

		// set the certificates if dev mode is false
		if(false == CCommon::getInstance().isDevMode())
		{
			mqtt::ssl_options sslopts;
			sslopts.set_trust_store("/run/secrets/ca_broker");
			sslopts.set_key_store("/run/secrets/client_cert");
			sslopts.set_private_key("/run/secrets/client_key");
			sslopts.set_enable_server_cert_auth(true);
			m_SSLConnOpts.set_ssl(sslopts);
		}
		m_IntPublisher.set_callback(m_publisherCB);

		if(connect(m_IntPublisher, m_SSLConnOpts))
		{
			cout << "Internal MQTT publisher connected" << endl;
		}
		else
		{
			cout << "Internal MQTT publisher failed to connect" << endl;
		}


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
CPublisher& CPublisher::instance()
{
	static string strExtMQTTUrl = CCommon::getInstance().getExtMqttURL();
	static string strIntMQTTUrl = CCommon::getInstance().getIntMqttURL();
	static int nQos = CCommon::getInstance().getMQTTQos();

	if(strExtMQTTUrl.empty() || strIntMQTTUrl.empty())
	{
		DO_LOG_ERROR("INTERNAL_MQTT_URL/ EXTERNAL_MQTT_URL Environment variable is not set");
		std::cout << __func__ << ":" << __LINE__ << " Error : INTERNAL_MQTT_URL/ EXTERNAL_MQTT_URL Environment variable is not set" <<  std::endl;
		//exit(EXIT_FAILURE);
		throw std::runtime_error("Missing required config..");
	}

	DO_LOG_DEBUG("Publisher is connecting with QOS : " + to_string(nQos));
	static CPublisher handler(strExtMQTTUrl, strIntMQTTUrl, nQos);
	return handler;
}

/**
 * Checks if publisher has been connected with the  MQTT broker
 * @param none
 * @return true/false as per the connection status of the publisher
 */
bool CPublisher::isPublisherConnected()
{
	return (m_ExtPublisher.is_connected() && m_IntPublisher.is_connected());
}

/**
 * MQTT m_ExtPublisher connects with MQTT broker
 * @param None
 * @return true/false based on success/failure
 */
bool CPublisher::connect(mqtt::async_client& a_mqttClient, mqtt::connect_options& a_connOpts)
{

	bool bFlag = true;
	try
	{
		m_conntok = a_mqttClient.connect(a_connOpts);
		// Wait for 2 seconds to get connected
		if (false == m_conntok->wait_for(2000))
		{
			bFlag = false;
		}
		else
		{
		    bFlag = true;
		}
	}
	catch (const std::exception &e)
	{
		DO_LOG_FATAL(e.what());
		bFlag = false;
	}
	return bFlag;
}

/**
 * Publish message on MQTT broker for MQTT-Export
 * @param a_sMsg :[in] message to publish
 * @param a_sTopic :[in] topic on which to publish message
 * @return true/false based on success/failure
 */
bool CPublisher::publishIntMqttMsg(std::string &a_sMsg, std::string &a_sTopic)
{
	try
	{
		// Check if topic is blank
		if (true == a_sTopic.empty())
		{
			DO_LOG_ERROR("Blank topic. Message not posted");
			return false;
		}


		mqtt::message_ptr pubmsg = mqtt::make_message(a_sTopic, a_sMsg, m_QOS, false);

		m_IntPublisher.publish(pubmsg, nullptr, m_listener);

		DO_LOG_DEBUG("Published message on Internal MQTT broker successfully with QOS:"+ std::to_string(m_QOS));

		a_sMsg.clear();
		return true;
	}
	catch (const mqtt::exception &exc)
	{
		DO_LOG_FATAL(exc.what());
		//cout << "Exception : " << exc.what() << endl;
	}
	return false;
}

/**
 *
 * Publish message on MQTT broker for MQTT-Export
 * @param a_ddata_payload :[in] spark plug message to publish
 * @param a_topic :[in] topic on which to publish message
 * @return true/false based on success/failure
 */
bool CPublisher::publishSparkplugMsg(org_eclipse_tahu_protobuf_Payload& a_payload, string a_topic)
{
	try
	{
		// Encode the payload into a binary format so it can be published in the MQTT message.
		size_t buffer_length = 0;

		bool encode_passed = pb_get_encoded_size(&buffer_length, org_eclipse_tahu_protobuf_Payload_fields, &a_payload);
		if(encode_passed == false)
		{
			DO_LOG_ERROR("Failed to calculate the payload length");
			return false;
		}

		uint8_t *binary_buffer = (uint8_t *)malloc(buffer_length * sizeof(uint8_t));
		if(binary_buffer == NULL)
		{
			DO_LOG_ERROR("Failed to allocate new memory");
			return false;
		}
		size_t message_length = encode_payload(binary_buffer, buffer_length, &a_payload);
		if(message_length == 0)
		{
			DO_LOG_ERROR("Failed to encode payload");

			if(binary_buffer != NULL)
			{
				free(binary_buffer);
			}
			return false;
		}

		// Publish the DDATA on the appropriate topic
		mqtt::message_ptr pubmsg = mqtt::make_message(a_topic, (void*)binary_buffer, message_length, m_QOS, false);

		m_ExtPublisher.publish(pubmsg, nullptr, m_listener);

		// Free the memory
		if(binary_buffer != NULL)
		{
			free(binary_buffer);
		}
		return true;
	}
	catch(exception& ex)
	{
		DO_LOG_FATAL(ex.what());
		return false;
	}
}

/**
 * Clean up, destroy semaphores, disables callback, disconnect from MQTT broker
 * @param None
 * @return None
 */
void CPublisher::cleanup()
{
	DO_LOG_DEBUG("Destroying CPublisher instance ...");

	m_ExtPublisher.disable_callbacks();

	if(m_ExtPublisher.is_connected())
	{
		m_ExtPublisher.disconnect();
	}

	m_IntPublisher.disable_callbacks();

	if(m_IntPublisher.is_connected())
	{
		m_IntPublisher.disconnect();
	}
	DO_LOG_DEBUG("Destroyed CPublisher instance");
}

/**
 * Destructor
 */
CPublisher::~CPublisher()
{
}
