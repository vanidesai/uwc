/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/
#include <string.h>
#include "SparkPlugDevices.hpp"

#include <ctime>

/**
 * Process new device birth message from vendor app and extract all the information about metric
 * @param a_MetricList :[in] list of metrics present in a message payload received on internal MQTT broker
 * @param a_bIsOnlyValChange: [out] a flag indicates whether changes in BIRTH message are only changes in value which result in DDATA message rather than a DBIRTH message 
 * @return structured map of metrics
 */
metricMap_t CSparkPlugDev::processNewBirthData(metricMap_t a_MetricList, bool &a_bIsOnlyValChange)
{
	// There could be following cases:
	// Case 1: All are new metrics => This results in a DBIRTH message
	// Case 2: Datatype of few metrics are changed => This results in a DBIRTH message
	// Case 3: Only values of few metrics are changed. There are no other changes => This results in a DDATA message
	// Case 4: There are no changes. 
	
	metricMap_t oMetricMap;
	// This is a special case: In BIRTH message when we just recceive a change in 
	metricMap_t oMetricMapData;
	// Default value: false: This means output is a BIRTH message
	a_bIsOnlyValChange = false;
	do
	{
		try
		{
			std::lock_guard<std::mutex> lck(m_mutexMetricList);
			for (auto &itrInputMetric : a_MetricList)
			{
				auto itrMyMetric = m_mapMetrics.find(itrInputMetric.first);
				if (m_mapMetrics.end() == itrMyMetric)
				{
					m_mapMetrics.emplace(itrInputMetric.first,
							itrInputMetric.second);
					oMetricMap.emplace(itrInputMetric.first,
							itrInputMetric.second);
				}
				else
				{
					// Compare data type
					switch (itrInputMetric.second.getValue().compareValue(
							itrMyMetric->second.getValue()))
					{
					case SAMEVALUE_OR_DTATYPE:
						// No action
						break;

					case VALUES_DIFFERENT:
						// TODO: Should we assign value here ?
						itrMyMetric->second.getValue().assignValue(
								itrInputMetric.second.getValue());
						// Add data to a separate metric map for maintaining data
						oMetricMapData.emplace(itrInputMetric.first,
								itrInputMetric.second);
						break;

					case DATATYPE_DIFFERENT:
						// Datatype is different
						itrMyMetric->second.getValue().assignNewDataTypeValue(
								itrInputMetric.second.getValue().getDataType(),
								itrInputMetric.second.getValue());
						// Add data to a separate metric map for maintaining changes in BIRTH
						oMetricMap.emplace(itrInputMetric.first,
								itrInputMetric.second);
						break;
					}
				}
			}
		} catch (const std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
		}
	} while (0);

	// Check if any changes occurred for BIRTH message
	if (0 != oMetricMap.size())
	{
		a_bIsOnlyValChange = false;
		return oMetricMap;
	}
	// No changes occurred for BIRTH
	// Check if any changes occurred for DATA change i.e. only change in value
	if (0 != oMetricMapData.size())
	{
		a_bIsOnlyValChange = true;
		return oMetricMapData;
	}
	// No chnages occurred for BIRTH or DATA
	a_bIsOnlyValChange = false;
	return oMetricMap;
}

/**
 * Process new device data message from vendor app and extract all the information about metric
 * @param a_MetricList :[in] list of metrics present in a message payload received on internal MQTT broker
 * @return structured map of metrics
 */
metricMap_t CSparkPlugDev::processNewData(metricMap_t a_MetricList)
{
	metricMap_t oMetricMap;
	do
	{
		try
		{
			std::lock_guard<std::mutex> lck(m_mutexMetricList);
			for (auto &itrInputMetric : a_MetricList)
			{
				auto itrMyMetric = m_mapMetrics.find(itrInputMetric.first);
				if (m_mapMetrics.end() == itrMyMetric)
				{
					DO_LOG_ERROR(
							itrInputMetric.first
									+ ": Metric not found in device: "
									+ m_sSparkPlugName
									+ ". Ignoring this metric data");
				}
				else
				{
					// Compare data type
					switch (itrInputMetric.second.getValue().compareValue(
							itrMyMetric->second.getValue()))
					{
					case SAMEVALUE_OR_DTATYPE:
						// No action
						break;

					case VALUES_DIFFERENT:
						// TODO: Should we assign value here ?
						itrMyMetric->second.getValue().assignValue(
								itrInputMetric.second.getValue());
						oMetricMap.emplace(itrInputMetric.first,
								itrInputMetric.second);
						break;

					case DATATYPE_DIFFERENT:
						// Datatype is different
						DO_LOG_ERROR(
								itrInputMetric.first
										+ ": Metric data-types differ in device: "
										+ m_sSparkPlugName
										+ ". Expected datatype:"
										+ std::to_string(
												itrMyMetric->second.getValue().getDataType())
										+ ", Received datatype:"
										+ std::to_string(
												itrInputMetric.second.getValue().getDataType())
										+ ". Ignoring this data")
						;
						break;
					}
				}
			}
		} catch (const std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
		}
	} while (0);

	return oMetricMap;
}

/**
 * Add metric to SprakPlugDev for Modbus device
 * @param a_rUniqueDataPoint :[in] reference of unique data point
 */
void CSparkPlugDev::addMetric(const network_info::CUniqueDataPoint &a_rUniqueDataPoint)
{
	do
	{
		try
		{
			std::lock_guard<std::mutex> lck(m_mutexMetricList);
			auto itrMyMetric = m_mapMetrics.find(a_rUniqueDataPoint.getID());
			if (m_mapMetrics.end() == itrMyMetric)
			{
				// Add a metric if not already present
				CMetric oMetric{a_rUniqueDataPoint};
				m_mapMetrics.emplace(oMetric.getSparkPlugName(), oMetric);
			}
			else
			{
				// Ignore this datapoint. It is already present.
				DO_LOG_ERROR(a_rUniqueDataPoint.getID() + " : Datapoint is already present in SparkPlug device: " 
								+ m_sSparkPlugName + ". Ignoring most recent instances.");
			}
		} catch (const std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
		}
	} while (0);
}

/**
 * Prepare device birth messages to be published on SCADA system
 * @param a_rTahuPayload :[out] reference of spark plug message payload in which to store birth messages
 * @param a_bIsNBIRTHProcess: [in] indicates whether DBIRTH is needed as a part of NBIRTH process
 * @return true/false depending on the success/failure
 */
bool CSparkPlugDev::prepareDBirthMessage(org_eclipse_tahu_protobuf_Payload& a_rTahuPayload, bool a_bIsNBIRTHProcess)
{
	try
	{
		std::lock_guard<std::mutex> lck(m_mutexMetricList);
		
		// Check if last known status of device is DOWN or not
		if(enDEVSTATUS_DOWN == getLastKnownDevStatus())
		{
			DO_LOG_INFO(m_sSparkPlugName + ": Last known dev status is DOWN. DBIRTH message is not prepared.");
			// Last know status is DOWN. Do NOT prepare a birth message
			return false;
		}
		// Check whether this is node (re)birth scenario
		if(false == a_bIsNBIRTHProcess)
		{
			// Following check should be done when DBIRTH is done as a part of device status and
			// and not when node (re)birth is happening
			// Check if last published status of device is UP or not
			if(enDEVSTATUS_UP == getLastPublishedDevStatus())
			{
				DO_LOG_INFO(m_sSparkPlugName + ": Last published dev status is UP. DBIRTH message is not prepared.");
				// Last know status is not UP. Do NOT prepare a birth message
				return false;
			}
		}
		{
			for(auto &itr: m_mapMetrics)
			{
				uint64_t current_time = itr.second.getTimestamp();

				org_eclipse_tahu_protobuf_Payload_Metric metric = {NULL, false, 0, true, current_time , true,
						itr.second.getValue().getDataType(), false, 0, false, 0, false, true, false,
						org_eclipse_tahu_protobuf_Payload_MetaData_init_default,
						false, org_eclipse_tahu_protobuf_Payload_PropertySet_init_default, 0, {0}};

				if(true == itr.second.addMetricForBirth(metric))
				{
					add_metric_to_payload(&a_rTahuPayload, &metric);
				}
				else
				{
					DO_LOG_ERROR(itr.second.getSparkPlugName() + ":Could not add metric to device. Trying to add other metrics.");
				}
			}
		}
		if (true == std::holds_alternative<std::reference_wrapper<const network_info::CUniqueDataDevice>>(m_rDirectDevRef))
		{
			auto &orUniqueDev = std::get<std::reference_wrapper<const network_info::CUniqueDataDevice>>(m_rDirectDevRef);
			std::string sSiteName{orUniqueDev.get().getWellSite().getID()};
			add_simple_metric(&a_rTahuPayload, "Properties/Site info name", false, 0,
				METRIC_DATA_TYPE_STRING, false, false, sSiteName.c_str(), sSiteName.size()+1);
			
			// Add device connection information 
			auto &oWellSiteDev = orUniqueDev.get().getWellSiteDev();
			uint64_t current_time = get_current_timestamp();

			org_eclipse_tahu_protobuf_Payload_Metric metric = {NULL, false, 0, true, current_time , true,
				METRIC_DATA_TYPE_UINT16, false, 0, false, 0, false, true, false,
				org_eclipse_tahu_protobuf_Payload_MetaData_init_default,
				false, org_eclipse_tahu_protobuf_Payload_PropertySet_init_default, 0, {0}};
			metric.name = strdup("Properties/Dev-ID");
			metric.which_value = org_eclipse_tahu_protobuf_Payload_Metric_int_value_tag;
			if(network_info::eNetworkType::eTCP == oWellSiteDev.getAddressInfo().m_NwType)
			{
				metric.value.int_value = oWellSiteDev.getAddressInfo().m_stTCP.m_uiUnitID;

				org_eclipse_tahu_protobuf_Payload_PropertySet prop = org_eclipse_tahu_protobuf_Payload_PropertySet_init_default;
				std::string sProtocol{"Modbus TCP"};
				add_property_to_set(&prop, "protocol", PROPERTY_DATA_TYPE_STRING, sProtocol.c_str(), sProtocol.length());

				const std::string sIPAddr{oWellSiteDev.getAddressInfo().m_stTCP.m_sIPAddress};
				add_property_to_set(&prop, "ipaddress", PROPERTY_DATA_TYPE_STRING, sIPAddr.c_str(), sIPAddr.length());

				auto uiPort = oWellSiteDev.getAddressInfo().m_stTCP.m_ui16PortNumber;
				add_property_to_set(&prop, "port", PROPERTY_DATA_TYPE_UINT16, &uiPort, sizeof(uiPort));

				add_propertyset_to_metric(&metric, &prop);
			}
			else if(network_info::eNetworkType::eRTU == oWellSiteDev.getAddressInfo().m_NwType)
			{
				metric.value.int_value = oWellSiteDev.getAddressInfo().m_stRTU.m_uiSlaveId;
				auto &oInfoRTU = oWellSiteDev.getRTUNwInfo();

				org_eclipse_tahu_protobuf_Payload_PropertySet prop = org_eclipse_tahu_protobuf_Payload_PropertySet_init_default;
				std::string sProtocol{"Modbus RTU"};
				add_property_to_set(&prop, "protocol", PROPERTY_DATA_TYPE_STRING, sProtocol.c_str(), sProtocol.length());

				const std::string sPort{oInfoRTU.getPortName()};
				add_property_to_set(&prop, "com_port_name", PROPERTY_DATA_TYPE_STRING, sPort.c_str(), sPort.length());

				const std::string sParity{oInfoRTU.getParity()};
				add_property_to_set(&prop, "parity", PROPERTY_DATA_TYPE_STRING, sParity.c_str(), sParity.length());

				auto iBaudRate = oInfoRTU.getBaudRate();
				add_property_to_set(&prop, "baudrate", PROPERTY_DATA_TYPE_UINT32, &iBaudRate, sizeof(iBaudRate));

				add_propertyset_to_metric(&metric, &prop);
			}

			add_metric_to_payload(&a_rTahuPayload, &metric);
		}
	}
	catch(exception &ex)
	{
		DO_LOG_FATAL(ex.what());
		return false;
	}
	return true;
}

/**
 * Parses real device update message and stores metrics and corresponding values
 * @param a_sPayLoad :[in] payload containing metrics
 * @param a_sMetric :[out] stores value of "metric" key
 * @param a_sValue :[out] stores value of "value" key
 * @param a_sStatus :[out] stores value of "status" key
 * @param a_usec :[out] stores value of "usec" key
 * @param a_lastGoodUsec :[out] stores value of "lastGoodUsec" key
 * @param a_error_code :[out] stores value of "error_code" key
 * @return true or false based on success
 */
bool CSparkPlugDev::parseRealDeviceUpdateMsg(const std::string &a_sPayLoad, 
		std::string &a_sMetric, std::string &a_sValue, std::string &a_sStatus,
		uint64_t &a_usec, uint64_t &a_lastGoodUsec, uint32_t &a_error_code)
{
	cJSON *pjRoot = NULL;
	try
	{
		// Set initial values
		a_sMetric.clear();
		a_sValue.clear();
		a_sStatus.clear();
		a_usec = a_lastGoodUsec = a_error_code = 0;

		// Parse JSON file
		pjRoot = cJSON_Parse(a_sPayLoad.c_str());
		if (NULL == pjRoot)
		{
			DO_LOG_ERROR("Message received from MQTT could not be parsed in json format");
			return false;
		}

		// Lambda to read a parameter from JSON
		auto readFieldFromJSON = [](std::string &a_sFieldValue, std::string a_sFieldName, cJSON *a_cjRoot) -> bool
		{
			a_sFieldValue.clear();
			if(NULL != a_cjRoot)
			{
				cJSON *cjValue = cJSON_GetObjectItem(a_cjRoot, a_sFieldName.c_str());
				if(cjValue == NULL)
				{
					DO_LOG_DEBUG(a_sFieldName + ": Field not found in input JSON");
					return false;
				}
				else
				{
					char *pcValue = cJSON_GetStringValue(cjValue);
					if (pcValue == NULL)
					{
						DO_LOG_ERROR("Invalid payload: No value found for field: " + a_sFieldName);
						return false;
					}
					a_sFieldValue.assign(pcValue);
				}
			}
			return true;
		};

		// Read metric
		if(false == readFieldFromJSON(a_sMetric, "metric", pjRoot))
		{
			DO_LOG_ERROR("metric key not found in message: " + a_sPayLoad);
			return false;
		}

		// Read status
		if(false == readFieldFromJSON(a_sStatus, "status", pjRoot))
		{
			DO_LOG_ERROR("status key not found in message" + a_sPayLoad);
			return false;
		}
	
		// Read value
		if(false == readFieldFromJSON(a_sValue, "value", pjRoot))
		{
			DO_LOG_ERROR("value key not found in message" + a_sPayLoad);
			return false;
		}

		std::string sTemp{""};
		//timestamp is optional parameter
		if(false == readFieldFromJSON(sTemp, "usec", pjRoot))
		{
			a_usec = get_current_timestamp();
		}
		else
		{
			char *end = NULL;
			a_usec = strtoul(sTemp.c_str(), &end, 10);
		}

		//lastGoodUsec is optional parameter
		if(true == readFieldFromJSON(sTemp, "lastGoodUsec", pjRoot))
		{
			char *end = NULL;
			a_lastGoodUsec = strtoul(sTemp.c_str(), &end, 10);
		}

		//error_code is optional parameter
		if(true == readFieldFromJSON(sTemp, "error_code", pjRoot))
		{
			char *end = NULL;
			a_error_code = strtoul(sTemp.c_str(), &end, 10);
		}
	}
	catch (std::exception &ex)
	{
		DO_LOG_FATAL(ex.what());
	}

	if (NULL != pjRoot)
	{
		cJSON_Delete(pjRoot);
	}

	return true;
}

/**
 * Validates data parsed from update message of a real device
 * @param a_sValue :[in] value of "value" key
 * @param a_sStatus :[in] value of "status" key
 * @param a_error_code :[in] value of "error_code" key
 * @param a_bIsGood :[out] Indicates whether the status is good or bad
 * @param a_bIsDeathCode :[out] Indicates error code is for a device being unreachable
 * @return true or false based on success
 */
bool CSparkPlugDev::validateRealDeviceUpdateData(
		const std::string &a_sValue, std::string a_sStatus,
		uint32_t a_error_code,
		bool &a_bIsGood, bool &a_bIsDeathCode)
{
	try
	{
		// Check status
		a_bIsGood = true;
		a_bIsDeathCode = false;
		std::transform(a_sStatus.begin(), a_sStatus.end(), a_sStatus.begin(), ::tolower);
		if("good" != a_sStatus)
		{
			if("bad" != a_sStatus)
			{
				DO_LOG_ERROR("Unknown status. Ignoring the message");
				return false;
			}
			// Read error_code
			if(0 == a_error_code)
			{
				DO_LOG_ERROR("Either error_code key not found or value is 0 in message with bad status. Ignoring the message");
				return false;
			}

			// status is bad. Check error code
			a_bIsGood = false;
			
			// Following error codes are considered under device connection being lost
			// 2002	STACK_ERROR_SOCKET_FAILED
			// 2003	STACK_ERROR_CONNECT_FAILED
			// 2006	STACK_ERROR_RECV_TIMEOUT

			// As of now, following are not considered under device connection being lost
			// 2004	STACK_ERROR_SEND_FAILED
			// 2005	STACK_ERROR_RECV_FAILED
			switch(a_error_code)
			{
				case 2002:
				case 2003:
				case 2006:
				a_bIsDeathCode = true;
				break;

				default:
				a_bIsDeathCode = false;
				break;
			}
		}
		else 
		{
			// Check if value is present. 
			if(a_sValue.empty())
			{
				DO_LOG_ERROR("Value in string format is not present. Ignoring the message");
				return false;
			}
		}
	}
	catch (std::exception &ex)
	{
		DO_LOG_FATAL(ex.what());
		return false;
	}

	return true;
}

/**
 * Parses real device update message and stores metrics and corresponding values
 * @param a_sPayLoad :[in] payload containing metrics
 * @param a_stRefActionVec :[out] action vector
 * @return map containing metric and corresponding values
 */
bool CSparkPlugDev::processRealDeviceUpdateMsg(const std::string a_sPayLoad, std::vector<stRefForSparkPlugAction> &a_stRefActionVec)
{
	try
	{
		std::string sMetric{""}, sValue{""}, sStatus{""};
		uint64_t usec{0}, lastGoodUsec{0};
		uint32_t error_code{0};
		bool bRet = parseRealDeviceUpdateMsg(a_sPayLoad, 
			sMetric, sValue, sStatus, usec, lastGoodUsec, error_code);
		if(false == bRet)
		{
			DO_LOG_ERROR("Unable to parse message: " + a_sPayLoad);
			return false;
		}

		// At this point, all required fields are retrieved to make decisions
		// metric-name, status, error-code (if any), value, usec, lastGoodUsec (if present)
		// Now validate these fields values

		bool bIsGood = true, bDeathErrorCode = false;
		bRet = validateRealDeviceUpdateData(sValue, sStatus, error_code, bIsGood, bDeathErrorCode);
		if(false == bRet)
		{
			DO_LOG_ERROR("Message validation failed: " + a_sPayLoad);
			return false;
		}
		
		std::lock_guard<std::mutex> lck(m_mutexMetricList);
		// Check if metric is a part of this device
		auto itrMyMetric = m_mapMetrics.find(sMetric);
		if (m_mapMetrics.end() == itrMyMetric)
		{
			DO_LOG_ERROR(sMetric + ": Metric not found in device: "
						+ m_sSparkPlugName + ". Ignoring this metric data");
			return false;
		}
		CMetric &refMyMetric = itrMyMetric->second;

		// All validations are done
		// Now, action can be determined based on values in this message and 
		// last known status informed to SCADA Master
		CValObj oValObj{sValue};

		// Scenarios: last status to SCADA = DEVICE UP
		//		1. Now Good status and 
		//			a. change in value => DDATA
		//			b. no change in value => no action
		//		2. Now Bad status and 
		//			a. error code other than death scenario => no action
		//			b. error code meaning death scenario => DDEATH

		// Scenarios: last status to SCADA = DEVICE DOWN
		//		1. Now Good status => DBIRTH
		//		2. Now Bad status => NO action
		
		auto uiValCompareResult = refMyMetric.getValue().compareValue(oValObj);

		// Lambda to read a parameter from JSON
		auto addToActionVector = [&](eMsgAction a_eMsgAction) -> bool
		{
			metricMap_t mapChangedMetricsFromUpdate;
			if((enMSG_DATA == a_eMsgAction) || (enMSG_BIRTH == a_eMsgAction))
			{
				refMyMetric.setValue(oValObj);
				refMyMetric.setTimestamp(usec);

				if(enMSG_DATA == a_eMsgAction)
				{
					mapChangedMetricsFromUpdate.emplace(refMyMetric.getSparkPlugName(), refMyMetric);
				}
			}
			
			stRefForSparkPlugAction stDummyAction
				{ std::ref(*this), a_eMsgAction, mapChangedMetricsFromUpdate};
				a_stRefActionVec.push_back(stDummyAction);
			return true;
		};
		
		// Now to prepare action vector, check all scenarios
		switch(getLastPublishedDevStatus())
		{
			case enDEVSTATUS_NONE:
			// No action as of now. Wait for SCADA-RTU to send a 1st dbirth message
			break;

			case enDEVSTATUS_DOWN:
			if(true == bIsGood)
			{
				// Now it is a good data
				// case for dbirth
				addToActionVector(enMSG_BIRTH);
				setKnownDevStatus(enDEVSTATUS_UP);
			}
			else
			{
				// All other cases are ignored. No action

				// Check lastgoodsec and value. Set it to be used for next DBIRTH message
				if((0 != lastGoodUsec) && (VALUES_DIFFERENT == uiValCompareResult)
						&& (false == sValue.empty()))
				{
					refMyMetric.setTimestamp(lastGoodUsec);
					refMyMetric.setValue(oValObj);
				}
			}
			
			break;

			case enDEVSTATUS_UP:
			// Last report was: device is up
			if(true == bIsGood)
			{
				refMyMetric.setTimestamp(usec);

				if(VALUES_DIFFERENT == uiValCompareResult)
				{
					addToActionVector(enMSG_DATA);
					setKnownDevStatus(enDEVSTATUS_UP);
				}
			}
			else if((false == bIsGood) && (true == bDeathErrorCode))
			{
				// Check lastgoodsec and value. 
				if((0 != lastGoodUsec) && (VALUES_DIFFERENT == uiValCompareResult)
					&& (false == sValue.empty()))
				{
					// This case means there is some value which we did not have earlier.
					// Set it to be used for next DBIRTH message
					refMyMetric.setTimestamp(lastGoodUsec);
					refMyMetric.setValue(oValObj);
				}
				addToActionVector(enMSG_DEATH);
				setKnownDevStatus(enDEVSTATUS_DOWN);
			}
			else if(false == bIsGood)
			{
				// All other cases are ignored. No action
				// Check lastgoodsec and value. 
				if((0 != lastGoodUsec) && (VALUES_DIFFERENT == uiValCompareResult)
					&& (false == sValue.empty()))
				{
					addToActionVector(enMSG_DATA);
					setKnownDevStatus(enDEVSTATUS_UP);
				}
			}
			break;

			default:
			break;
		}
	}
	catch (std::exception &ex)
	{
		DO_LOG_FATAL(ex.what());
	}

	return true;
}

/**
 * Prepare cJSON message for write-on-demand to publish on internal MQTT
 * @param a_sTopic :[in] topic on which to publish the message
 * @param a_root :[out] cJSON reference containing message payload
 * @param a_metrics :[in] changed metrics for which to publish WOD message
 * @return true/false based on success/failure
 */
bool CSparkPlugDev::getWriteMsg(string& a_sTopic, cJSON *a_root, pair<const string,CMetric>& a_metric, const int& a_appSeqNo)
{
	try
	{
		string pubTopic = "";
		string pubMsg = "";

		vector<string> vParsedTopic = { };
		CCommon::getInstance().getTopicParts(getSparkPlugName(), vParsedTopic, "-");

		if(vParsedTopic.size() != 2)
		{
			cout << "Invalid device name to prepare WOD request" << endl;
			return false;
		}
		if(vParsedTopic[0].empty() || vParsedTopic[1].empty())
		{
			cout << "Invalid device name or site name to prepare WOD request" << endl;
			return false;
		}
		string strDevice = vParsedTopic[0];
		string strSite = vParsedTopic[1];
		string strMetricName = "";
		strMetricName.assign(a_metric.first);

		a_sTopic = "/" + strDevice + "/" + strSite + "/" + strMetricName + "/write";

		//there should be only one metric in this map for real device
		//since mqtt-export can support only one metric at a time
		//for(auto& itrMetric : a_metrics)
		{
			string a_strDataPoint = a_metric.second.getName().c_str();

			cJSON_AddItemToObject(a_root, "wellhead", cJSON_CreateString(vParsedTopic[1].c_str()));
			cJSON_AddItemToObject(a_root, "command", cJSON_CreateString(a_metric.second.getName().c_str()));

			if(false == a_metric.second.getValue().assignToCJSON(a_root))
			{
				return false;
			}

			time_t t = a_metric.second.getTimestamp() / 1000;

			char cTimestamp[100];
			auto ptrTime = std::localtime(&t);
			if(NULL != ptrTime)
			{
				if (std::strftime(cTimestamp, sizeof(cTimestamp), "%F %T", ptrTime))
				{
					cJSON_AddItemToObject(a_root, "timestamp", cJSON_CreateString(cTimestamp));
				}
				else
				{
					DO_LOG_ERROR("Cannot assign timestamp in write-on-demand request");
					return false;
				}
			}

			cJSON_AddItemToObject(a_root, "usec", cJSON_CreateString(std::to_string(a_metric.second.getTimestamp()).c_str()));

			cJSON_AddItemToObject(a_root, "version", cJSON_CreateString(CCommon::getInstance().getVersion().c_str()));

			cJSON_AddItemToObject(a_root, "app_seq", cJSON_CreateString(std::to_string(a_appSeqNo).c_str()));
		}
	}
	catch(exception& ex)
	{
		DO_LOG_FATAL(ex.what());
		return false;
	}
	return true;
}

/**
 * Prepare cJSON message for vendor app CMD msg to publish on internal MQTT
 * @param a_sTopic :[out] topic on which to publish the message
 * @param a_root :[out] cJSON reference containing message payload
 * @param a_metrics :[in] changed metrics for which to publish CMD message
 * @return true/false based on success/failure
 */
bool CSparkPlugDev::getCMDMsg(string& a_sTopic, metricMap_t& m_metrics, cJSON *metricArray)
{
	bool bRet = false;

	try
	{
		vector<string> vParsedTopic = { };
		CCommon::getInstance().getTopicParts(getSparkPlugName(), vParsedTopic, "-");

		if(vParsedTopic.size() != 2)
		{
			cout << "Invalid device name to prepare WOD request" << endl;
			return false;
		}
		if(vParsedTopic[0].empty() || vParsedTopic[1].empty())
		{
			cout << "Invalid device name or site name to prepare WOD request" << endl;
			return false;
		}

		//assuming site name exists after the last occurrence of -
		string device = vParsedTopic[0];
		string siteName = vParsedTopic[1];
		a_sTopic = "CMD/" + device + "/" + siteName;

		//these shall be part of a single sparkplug msg
		for (auto &itrMetric : m_metrics)
		{
			cJSON *cjMetric = cJSON_CreateObject();
			if(cjMetric == NULL )
			{
				DO_LOG_ERROR("Error while creating CJSON message");
				return false;
			}

			cJSON_AddItemToObject(cjMetric, "name", cJSON_CreateString(itrMetric.second.getName().c_str()));

			if(false == itrMetric.second.getValue().assignToCJSON(cjMetric))
			{
				DO_LOG_ERROR("Failed to assign value to CJSON");
				return false;
			}

			cJSON_AddItemToObject(cjMetric, "timestamp", cJSON_CreateNumber(itrMetric.second.getTimestamp()));

		    cJSON_AddItemToArray(metricArray, cjMetric);

		}//metric ends
		bRet = true;
	}
	catch(exception& ex)
	{
		DO_LOG_FATAL(ex.what());
		bRet = false;
	}

	return bRet;
}
