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

#define PUBLISHER_ID "SCADARTU_PUBLISHER"

int iPublishQOS = 0;

/**
 * Constructor Initializes MQTT m_publisher
 * @param strPlBusUrl :[in] MQTT broker URL
 * @param strClientID :[in] client ID with which to subscribe (this is topic name)
 * @param iQOS :[in] QOS value with which m_publisher will publish messages
 * @return None
 */
CPublisher::CPublisher(std::string strPlBusUrl, int iQOS):
		m_publisher(strPlBusUrl, PUBLISHER_ID)
{
	try
	{
		m_QOS = iQOS;
		//connect options for m_publisher
		m_connOpts.set_keep_alive_interval(20);
		m_connOpts.set_clean_session(true);
		m_connOpts.set_automatic_reconnect(1, 10);

		m_publisher.set_callback(m_publisherCB);

		if(connect())
		{
			m_bIsFirst = false;
		}
		else
		{
			m_bIsFirst = true;
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
	static string strPlBusUrl = CCommon::getInstance().getStrMqttURL();

	if(strPlBusUrl.empty())
	{
		DO_LOG_ERROR("MQTT_URL Environment variable is not set");
		std::cout << __func__ << ":" << __LINE__ << " Error : MQTT_URL Environment variable is not set" <<  std::endl;
		exit(EXIT_FAILURE);
	}

	static CPublisher handler(strPlBusUrl.c_str(), iPublishQOS);
	return handler;
}

/**
 * Checks if publisher has been connected with the  MQTT broker
 * @param none
 * @return true/false as per the connection status of the publisher
 */
bool CPublisher::isPublisherConnected()
{
	return m_publisher.is_connected();
}

/**
 * MQTT m_publisher connects with MQTT broker
 * @param None
 * @return true/false based on success/failure
 */
bool CPublisher::connect()
{

	bool bFlag = true;
	try
	{
		m_conntok = m_publisher.connect(m_connOpts);
		// Wait for 2 seconds to get connected
		if (false == m_conntok->wait_for(2000))
		{
		    std::cout << __func__ << ":" << __LINE__ << " MQTT publisher failed to connect with MQTT broker" << std::endl;
		    DO_LOG_DEBUG("MQTT publisher failed to connect with MQTT broker");

			bFlag = false;
		}
		else
		{
		    std::cout << __func__ << ":" << __LINE__ << " MQTT publisher connected with MQTT broker" << std::endl;
		    DO_LOG_DEBUG("MQTT publisher connected with MQTT broker");

		    bFlag = true;
		}
	}
	catch (const std::exception &e)
	{
		DO_LOG_FATAL(e.what());
		std::cout << __func__ << ":" << __LINE__ << " Exception : MQTT publisher failed to connect with MQTT broker: " << e.what() << std::endl;

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
bool CPublisher::publishMqttExportMsg(std::string &a_sMsg, std::string &a_sTopic)
{
	try
	{
		if (true == m_bIsFirst)
		{
			connect();
			m_bIsFirst = false;
		}

		// Check if topic is blank
		if (true == a_sTopic.empty())
		{
			DO_LOG_ERROR("Blank topic. Message not posted");
			return false;
		}

		if(false == m_publisher.is_connected())
		{
			if(false == connect())
			{
				DO_LOG_ERROR("MQTT m_publisher is not connected with MQTT broker");

				m_bIsFirst = true;

				return false;
			}
		}

		mqtt::message_ptr pubmsg = mqtt::make_message(a_sTopic, a_sMsg, m_QOS, false);

		m_publisher.publish(pubmsg, nullptr, m_listener);
		DO_LOG_DEBUG("Published message on MQTT broker successfully with QOS:"+ std::to_string(m_QOS));

		a_sMsg.clear();
		return true;
	}
	catch (const mqtt::exception &exc)
	{
		DO_LOG_FATAL(exc.what());
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
bool CPublisher::publishSparkplugMsg(org_eclipse_tahu_protobuf_Payload& a_ddata_payload, string a_topic)
{
	try
	{
		// Encode the payload into a binary format so it can be published in the MQTT message.
		// The binary_buffer must be large enough to hold the contents of the binary payload
		size_t buffer_length =  9216; // 100 data points take around 8513 bytes
		uint8_t *binary_buffer = (uint8_t *)malloc(buffer_length * sizeof(uint8_t));
		if(binary_buffer == NULL)
		{
			DO_LOG_ERROR("Failed to allocate new memory");
			return false;
		}
		size_t message_length = encode_payload(&binary_buffer, buffer_length, &a_ddata_payload);
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
		mqtt::message_ptr pubmsg = mqtt::make_message(a_topic, (void*)binary_buffer, message_length, 0, false);

		m_publisher.publish(pubmsg, nullptr, m_listener);

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

	m_publisher.disable_callbacks();

	if(m_publisher.is_connected())
	{
		m_publisher.disconnect();
	}
	DO_LOG_DEBUG("Destroyed CPublisher instance");
}

/**
 * Destructor
 */
CPublisher::~CPublisher()
{
}
