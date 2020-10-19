/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/
#include "Logger.hpp"
#include "ConfigManager.hpp"
#include "EnvironmentVarHandler.hpp"

/**
 * function to read environment variables
 * @param vecEnvironment :[in] vector containing all environment variables
 * @return : true for success, false otherwise
 */
bool EnvironmentInfo::readCommonEnvVariables(std::vector<string> vecEnvironment)
{
	if(vecEnvironment.empty())
	{
		DO_LOG_ERROR("No Environment variable to read.");
		return false;
	}

	bool bRetVal = false;

	for(auto ele : vecEnvironment)
	{
		char *cEvar = getenv(ele.c_str());
		if (NULL != cEvar)
		{
			bRetVal = true;
			string strTempVal = std::string(cEvar);
			addDataToEnvMap(ele, strTempVal);
			DO_LOG_DEBUG(std::string(ele) + " environment variable is set to ::" + std::string(cEvar));
		}
		else
		{
			bRetVal = false;
			DO_LOG_ERROR(std::string(ele) + " environment variable is not found");
			std::cout << __func__ << ":" << __LINE__ << " Error : " + std::string(ele) + " environment variable is not found" <<  std::endl;

		}
	}

	return bRetVal;
}

/**
 * function to add environment variable data to Map
 * @param strKey :[in] key value containing the environment variables name
 * @param strVal :[in] value containing the value for each key
 * @return : true for success, false otherwise
 */
bool EnvironmentInfo::addDataToEnvMap(string strKey, string strVal)
{
	bool bRet = true;
	try
	{
		m_umapEnv.insert({strKey, strVal});
	}
	catch (exception &e)
	{
		DO_LOG_FATAL(e.what());
		bRet = false;
	}

	return bRet;
}

/**
 * function to read data from map
 * @param strKey :[in] key string used to get data from map
 * @return : string containing value of given key
 */
string EnvironmentInfo::getDataFromEnvMap(string strKey)
{
	string strVal = "";
	try
	{
		strVal = m_umapEnv.at(strKey);
	}
	catch(exception &e)
	{
		DO_LOG_FATAL(LOGDETAILS(e.what()));
	}

	return strVal;
}


