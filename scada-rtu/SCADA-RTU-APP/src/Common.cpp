/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#include "Common.hpp"
#include <iterator>
#include <iostream>
#include <fstream>
#include <bits/stdc++.h>
/**
 * Constructor initializes CCommon instance and retrieves common environment variables
 * @param None
 * @return None if successful;
 * 			In case of error or exception, application exits
 */
CCommon::CCommon()
{
	setScadaRTUIds();

	if(false == readCommonEnvVariables())
	{
		std::cout << "Error while reading common environment variables" << std::endl;
	}
}

void CCommon::setScadaRTUIds()
{
	// load global configuration for container real-time setting
	bool bRetVal = globalConfig::loadGlobalConfigurations();
	if(!bRetVal)
	{
		DO_LOG_INFO("Global configuration is set with some default parameters");
		cout << "\nGlobal configuration is set with some default parameters\n\n";
	}
	else
	{
		DO_LOG_INFO("Global configuration is set successfully");
		cout << "\nGlobal configuration for container real-time is set successfully\n\n";
	}

	m_strGroupId = globalConfig::CGlobalConfig::getInstance().getSparkPlugInfo().getGroupId();
	if(m_strGroupId.empty())
	{
		std::cout << "Group id for scada-rtu is not set, exiting application" << std::endl;
		return;
	}

	const stEdgeNodeId& stEdgeNodeId = globalConfig::CGlobalConfig::getInstance().getSparkPlugInfo().getObjEdgeNodeId();
	if(stEdgeNodeId.m_stNodeName.empty())
	{
		std::cout << "Edge node id name is not set for scada-rtu, exiting application" << std::endl;
		return;
	}

	m_strEdgeNodeID.assign(stEdgeNodeId.m_stNodeName);

	if(stEdgeNodeId.m_stGenUniquename == true)
	{
		//generate name with MAC address
		m_strEdgeNodeID.append("-");

		string strInterfaceName = globalConfig::CGlobalConfig::getInstance().getSparkPlugInfo().getInterfaceName();
		if(strInterfaceName.empty())
		{
			std::cout << "Edge node id name is not set for scada-rtu, exiting application" << std::endl;
			return;
		}

		m_strEdgeNodeID.append(getMACAddress(strInterfaceName));
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
		DO_LOG_ERROR("Environment variable to read is NULL");
		return false;
	}
	bool bRetVal = false;

	char *cEvar = getenv(pEnvVarName);
	if (NULL != cEvar)
	{
		bRetVal = true;
		std::string tmp (cEvar);
		storeVal = tmp;
		DO_LOG_DEBUG(std::string(pEnvVarName) + " environment variable is set to ::" + storeVal);
	}
	else
	{
		DO_LOG_ERROR(std::string(pEnvVarName) + " environment variable is not found");
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

		std::list<std::string> topicList{"AppName", "MQTT_URL", "DEV_MODE",
			"NETWORK_TYPE", "DEVICES_GROUP_LIST_FILE_NAME"};

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

		setStrAppName(envTopics.at("AppName"));
		setStrMqttURL(envTopics.at("MQTT_URL"));
		setNetworkType(envTopics.at("NETWORK_TYPE"));
		setSiteListFileName(envTopics.at("DEVICES_GROUP_LIST_FILE_NAME"));

		string devMode = envTopics.at("DEV_MODE");
		transform(devMode.begin(), devMode.end(), devMode.begin(), ::toupper);

		if (devMode == "TRUE")
		{
			setDevMode(true);
			DO_LOG_INFO("DEV_MODE is set to true");
			cout << "DEV_MODE is set to true\n";

		}
		else if (devMode == "FALSE")
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
	}
	catch(exception &ex)
	{
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << ex.what() << std::endl;
		return false;
	}
	return true;
}

/**
 * Get MAC address of machine
 * @param : [in] Interface name of which to find MAC address
 * @return MAC address in string format
 */
string CCommon::getMACAddress(const string& a_strInterfaceName)
{
	ifstream iface("/sys/class/net/" + a_strInterfaceName + "/address");

	string strMAC((istreambuf_iterator<char>(iface)), istreambuf_iterator<char>());

	if (strMAC.length() > 0)
	{
		string hex = regex_replace(strMAC, std::regex("\n"), "");
		return hex;
	}
	else
	{
		return "00";
	}
}
/**
 * Destructor
 */
CCommon::~CCommon()
{
}
