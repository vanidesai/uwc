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
	CMQTTBaseHandler(strPlBusUrl, strClientID, iQOS, (false == CcommonEnvManager::Instance().gEtDevMode()),
	"/run/secrets/ca_broker", "/run/secrets/client_cert", 
	"/run/secrets/client_key", "MQTTSubListener")
{
	try
	{
		connect();
		DO_LOG_DEBUG("MQTT initialized successfully. QOS to be used: " + std::to_string(m_QOS));
	}
	catch (const std::exception &e)
	{
		DO_LOG_FATAL(e.what());
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
	}
}

/**
 * Publish message on MQTT broker
 * @param a_sMsg :[in] message to publish
 * @param a_sTopic :[in] topic on which to publish message
 * @param a_tsMsgRcvd :[in] time stamp to add while publishing
 * @return true/false based on success/failure
 */
bool CMQTTPublishHandler::createNPubMsg(std::string &a_sMsg, std::string &a_sTopic)
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

		// Add timestamp to message
		struct timespec tsMsgPublish;
		timespec_get(&tsMsgPublish, TIME_UTC);
		std::string strTsPublish = std::to_string(CCommon::getInstance().get_micros(tsMsgPublish));

		// remove } bracket to add new key value pair to existing json
		a_sMsg.pop_back();

		std::string msgWithTimeStamp =  a_sMsg + ","+ "\"tsMsgReadyForPublish\"" + ":" + "\"" + strTsPublish + "\"" + "}";

		//publish data to MQTT
#ifdef INSTRUMENTATION_LOG
		DO_LOG_DEBUG("ZMQ Message: Time: "
				+ std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
		+ ", Msg: " + msgWithTimeStamp);
#endif

		publishMsg(msgWithTimeStamp, a_sTopic);
		return true;
	}
	catch (const std::exception &exc)
	{
		DO_LOG_ERROR(exc.what());
	}
	return false;
}

/**
 * Destructor
 */
CMQTTPublishHandler::~CMQTTPublishHandler()
{
}
