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
 * @return true/false depending on the success/failure
 */
bool CSparkPlugDev::prepareDBirthMessage(org_eclipse_tahu_protobuf_Payload& a_rTahuPayload)
{
	try
	{
		{
			std::lock_guard<std::mutex> lck(m_mutexMetricList);
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
