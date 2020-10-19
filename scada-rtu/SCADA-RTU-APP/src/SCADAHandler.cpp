/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include <thread>
#include "SCADAHandler.hpp"
#include "InternalMQTTSubscriber.hpp"

extern std::atomic<bool> g_shouldStop;

/**
 * constructor Initializes MQTT m_subscriber
 * @param strPlBusUrl :[in] MQTT broker URL
 * @param iQOS :[in] QOS with which to get messages
 * @return None
 */
CSCADAHandler::CSCADAHandler(const std::string &strMqttURL, int iQOS) :
	m_MQTTClient{strMqttURL, SUBSCRIBERID, iQOS, CCommon::getInstance().isScadaTLS(), 
	"/run/secrets/scadahost_ca_cert", "/run/secrets/scadahost_client_cert", 
	"/run/secrets/scadahost_client_key", "SCADAMQTTListener"}
		
{
	try
	{
		m_QOS = iQOS;

		prepareNodeDeathMsg(false);
		m_MQTTClient.setNotificationConnect(CSCADAHandler::connected);
		m_MQTTClient.setNotificationDisConnect(CSCADAHandler::disconnected);
		m_MQTTClient.setNotificationMsgRcvd(CSCADAHandler::msgRcvd);

		//get values of all the data points and store in data points repository
		//initDataPoints();

		init();

		DO_LOG_DEBUG("MQTT initialized successfully");
	}
	catch (const std::exception &e)
	{
		DO_LOG_FATAL(e.what());
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
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
bool CSCADAHandler::init()
{
	// Initiate semaphore for requests
	int retVal = sem_init(&m_semSCADAConnSuccess, 0, 0 /* Initial value of zero*/);
	if (retVal == -1)
	{
		std::cout << "*******Could not create unnamed semaphore for SCADA success connection\n";
		return false;
	}
	std::thread{ std::bind(&CSCADAHandler::handleSCADAConnectionSuccessThread,
			std::ref(*this)) }.detach();

	retVal = sem_init(&m_semIntMQTTConnLost, 0, 0 /* Initial value of zero*/);
	if (retVal == -1)
	{
		std::cout << "*******Could not create unnamed semaphore for Internal MQTT connection lost\n";
		return false;
	}
	std::thread{ std::bind(&CSCADAHandler::handleIntMQTTConnLostThread,
			std::ref(*this)) }.detach();

	//publish vendor app birth message
	vendor_app_birth_request();
	
	return true;
}

/**
 * Thread function to handle SCADA connection success scenario.
 * It listens on a semaphore to know the connection status.
 * @return none
 */
void CSCADAHandler::handleSCADAConnectionSuccessThread()
{
	while(false == g_shouldStop.load())
	{
		try
		{
			do
			{
				if((sem_wait(&m_semSCADAConnSuccess)) == -1 && errno == EINTR)
				{
					// Continue if interrupted by handler
					continue;
				}
				if(true == g_shouldStop.load())
				{
					break;
				}

				// As a process first subscribe to topics
				subscribeTopics();

				// Publish the NBIRTH
				publish_node_birth();

				// Publish the DBIRTH for all devices
				publishAllDevBirths();

				setInitStatus(true);
			} while(0);

		}
		catch (exception &e)
		{
			DO_LOG_ERROR("failed to send birth messages :: " + std::string(e.what()));
		}
	}
}

/**
 * Thread function to handle internal MQTT connection lost scenario.
 * It listens on a semaphore to know the connection status.
 * @return none
 */
void CSCADAHandler::handleIntMQTTConnLostThread()
{
	while(false == g_shouldStop.load())
	{
		try
		{
			do
			{
				if((sem_wait(&m_semIntMQTTConnLost)) == -1 && errno == EINTR)
				{
					// Continue if interrupted by handler
					continue;
				}
				if(true == g_shouldStop.load())
				{
					break;
				}
				DO_LOG_ERROR("INFO: Internal MQTT connection lost. DDEATH to be sent");

				// Publish DDEATH for each device
				auto vDevList = CSparkPlugDevManager::getInstance().getDeviceList();
				for(auto &itrDevice : vDevList)
				{
					// Check if internal MQTT connection is established.
					if(true == CIntMqttHandler::instance().isConnected())
					{
						// Connection is established.
						// No need to send remaining DDEATH messages
						DO_LOG_ERROR("Info: Internal MQTT connection is established. No More DDEATH to be sent.");
						break;
					}
					if(false == getInitStatus())
					{
						DO_LOG_ERROR("Node init is not done. SparkPlug message publish is not done");
						break;
					}
					DO_LOG_ERROR("Sending DDEATH for : " + itrDevice);
					publishMsgDDEATH(itrDevice);
					CSparkPlugDevManager::getInstance().setMsgPublishedStatus(enDEVSTATUS_DOWN, itrDevice);
				}
			} while(0);
		}
		catch (exception &e)
		{
			DO_LOG_ERROR("ERROR :: " + std::string(e.what()));
		}
	}
}

/**
 *
 * Signals that initernal MQTT connection is lost
 * @return none
 */
void CSCADAHandler::signalIntMQTTConnLostThread()
{
	sem_post(&m_semIntMQTTConnLost);
}

/**
 *
 * Publish message on MQTT broker for MQTT-Export
 * @param a_ddata_payload :[in] spark plug message to publish
 * @param a_topic :[in] topic on which to publish message
 * @return true/false based on success/failure
 */
bool CSCADAHandler::publishSparkplugMsg(org_eclipse_tahu_protobuf_Payload& a_payload, string a_topic)
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

		m_MQTTClient.publishMsg(pubmsg);

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
 * Prepare a will message for subscriber. That will
 * act as a death message when this node goes down.
 * @param none
 * @return none
 */
void CSCADAHandler::prepareNodeDeathMsg(bool a_bPublishMsg)
{
	try
	{
		// Create the NDEATH payload
		org_eclipse_tahu_protobuf_Payload ndeath_payload;
		get_next_payload(&ndeath_payload);

		add_simple_metric(&ndeath_payload, "bdSeq", false, 0,
					METRIC_DATA_TYPE_UINT64, false, false, &m_uiBDSeq, sizeof(m_uiBDSeq));

		size_t buffer_length = 0;

		bool encode_passed = pb_get_encoded_size(&buffer_length, org_eclipse_tahu_protobuf_Payload_fields, &ndeath_payload);
		if(encode_passed == false)
		{
			DO_LOG_ERROR("Failed to calculate the payload length");
			return ;
		}
		uint8_t *binary_buffer = (uint8_t *)malloc(buffer_length * sizeof(uint8_t));
		if(binary_buffer == NULL)
		{
			DO_LOG_ERROR("Failed to allocate new memory");
			return;
		}
		size_t message_length = encode_payload(binary_buffer, buffer_length, &ndeath_payload);

		// Publish the DDATA on the appropriate topic
		mqtt::message_ptr pubmsg = mqtt::make_message(CCommon::getInstance().getDeathTopic(), (void*)binary_buffer, message_length, m_QOS, false);

		//connect options for async m_subscriber
		m_MQTTClient.setWillMsg(pubmsg);
		if(true == a_bPublishMsg)
		{
			m_MQTTClient.publishMsg(pubmsg);
		}

		if(binary_buffer != NULL)
		{
			free(binary_buffer);
		}
		free_payload(&ndeath_payload);
	}
	catch(exception& ex)
	{
		DO_LOG_ERROR(ex.what());
	}
}

/**
 * Publish node birth messages to SCADA
 * @param none
 * @return none
 */
void CSCADAHandler::startSCADAConnectionSuccessProcess()
{
	try
	{
		sem_post(&m_semSCADAConnSuccess);
	}
	catch(exception& ex)
	{
		DO_LOG_ERROR(ex.what());
	}
}

/**
 * Publish device birth message on SCADA for all devices
 * @param a_deviceName : [in] device for which to publish birth message
 * @return none
 */
void CSCADAHandler::publishAllDevBirths()
{
	try
	{
		auto vDevList = CSparkPlugDevManager::getInstance().getDeviceList();
		for(auto &itrDevice : vDevList)
		{
			DO_LOG_DEBUG("Device : " + itrDevice);
			publish_device_birth(itrDevice, true);
		}
	}
	catch(exception &ex)
	{
		DO_LOG_ERROR(ex.what());
	}
}

/**
 * Publish device birth message on SCADA
 * @param a_deviceName : [in] device for which to publish birth message
 * @param a_bIsNBIRTHProcess: [in] indicates whether DBIRTH is needed as a part of NBIRTH process
 * @return none
 */
void CSCADAHandler::publish_device_birth(string a_deviceName, bool a_bIsNBIRTHProcess)
{
	// Create the DBIRTH payload
	org_eclipse_tahu_protobuf_Payload dbirth_payload;
	get_next_payload(&dbirth_payload);

	try
	{
		if(true == CSparkPlugDevManager::getInstance().prepareDBirthMessage(dbirth_payload, a_deviceName, a_bIsNBIRTHProcess))
		{
			string strDBirthTopic = CCommon::getInstance().getDBirthTopic() + "/" + a_deviceName;

			publishSparkplugMsg(dbirth_payload, strDBirthTopic);
			CSparkPlugDevManager::getInstance().setMsgPublishedStatus(enDEVSTATUS_UP, a_deviceName);
		}
	}
	catch(exception &ex)
	{
		DO_LOG_ERROR(ex.what());
	}
	// Free the memory
	free_payload(&dbirth_payload);
}

/**
 * Maintain single instance of this class
 * @param None
 * @return Reference of this instance of this class, if successful;
 * 			Application exits in case of failure
 */
CSCADAHandler& CSCADAHandler::instance()
{
	static bool bIsFirst = true;
	static string strMqttUrl = CCommon::getInstance().getExtMqttURL();
	static int nQos = CCommon::getInstance().getMQTTQos();
	if(bIsFirst)
	{
		if(strMqttUrl.empty())
		{
			DO_LOG_ERROR("EXTERNAL_MQTT_URL variable is not set in config file");
			std::cout << __func__ << ":" << __LINE__ << " Error : EXTERNAL_MQTT_URL variable is not set in config file" <<  std::endl;
			throw std::runtime_error("Missing required config..");
		}
	}
	DO_LOG_DEBUG("External MQTT subscriber is connecting with QOS : " + to_string(nQos));
	static CSCADAHandler handler(CCommon::getInstance().getExtMqttURL(), nQos);

	if(bIsFirst)
	{
		handler.connect();
		bIsFirst = false;
	}

	return handler;
}

/**
 * Clean up, destroy semaphores, disables callback, disconnect from MQTT broker
 * @param None
 * @return None
 */
void CSCADAHandler::cleanup()
{
	DO_LOG_DEBUG("Destroying CSCADAHandler instance ...");

	DO_LOG_DEBUG("Destroyed CSCADAHandler instance");
}

/**
 * Destructor
 */
CSCADAHandler::~CSCADAHandler()
{
	sem_destroy(&m_semSCADAConnSuccess);
	sem_destroy(&m_semIntMQTTConnLost);
}

/**
 * Publish node birth message on SCADA
 * @param none
 * @return none
 */
void CSCADAHandler::publish_node_birth()
{
	reset_sparkplug_sequence();
	org_eclipse_tahu_protobuf_Payload nbirth_payload;
	get_next_payload(&nbirth_payload);
	try
	{
		// Create the NBIRTH payload
		string strAppName = EnvironmentInfo::getInstance().getDataFromEnvMap("AppName");
		if(strAppName.empty())
		{
			DO_LOG_ERROR("App name is empty");
			return;
		}

		nbirth_payload.uuid = (char*) strAppName.c_str();

		// Add getNBirthTopicsome device metrics
		add_simple_metric(&nbirth_payload, "Name", false, 0,
				METRIC_DATA_TYPE_STRING, false, false,
				CCommon::getInstance().getEdgeNodeID().c_str(), CCommon::getInstance().getEdgeNodeID().length()+1);

			add_simple_metric(&nbirth_payload, "bdSeq", false, 0, METRIC_DATA_TYPE_UINT64, false, false,
					&m_uiBDSeq, sizeof(m_uiBDSeq));

		std::cout << "Publishing nbirth message ..." << endl;
			publishSparkplugMsg(nbirth_payload, CCommon::getInstance().getNBirthTopic());

		nbirth_payload.uuid = NULL;
	}
	catch(exception& ex)
	{
		DO_LOG_ERROR(ex.what());
	}
	free_payload(&nbirth_payload);
}

/**
 * Initialize data points repository reading data points
 * from yaml file
 * @param none
 * @return true/false depending on success/failure
 */
/*bool CSCADAHandler::initDataPoints()
{
	try
	{
		string strNetWorkType = CCommon::getInstance().getNetworkType();
		string strSiteListFileName = CCommon::getInstance().getSiteListFileName();

		if(strNetWorkType.empty() || strSiteListFileName.empty())
		{
			DO_LOG_ERROR("Network type or device list file name is not present");
			return false;
		}

		network_info::buildNetworkInfo(strNetWorkType, strSiteListFileName);

		// Create SparkPlug devices corresponding to Modbus devices
		CSparkPlugDevManager::getInstance().addRealDevices();
	}
	catch(exception &ex)
	{
		DO_LOG_ERROR(ex.what());
	}
	return true;
}*/

/**
 * Subscribe to required topics for SCADA MQTT
 * @param none
 * @return none
 */
void CSCADAHandler::subscribeTopics()
{
	try
	{
		m_MQTTClient.subscribe("+/+/DCMD/+/+");

		DO_LOG_DEBUG("Subscribed with topics from SCADA master");
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
void CSCADAHandler::connected(const std::string &a_sCause)
{
	try
	{
		DO_LOG_ERROR("INFO: Connected");
		// Publish the NBIRTH and DBIRTH Sparkplug messages
		CSCADAHandler::instance().startSCADAConnectionSuccessProcess();
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
void CSCADAHandler::disconnected(const std::string &a_sCause)
{
	try
	{
		DO_LOG_ERROR("INFO: Disconnected");
		CSCADAHandler::instance().disconnect();
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
void CSCADAHandler::msgRcvd(mqtt::const_message_ptr a_pMsg)
{
	try
	{
		CSCADAHandler::instance().pushMsgInQ(a_pMsg);
	}
	catch(exception &ex)
	{
		DO_LOG_ERROR(ex.what());
	}
}

/**
 * Publish message on internal MQTT for vendor app
 * so that it will publish device birth message
 */
void CSCADAHandler::vendor_app_birth_request()
{
	/*try
	{
    //get birth details from vendor app by publishing messages for them
    //prepare CJSON message to publish on internal MQTT
	string strPubTopic = "START_BIRTH_PROCESS";
	string strBlankMsg = "";
		//CPublisher::instance().publishIntMqttMsg(strBlankMsg, strPubTopic);
		CIntMqttHandler::instance().publishIntMqttMsg(strBlankMsg, strPubTopic);
	}
	catch(exception &ex)
	{
		DO_LOG_ERROR(ex.what());
	}*/
}

/**
 * Prepare and publish a DDATA message in sparkplug format for a device in a_stRefAction
 * @param a_stRefAction :[in] device and respective data-points which need to be
 * published on External MQTT broker
 * @return true/false based on success/failure
 */
bool CSCADAHandler::publishMsgDDATA(const stRefForSparkPlugAction& a_stRefAction)
{
	//prepare and publish one sparkplug msg for this device
	org_eclipse_tahu_protobuf_Payload sparkplug_payload;
	get_next_payload(&sparkplug_payload);
	try
	{
		if(enMSG_DATA != a_stRefAction.m_enAction)
		{
			return false;
		}
		//get this device name to add in topic
		std::string strDeviceName{a_stRefAction.m_refSparkPlugDev.get().getSparkPlugName()};

		if (strDeviceName.size() == 0)
		{
			DO_LOG_ERROR("Device name is blank");
			return false;
		}

		string strMsgTopic = CCommon::getInstance().getDDataTopic() + "/" + strDeviceName;

		//these shall be part of a single sparkplug msg
		for (auto &itrMetric : a_stRefAction.m_mapChangedMetrics)
		{
			uint64_t timestamp = itrMetric.second.getTimestamp();
			string strMetricName = itrMetric.second.getName();

			org_eclipse_tahu_protobuf_Payload_Metric metric =
					{ NULL, false, 0, true, timestamp, true,
						(const_cast<CMetric&>(itrMetric.second)).getValue().getDataType(), false, 0, false, 0, false,
							true, false,
				org_eclipse_tahu_protobuf_Payload_MetaData_init_default,
							false,
									org_eclipse_tahu_protobuf_Payload_PropertySet_init_default,
							0,
							{ 0 } };

			if(false == (const_cast<CMetric&>(itrMetric.second)).addMetricNameValue(metric))
			{
				DO_LOG_ERROR(itrMetric.second.getName() + ":Failed to add metric name and value");
			}
			else
			{
				add_metric_to_payload(&sparkplug_payload, &metric);
			}
		}//metric ends

		//publish sparkplug message
		publishSparkplugMsg(sparkplug_payload, strMsgTopic);
		a_stRefAction.m_refSparkPlugDev.get().setPublishedStatus(enDEVSTATUS_UP);
	}
	catch(exception &ex)
	{
		DO_LOG_ERROR(ex.what());
		return false;
	}
	free_payload(&sparkplug_payload);
	return true;
}

/**
 * Prepare and publish a DDEATH message in sparkplug format for a device in a_stRefAction
 * @param a_stRefAction :[in] device and respective data-points which need to be
 * published on External MQTT broker
 * @return true/false based on success/failure
 */
bool CSCADAHandler::publishMsgDDEATH(const stRefForSparkPlugAction& a_stRefAction)
{
	//prepare and publish one sparkplug msg for this device
	org_eclipse_tahu_protobuf_Payload sparkplug_payload;
	get_next_payload(&sparkplug_payload);
	try
	{
		if(enMSG_DEATH != a_stRefAction.m_enAction)
		{
			return false;
		}
		//get this device name to add in topic
		std::string strDeviceName{a_stRefAction.m_refSparkPlugDev.get().getSparkPlugName()};

		if (strDeviceName.size() == 0)
		{
			DO_LOG_ERROR("Device name is blank");
			return false;
		}

		string strMsgTopic = CCommon::getInstance().getDDeathTopic() + "/" + strDeviceName;

		sparkplug_payload.has_timestamp = true;
		sparkplug_payload.timestamp = a_stRefAction.m_refSparkPlugDev.get().getDeathTime();

		//publish sparkplug message
		publishSparkplugMsg(sparkplug_payload, strMsgTopic);
		a_stRefAction.m_refSparkPlugDev.get().setPublishedStatus(enDEVSTATUS_DOWN);
	}
	catch(exception &ex)
	{
		DO_LOG_ERROR(ex.what());
		return false;
	}
	free_payload(&sparkplug_payload);
	return true;
}

/**
 * Prepare a MQTT message with sparkplug format for devices mentioned in a_stRefActionVec
 * @param a_stRefActionVec :[in] devices and respective data-points which need to be
 * published on External MQTT broker
 * @return true/false based on success/failure
 */
bool CSCADAHandler::prepareSparkPlugMsg(std::vector<stRefForSparkPlugAction>& a_stRefActionVec)
{
	try
	{
		if(false == getInitStatus())
		{
			DO_LOG_ERROR("Node init is not done. SparkPlug message publish is not done");
			return false;
		}
		//for loop having all the devices for which to publish sparkplug message
		for (auto &itr : a_stRefActionVec)
		{
			//depending on action, call the topic name
			switch (itr.m_enAction)
			{
			case enMSG_BIRTH:
				publish_device_birth(itr.m_refSparkPlugDev.get().getSparkPlugName(), false);
				break;
			case enMSG_DEATH:
				publishMsgDDEATH(itr);
				break;
			case enMSG_DATA:
				publishMsgDDATA(itr);
				break;
			default:
				DO_LOG_ERROR("Invalid message type received");
				return false;
			}
		}
	}
	catch(exception &ex)
	{
		DO_LOG_ERROR(ex.what());
		return false;
	}
	return true;
}

/**
 * Prepare and publish a DDEATH message in sparkplug format for a device 
 * @param a_sDevName :[in] device name
 * @return true/false based on success/failure
 */
bool CSCADAHandler::publishMsgDDEATH(const std::string &a_sDevName)
{
	//prepare and publish one sparkplug msg for this device
	org_eclipse_tahu_protobuf_Payload sparkplug_payload;
	get_next_payload(&sparkplug_payload);
	try
	{
		if (a_sDevName.size() == 0)
		{
			DO_LOG_ERROR("Device name is blank");
			return false;
		}

		string strMsgTopic = CCommon::getInstance().getDDeathTopic() + "/" + a_sDevName;

		sparkplug_payload.has_timestamp = true;
		sparkplug_payload.timestamp = get_current_timestamp();

		//publish sparkplug message
		publishSparkplugMsg(sparkplug_payload, strMsgTopic);
	}
	catch(exception &ex)
	{
		DO_LOG_ERROR(ex.what());
		return false;
	}
	free_payload(&sparkplug_payload);
	return true;
}

/**
 * Helper function to disconnect MQTT client from External MQTT broker
 * @param None
 * @return None
 */
void CSCADAHandler::disconnect()
{
	try
	{
		if(true == m_MQTTClient.isConnected())
		{
			/*this->prepareNodeDeathMsg(true);*/
			m_MQTTClient.disconnect();
		}

		++m_uiBDSeq;
		setInitStatus(false);
		prepareNodeDeathMsg(false);
	}
	catch(exception &ex)
	{
		DO_LOG_ERROR(ex.what());
	}
}

/**
 * Helper function to connect MQTT client to External MQTT broker
 * @param None
 * @return None
 */
void CSCADAHandler::connect()
{
	try
	{
		DO_LOG_INFO("Connecting to External MQTT ... ");
		m_MQTTClient.connect();
	}
	catch(exception &ex)
	{
		DO_LOG_ERROR(ex.what());
	}
}

/**
 * Process message received from external MQTT broker
 * @param a_msg :[in] mqtt message to be processed
 * @return none
 */
bool CSCADAHandler::processDCMDMsg(mqtt::const_message_ptr a_msg, std::vector<stRefForSparkPlugAction>& a_stRefActionVec)
{
	org_eclipse_tahu_protobuf_Payload dcmd_payload = org_eclipse_tahu_protobuf_Payload_init_zero;
	bool bRet = false;

	try
	{
		int msgLen = a_msg->get_payload().length();

		if(decode_payload(&dcmd_payload, (uint8_t* )a_msg->get_payload().data(), msgLen) >= 0)
		{
			bRet = CSparkPlugDevManager::getInstance().processExternalMQTTMsg(a_msg->get_topic(),
					dcmd_payload, a_stRefActionVec);
		}
		else
		{
			DO_LOG_ERROR("Failed to decode the sparkplug payload");
		}

		bRet = true;
	}
	catch(exception &ex)
	{
		DO_LOG_FATAL(ex.what());
		bRet = false;
	}
		free_payload(&dcmd_payload);
	return bRet;
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
		QMgr::getScadaSubQ().pushMsg(msg);

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
