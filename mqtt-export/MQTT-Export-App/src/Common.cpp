/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#include "ConfigManager.hpp"
#include "Common.hpp"

/**
 * Constructor initializes CCommon instance and retrieves common environment variables
 * @param None
 * @return None if successful;
 * 			In case of error or exception, application exits
 */
CCommon::CCommon()
{
	EnvironmentInfo::getInstance().readCommonEnvVariables(m_vecEnv);
	string strDevMode = EnvironmentInfo::getInstance().getDataFromEnvMap("DEV_MODE");
	transform(strDevMode.begin(), strDevMode.end(), strDevMode.begin(), ::toupper);
	if (strDevMode == "TRUE")
	{
		setDevMode(true);
		DO_LOG_INFO("DEV_MODE is set to true");
		cout << "DEV_MODE is set to true\n";

	}
	else if (strDevMode == "FALSE")
	{
		setDevMode(false);
		DO_LOG_INFO("DEV_MODE is set to false");
		cout << "DEV_MODE is set to false\n";
	}
	else
	{
		// default set to false
		DO_LOG_ERROR("Invalid value for DEV_MODE env variable");
		DO_LOG_INFO("Set the dev mode to default (i.e. true)");
		cout << "DEV_MODE is set to default false\n";
	}

	string strAppName = EnvironmentInfo::getInstance().getDataFromEnvMap("AppName");
	if(strAppName.empty())
	{
		DO_LOG_ERROR("AppName Environment Variable is not set");
		std::cout << __func__ << ":" << __LINE__ << " Error : AppName Environment Variable is not set" <<  std::endl;
		exit(1);
	}
	initializeCommonData(strDevMode, strAppName);
}

/**
 *
 * DESCRIPTION
 * Function to initialise the structure values of stUWCComnDataVal_t of uwc-lib
 *
 * @param strDevMode	[in] describes is devMode enabled
 * @param strAppName	[in] application name
 *
 * @return
 */
void CCommon::initializeCommonData(string strDevMode, string strAppName)
{
	stUWCComnDataVal_t stUwcData;
	stUwcData.m_devMode = false;
	stUwcData.m_sAppName = strAppName;
	stUwcData.m_isCommonDataInitialised = true;
	transform(strDevMode.begin(), strDevMode.end(), strDevMode.begin(), ::toupper);
	if("TRUE" == strDevMode)
	{
		stUwcData.m_devMode = true;
	}
	CcommonEnvManager::Instance().ShareToLibUwcCmnData(stUwcData);
}

/**
 * Destructor
 */
CCommon::~CCommon()
{
}

/**
 * Get current epoch time in string
 * @param strCurTime :[out] set current epoch time in string format
 * @return None
 */
void CCommon::getCurrentTimestampsInString(std::string &strCurTime)
{
	try
	{
		struct timespec tsMsgReceived;
		timespec_get(&tsMsgReceived, TIME_UTC);
		strCurTime = std::to_string(get_micros(tsMsgReceived));
	}
	catch(exception &e)
	{
		DO_LOG_FATAL("Cannot get current time in string :: " +
				std::string(e.what()));
	}
}

/**
 * Add current time stamp in message payload
 * @param a_sMsg 		:[in] message in which to add time
 * @param tsKey			:[in] key name against which to add time stamp
 * @param strTimestamp	:[in] time stamp in string format
 * @return true/false based on success/failure
 */
bool CCommon::addTimestampsToMsg(std::string &a_sMsg, string tsKey, string strTimestamp)
{
	cJSON *root = NULL;
	try
	{
		root = cJSON_Parse(a_sMsg.c_str());
		if (NULL == root)
		{
			DO_LOG_ERROR("ZMQ Message could not be parsed in json format");
			return false;
		}

		if(NULL == cJSON_AddStringToObject(root, tsKey.c_str(), strTimestamp.c_str()))
		{
			DO_LOG_ERROR("Could not add timestamp in message");
			if(root != NULL)
			{
				cJSON_Delete(root);
			}
			return false;
		}

		a_sMsg.clear();
		char *psNewJson = cJSON_Print(root);
		if(NULL != psNewJson)
		{
			a_sMsg.assign(psNewJson);
			free(psNewJson);
			psNewJson = NULL;
		}

		if(root != NULL)
		{
			cJSON_Delete(root);
		}

		DO_LOG_DEBUG("Added timestamp in payload for MQTT");
		return true;

	}
	catch (exception &ex)
	{
		DO_LOG_DEBUG("Failed to add timestamp in payload for MQTT" + std::string(ex.what()));

		if(root != NULL)
		{
			cJSON_Delete(root);
		}

		return false;
	}
}
