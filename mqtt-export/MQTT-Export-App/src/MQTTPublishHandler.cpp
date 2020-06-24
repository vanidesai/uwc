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

		DO_LOG_DEBUG("MQTT initialized successfully. QOS to be used: " + std::to_string(m_QOS));
	}
	catch (const std::exception &e)
	{
		DO_LOG_FATAL(e.what());
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
		DO_LOG_FATAL(e.what());
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
		// Check if topic is blank
		if (true == a_sTopic.empty())
		{
			DO_LOG_ERROR("Empty topic. Message not processed");
			return false;
		}
		// Check if message is blank
		if (true == a_sMsg.empty())
		{
			DO_LOG_ERROR("Empty Message. No action for topic: " + a_sTopic);
			return false;
		}

		if (true == m_bIsFirst)
		{
			connect();
			m_bIsFirst = false;
		}

		std::lock_guard<std::mutex> lock(mqttMutexLock);

		// Add timestamp to message
		struct timespec tsMsgPublish;
		timespec_get(&tsMsgPublish, TIME_UTC);
		std::string strTsPublish = std::to_string(CCommon::getInstance().get_micros(tsMsgPublish));

		// remove } bracket to add new key value pair to existing json
		a_sMsg.pop_back();

		string msgWithTimeStamp =  a_sMsg + ","+ "\"tsMsgReadyForPublish\"" + ":" + "\"" + strTsPublish + "\"" + "}";

		//publish data to MQTT
#ifdef INSTRUMENTATION_LOG
		DO_LOG_DEBUG("ZMQ Message: Time: "
				+ std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
		+ ", Msg: " + msgWithTimeStamp);
#endif

#ifdef PERFTESTING
		CMQTTPublishHandler::m_ui32PublishReq++;
#endif

		if(false == publisher.is_connected())
		{
			if(false == connect())
			{
				DO_LOG_ERROR("MQTT publisher is not connected with MQTT broker");
				DO_LOG_ERROR("Failed to publish msg on MQTT : " + msgWithTimeStamp);

				m_bIsFirst = true;

				return false;
			}
		}

		mqtt::message_ptr pubmsg = mqtt::make_message(a_sTopic, msgWithTimeStamp, this->m_QOS, false);

		publisher.publish(pubmsg, nullptr, listener);
		DO_LOG_DEBUG("Published message on MQTT broker successfully with QOS:"+ std::to_string(this->m_QOS));

		msgWithTimeStamp.clear();
		msgWithTimeStamp = "";

		return true;
	}
	catch (const mqtt::exception &exc)
	{
#ifdef PERFTESTING
			m_ui32PublishExcep++;
#endif
		DO_LOG_FATAL(exc.what());
	}
	return false;
}

#ifdef PERFTESTING
std::atomic<uint32_t> CMQTTPublishHandler::m_ui32PublishReq(0);
std::atomic<uint32_t> CMQTTPublishHandler::m_ui32PublishReqErr(0);
std::atomic<uint32_t> CMQTTPublishHandler::m_ui32Published(0);
std::atomic<uint32_t> CMQTTPublishHandler::m_ui32PublishFailed(0);
std::atomic<uint32_t> CMQTTPublishHandler::m_ui32ConnectionLost(0);
std::atomic<uint32_t> CMQTTPublishHandler::m_ui32Connection(0);
std::atomic<uint32_t> CMQTTPublishHandler::m_ui32PublishSkipped(0);
std::atomic<uint32_t> CMQTTPublishHandler::m_ui32PublishExcep(0);
std::atomic<uint32_t> CMQTTPublishHandler::m_ui32PublishReqTimeOut(0);
std::atomic<uint32_t> CMQTTPublishHandler::m_ui32Disconnected(0);
std::atomic<uint32_t> CMQTTPublishHandler::m_ui32PublishStrReq(0);
std::atomic<uint32_t> CMQTTPublishHandler::m_ui32PublishStrReqErr(0);
std::atomic<uint32_t> CMQTTPublishHandler::m_ui32PublishStrExcep(0);
std::atomic<uint32_t> CMQTTPublishHandler::m_ui32DelComplete(0);
std::atomic<uint32_t> CMQTTPublishHandler::m_uiQReqTried(0);

/**
 * Print counters
 * @return
 */
void CMQTTPublishHandler::printCounters()
{
	DO_LOG_DEBUG("Req rcvd: " + std::to_string(m_ui32PublishReq));
	DO_LOG_DEBUG("Req err: "  + std::to_string(m_ui32PublishReqErr));
	DO_LOG_DEBUG("Req sendmsg excep: "  + std::to_string(m_ui32PublishSkipped));
	DO_LOG_DEBUG("Req publish excep: "  + std::to_string(m_ui32PublishExcep));
	DO_LOG_DEBUG("Req published: "  + std::to_string(m_ui32Published));
	DO_LOG_DEBUG("Req publish failed: "  + std::to_string(m_ui32PublishFailed));
	DO_LOG_DEBUG("Req publish timeout: "  + std::to_string(m_ui32PublishReqTimeOut));
	DO_LOG_DEBUG("Req during no connection: "  + std::to_string(m_ui32Disconnected));
	DO_LOG_DEBUG("Req conn lost: " + std::to_string(m_ui32ConnectionLost));
	DO_LOG_DEBUG("Req conn done: " + std::to_string(m_ui32Connection));
	DO_LOG_DEBUG("*****Str Req: " + std::to_string(m_ui32PublishStrReq));
	DO_LOG_DEBUG("*****Str Req err: " + std::to_string(m_ui32PublishStrReqErr));
	DO_LOG_DEBUG("*****Str Req excep: " + std::to_string(m_ui32PublishStrExcep));
	DO_LOG_DEBUG("++++Req posted from Q: " + std::to_string(m_uiQReqTried));
	DO_LOG_DEBUG("$$$$Delivery completed: " + std::to_string(m_ui32DelComplete));
}
#endif

/**
 * Clean up, destroy semaphores, disables callback, disconnect from MQTT broker
 * @param None
 * @return None
 */
void CMQTTPublishHandler::cleanup()
{
	DO_LOG_DEBUG("Destroying CMQTTPublishHandler instance ...");

	if(publisher.is_connected())
	{
		publisher.disconnect();
	}
	DO_LOG_DEBUG("Destroyed CMQTTPublishHandler instance");
}

/**
 * Destructor
 */
CMQTTPublishHandler::~CMQTTPublishHandler()
{
}
