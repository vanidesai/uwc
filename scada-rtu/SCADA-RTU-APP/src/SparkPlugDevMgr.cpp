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
#include "SCADAHandler.hpp"

/**
 * Splits topic name with delimeter '/'
 * @param a_sTopic :[in] topic to split
 * @param a_vsTopicParts :[out] stores all the split names in this vector
 * @return true/false based on true/false
 */
bool CSparkPlugDevManager::getTopicParts(std::string a_sTopic,
		std::vector<std::string> &a_vsTopicParts, const string& a_delimeter)
{
	try
	{
		std::string delimiter = "/";
		if(! a_delimeter.empty())
		{
			delimiter = a_delimeter;
		}
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
bool CSparkPlugDevManager::processExternalMQTTMsg(std::string a_sTopic,
		org_eclipse_tahu_protobuf_Payload& a_payload,
		std::vector<stRefForSparkPlugAction> &a_stRefActionVec)
{
	metricMap_t oMetricMap;

	try
	{
		//parse topic to get site name and device name
		vector<string> vSplitTopic;

		if(false == getTopicParts(a_sTopic, vSplitTopic, "/"))
		{
			DO_LOG_ERROR("Could not split topic name");
			return false;
		}

		if(vSplitTopic.size() != 5)
		{
			DO_LOG_ERROR("Invalid topic while checking for device validity");
			return false;
		}

		//split device-site name into device and site
		vector<string> mDevName;
		//device-site
		string strDevName = vSplitTopic[4];
		getTopicParts(strDevName, mDevName, "-");

		bool bIsFound = false;
		auto itr = [&] () mutable -> devSparkplugMap_t::iterator
		{
			bIsFound = false;
			std::lock_guard<std::mutex> lck(m_mutexDevList);
			auto i = m_mapSparkPlugDev.find(strDevName);
			if (m_mapSparkPlugDev.end() != i)
			{
				bIsFound = true;
			}
			return i;
		}();

		if(false == bIsFound)
		{
			DO_LOG_ERROR("Device is not present in list : " + strDevName);
			return false;
		}
		
		for (pb_size_t i = 0; i < a_payload.metrics_count; i++)
		{
			CMetric oMetric;
			if (false == processDCMDMetric(oMetric, a_payload.metrics[i]))
			{
				DO_LOG_DEBUG("Could not process metric");
				return false;
			}
			else
			{
				std::map<std::string, CMetric> dcmdMetricMap = {{oMetric.getName(), oMetric}};

				oMetricMap.insert(dcmdMetricMap.begin(), dcmdMetricMap.end());
			}
		}

		auto &mapSparkPlugDev = itr->second;
		stRefForSparkPlugAction stCMDAction
		{ std::ref(mapSparkPlugDev), enMSG_DCMD_WRITE, oMetricMap };
		a_stRefActionVec.push_back(stCMDAction);
	}
	catch (std::exception &e)
	{
		DO_LOG_FATAL(e.what());
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
			getTopicParts(a_sTopic, vsTopicParts, "/");
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
					processDeathMsg(vsTopicParts[1], a_sPayLoad, a_stRefActionVec);
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
				{
					std::string sSubTopic{ vsTopicParts[4] };
					std::transform(sSubTopic.begin(), sSubTopic.end(), sSubTopic.begin(), ::tolower);

					if (0 == sSubTopic.compare("update"))
					{
						processUpdateMsg(vsTopicParts[1], vsTopicParts[2], a_sPayLoad,
								a_stRefActionVec);
					}
				}
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
 * @param a_cjArrayElemMetric :[in] cJSON array element containing details about the metric
 * @return true/false based on success/failure
 */
bool CSparkPlugDevManager::processMetric(CMetric &a_oMetric, cJSON *a_cjArrayElemMetric)
{
	do
	{
		try
		{
			if (a_cjArrayElemMetric == NULL)
			{
				DO_LOG_ERROR("Invalid payload: Name or datatype or value is missing.");
				return false;
			}

			cJSON *cjName = cJSON_GetObjectItem(a_cjArrayElemMetric, "name");
			char *strName = cjName->valuestring;
			if (strName == NULL)
			{
				DO_LOG_ERROR("Invalid payload: Name is not found.");
				return false;
			}
			a_oMetric.setName(strName);
			a_oMetric.setSparkPlugName(strName);

			//timestamp is optional parameter
			uint64_t current_time = 0;
			cJSON *cjTimestamp = cJSON_GetObjectItem(a_cjArrayElemMetric,"timestamp");
			if(cjTimestamp == NULL)
			{
				current_time = get_current_timestamp();
			}
			else
			{
				current_time = cjTimestamp->valuedouble;
			}

			a_oMetric.setTimestamp(current_time);

			cJSON *cjDataType = cJSON_GetObjectItem(a_cjArrayElemMetric, "dataType");
			if (cjDataType == NULL)
			{
				DO_LOG_ERROR("Invalid payload: dataType is not found.");
				return false;
			}

			if (cjDataType->valuestring == NULL)
			{
				DO_LOG_ERROR("Datatype is NULL. Cannot parse payload.");
				return false;
			}

			std::string sDataType{ cjDataType->valuestring };

			cJSON *cjValue = cJSON_GetObjectItem(a_cjArrayElemMetric, "value");
			if (cjValue == NULL)
			{
				DO_LOG_ERROR("Invalid payload: value is not found.");
				return false;
			}

			//map data-type with Sparkplug data-type
			if (false == a_oMetric.setValObj(sDataType, cjValue))
			{
				DO_LOG_ERROR("Cannot parse value.");
				return false;
			}
		}
		catch (std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
			return false;
		}
	} while (0);
	return true;
}

/**
 * Processes metric to parse its data-type and value from real device update msg
 * sets in CValueObj corresponding to the metric
 * @param a_oMetric :[in] metric to be parsed
 * @param a_cjRoot :[in] cJSON element containing details about the metric
 * @return true/false based on success/failure
 */
bool CSparkPlugDevManager::processMetricFromUpdateMsg(CMetric &a_oMetric, cJSON *a_cjRoot)
{
	do
	{
		try
		{
			if (a_cjRoot == NULL)
			{
				DO_LOG_ERROR("Invalid payload: Name or datatype or value is missing.");
				return false;
			}

			//in case of error, memory will be freed in callee function
			cJSON *cjName = cJSON_GetObjectItem(a_cjRoot, "metric");
			if(cjName == NULL)
			{
				DO_LOG_ERROR("Invalid payload: metric is not found.");
				return false;
			}
			char *strName = cjName->valuestring;
			if (strName == NULL)
			{
				DO_LOG_ERROR("Invalid payload: metric name is not found.");
				return false;
			}
			a_oMetric.setName(strName);
			a_oMetric.setSparkPlugName(strName);

			//timestamp is optional parameter
			uint64_t current_time = 0;
			cJSON *cjTimestamp = cJSON_GetObjectItem(a_cjRoot,"timestamp");
			if(cjTimestamp == NULL)
			{
				current_time = get_current_timestamp();
			}
			else
			{
				// Initialize result
				uint64_t res = 0;
				string strTimeStamp = cjTimestamp->valuestring;

				if(strTimeStamp.empty())
				{
					DO_LOG_ERROR("Invalid timestamp received in update message");
					return false;
				}

				//Iterate through all characters
				//of input string and update result
				for (int i = 0; cjTimestamp->valuestring[i]!= '\0';++i)
					res = res * 10 + cjTimestamp->valuestring[i] - '0';

				current_time = res;
			}

			a_oMetric.setTimestamp(current_time);

			//consider data-type as string for update msg as it does not contain datatype
			std::string sDataType{ "String" };

			cJSON *cjValue = cJSON_GetObjectItem(a_cjRoot, "value");
			if (cjValue == NULL)
			{
				DO_LOG_ERROR("Invalid payload: value is not found.");
				return false;
			}

			//map data-type with Sparkplug data-type
			if (false == a_oMetric.setValObj(sDataType, cjValue))
			{
				DO_LOG_ERROR("Cannot parse value.");
				return false;
			}
		}
		catch (std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
			return false;
		}
	} while (0);
	return true;
}

/**
 * Processes metric to parse its data-type and value; sets in CValueObj corresponding to the metric
 * @param a_oMetric :[in] metric to be parsed
 * @param a_cjName :[in] name of metric
 * @param a_cjDatatype :[in] data-type of metric
 * @param a_cjValue :[in] metric value
 * @return true/false based on success/failure
 */
bool CSparkPlugDevManager::processDCMDMetric(CMetric &a_oMetric, org_eclipse_tahu_protobuf_Payload_Metric& a_sparkplugMetric)
{
	try
	{
		if(a_sparkplugMetric.name == NULL)
		{
			return false;
		}

		a_oMetric.setName(a_sparkplugMetric.name);

		a_oMetric.setTimestamp(a_sparkplugMetric.timestamp);

		//map data-type with Sparkplug data-type
		if (false == a_oMetric.setValObj(a_sparkplugMetric))
		{
			DO_LOG_ERROR("Cannot parse value.");
			return false;
		}
	}
	catch (std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
			return false;
		}
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
metricMap_t CSparkPlugDevManager::parseVendorAppBirthMessage(std::string a_sPayLoad)
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
					CMetric oMetric;
					if (false == processMetric(oMetric, cjArrayElemMetric))
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
 * Parses real device update message and stores metrics and corresponding values
 * @param a_sPayLoad :[iin] payload containing metrics
 * @return map containing metric and corresponding values
 */
CMetric CSparkPlugDevManager::parseRealDeviceUpdateMsg(std::string a_sPayLoad)
{
	CMetric oMetric;
	cJSON *root = NULL;
	try
	{
		root = cJSON_Parse(a_sPayLoad.c_str());
		if (NULL == root)
		{
			DO_LOG_ERROR("Message received from MQTT could not be parsed in json format");
			return oMetric;
		}

		//consider processing for vendor app array of metrics if
		//root is array
		//else if root is not array treat it as update msg with
		//single metric
		if(cJSON_IsArray(root))
		{
			//json has needed values, fill in map
			cJSON *param = root->child;
			if(param != NULL)
			{
				processMetric(oMetric, param);
			}
			else
			{
				DO_LOG_ERROR("Param is null in update msg parsing");
			}
		}
		else
		{
			processMetricFromUpdateMsg(oMetric, root);
		}

	}
	catch (std::exception &ex)
	{
		DO_LOG_FATAL(ex.what());
	}

	if (NULL != root)
	{
		cJSON_Delete(root);
	}

	return oMetric;
}
/**
 * Parses birth message of vendor app; stores metrics and corresponding values
 * @param a_sPayLoad :[iin] payload containing metrics
 * @return map containing metric and corresponding values
 */
uint64_t CSparkPlugDevManager::parseVendorAppDeathMessage(std::string& a_sPayLoad)
{
	uint64_t uiDeathTime = 0;
	cJSON *root = NULL;

	try
	{
		root = cJSON_Parse(a_sPayLoad.c_str());
		if (NULL == root)
		{
			DO_LOG_ERROR("Message received from MQTT could not be parsed in json format");
			return get_current_timestamp();
		}

		if (!cJSON_HasObjectItem(root, "timestamp"))
		{
			uiDeathTime = get_current_timestamp();
		}
		else
		{
			uiDeathTime = cJSON_GetObjectItem(root, "timestamp")->valuedouble;
		}
	}
	catch(std::exception &ex)
	{
		DO_LOG_FATAL(ex.what());
	}

	if (NULL != root)
	{
		cJSON_Delete(root);
	}

	return uiDeathTime;
}

/**
 * Processes DEATH message from vendor app
 * @param a_sAppName :[in] vendor app name for which DEATH has received
 * @param a_stRefActionVec :[out] structure containing device information to be marked dead
 */
void CSparkPlugDevManager::processDeathMsg(std::string a_sAppName,
		std::string a_sPayLoad,
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

			CVendorApp *pVendorApp = NULL;
			{
				std::lock_guard<std::mutex> lck(m_mutexDevList);
				pVendorApp = m_objVendorAppList.getVendorApp(a_sAppName);
			}
			if (NULL == pVendorApp)
			{
				DO_LOG_ERROR(
						a_sAppName
								+ ": Vendor app not found to process DEATH message. Ignoring.");
				break;
			}

			uint64_t uiDeathTime = parseVendorAppDeathMessage(a_sPayLoad);

			metricMap_t mapChangedMetrics;
			// mark all subdevices under this vendor app for DEATH
			auto &mapDev = pVendorApp->getDevices();
			for (auto &itr : mapDev)
			{
				itr.second.get().setDeathTime(uiDeathTime);

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
	// Case 5: Last published status was DDEATH. Then a next message should be DBIRTH
	
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
			metricMap_t mapMetricsInMsg = parseVendorAppBirthMessage(a_sPayLoad);

			// Find the device in list
			bool bIsNew = false;

			auto& oDev = [&] () mutable -> CSparkPlugDev&
			{
				std::lock_guard<std::mutex> lck(m_mutexDevList);
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

				return itr->second;
			}();
			bool bIsOnlyValChange = false;
			mapChangedMetricsFromBirth = oDev.processNewBirthData(
					mapMetricsInMsg, bIsOnlyValChange);

			// Check if last published status was DDEATH
			if(oDev.getLastPublishedDevStatus() == enDEVSTATUS_DOWN)
			{
				// If last status was DDEATH, then a DBIRTH should be published
				stRefForSparkPlugAction stDummyAction
				{ std::ref(oDev), enMSG_BIRTH, mapChangedMetricsFromBirth };
				a_stRefActionVec.push_back(stDummyAction);

				break;
			}
			// Check if any changes have occurred
			else if (0 != mapChangedMetricsFromBirth.size())
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
 * Processes real device update message and updates storage accordingly
 * @param a_sDeviceName :[in] real device name
 * @param a_sSubDev :[in] sub device name
 * @param a_sPayLoad :[in] payload of update message from real device
 * @param a_stRefActionVec : [out] map containing metric and corresponding values
 */
void CSparkPlugDevManager::processUpdateMsg(std::string a_sDeviceName,
		std::string a_sSubDev, std::string a_sPayLoad,
		std::vector<stRefForSparkPlugAction> &a_stRefActionVec)
{
	// There could be following cases:
	// Case 1: Only values of few metrics are changed => This results in no DDATA message
	// Case 2: There are no changes => No DDATA message

	metricMap_t mapChangedMetricsFromUpdate;
	do
	{
		try
		{
			if (a_sDeviceName.empty())
			{
				DO_LOG_ERROR("App-name is empty. Ignoring.");
				break;
			}
			if (a_sSubDev.empty())
			{
				DO_LOG_ERROR("Subdevice is empty. Ignoring.");
				break;
			}

			std::string sDevName(a_sDeviceName + SUBDEV_SEPARATOR_CHAR + a_sSubDev);
			DO_LOG_INFO("Device name is: " + sDevName);

			// Parse message and get metric info
			CMetric oMetric = parseRealDeviceUpdateMsg(a_sPayLoad);

			metricMap_t mapMetricsInMsg;
			mapMetricsInMsg.insert({oMetric.getName(), oMetric});

			// Find the device in list
			bool bIsFound = false;			
			auto itr = [&] () mutable -> devSparkplugMap_t::iterator
			{
				bIsFound = false;
				std::lock_guard<std::mutex> lck(m_mutexDevList);
				auto i = m_mapSparkPlugDev.find(sDevName);
				if (m_mapSparkPlugDev.end() != i)
				{
					bIsFound = true;
				}
				return i;
			}();

			if (false == bIsFound)
			{
				DO_LOG_ERROR("Invalid real device. Ignoring.");
				break;
			}

			auto &oDev = itr->second;
			bool bIsOnlyValChange = false;
			mapChangedMetricsFromUpdate = oDev.processNewBirthData(mapMetricsInMsg, bIsOnlyValChange);

			// Check if any changes have occurred
			if (0 != mapChangedMetricsFromUpdate.size())
			{
				// Check if it is BIRTH message
				if (true == bIsOnlyValChange)
				{
					DO_LOG_DEBUG("Real device value has been changed in update msg.");

					stRefForSparkPlugAction stDummyAction
					{ std::ref(oDev), enMSG_DATA, mapChangedMetricsFromUpdate};
					a_stRefActionVec.push_back(stDummyAction);
				}
				else
				{
					DO_LOG_ERROR("Real device update message is wrong.");
					break;
				}
			}
		}
		catch (const std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
		}
	} while (0);
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
			bool bIsFound = false;
			auto itr = [&] () mutable -> devSparkplugMap_t::iterator
			{
				bIsFound = false;
				std::lock_guard<std::mutex> lck(m_mutexDevList);
				auto i = m_mapSparkPlugDev.find(sDevName);
				if (m_mapSparkPlugDev.end() != i)
				{
					bIsFound = true;
				}
				return i;
			}();
			if (false == bIsFound)
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

					// Check if last published status was DDEATH
					if(oDev.getLastPublishedDevStatus() == enDEVSTATUS_DOWN)
					{
						// If last status was DDEATH, then a DBIRTH should be published
						stRefForSparkPlugAction stDummyAction
						{ std::ref(oDev), enMSG_BIRTH, mapChangedMetricsFromData };
						a_stRefActionVec.push_back(stDummyAction);
					}
					else if (0 != mapChangedMetricsFromData.size())
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

/**
 * Function to add a real Modbus devices as SparkPlu devices
 * @return true/false depending on the success/failure
 */
bool CSparkPlugDevManager::addRealDevices()
{
	do
	{
		try
		{
			using network_info::CUniqueDataPoint;

			std::lock_guard<std::mutex> lck(m_mutexDevList);
			auto &mapUniqueDevice =	network_info::getUniqueDeviceList();
			for (auto &rUniqueDev : mapUniqueDevice)
			{
				std::string sUniqueDev{rUniqueDev.second.getWellSiteDev().getID() + SUBDEV_SEPARATOR_CHAR + 
						rUniqueDev.second.getWellSite().getID()};

				auto itr = m_mapSparkPlugDev.find(sUniqueDev);
				// Create a new device, if not present
				if (m_mapSparkPlugDev.end() == itr)
				{
					DO_LOG_INFO("New device found: " + sUniqueDev);
					CSparkPlugDev oDev{ rUniqueDev.second, sUniqueDev };
					m_mapSparkPlugDev.emplace(sUniqueDev, oDev);

					// Now add datapoints to SparkPlug device as a metric
					auto& rSparkPlugDev = m_mapSparkPlugDev.at(sUniqueDev);
					auto& rPointList = rUniqueDev.second.getPoints();
					for (auto &rPoint : rPointList)
					{
						rSparkPlugDev.addMetric(rPoint.get());
					}
				}
				else
				{
					DO_LOG_ERROR(sUniqueDev + ": Repeat device found. Ignoring recent instance.");
					//return false;
				}
				
				//return true;
			}
		} 
		catch (const std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
		}
	} while (0);
	return true;
}

/**
 * Prepare device birth messages to be published on SCADA system
 * @param a_rTahuPayload :[out] reference of spark plug message payload in which to store birth messages
 * @param  a_sDevName:[in] device name for which birth message to be generated
 * @return true/false depending on the success/failure
 */
bool CSparkPlugDevManager::prepareDBirthMessage(org_eclipse_tahu_protobuf_Payload& a_rTahuPayload, std::string a_sDevName)
{
	try
	{
		std::lock_guard<std::mutex> lck(m_mutexDevList);
		auto itr = m_mapSparkPlugDev.find(a_sDevName);
		// Check if device is found
		if (m_mapSparkPlugDev.end() != itr)
		{
			return itr->second.prepareDBirthMessage(a_rTahuPayload);
		}
	}
	catch(exception &ex)
	{
		DO_LOG_FATAL(ex.what());
		return false;
	}
	return false;
}

/**
 * Sets the status of last published message for thsi device
 * @param a_enStatus :[in] Publish status for this device for last message
 * @param  a_sDevName:[in] device name for which birth message to be generated
 * @return true/false depending on the success/failure
 */
bool CSparkPlugDevManager::setMsgPublishedStatus(eDevStatus a_enStatus, std::string a_sDevName)
{
	try
	{
		std::lock_guard<std::mutex> lck(m_mutexDevList);
		auto itr = m_mapSparkPlugDev.find(a_sDevName);
		// Check if device is found
		if (m_mapSparkPlugDev.end() != itr)
		{
			itr->second.setPublishedStatus(a_enStatus);
		}
	}
	catch(exception &ex)
	{
		DO_LOG_FATAL(ex.what());
		return false;
	}
	return true;
}
