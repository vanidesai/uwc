/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#include "TopicMapper.hpp"

#include "ConfigManager.hpp"
#include <algorithm>

#include <pthread.h>
#include <sched.h>

CTopicMapper::CTopicMapper() {
	// TODO Auto-generated constructor stub
	//ParseJson();
	readCommonEnvVariables();
}

/** This function is used to read environment variable
 *
 * @sEnvVarName : environment variable to be read
 * @storeVal : variable to store env variable value
 * @return: true/false based on success or error
 */
bool CTopicMapper::readEnvVariable(const char *pEnvVarName, string &storeVal)
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

/** This function is used to read common environment variables
 *
 * @return: true/false based on success or error
 */
bool CTopicMapper::readCommonEnvVariables()
{
	try
	{
		bool bRetVal = false;

		std::list<std::string> topicList{"ReadRequest", "WriteRequest",
			"AppName", "MQTT_URL_FOR_EXPORT", "DEV_MODE"};

#ifdef REALTIME_THREAD_PRIORITY
		topicList.push_back({"THREAD_PRIORITY", "THREAD_POLICY"});
#endif
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
		setStrAppName(envTopics.at("AppName"));
		setStrMqttExportURL(envTopics.at("MQTT_URL_FOR_EXPORT"));

#ifdef REALTIME_THREAD_PRIORITY
		setStrThreadPriority(envTopics.at("THREAD_PRIORITY"));
		setStrThreadPolicy(envTopics.at("THREAD_POLICY"));
#endif
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
	catch(exception &ex) {
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << ex.what() << std::endl;
	}
	return true;
}

#ifdef REALTIME_THREAD_PRIORITY
void CTopicMapper::set_thread_priority() {
	//set priority
	sched_param param;
	int iThreadPriority = getIntThreadPriority();
	int iThreadPolicy = getIntThreadPolicy();

	int defaultPolicy;
	if(0 != pthread_getschedparam(pthread_self(), &defaultPolicy, &param)){
		CLogger::getInstance().log(WARN, LOGDETAILS("Cannot fetch scheduling parameters of current thread"));
		std::cout << __func__ << ":" << __LINE__ << "Cannot fetch scheduling parameters of current thread" << std::endl;
	}
	else
	{
		param.sched_priority = iThreadPriority;
		int result = pthread_setschedparam(pthread_self(), iThreadPolicy, &param);
		if(0 != result)
		{
			CLogger::getInstance().log(WARN, LOGDETAILS("Cannot set thread priority to : " + std::to_string(iThreadPriority)));
			std::cout << __func__ << ":" << __LINE__ << " Cannot set thread priority, result : " << result << std::endl;
		}
	}
	//end of set priority for current thread
}
#endif

CTopicMapper::~CTopicMapper() {
	// TODO Auto-generated destructor stub
}
