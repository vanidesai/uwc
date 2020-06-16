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

	if(false == readCommonEnvVariables())
	{
		std::cout << "Error while reading common environment variables, exiting application" << std::endl;
		exit(-1);
	}
}

/**
 * Retrieve value of a given environment variable
 * @param pEnvVarName :[in] environment variable name to be read
 * @param storeVal :[out] variable to store env variable value
 * @return true/false based on success/failure
 */
bool CCommon::readEnvVariable(const char *pEnvVarName, string &storeVal)
{
	if(NULL == pEnvVarName)
	{
		std::cout << "Environment variable to read is NULL" << std::endl;
		return false;
	}
	bool bRetVal = false;

	char *cEvar = getenv(pEnvVarName);
	if (NULL != cEvar)
	{
		bRetVal = true;
		std::string tmp (cEvar);
		storeVal = tmp;
		cout << std::string(pEnvVarName) << " environment variable is set to ::" << storeVal << endl;
	}
	else
	{
		std::cout << __func__ << ":" << __LINE__ << " Error : " + std::string(pEnvVarName) + " environment variable is not found" <<  std::endl;

	}
	return bRetVal;
}

/**
 * Retrieve and store all common environment variables in CCommon instance.
 * @param None
 * @return true/false based on success/failure
 */
bool CCommon::readCommonEnvVariables()
{
	try
	{
		bool bRetVal = false;

		std::list<std::string> topicList{"ReadRequest", "WriteRequest",
			"AppName", "MQTT_URL_FOR_EXPORT", "DEV_MODE", "ReadRequest_RT", "WriteRequest_RT"};

		std::map <std::string, std::string> envTopics;

		for (auto &topic : topicList)
		{
			std::string envVar = "";
			bRetVal = readEnvVariable(topic.c_str(), envVar);
			if(!bRetVal)
			{
				return false;
			}
			else
			{
				envTopics.emplace(topic, envVar);
			}
		}

		setStrReadRequest(envTopics.at("ReadRequest"));
		setStrWriteRequest(envTopics.at("WriteRequest"));

		//for RT
		setStrRTReadRequest(envTopics.at("ReadRequest_RT"));
		setStrRTWriteRequest(envTopics.at("WriteRequest_RT"));
		//end

		// Write Delay
		std::string envVar = "";
		bRetVal = readEnvVariable("WriteRequest_Delay", envVar);
		if(!bRetVal)
		{
			setStrWRDelay(0);
		}
		else
		{
			setStrWRDelay(stoi(envVar));
		}

		setStrAppName(envTopics.at("AppName"));
		setStrMqttExportURL(envTopics.at("MQTT_URL_FOR_EXPORT"));

		string devMode = envTopics.at("DEV_MODE");
		transform(devMode.begin(), devMode.end(), devMode.begin(), ::toupper);

		if (devMode == "TRUE")
		{
			setDevMode(true);
			cout << "DEV_MODE is set to true\n";

		}
		else if (devMode == "FALSE")
		{
			setDevMode(false);
			cout << "DEV_MODE is set to false\n";
		}
		else
		{
			// default set to false
			cout << "DEV_MODE is set to default false\n";
		}
	}
	catch(exception &ex)
	{
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << ex.what() << std::endl;
		return false;
	}
	return true;
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
		std::cout << "Cannot get current time in string :: " << std::string(e.what()) << std::endl;
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
			return false;
		}

		if(NULL == cJSON_AddStringToObject(root, tsKey.c_str(), strTimestamp.c_str()))
		{
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

		return true;

	}
	catch (exception &ex)
	{
		cout << "Failed to add timestamp in payload for MQTT" << std::string(ex.what()) << endl;

		if(root != NULL)
		{
			cJSON_Delete(root);
		}

		return false;
	}
}
