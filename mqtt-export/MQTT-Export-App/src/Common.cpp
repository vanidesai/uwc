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
#include <algorithm>

#include <pthread.h>
#include <sched.h>
#include "Common.hpp"

/**
 * Constructor
 */
CCommon::CCommon()
{

	readCommonEnvVariables();
}

/**
 * This function is used to read environment variable
 * @param pEnvVarName :[in] environment variable to be read
 * @param storeVal :[out] variable to store env variable value
 * @return 	true : on success,
 * 			false : on error
 */
bool CCommon::readEnvVariable(const char *pEnvVarName, string &storeVal)
{
	if(NULL == pEnvVarName)
	{
		return false;
	}
	bool bRetVal = false;
	char *cEvar = getenv(pEnvVarName);
	if (NULL != cEvar)
	{
		bRetVal = true;
		std::string tmp (cEvar);
		storeVal = tmp;
		CLogger::getInstance().log(DEBUG, LOGDETAILS(std::string(pEnvVarName) + " environment variable is set to ::" + storeVal));
	}
	else
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS(std::string(pEnvVarName) + " environment variable is not found"));
		std::cout << __func__ << ":" << __LINE__ << " Error : " + std::string(pEnvVarName) + " environment variable is not found" <<  std::endl;

	}
	return bRetVal;
}

/**
 * This function is used to read common environment variables
 * @return 	true : on success,
 * 			false : on error
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

		setStrAppName(envTopics.at("AppName"));
		setStrMqttExportURL(envTopics.at("MQTT_URL_FOR_EXPORT"));

		string devMode = envTopics.at("DEV_MODE");
		transform(devMode.begin(), devMode.end(), devMode.begin(), ::toupper);

		if (devMode == "TRUE")
		{
			setDevMode(true);
			CLogger::getInstance().log(INFO, LOGDETAILS("DEV_MODE is set to true"));
			cout << "DEV_MODE is set to true\n";

		}
		else if (devMode == "FALSE")
		{
			setDevMode(false);
			CLogger::getInstance().log(INFO, LOGDETAILS("DEV_MODE is set to false"));
			cout << "DEV_MODE is set to false\n";
		}
		else
		{
			/// default set to false
			CLogger::getInstance().log(ERROR, LOGDETAILS("Invalid value for DEV_MODE env variable"));
			CLogger::getInstance().log(INFO, LOGDETAILS("Set the dev mode to default (i.e. true)"));
			cout << "DEV_MODE is set to default false\n";
		}
	}
	catch(exception &ex)
	{
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << ex.what() << std::endl;
	}
	return true;
}

/**
 * Destructor
 */
CCommon::~CCommon()
{
	// TODO Auto-generated destructor stub
}

/*
 * Gets current time in nano seconds
 * @param ts :[in] structure of current time
 * @return current time in nano seconds
*/
unsigned long get_nanos(struct timespec ts)
{
    return (unsigned long)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

/**
 * Get current epoch time in string
 * @param strCurTime
 */
void CCommon::getCurrentTimestampsInString(std::string &strCurTime)
{
	try
	{
		struct timespec tsMsgReceived;
		timespec_get(&tsMsgReceived, TIME_UTC);
		strCurTime = std::to_string(get_nanos(tsMsgReceived));
	}
	catch(exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS("Cannot get current time in string :: " +
				std::string(e.what())));
	}
}

/**
 * Add current time stamp in message payload
 * @param a_sMsg 		:[in] message in which to add time
 * @param a_tsMsgRcvd	:[in] time stamp in nano seconds
 * @return 	true : on success,
 * 			false : on error
 */
bool CCommon::addTimestampsToMsg(std::string &a_sMsg, string tsKey, string strTimestamp)
{
	cJSON *root = NULL;
	try
	{
		root = cJSON_Parse(a_sMsg.c_str());
		if (NULL == root)
		{
			CLogger::getInstance().log(ERROR,
					LOGDETAILS("ZMQ Message could not be parsed in json format"));
			return false;
		}

		cJSON_AddStringToObject(root, tsKey.c_str(), strTimestamp.c_str());

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

		CLogger::getInstance().log(DEBUG, LOGDETAILS("Added timestamp in payload for MQTT"));
		return true;

	}
	catch (exception &ex)
	{
		CLogger::getInstance().log(DEBUG, LOGDETAILS("Failed to add timestamp in payload for MQTT" + std::string(ex.what())));

		if(root != NULL)
		{
			cJSON_Delete(root);
		}

		return false;
	}
}
