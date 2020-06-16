/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "SCADAHandler.hpp"
#include <iterator>
#include <vector>
#include "cjson/cJSON.h"

int iScadaQOS = 0;

/**
 * constructor Initializes MQTT m_subscriber
 * @param strPlBusUrl :[in] MQTT broker URL
 * @param iQOS :[in] QOS with which to get messages
 * @return None
 */
CSCADAHandler::CSCADAHandler(std::string strMqttURL, int iQOS) :
		m_subscriber(strMqttURL, SUBSCRIBERID)
{
	try
	{
		m_QOS = iQOS;

		prepareNodeDeathMsg();

		//connect options for async m_subscriber
		m_subscriberConopts.set_keep_alive_interval(20);
		m_subscriberConopts.set_clean_session(true);
		m_subscriberConopts.set_automatic_reconnect(1, 10);

		m_subscriber.set_callback(m_scadaSubscriberCB);

		connectSubscriber();

        // Publish the NBIRTH and DBIRTH Sparkplug messages (Birth Certificates)
        publish_births();

		DO_LOG_DEBUG("MQTT initialized successfully");
	}
	catch (const std::exception &e)
	{
		DO_LOG_FATAL(e.what());
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
	}
}

/**
 * Prepare a will message for subscriber. That will
 * act as a death message when this node goes down.
 * @param none
 * @return none
 */
void CSCADAHandler::prepareNodeDeathMsg()
{
	// Create the NDEATH payload
	org_eclipse_tahu_protobuf_Payload ndeath_payload;
	get_next_payload(&ndeath_payload);

	uint64_t badSeq_value = 0;
	add_simple_metric(&ndeath_payload, "bdSeq", true, 10,
			METRIC_DATA_TYPE_UINT64, false, false, false, &badSeq_value, sizeof(badSeq_value));

	size_t buffer_length = 1024;
	uint8_t *binary_buffer = (uint8_t *)malloc(buffer_length * sizeof(uint8_t));
	size_t message_length = encode_payload(&binary_buffer, buffer_length, &ndeath_payload);

	// Publish the DDATA on the appropriate topic
	mqtt::message_ptr pubmsg = mqtt::make_message(CCommon::getInstance().getDeathTopic(), (void*)binary_buffer, message_length, 0, false);

	//connect options for async m_subscriber
	m_subscriberConopts.set_will_message(pubmsg);

	free(binary_buffer);
	free_payload(&ndeath_payload);
}

/**
 * Publish node birth messages to SCADA
 * @param none
 * @return none
 */
void CSCADAHandler::publish_births()
{
	std::cout << "Publishing birth messages ..." << endl;
	// Initialize the sequence number for Sparkplug MQTT messages
	// This must be zero on every NBIRTH publish
	seq = 0;

	std::cout << "SCADA publisher is connected while sending birth messages\n";

	// Publish the NBIRTH
	publish_node_birth();
}

/**
 * Maintain single instance of this class
 * @param None
 * @return Reference of this instance of this class, if successful;
 * 			Application exits in case of failure
 */
CSCADAHandler& CSCADAHandler::instance()
{
	static string strMqttUrl = CCommon::getInstance().getStrMqttURL();

	if(strMqttUrl.empty())
	{
		DO_LOG_ERROR("MQTT_URL Environment variable is not set");
		std::cout << __func__ << ":" << __LINE__ << " Error : MQTT_URL Environment variable is not set" <<  std::endl;
		exit(EXIT_FAILURE);
	}

	static CSCADAHandler handler(strMqttUrl.c_str(), iScadaQOS);
	return handler;
}

/**
 * Subscribe with MQTT broker for topics for on-demand operations
 * @return true/false based on success/failure
 */
bool CSCADAHandler::subscribeToTopics()
{
	std::vector<std::string> vMqttTopics;

	try
	{
		//test topic
		vMqttTopics.push_back(CCommon::getInstance().getScadaTopicToSubscribe());

		for (auto topic : vMqttTopics)
		{
			m_subscriber.subscribe(topic , 0, nullptr, m_listener);
		}

		std::cout << __func__ << ":" << __LINE__ << "SCADAHandler subscribed topics with MQTT broker" << std::endl;
	}
	catch(exception &ex)
	{
		DO_LOG_FATAL(ex.what());
		std::cout << __func__ << ":" << __LINE__ << "SCADAHandler Exception : " << ex.what() << std::endl;
		return false;
	}

	DO_LOG_DEBUG("SCADAHandler subscribed topics with MQTT broker");

	return true;
}

/**
 * Connect m_subscriber with MQTT broker
 * @return true/false based on success/failure
 */
bool CSCADAHandler::connectSubscriber()
{
	bool bFlag = true;
	try
	{
		m_conntok = m_subscriber.connect(m_subscriberConopts, nullptr, m_listener);
		// Wait for 2 seconds to get connected
		if (false == m_conntok->wait_for(2000))
		 {
			 std::cout << "Error::Failed to connect m_subscriber to the platform bus\n";
			 bFlag = false;
		 }

		std::cout << __func__ << ":" << __LINE__ << " SCADA Subscriber connected successfully with MQTT broker" << std::endl;
	}
	catch (const std::exception &e)
	{
		DO_LOG_FATAL(e.what());
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
		bFlag = false;
	}

	DO_LOG_DEBUG("SCADA m_subscriber connected successfully with MQTT broker");

	return bFlag;
}

/**
 * Push message in message queue to send on EIS
 * @param msg :[in] reference of message to push in queue
 * @return true/false based on success/failure
 */
bool CSCADAHandler::pushMsgInQ(mqtt::const_message_ptr msg)
{
	bool bRet = true;
	try
	{
		QMgr::stMqttMsg scadaMsgRecvd;
		scadaMsgRecvd.m_mqttMsg = msg;

		QMgr::getScada().pushMsg(scadaMsgRecvd);

		DO_LOG_DEBUG("Pushed SCADA message in queue");
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
void CSCADAHandler::cleanup()
{
	DO_LOG_DEBUG("Destroying CSCADAHandler instance ...");

	m_subscriberConopts.set_automatic_reconnect(0);

	m_subscriber.disable_callbacks();

	if(m_subscriber.is_connected())
		m_subscriber.disconnect();

	DO_LOG_DEBUG("Destroyed CSCADAHandler instance");
}

/**
 * Destructor
 */
CSCADAHandler::~CSCADAHandler()
{
}

/**
 * Publish node birth message on SCADA
 * @param none
 * @return none
 */
void CSCADAHandler::publish_node_birth()
{
	// Create the NBIRTH payload
	org_eclipse_tahu_protobuf_Payload nbirth_payload;
	get_next_payload(&nbirth_payload);

	nbirth_payload.uuid = (char*)malloc((strlen("MyUUID")+1) * sizeof(char));
	strcpy(nbirth_payload.uuid, CCommon::getInstance().getStrAppName().c_str());

	// Add getNBirthTopicsome device metrics

	string scadaRTUName = CCommon::getInstance().getStrAppName();
	add_simple_metric(&nbirth_payload, "Name", true, 10, METRIC_DATA_TYPE_STRING, false, false, false, scadaRTUName.c_str(), scadaRTUName.length());

	std::cout << "Publishing nbirth message ..." << endl;
	CPublisher::instance().publishSparkplugMsg(nbirth_payload, CCommon::getInstance().getNBirthTopic());

	// Free the memory
	free(nbirth_payload.uuid);
	free_payload(&nbirth_payload);
}
