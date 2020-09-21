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
#include "SparkPlugDevices.hpp"

#include <chrono>
#include <ctime>

#define SUBSCRIBER_ID "INTERNAL_MQTT_SUBSCRIBER"

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
		m_appSeqNo = 0;

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
	static int nQos = CCommon::getInstance().getMQTTQos();

	if(strPlBusUrl.empty())
	{
		DO_LOG_ERROR("MQTT_URL Environment variable is not set");
		std::cout << __func__ << ":" << __LINE__ << " Error : MQTT_URL Environment variable is not set" <<  std::endl;
		throw std::runtime_error("Missing required config..");
	}

	DO_LOG_DEBUG("Internal MQTT subscriber is connecting with QOS : " + to_string(nQos));
	static CIntMqttHandler handler(strPlBusUrl.c_str(), nQos);
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
		std::cout << "Trying to connect with internal mqtt broker..." << std::endl;

		if (false == m_subscriber.connect(m_connOpts, nullptr, m_listener)->wait_for(2000))
		{
			bFlag = false;
			std::cout << __func__ << ":" << __LINE__ << "Failed to connect MQTT publisher & m_subscriber with internal broker" << std::endl;
 			DO_LOG_DEBUG("MQTT publisher & m_subscriber connection with internal MQTT broker is failed");
		}
		else
		{
		    bFlag = true;
			    std::cout << __func__ << ":" << __LINE__ << " MQTT publisher & m_subscriber connected with internal MQTT broker" << std::endl;
	    DO_LOG_DEBUG("MQTT publisher & m_subscriber connected with internal MQTT broker");
		}


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

		std::cout << __func__ << ":" << __LINE__ << "Internal MQTT handler subscribed topics with MQTT broker" << std::endl;
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

/**
 * Get app sequence no needed for write-on-demand
 * @return app sequence no in int format
 */
int CIntMqttHandler::getAppSeqNo()
{
	m_appSeqNo++;
	if(m_appSeqNo > 65535)
	{
		m_appSeqNo = 0;//todo - should we reset this to 0 after reconnection ?
	}

	return m_appSeqNo;
}

/**
 * Prepare a message in CJSON format to be sent on Inetnal MQTT
 * for vendor app and MQTT-Export
 * @param a_stRefActionVec :[in] vector of structure containing metrics with values
 * for which to prepare the request
 * @return true/false based on success/failure
 */
bool CIntMqttHandler::prepareCJSONMsg(std::vector<stRefForSparkPlugAction>& a_stRefActionVec)
{
	cJSON *root = NULL, *metricArray = NULL;

	try
	{
		//there should be only one device while forming CMD message from DCMD msg
		for (auto &itr : a_stRefActionVec)
		{
			string strMsgTopic = "";

			//get this device name to add in topic
			string strDeviceName = "";
			strDeviceName.append(itr.m_refSparkPlugDev.get().getSparkPlugName());

			if (strDeviceName.size() == 0)
			{
				DO_LOG_ERROR("Device name is blank");
				return false;
			}

			//parse the site name from the topic
			vector<string> vParsedTopic = { };
			CSparkPlugDevManager::getInstance().getTopicParts(strDeviceName, vParsedTopic, "-");

			if (vParsedTopic.size() != 2)
			{
				DO_LOG_ERROR("Invalid device name found while preparing request for internal MQTT");
				return false;
			}

			root = cJSON_CreateObject();
			if (root == NULL)
			{
				DO_LOG_ERROR("Creation of CJSON object failed");
				return false;
			}

			metricArray = cJSON_CreateArray();
			if (metricArray == NULL)
			{
				DO_LOG_ERROR("Creation of CJSON array failed");
				if (root != NULL)
				{
					cJSON_Delete(root);
					root = NULL;
				}
				return false;
			}

			//list of changed metrics for which to send CMD or write-on-demand CJSON request
			metricMap_t m_metrics = itr.m_mapChangedMetrics;

			if(itr.m_refSparkPlugDev.get().isVendorApp())
			{
				if (false == itr.m_refSparkPlugDev.get().getCMDMsg(strMsgTopic, m_metrics, metricArray))
				{
					DO_LOG_ERROR("Failed to prepare CJSON message for internal MQTT");
				}
				else
				{
					cJSON_AddItemToObject(root, "metrics", metricArray);

					string strPubMsg = cJSON_Print(root);

					DO_LOG_DEBUG("Publishing message on internal MQTT for CMD:");
					DO_LOG_DEBUG("Topic : " + strMsgTopic);
					DO_LOG_DEBUG("Payload : " + strPubMsg);

					//publish sparkplug message
					CPublisher::instance().publishIntMqttMsg(strPubMsg, strMsgTopic);
				}
			}
			else
			{
				for(auto& metric : m_metrics)
				{
					if(false == itr.m_refSparkPlugDev.get().getWriteMsg(strMsgTopic, root, metric, getAppSeqNo()))
					{
						DO_LOG_ERROR("Failed to prepare CJSON message for internal MQTT");
					}
					else
					{
						if((root != NULL) && (! strMsgTopic.empty()))
						{
							//publish sparkplug message
							string strPubMsg = cJSON_Print(root);
							CPublisher::instance().publishIntMqttMsg(strPubMsg, strMsgTopic);
						}
					}
					if (root != NULL)
					{
						cJSON_Delete(root);
						root = NULL;
					}
				}
			}

			if (root != NULL)
			{
				cJSON_Delete(root);
				root = NULL;
			}
		}//action structure ends
	}
	catch (exception &ex)
	{
		DO_LOG_FATAL(ex.what());
		if (root != NULL)
		{
			cJSON_Delete(root);
			root = NULL;
		}
		return false;
	}
	return true;
}

