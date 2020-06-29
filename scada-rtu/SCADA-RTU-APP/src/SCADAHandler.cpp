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

		//get values of all the data points and store in data points repository
		initDataPoints();

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
	add_simple_metric(&ndeath_payload, "bdSeq", false, 0,
			METRIC_DATA_TYPE_UINT64, false, false, false, &badSeq_value, sizeof(badSeq_value));

	size_t buffer_length = 1024;
	uint8_t *binary_buffer = (uint8_t *)malloc(buffer_length * sizeof(uint8_t));
	if(binary_buffer == NULL)
	{
		DO_LOG_ERROR("Failed to allocate new memory");
		return;
	}
	size_t message_length = encode_payload(&binary_buffer, buffer_length, &ndeath_payload);

	// Publish the DDATA on the appropriate topic
	mqtt::message_ptr pubmsg = mqtt::make_message(CCommon::getInstance().getDeathTopic(), (void*)binary_buffer, message_length, 0, false);

	//connect options for async m_subscriber
	m_subscriberConopts.set_will_message(pubmsg);

	if(binary_buffer != NULL)
	{
		free(binary_buffer);
	}
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

	// Publish the DBIRTH
	for(auto &itrDevice : m_deviceDataPoints)
	{
		DO_LOG_DEBUG("Device : " + itrDevice.first);

		publish_device_birth(itrDevice.first, itrDevice.second);
	}
}

/**
 * Prepare device birth messages to be published on SCADA system
 * @param dbirth_payload :[out] reference of spark plug message payload in which to store birth messages
 * @param a_dataPoints :[in] map of datapoints corresponding to the device
 * @return true/false depending on the success/failure
 */
bool CSCADAHandler::prepareDBirthMessage(org_eclipse_tahu_protobuf_Payload& dbirth_payload, std::map<string, stDataPointRepo>& a_dataPoints, string& a_siteName)
{
	try
	{
		for(auto &dataPoint : a_dataPoints)
		{
			a_siteName = dataPoint.second.m_objUniquePoint.getWellSite().getID();

			string strDeviceName = "";
			if(dataPoint.second.m_objUniquePoint.getDataPoint().isInputPoint() == true)//data point is input
			{
				strDeviceName.assign("Inputs/");
			}
			else
			{
				strDeviceName.assign("Outputs/");
			}
			strDeviceName.append(dataPoint.second.m_objUniquePoint.getDataPoint().getID());

			uint64_t current_time = get_current_timestamp();

			org_eclipse_tahu_protobuf_Payload_Metric metric = {NULL, false, 0, true, current_time , true,
					METRIC_DATA_TYPE_STRING, false, 0, false, 0, false, true, false,
					org_eclipse_tahu_protobuf_Payload_MetaData_init_default,
					false, org_eclipse_tahu_protobuf_Payload_PropertySet_init_default, 0, {0}};

			metric.name = (char*)malloc(strDeviceName.size());
			if(metric.name == NULL)
			{
				DO_LOG_ERROR("Failed to allocate new memory");
				return false;
			};
			strDeviceName.copy(metric.name, strDeviceName.size());
			metric.name[strDeviceName.size()] = '\0';
			metric.has_is_null = true;

			org_eclipse_tahu_protobuf_Payload_PropertySet prop = org_eclipse_tahu_protobuf_Payload_PropertySet_init_default;

			uint32_t iPollingInterval = dataPoint.second.m_objUniquePoint.getDataPoint().getPollingConfig().m_uiPollFreq;
			add_property_to_set(&prop, "Pollinterval", PROPERTY_DATA_TYPE_UINT32, false, &iPollingInterval, sizeof(iPollingInterval));

			bool bVal = dataPoint.second.m_objUniquePoint.getDataPoint().getPollingConfig().m_bIsRealTime;
			add_property_to_set(&prop, "Realtime", PROPERTY_DATA_TYPE_BOOLEAN, false, &bVal, sizeof(bVal));

			add_propertyset_to_metric(&metric, &prop);
			add_metric_to_payload(&dbirth_payload, &metric);
		}

		add_simple_metric(&dbirth_payload, "Properties/Site info name", false, 0,
				METRIC_DATA_TYPE_STRING, false, false, false, a_siteName.c_str(), a_siteName.size()+1);
	}
	catch(exception &ex)
	{
		DO_LOG_FATAL(ex.what());
		std::cout << "Exception : " << ex.what() << endl;
		return false;
	}
	return true;
}

/**
 * Publish device birth message on SCADA
 * @param a_deviceName : [in] device for which to publish birth message
 * @param a_dataPointInfo : [in] device info
 * @return none
 */
void CSCADAHandler::publish_device_birth(string a_deviceName, std::map<string, stDataPointRepo>& a_dataPointInfo)
{
	// Create the DBIRTH payload
	org_eclipse_tahu_protobuf_Payload dbirth_payload;
	get_next_payload(&dbirth_payload);

	try
	{
		string strSiteName = "";

		if(a_dataPointInfo.empty())
		{
			DO_LOG_ERROR("No data points are available to publish DBIRTH message");
			return;
		}

		if(true == prepareDBirthMessage(dbirth_payload, a_dataPointInfo, strSiteName))
		{
			string strDBirthTopic = CCommon::getInstance().getDBirthTopic() + a_deviceName;

			CPublisher::instance().publishSparkplugMsg(dbirth_payload, strDBirthTopic);
		}
	}
	catch(exception &ex)
	{
		DO_LOG_FATAL(ex.what());
		std::cout << " Exception : " << endl;
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
	string strAppName = CCommon::getInstance().getStrAppName();
	if(strAppName.empty())
	{
		DO_LOG_ERROR("App name is empty");
		return;
	}

	org_eclipse_tahu_protobuf_Payload nbirth_payload;
	get_next_payload(&nbirth_payload);

	nbirth_payload.uuid = (char*) strAppName.c_str();

	// Add getNBirthTopicsome device metrics
	add_simple_metric(&nbirth_payload, "Name", false, 0,
			METRIC_DATA_TYPE_STRING, false, false, false,
			CCommon::getInstance().getEdgeNodeID().c_str(), CCommon::getInstance().getEdgeNodeID().size()+1);

	std::cout << "Publishing nbirth message ..." << endl;
	CPublisher::instance().publishSparkplugMsg(nbirth_payload, CCommon::getInstance().getNBirthTopic());

	nbirth_payload.uuid = NULL;
	free_payload(&nbirth_payload);
}

/**
 * Populate data points from all the devices from all the sites
 * and store in data points repository
 * @param none
 * @return none
 */
void CSCADAHandler::populateDataPoints()
{
	using network_info::CUniqueDataPoint;

	const std::map<std::string, CUniqueDataPoint> &mapUniquePoint =
			network_info::getUniquePointList();

	for (auto &pt : mapUniquePoint)
	{
		stDataPointRepo stNewDataPoint(pt.second);

		std::string sUniqueDev(pt.second.getWellSiteDev().getID() + SEPARATOR_CHAR + pt.second.getWellSite().getID());

		 m_deviceDataPoints[sUniqueDev].insert(
				 std::make_pair(stNewDataPoint.m_objUniquePoint.getDataPoint().getID(), stNewDataPoint));
	}
}

/**
 * Initialize data points repository reading data points
 * from yaml file
 * @param none
 * @return true/false depending on success/failure
 */
bool CSCADAHandler::initDataPoints()
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

		//fill up data points repository
		populateDataPoints();
	}
	catch(exception &ex)
	{
		DO_LOG_FATAL(ex.what());
	}
	return true;
}
