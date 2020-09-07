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
#include "SparkPlugDevMgr.hpp"

/**
 * Splits topic name with delimeter '/'
 * @param a_sTopic :[in] topic to split
 * @param a_vsTopicParts :[out] stores all the split names in this vector
 * @return true/false based on true/false
 */
bool CSparkPlugDevManager::getTopicParts(std::string a_sTopic,
		std::vector<std::string> &a_vsTopicParts)
{
	try
	{
		std::string delimiter = "/";
		size_t last = 0;
		size_t next = 0;
		while ((next = a_sTopic.find(delimiter, last)) != std::string::npos)
		{
			a_vsTopicParts.push_back(a_sTopic.substr(last, next - last));
			last = next + 1;
		}
		a_vsTopicParts.push_back(a_sTopic.substr(last));
	}
	catch (std::exception &e)
	{
		DO_LOG_ERROR(std::string("Error:") + e.what());
		a_vsTopicParts.clear();
		return false;
	}
	return true;
}

/**
 * Processes messages received on internal MQTT broker and returns metrics along with
 * action BIRTH, DATA and DEATH
 * @param a_sTopic : [in] topic of message received on internal MQTT
 * @param a_sPayLoad :[in] payload of message received on internal MQTT
 * @param a_stRefActionVec :[out] stores all the metrics in this structure
 * @return true/false based on success/failure
 */
bool CSparkPlugDevManager::processInternalMQTTMsg(std::string a_sTopic,
		std::string a_sPayLoad,
		std::vector<stRefForSparkPlugAction> &a_stRefActionVec)
{
	bool bRet = true;
	do
	{
		try
		{
			std::vector<std::string> vsTopicParts;
			getTopicParts(a_sTopic, vsTopicParts);
			switch (vsTopicParts.size())
			{
			// DEATH message
			// DEATH/{NAON_UWCP_ID}
			case 2:
			{
				std::string sSubTopic
				{ vsTopicParts[0] };
				std::transform(sSubTopic.begin(), sSubTopic.end(),
						sSubTopic.begin(), ::tolower);

				if (0 == sSubTopic.compare("death"))
				{
					processDeathMsg(vsTopicParts[1], a_stRefActionVec);
				}
				else
				{
					DO_LOG_ERROR(
							"Ignoring the message. Incorrect topic: "
									+ a_sTopic);
					bRet = false;
				}
			}
				break;

				// BIRTH and DATA message:
				// BIRTH/{NAON_UWCP_ID}/{WellheadID}
				// DATA/{NAON_UWCP_ID}/{WellheadID}
			case 3:
			{
				std::string sSubTopic
				{ vsTopicParts[0] };
				std::transform(sSubTopic.begin(), sSubTopic.end(),
						sSubTopic.begin(), ::tolower);

				if (0 == sSubTopic.compare("birth"))
				{
					processBirthMsg(vsTopicParts[1], vsTopicParts[2],
							a_sPayLoad, a_stRefActionVec);
				}
				else if (0 == sSubTopic.compare("data"))
				{
					processDataMsg(vsTopicParts[1], vsTopicParts[2], a_sPayLoad,
							a_stRefActionVec);
				}
				else
				{
					DO_LOG_ERROR(
							"Ignoring the message. Incorrect topic: "
									+ a_sTopic);
					bRet = false;
				}
			}
				break;

				// update message:
				// /device/wellhead/point/update
				// /device/wellhead/point/readResponse
				// /device/wellhead/point/writeResponse
			case 5:
				break;

			default:
				DO_LOG_ERROR(
						"Ignoring the message. Incorrect topic: " + a_sTopic)
				;
				bRet = false;
				break;
			}
		} catch (std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
			bRet = false;
		}
	} while (0);
	return bRet;
}

/**
 * Processes metric to parse its data-type and value; sets in CValueObj corresponding to the metric
 * @param a_oMetric :[in] metric to be parsed
 * @param a_cjName :[in] name of metric
 * @param a_cjDatatype :[in] data-type of metric
 * @param a_cjValue :[in] metric value
 * @return true/false based on success/failure
 */
bool CSparkPlugDevManager::processMetric(CMetric &a_oMetric, cJSON *a_cjName,
		cJSON *a_cjDatatype, cJSON *a_cjValue)
{
	do
	{
		try
		{
			if (a_cjName == NULL || a_cjDatatype == NULL || a_cjValue == NULL)
			{
				DO_LOG_ERROR(
						"Invalid payload: Name or datatype or value is missing.");
				return false;
			}

			char *strName = a_cjName->valuestring;
			if (strName == NULL)
			{
				DO_LOG_ERROR("Invalid payload: Name is not found.");
				return false;
			}
			a_oMetric.setName(strName);
			if (a_cjDatatype->valuestring == NULL)
			{
				DO_LOG_ERROR("Datatype is NULL. Cannot parse payload.");
				return false;
			}
			std::string sDataType
			{ a_cjDatatype->valuestring };

			//map data-type with Sparkplug data-type
			if (false
					== a_oMetric.setValObj(a_cjDatatype->valuestring,
							a_cjValue))
			{
				DO_LOG_ERROR("Cannot parse value.");
				return false;
			}
		} catch (std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
			return false;
		}
	} while (0);
	return true;
}

/**
 * Gets instance of CSparkPlugDevManager
 * @return instance of CSparkPlugDevManager
 */
CSparkPlugDevManager& CSparkPlugDevManager::getInstance()
{
	static CSparkPlugDevManager _self;
	return _self;
}

/**
 * Parses birth message of vendor app; stores metrics and corresponding values
 * @param a_sPayLoad :[iin] payload containing metrics
 * @return map containing metric and corresponding values
 */
metricMap_t CSparkPlugDevManager::parseVendorAppBirthMessage(
		std::string a_sPayLoad)
{
	metricMap_t oMetricMap;
	do
	{
		try
		{
			cJSON *cjRoot = cJSON_Parse(a_sPayLoad.c_str());
			if (NULL == cjRoot)
			{
				DO_LOG_ERROR(
						"Message received from MQTT could not be parsed in json format");
				break;
			}

			//start parsing the cjson payload
			cJSON *cjMetrics = cJSON_GetObjectItem(cjRoot, "metrics");
			if (NULL == cjMetrics)
			{
				DO_LOG_ERROR(
						"No \"metrics\" key found in input message: "
								+ a_sPayLoad);
				break;
			}
			int iTotalMetrics = cJSON_GetArraySize(cjMetrics);
			for (int iLoop = 0; iLoop < iTotalMetrics; ++iLoop)
			{
				cJSON *cjArrayElemMetric = cJSON_GetArrayItem(cjMetrics, iLoop);
				if (NULL != cjArrayElemMetric)
				{
					cJSON *cjName = cJSON_GetObjectItem(cjArrayElemMetric,
							"name");
					cJSON *cjDataType = cJSON_GetObjectItem(cjArrayElemMetric,
							"dataType");
					cJSON *cjValue = cJSON_GetObjectItem(cjArrayElemMetric,
							"value");

					CMetric oMetric;
					if (false
							== processMetric(oMetric, cjName, cjDataType,
									cjValue))
					{
						DO_LOG_ERROR("Cannot parse payload");
					}
					else
					{
						oMetricMap.emplace(oMetric.getName(), oMetric);
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
 * Processes DEATH message from vendor app
 * @param a_sAppName :[in] vendor app name for which DEATH has received
 * @param a_stRefActionVec :[out] structure containing device information to be marked dead
 */
void CSparkPlugDevManager::processDeathMsg(std::string a_sAppName,
		std::vector<stRefForSparkPlugAction> &a_stRefActionVec)
{
	metricMap_t mapChangedMetricsFromBirth;
	do
	{
		try
		{
			if (a_sAppName.empty())
			{
				DO_LOG_ERROR("App-name is empty");
				break;
			}

			CVendorApp *pVendorApp = m_objVendorAppList.getVendorApp(
					a_sAppName);
			if (NULL == pVendorApp)
			{
				DO_LOG_ERROR(
						a_sAppName
								+ ": Vendor app not found to process DEATH message. Ignoring.");
				break;
			}

			metricMap_t mapChangedMetrics;
			// mark all subdevices under this vendor app for DEATH
			auto &mapDev = pVendorApp->getDevices();
			for (auto &itr : mapDev)
			{
				stRefForSparkPlugAction stDummyAction
				{ std::ref(itr.second.get()), enMSG_DEATH, mapChangedMetrics };
				a_stRefActionVec.push_back(stDummyAction);
			}
		} catch (const std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
		}
	} while (0);

	//return mapChangedMetricsFromBirth;
}

/**
 * Processes device birth message and updates storage accordingly
 * @param a_sAppName :[in] vendor app name
 * @param a_sSubDev :[in] sub device name
 * @param a_sPayLoad :[in] payload of birth message from vendor app
 * @param a_stRefActionVec : [out] map containing metric and corresponding values
 */
void CSparkPlugDevManager::processBirthMsg(std::string a_sAppName,
		std::string a_sSubDev, std::string a_sPayLoad,
		std::vector<stRefForSparkPlugAction> &a_stRefActionVec)
{
	// There could be following cases:
	// Case 1: All are new metrics => This results in a DBIRTH message
	// Case 2: Datatype of few metrics are changed => This results in a DBIRTH message
	// Case 3: Only values of few metrics are changed. There are no other changes => This results in a DDATA message
	// Case 4: There are no changes. 
	
	metricMap_t mapChangedMetricsFromBirth;
	do
	{
		try
		{
			if (a_sAppName.empty())
			{
				DO_LOG_ERROR("App-name is empty");
				break;
			}
			if (a_sSubDev.empty())
			{
				DO_LOG_ERROR("Subdevice is empty");
				break;
			}

			std::string sDevName(
					a_sAppName + SUBDEV_SEPARATOR_CHAR + a_sSubDev);
			DO_LOG_INFO("Device name is: " + sDevName);

			// Parse message
			metricMap_t mapMetricsInMsg = parseVendorAppBirthMessage(
					a_sPayLoad);

			// Find the device in list
			bool bIsNew = false;
			auto itr = m_mapSparkPlugDev.find(sDevName);
			if (m_mapSparkPlugDev.end() == itr)
			{
				bIsNew = true;
				DO_LOG_INFO("New device found: " + sDevName);
				CSparkPlugDev oDev
				{ a_sSubDev, sDevName, true };
				m_mapSparkPlugDev.emplace(sDevName, oDev);
				itr = m_mapSparkPlugDev.find(sDevName);

				m_objVendorAppList.addDevice(a_sAppName, itr->second);
			}
			auto &oDev = itr->second;
			bool bIsOnlyValChange = false;
			mapChangedMetricsFromBirth = oDev.processNewBirthData(
					mapMetricsInMsg, bIsOnlyValChange);

			// Check if any changes have occurred
			if (0 != mapChangedMetricsFromBirth.size())
			{
				// Check if it is BIRTH message 
				if (false == bIsOnlyValChange)
				{
					// Check if Device is new or not
					if (false == bIsNew)
					{
						// Not a new device. First send a DDEATH message
						// Changes have occurred. 1st publish a DDEATH message.
						metricMap_t mapChangedMetrics;
						stRefForSparkPlugAction stDummyAction
						{ std::ref(oDev), enMSG_DEATH, mapChangedMetrics };
						a_stRefActionVec.push_back(stDummyAction);
					}

					// Action for DBIRTH message
					stRefForSparkPlugAction stDummyAction
					{ std::ref(oDev), enMSG_BIRTH, mapChangedMetricsFromBirth };
					a_stRefActionVec.push_back(stDummyAction);
				}
				else
				{
					// Action for DDATA message
					stRefForSparkPlugAction stDummyAction
					{ std::ref(oDev), enMSG_DATA, mapChangedMetricsFromBirth };
					a_stRefActionVec.push_back(stDummyAction);
				}
			}
		} catch (const std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
		}
	} while (0);

	//return mapChangedMetricsFromBirth;
}

/**
 * Processes DATA message received from vendor app and updates storage accordingly
 * @param a_sAppName :[in] vendor app name
 * @param a_sSubDev :[in] sub device name
 * @param a_sPayLoad :[in] payload of DATA message
 * @param a_stRefActionVec :[out] map containing metric and corresponding values
 */
void CSparkPlugDevManager::processDataMsg(std::string a_sAppName,
		std::string a_sSubDev, std::string a_sPayLoad,
		std::vector<stRefForSparkPlugAction> &a_stRefActionVec)
{
	metricMap_t mapChangedMetricsFromData;
	do
	{
		try
		{
			if (a_sAppName.empty())
			{
				DO_LOG_ERROR("App-name is empty");
				break;
			}
			if (a_sSubDev.empty())
			{
				DO_LOG_ERROR("Subdevice is empty");
				break;
			}
			std::string sDevName(a_sAppName + "-" + a_sSubDev);
			DO_LOG_INFO("Device name is: " + sDevName);

			// Find the device in list
			auto itr = m_mapSparkPlugDev.find(sDevName);
			if (m_mapSparkPlugDev.end() == itr)
			{
				DO_LOG_ERROR(
						sDevName
								+ ": Not found in dev-ist. Ignoring DATA message: "
								+ a_sPayLoad);
			}
			else
			{
				// Parse message
				metricMap_t mapMetricsInMsg = parseVendorAppBirthMessage(
						a_sPayLoad);

				if (mapMetricsInMsg.size() > 0)
				{
					auto &oDev = itr->second;
					mapChangedMetricsFromData = oDev.processNewData(
							mapMetricsInMsg);

					if (0 != mapChangedMetricsFromData.size())
					{
						// Action for DDATA message
						stRefForSparkPlugAction stDummyAction
						{ std::ref(oDev), enMSG_DATA, mapChangedMetricsFromData };
						a_stRefActionVec.push_back(stDummyAction);
					}
				}
			}
		} catch (const std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
		}
	} while (0);

	//return mapChangedMetricsFromData;
}

/**
 * Helper function to print devices, its actions and metrics
 * @param a_stRefActionVec :[in] map which needs to be printed
 */
void CSparkPlugDevManager::printRefActions(std::vector<stRefForSparkPlugAction> &a_stRefActionVec)
{
	/*std::cout
			<< "--------------------------------- Printing actions ---------------------------------\n";
	for (auto &itr : a_stRefActionVec)
	{
		std::cout << "**********************\n";
		std::cout << "Device: "
				<< itr.m_refSparkPlugDev.get().getSparkPlugName()
				<< "\tAction: " << itr.m_enAction << "\tChanged metrics:\n";

		for (auto &itrMetric : itr.m_mapChangedMetrics)
		{
			itrMetric.second.print();
		}
		std::cout << "**********************\n";
	}
	std::cout << "---------------------------------\n";*/
}
