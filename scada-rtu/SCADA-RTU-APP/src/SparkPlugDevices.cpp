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
