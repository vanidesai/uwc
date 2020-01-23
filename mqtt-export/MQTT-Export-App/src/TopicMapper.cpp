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
							"AppName", "APP_VERSION", "MQTT_URL_FOR_EXPORT"};
		std::map <std::string, std::string> envTopics;

		for (auto topic : topicList)
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
		setStrAppVersion(envTopics.at("APP_VERSION"));
		setStrMqttExportURL(envTopics.at("MQTT_URL_FOR_EXPORT"));
	}
	catch(exception &ex) {
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << ex.what() << std::endl;
	}
	return true;
}

CTopicMapper::~CTopicMapper() {
	// TODO Auto-generated destructor stub
}
