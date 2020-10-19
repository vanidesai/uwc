/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "MqttHandler.hpp"
#include "Logger.hpp"
#include "CommonDataShare.hpp"
#include "EnvironmentVarHandler.hpp"
#include "KPIAppConfigMgr.hpp"

#define SUBSCRIBER_ID "KPIAPP_MQTT_SUBSCRIBER"

extern std::atomic<bool> g_stopThread;

/**
 * Constructor Initializes MQTT publisher
 * @param strPlBusUrl :[in] MQTT broker URL
 * @param strClientID :[in] client ID with which to subscribe (this is topic name)
 * @param iQOS :[in] QOS value with which publisher will publish messages
 * @return None
 */
CMqttHandler::CMqttHandler(const std::string &strPlBusUrl, int iQOS):
	m_MQTTClient{strPlBusUrl, SUBSCRIBER_ID, iQOS, (false == CcommonEnvManager::Instance().gEtDevMode()), 
	"/run/secrets/ca_broker", "/run/secrets/client_cert", 
	"/run/secrets/client_key", "InternalMQTTListener"},
	m_enLastConStatus{enCON_NONE}, m_bIsInTimeoutState{false}
{
	try
	{
		m_QOS = iQOS;
		m_appSeqNo = 0;

		m_MQTTClient.setNotificationConnect(CMqttHandler::connected);
		m_MQTTClient.setNotificationDisConnect(CMqttHandler::disconnected);
		m_MQTTClient.setNotificationMsgRcvd(CMqttHandler::msgRcvd);

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
CMqttHandler& CMqttHandler::instance()
{
	static bool bIsFirst = true;
	static string strPlBusUrl = EnvironmentInfo::getInstance().getDataFromEnvMap("MQTT_URL");
	static int nQos = 1;

	if(bIsFirst)
	{
		if(strPlBusUrl.empty())
		{
			DO_LOG_ERROR("MQTT_URL Environment variable is not set");
			std::cout << __func__ << ":" << __LINE__ << " Error : MQTT_URL Environment variable is not set" <<  std::endl;
			throw std::runtime_error("Missing required config..");
			//throw ("Missing required config..");
		}
	}

	DO_LOG_DEBUG("Internal MQTT subscriber is connecting with QOS : " + to_string(nQos));
	static CMqttHandler handler(strPlBusUrl.c_str(), nQos);

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
void CMqttHandler::disconnect()
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
void CMqttHandler::connect()
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
void CMqttHandler::subscribeTopics()
{
	try
	{
		auto subList = [&](const std::vector<std::string> &a_sList) 
		{
			for(auto &itr: a_sList)
			{
				m_MQTTClient.subscribe(itr);
			}
		};
		
		subList(CKPIAppConfig::getInstance().getControlLoopMapper().getPollingTopics());
		subList(CKPIAppConfig::getInstance().getControlLoopMapper().getWrRspTopics());
		
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
void CMqttHandler::connected(const std::string &a_sCause)
{
	try
	{
		CMqttHandler::instance().signalIntMQTTConnDoneThread();
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
void CMqttHandler::disconnected(const std::string &a_sCause)
{
	try
	{
		//CMqttHandler::instance().disconnect();
		CMqttHandler::instance().signalIntMQTTConnLostThread();
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
void CMqttHandler::msgRcvd(mqtt::const_message_ptr a_pMsg)
{
	try
	{
		CMqttHandler::instance().pushMsgInQ(a_pMsg);
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
bool CMqttHandler::init()
{
	// Initiate semaphore for requests
	int retVal = sem_init(&m_semConnSuccess, 0, 0 /* Initial value of zero*/);
	if (retVal == -1)
	{
		DO_LOG_ERROR("Could not create semaphore for success connection");
		return false;
	}

	retVal = sem_init(&m_semConnLost, 0, 0 /* Initial value of zero*/);
	if (retVal == -1)
	{
		DO_LOG_ERROR("Could not create semaphore for lost connection");
		return false;
	}

	std::thread{ std::bind(&CMqttHandler::handleConnSuccessThread,
			std::ref(*this)) }.detach();

	return true;
}

/**
 * Thread function to handle SCADA connection success scenario.
 * It listens on a semaphore to know the connection status.
 * @return none
 */
void CMqttHandler::handleConnSuccessThread()
{
	while(false == g_stopThread.load())
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
				if(true == g_stopThread.load())
				{
					break;
				}

				if(true == m_MQTTClient.isConnected())
				{
					// Client is connected. 
					// Semaphore got signalled means connection is successful
					// As a process first subscribe to topics
					subscribeTopics();
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
 * Push message in message queue to send on EIS
 * @param msg :[in] reference of message to push in queue
 * @return true/false based on success/failure
 */
bool CMqttHandler::pushMsgInQ(mqtt::const_message_ptr& a_msgMQTT)
{
	bool bRet = true;
	try
	{
		auto endsWith = [](const std::string &a_sMain, const std::string &a_sToMatch)
		{
			if((a_sMain.size() >= a_sToMatch.size()) && 
				(a_sMain.compare(a_sMain.size() - a_sToMatch.size(), a_sToMatch.size(), a_sToMatch) == 0))
			{
				return true;
			}
			else
			{
				return false;
			}
		};
		std::string sTopic{a_msgMQTT->get_topic()};
		// Decide whether to use polling queue or writeResponse queue
		if(true == endsWith(sTopic, "/update"))
		{
			// For now put msg in polling queue
			CMessageObject oMsg{sTopic, a_msgMQTT->get_payload()};
			QMgr::PollMsgQ().pushMsg(oMsg);
			bRet = true;
		}
		else if(true == endsWith(sTopic, "/writeResponse"))
		{
			// For now put msg in polling queue
			CMessageObject oMsg{sTopic, a_msgMQTT->get_payload()};
			QMgr::WriteRespMsgQ().pushMsg(oMsg);
			bRet = true;
		}
		else
		{
			DO_LOG_INFO(sTopic + " : does not match pattern. Ignored.");
		}		
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
void CMqttHandler::cleanup()
{
}

/**
 * Checks if internal MQTT subscriber has been connected with the  MQTT broker
 * @param none
 * @return true/false as per the connection status of the external MQTT subscriber
 */
bool CMqttHandler::isConnected()
{
	return m_MQTTClient.isConnected();
}

/**
 *
 * Signals that initernal MQTT connection is lost
 * @return none
 */
void CMqttHandler::signalIntMQTTConnLostThread()
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
void CMqttHandler::signalIntMQTTConnDoneThread()
{
	sem_post(&m_semConnSuccess);
	setLastConStatus(enCON_UP);
}

/**
 * Destructor
 */
CMqttHandler::~CMqttHandler()
{
	sem_destroy(&m_semConnSuccess);
	sem_destroy(&m_semConnLost);
}

/**
 * Publish message on MQTT broker for MQTT-Export
 * @param a_sMsg :[in] message to publish
 * @param a_sTopic :[in] topic on which to publish message
 * @return true/false based on success/failure
 */
bool CMqttHandler::publishMsg(const std::string &a_sMsg, const std::string &a_sTopic)
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
	}
	return false;
}
