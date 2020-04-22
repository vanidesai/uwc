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
#include <semaphore.h>
#include "Common.hpp"
#include "ConfigManager.hpp"
/**
 * constructor
 * @param strPlBusUrl :[in] MQTT broker URL
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

		publisher.set_callback(syncCallback);

		if(connect())
		{
			m_bIsFirst = false;
		}
		else
		{
			m_bIsFirst = true;
		}

		CLogger::getInstance().log(DEBUG, LOGDETAILS("MQTT initialized successfully"));
	}
	catch (const std::exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
	}
}

/**
 * MQTT publisher connects with MQTT broker
 * @return 	true : on success,
 * 			false : on error
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
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		std::cout << __func__ << ":" << __LINE__ << " MQTT publisher failed to connect with MQTT broker: exception : " << e.what() << std::endl;

		bFlag = false;
	}
	return bFlag;
}

/**
 * Get publisher current state
 * @return publisher state
 */
Mqtt_Config_state_t CMQTTPublishHandler::getMQTTConfigState()
{
	return ConfigState;
}

/**
 * Set publisher state to given
 * @param tempConfigState :[in] publisher state to set
 */
void CMQTTPublishHandler::setMQTTConfigState(Mqtt_Config_state_t tempConfigState)
{
	ConfigState = tempConfigState;
}

/**
 * Gets current time in nano seconds
 * @param ts :[in] structure of current time
 * @return current time in nano seconds
 */
static unsigned long get_nanos(struct timespec ts)
{
    return (unsigned long)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

/**
 * Add current time stamp in message payload
 * @param a_sMsg 		:[in] message in which to add time
 * @param a_tsMsgRcvd	:[in] time stamp in nano seconds
 * @return 	true : on success,
 * 			false : on error
 */
bool CMQTTPublishHandler::addTimestampsToMsg(std::string &a_sMsg, struct timespec a_tsMsgRcvd)
{
	cJSON *root = NULL;
	try
	{
		root = cJSON_Parse(a_sMsg.c_str());
		if (NULL == root)
		{
			CLogger::getInstance().log(ERROR, 
					LOGDETAILS("ZMQ Message could not be parsed in json format"));
			return false;
		}

		struct timespec tsMsgPublish;
		timespec_get(&tsMsgPublish, TIME_UTC);
		std::string strTsRcvd = std::to_string(get_nanos(a_tsMsgRcvd));
		std::string strTsPublish = std::to_string(get_nanos(tsMsgPublish));

		if(NULL == cJSON_AddStringToObject(root, "tsMsgReadyForPublish", strTsPublish.c_str()))
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("Could not add tsMsgReadyForPublish in message"));
			if(root != NULL)
			{
				cJSON_Delete(root);
			}
			return false;
		}

		if(NULL == cJSON_AddStringToObject(root, "tsMsgRcvdForProcessing", strTsRcvd.c_str()))
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("Could not add tsMsgRcvdForProcessing in message"));
			if(root != NULL)
			{
				cJSON_Delete(root);
			}
			return false;
		}

		a_sMsg.clear();
		char *psNewJson = cJSON_Print(root);
		if(NULL != psNewJson)
		{
			a_sMsg.assign(psNewJson);
			free(psNewJson);
			psNewJson = NULL;
		}

		if(root != NULL)
		{
			cJSON_Delete(root);
		}

		CLogger::getInstance().log(DEBUG, LOGDETAILS("Added timestamp in payload for MQTT"));
		return true;

	}
	catch (exception &ex)
	{
		CLogger::getInstance().log(DEBUG, LOGDETAILS("Failed to add timestamp in payload for MQTT" + std::string(ex.what())));
		
		if(root != NULL)
		{
			cJSON_Delete(root);
		}

		return false;
	}
}

/**
 * Publish message on MQTT broker
 * @param a_sMsg 	:[in] message to publish
 * @param a_sTopic	:[in] topic on which to publish message
 * @param a_tsMsgRcvd :[in] time stamp to add while publishing
 * @return 	true : on success,
 * 			false : on error
 */
bool CMQTTPublishHandler::publish(std::string &a_sMsg, std::string &a_sTopic, struct timespec a_tsMsgRcvd)
{
	try
	{
		if (true == m_bIsFirst)
		{
			connect();
			m_bIsFirst = false;
		}

		// Add timestamp to message
		addTimestampsToMsg(a_sMsg, a_tsMsgRcvd);

		std::lock_guard<std::mutex> lock(mqttMutexLock);

		// Check if topic is blank
		if (true == a_sTopic.empty())
		{
			if (true == a_sMsg.empty())
			{
				CLogger::getInstance().log(ERROR, LOGDETAILS("Blank topic and blank Message"));
			}
			else
			{
				CLogger::getInstance().log(ERROR, LOGDETAILS("Blank topic. Message not posted"));
			}
			return false;
		}

#ifdef PERFTESTING
		CMQTTPublishHandler::m_ui32PublishReq++;
#endif

		if(false == publisher.is_connected())
		{
			if(false == connect())
			{
				CLogger::getInstance().log(ERROR, LOGDETAILS("MQTT publisher is not connected with MQTT broker"));
				CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to publish msg on MQTT : " + a_sMsg));

				m_bIsFirst = true;

				return false;
			}
		}

		publisher.publish(mqtt::message(a_sTopic, a_sMsg, m_QOS, false));
		CLogger::getInstance().log(DEBUG, LOGDETAILS("Published message on MQTT broker successfully with QOS:"
									+ std::to_string(m_QOS)));
		return true;
	}
	catch (const mqtt::exception &exc)
	{
#ifdef PERFTESTING
			m_ui32PublishExcep++;
#endif
		CLogger::getInstance().log(FATAL, LOGDETAILS(exc.what()));
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
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Req rcvd: " + std::to_string(m_ui32PublishReq)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Req err: "  + std::to_string(m_ui32PublishReqErr)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Req sendmsg excep: "  + std::to_string(m_ui32PublishSkipped)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Req publish excep: "  + std::to_string(m_ui32PublishExcep)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Req published: "  + std::to_string(m_ui32Published)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Req publish failed: "  + std::to_string(m_ui32PublishFailed)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Req publish timeout: "  + std::to_string(m_ui32PublishReqTimeOut)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Req during no connection: "  + std::to_string(m_ui32Disconnected)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Req conn lost: " + std::to_string(m_ui32ConnectionLost)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Req conn done: " + std::to_string(m_ui32Connection)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("*****Str Req: " + std::to_string(m_ui32PublishStrReq)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("*****Str Req err: " + std::to_string(m_ui32PublishStrReqErr)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("*****Str Req excep: " + std::to_string(m_ui32PublishStrExcep)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("++++Req posted from Q: " + std::to_string(m_uiQReqTried)));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("$$$$Delivery completed: " + std::to_string(m_ui32DelComplete)));
}
#endif

/**
 * Clean up, destroy semaphores, disables callback, disconnect from MQTT broker
 */
void CMQTTPublishHandler::cleanup()
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Destroying CMQTTPublishHandler instance ..."));

	if(publisher.is_connected())
	{
		publisher.disconnect();
	}
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Destroyed CMQTTPublishHandler instance"));
}

/**
 * Destructor
 */
CMQTTPublishHandler::~CMQTTPublishHandler()
{
}

/////////////////////////////////////////////
