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
CCommon::CCommon() :
	m_strAppName {""}, m_strExtMqttURL{""}, m_nQos{1}, m_strIntMqttURL{""},
	m_siteListFileName{""}, m_strNodeConfPath{""}, m_strNetworkType{""}, 
	m_strGroupId{""}, m_strEdgeNodeID{""}, m_devMode{false}, m_bIsScadaTLS{true}
{
	setScadaRTUIds();

	if(false == readCommonEnvVariables())
	{
		std::cout << "Error while reading common environment variables" << std::endl;
	}
}

/**
 * load config required for scada-rtu container from scada_config.yml file
 * @param None
 * @return true/false based on condition
 */
bool CCommon::loadYMLConfig()
{
	YAML::Node config;
	bool bRet = false;

	try
	{
		config = YAML::LoadFile(SCADA_CONFIG_FILE_PATH);
	}
	catch(YAML::Exception &e)
	{
		DO_LOG_ERROR("scada_config.yml file not found :: " + std::string(e.what()));
		cout << "ERROR: scada_config.yml file not found :: " + std::string(e.what()) << endl;
		return false;
	}

	do
	{
		std::string sTemp{""};

		if((config["isTLS"])
				&& 0 == globalConfig::validateParam(config, "isTLS", globalConfig::eDataType::DT_BOOL))
		{
			if(config["isTLS"].as<bool>() == true)
			{
				sTemp = "ssl://";
				setScadaTLS(true);
				cout << "Set the scada external broker to TLS" << endl;
			}
			else
			{
				sTemp = "tcp://";
				setScadaTLS(false);
				cout << "Set the scada external broker to non-TLS" << endl;
			}
		}
		else
		{
			DO_LOG_ERROR("isTLS key is not present or type is invalid");
			cout << "isTLS key is not present or type is invalid" << endl;
			bRet = false;
			break;
		}

		if((config["mqttServerAddrSCADA"])
				&& 0 == globalConfig::validateParam(config, "mqttServerAddrSCADA", globalConfig::eDataType::DT_STRING))
		{
			sTemp += config["mqttServerAddrSCADA"].as<string>();
		}
		else
		{
			DO_LOG_ERROR("mqttServerAddrSCADA key is not present or type is invalid");
			cout << "mqttServerAddrSCADA key is not present or type is invalid" << endl;
			bRet = false;
			break;
		}

		if((config["mqttServerPortSCADA"])
				&& 0 == globalConfig::validateParam(config, "mqttServerPortSCADA", globalConfig::eDataType::DT_STRING))
		{
			sTemp += (":" + config["mqttServerPortSCADA"].as<string>());
			setExtMqttURL(sTemp);
			DO_LOG_INFO("EXTERNAL_MQTT_URL is set to :: " + getExtMqttURL())
			cout << "EXTERNAL_MQTT_URL is set to :: " + getExtMqttURL() << endl;
			bRet = true;
		}
		else
		{
			DO_LOG_ERROR("mqttServerPortSCADA key is not present or type is invalid");
			cout << "mqttServerPortSCADA key is not present or type is invalid" << endl;
			bRet = false;
			break;
		}
	} while(0);

	if((config["qos"])
			&& 0 == globalConfig::validateParam(config, "qos", globalConfig::eDataType::DT_INTEGER))
	{
		int nQos = config["qos"].as<std::int32_t>();
		if(nQos >= 0 && nQos <=2)
		{
			setMQTTQos(nQos);
			DO_LOG_INFO("QOS is set to :: " + to_string(getMQTTQos()))
			cout << "QOS is set to :: " + to_string(getMQTTQos()) << endl;
		}
		else
		{
			setMQTTQos(1);	// default QOS value is 1
			DO_LOG_ERROR("QOS key is not within valid range, set to default value 1");
			cout << "QOS key is not within valid range, set to default value :: " << to_string(getMQTTQos()) << endl;
		}
	}
	else
	{
		setMQTTQos(1);	// default QOS value is 1
		DO_LOG_ERROR("QOS key is not present, set to default value 1");
		cout << "QOS key is not present, set to default value :: " << to_string(getMQTTQos()) << endl;
	}

	return bRet;
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

		std::list<std::string> topicList{"AppName", "INTERNAL_MQTT_URL" , "DEV_MODE",
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
		setIntMqttURL(envTopics.at("INTERNAL_MQTT_URL"));
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


/**
 * Splits topic name with delimeter '/'
 * @param a_sTopic :[in] topic to split
 * @param a_vsTopicParts :[out] stores all the split names in this vector
 * @return true/false based on true/false
 */
bool CCommon::getTopicParts(std::string a_sTopic,
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
