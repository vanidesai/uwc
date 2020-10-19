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
#include "SCADAHandler.hpp"

#include <chrono>
#include <ctime>

#define SUBSCRIBER_ID "INTERNAL_MQTT_SUBSCRIBER"
#define RECONN_TIMEOUT_SEC (60)

extern std::atomic<bool> g_shouldStop;

/**
 * Constructor Initializes MQTT publisher
 * @param strPlBusUrl :[in] MQTT broker URL
 * @param strClientID :[in] client ID with which to subscribe (this is topic name)
 * @param iQOS :[in] QOS value with which publisher will publish messages
 * @return None
 */
CIntMqttHandler::CIntMqttHandler(const std::string &strPlBusUrl, int iQOS):
	m_MQTTClient{strPlBusUrl, SUBSCRIBER_ID, iQOS, (false == CCommon::getInstance().isDevMode()), 
	"/run/secrets/ca_broker", "/run/secrets/client_cert", 
	"/run/secrets/client_key", "InternalMQTTListener"},
	m_enLastConStatus{enCON_NONE}, m_bIsInTimeoutState{false}
{
	try
	{
		m_QOS = iQOS;
		m_appSeqNo = 0;

		m_MQTTClient.setNotificationConnect(CIntMqttHandler::connected);
		m_MQTTClient.setNotificationDisConnect(CIntMqttHandler::disconnected);
		m_MQTTClient.setNotificationMsgRcvd(CIntMqttHandler::msgRcvd);

		//connect options for sync publisher/client
		/*m_connOpts.set_keep_alive_interval(20);
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

		connect();*/

		DO_LOG_DEBUG("MQTT initialized successfully");
	}
	catch (const std::exception &e)
	{
		DO_LOG_ERROR(e.what());
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
	static bool bIsFirst = true;
	static string strPlBusUrl = EnvironmentInfo::getInstance().getDataFromEnvMap("INTERNAL_MQTT_URL");
	static int nQos = CCommon::getInstance().getMQTTQos();

	if(bIsFirst)
	{
	if(strPlBusUrl.empty())
	{
		DO_LOG_ERROR("MQTT_URL Environment variable is not set");
		std::cout << __func__ << ":" << __LINE__ << " Error : MQTT_URL Environment variable is not set" <<  std::endl;
		throw std::runtime_error("Missing required config..");
	}
	}

	DO_LOG_DEBUG("Internal MQTT subscriber is connecting with QOS : " + to_string(nQos));
	static CIntMqttHandler handler(strPlBusUrl.c_str(), nQos);

	if(bIsFirst)
	{
		handler.init();
		handler.connect();
		bIsFirst = false;
	}
	return handler;
}

/**
 * Helper function to disconnect MQTT client from Internal MQTT broker
 * @param None
 * @return None
 */
void CIntMqttHandler::disconnect()
{
	try
	{
		if(true == m_MQTTClient.isConnected())
		{
			m_MQTTClient.disconnect();
		}

	}
	catch(exception &ex)
	{
		DO_LOG_ERROR(ex.what());
	}
}

/**
 * Helper function to connect MQTT client to Internal MQTT broker
 * @param None
 * @return None
 */
void CIntMqttHandler::connect()
{
	try
	{
		DO_LOG_INFO("Connecting to Internal MQTT ... ");
		m_MQTTClient.connect();
	}
	catch(exception &ex)
	{
		DO_LOG_ERROR(ex.what());
	}
}

/**
 * Subscribe to required topics for SCADA MQTT
 * @param none
 * @return none
 */
void CIntMqttHandler::subscribeTopics()
{
	try
	{
		m_MQTTClient.subscribe("BIRTH/#");
		m_MQTTClient.subscribe("DATA/#");
		m_MQTTClient.subscribe("DEATH/#");
		m_MQTTClient.subscribe("/+/+/+/update");
		//m_MQTTClient.subscribe("ConnectSCADA");
		//m_MQTTClient.subscribe("StopSCADA");

		DO_LOG_DEBUG("Subscribed topics with internal broker");
	}
	catch(exception &ex)
	{
		DO_LOG_ERROR(ex.what());
	}
}

/**
 * This is a callback function which gets called when subscriber is connected with MQTT broker
 * @param a_sCause :[in] reason for connect
 * @return None
 */
void CIntMqttHandler::connected(const std::string &a_sCause)
{
	try
	{
		CIntMqttHandler::instance().signalIntMQTTConnDoneThread();
	}
	catch(exception &ex)
	{
		DO_LOG_ERROR(ex.what());
	}
}

/**
 * This is a callback function which gets called when subscriber is disconnected with MQTT broker
 * @param a_sCause :[in] reason for disconnect
 * @return None
 */
void CIntMqttHandler::disconnected(const std::string &a_sCause)
{
	try
	{
		//CIntMqttHandler::instance().disconnect();
		CIntMqttHandler::instance().signalIntMQTTConnLostThread();
	}
	catch(exception &ex)
	{
		DO_LOG_ERROR(ex.what());
	}
}

/**
 * This is a callback function which gets called when a msg is received
 * @param a_pMsg :[in] pointer to received message
 * @return None
 */
void CIntMqttHandler::msgRcvd(mqtt::const_message_ptr a_pMsg)
{
	try
	{
		CIntMqttHandler::instance().pushMsgInQ(a_pMsg);
	}
	catch(exception &ex)
	{
		DO_LOG_ERROR(ex.what());
	}
}

/**
 * This is a singleton class. Used to handle communication with SCADA master
 * through external MQTT.
 * Initiates number of threads, semaphores for handling connection successful
 * and internal MQTT connection lost scenario.
 * @param None
 * @return true on successful init
 */
bool CIntMqttHandler::init()
{
	// Initiate semaphore for requests
	int retVal = sem_init(&m_semConnSuccess, 0, 0 /* Initial value of zero*/);
	if (retVal == -1)
	{
		std::cout << "*******Could not create unnamed semaphore for success connection\n";
		return false;
	}

	retVal = sem_init(&m_semConnLost, 0, 0 /* Initial value of zero*/);
	if (retVal == -1)
	{
		std::cout << "*******Could not create unnamed semaphore for lost connection\n";
		return false;
	}

	retVal = sem_init(&m_semConnSuccessToTimeOut, 0, 0 /* Initial value of zero*/);
	if (retVal == -1)
	{
		std::cout << "*******Could not create unnamed semaphore for monitorig connection timeout\n";
		return false;
	}
	std::thread{ std::bind(&CIntMqttHandler::handleConnMonitoringThread,
			std::ref(*this)) }.detach();

	std::thread{ std::bind(&CIntMqttHandler::handleConnSuccessThread,
			std::ref(*this)) }.detach();

	return true;
}

/**
 * Thread function to handle SCADA connection success scenario.
 * It listens on a semaphore to know the connection status.
 * @return none
 */
void CIntMqttHandler::handleConnMonitoringThread()
{
	while(false == g_shouldStop.load())
	{
		try
		{
			do
			{
				if((sem_wait(&m_semConnLost)) == -1 && errno == EINTR)
				{
					// Continue if interrupted by handler
					continue;
				}
				if(true == g_shouldStop.load())
				{
					break;
				}

				// Connection is lost
				// Now check for 1 min to see if connection is established
				struct timespec ts;
				//int rc = clock_gettime(CLOCK_MONOTONIC, &ts);
				int rc = clock_gettime(CLOCK_REALTIME, &ts);
				if(0 != rc)
				{
					std::cout << "Fatal error: clock_gettime failed: " << errno << std::endl;
					break;
				}
				// Wait for timeout seconds to declare connection timeout
				ts.tv_sec += RECONN_TIMEOUT_SEC;
				
				setConTimeoutState(true);
				while ((rc = sem_timedwait(&m_semConnSuccessToTimeOut, &ts)) == -1 && errno == EINTR)
				{
               		// Continue if interrupted by handler
					continue;
				}
				setConTimeoutState(false);

				if(true == m_MQTTClient.isConnected())
				{
					// Client is connected. So wait for connection-lost
					break;
				}

				// Check status
				if(-1 == rc)
				{
					if(ETIMEDOUT == errno)
					{
						DO_LOG_ERROR("Connection lost for timeout period. Informing SCADA");
						// timeout occurred and connection is not yet established
						// Inform SCADA handler
						CSCADAHandler::instance().signalIntMQTTConnLostThread();
					}
					else
					{
						// No action for now
					}
				}
			} while(0);
		}
		catch (exception &e)
		{
			DO_LOG_ERROR("failed to initiate request :: " + std::string(e.what()));
		}
	}
}

/**
 * Thread function to handle SCADA connection success scenario.
 * It listens on a semaphore to know the connection status.
 * @return none
 */
void CIntMqttHandler::handleConnSuccessThread()
{
	while(false == g_shouldStop.load())
	{
		try
		{
			do
			{
				if((sem_wait(&m_semConnSuccess)) == -1 && errno == EINTR)
				{
					// Continue if interrupted by handler
					continue;
				}
				if(true == g_shouldStop.load())
				{
					break;
				}

				if(true == m_MQTTClient.isConnected())
				{
					// Client is connected. So wait for connection-lost
					// Semaphore got signalled means connection is successful
					// As a process first subscribe to topics
					subscribeTopics();

					// If disconnect state is monitoring connection timeout, then signal that
					if(true == getConTimeoutState())
					{
						sem_post(&m_semConnSuccessToTimeOut);
					}
				}
			} while(0);

		}
		catch (exception &e)
		{
			DO_LOG_ERROR("failed to initiate request :: " + std::string(e.what()));
		}
	}
}

/**
 * MQTT publisher connects with MQTT broker
 * @param None
 * @return true/false based on success/failure
 */
/*bool CIntMqttHandler::connect()
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
}*/

/**
 * Subscribe with MQTT broker for topics for on-demand operations
 * @return true/false based on success/failure
 */
/*bool CIntMqttHandler::subscribeToTopics()
{
	//get list of topics from topic mapper
	std::vector<std::string> vMqttTopics;

	try
	{
		vMqttTopics.push_back("BIRTH/#");
		vMqttTopics.push_back("DATA/#");
		vMqttTopics.push_back("DEATH/#");
		vMqttTopics.push_back("/+/+/+/update");
		vMqttTopics.push_back("ConnectSCADA");
		vMqttTopics.push_back("StopSCADA");

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
}*/

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
		DO_LOG_ERROR(e.what());
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

	DO_LOG_DEBUG("Destroyed CIntMqttHandler instance");
}

/**
 * Checks if internal MQTT subscriber has been connected with the  MQTT broker
 * @param none
 * @return true/false as per the connection status of the external MQTT subscriber
 */
bool CIntMqttHandler::isConnected()
{
	return m_MQTTClient.isConnected();
}

/**
 *
 * Signals that initernal MQTT connection is lost
 * @return none
 */
void CIntMqttHandler::signalIntMQTTConnLostThread()
{
	if(enCON_DOWN != getLastConStatus())
	{
		sem_post(&m_semConnLost);
	}
	else
	{
		// No action. Connection is not yet established. 
		// This callback is being executed from reconnect
	}
	setLastConStatus(enCON_DOWN);
}

/**
 *
 * Signals that initernal MQTT connection is established
 * @return none
 */
void CIntMqttHandler::signalIntMQTTConnDoneThread()
{
	sem_post(&m_semConnSuccess);
	setLastConStatus(enCON_UP);

	static bool m_bIsFirstConnectDone = true;
	if(true == m_bIsFirstConnectDone)
	{
		publishIntMqttMsg("", "START_BIRTH_PROCESS");
		m_bIsFirstConnectDone = false;
	}
}

/**
 * Destructor
 */
CIntMqttHandler::~CIntMqttHandler()
{
	sem_destroy(&m_semConnSuccess);
	sem_destroy(&m_semConnLost);
	sem_destroy(&m_semConnSuccessToTimeOut);
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
 * Publish message on MQTT broker for MQTT-Export
 * @param a_sMsg :[in] message to publish
 * @param a_sTopic :[in] topic on which to publish message
 * @return true/false based on success/failure
 */
bool CIntMqttHandler::publishIntMqttMsg(const std::string &a_sMsg, const std::string &a_sTopic)
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
		m_MQTTClient.publishMsg(pubmsg);

		DO_LOG_DEBUG("Published message on Internal MQTT broker successfully with QOS:"+ std::to_string(m_QOS));

		return true;
	}
	catch (const mqtt::exception &exc)
	{
		DO_LOG_ERROR(exc.what());
		//cout << "Exception : " << exc.what() << endl;
	}
	return false;
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

					publishIntMqttMsg(strPubMsg, strMsgTopic);					
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
							string strPubMsg = cJSON_Print(root);
							publishIntMqttMsg(strPubMsg, strMsgTopic);							
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

