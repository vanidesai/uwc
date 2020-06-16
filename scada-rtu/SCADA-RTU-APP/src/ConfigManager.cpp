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

using namespace std;

/** Constructor Initialized CfgManager instance depending on the value of DEV_MODE
 * @param None
 * @return None if successful, else application exits in case if config manager fails to initialize
*/
CfgManager:: CfgManager()
{
	env_config_client = env_config_new();

	if(CCommon::getInstance().isDevMode())
	{
		// create client without certificates
		config_mgr_client = config_mgr_new((char *)"etcd", (char *)"", (char *)"", (char *)"");
	}
	else
	{
		// create client with certificates
		string sCert = "/run/secrets/etcd_" + CCommon::getInstance().getStrAppName() + "_cert";
		string sKey = "/run/secrets/etcd_" + CCommon::getInstance().getStrAppName() + "_key";
		config_mgr_client = config_mgr_new((char *)"etcd", (char *)sCert.c_str(),
				(char *)sKey.c_str(),
				(char *)"/run/secrets/ca_etcd");
	}

	isClientCreated = false;

	if(config_mgr_client == NULL)
	{
		std::cout << "Failed to create configuration manager client" << std::endl;
		exit(-1);
	}
}

/**
 * Maintains single instance of CfgManager class
 * @param  : nothing
 * @return : this instance of CfgManager class
 */
CfgManager& CfgManager::Instance()
{
	static CfgManager _self;
	return _self;
}

/**
 * Check if config manager client is created or not
 * @param None
 * @return true/false - based on success/failure
 */
bool CfgManager::IsClientCreated()
{
	//config_mgr_t* cfgMangr = env_config.get_config_mgr_client();
	bool isClientCreated = (getConfigClient() !=NULL && getEnvClient() !=NULL)?true:false;
	return isClientCreated;
}

/**
 * Validate given key in YAML file
 * @param baseNode [in] : YAML node to read from
 * @param a_sKey [in] : key to validate
 * @param a_eDataType [in] : data type
 * @return 0 if success, -1 if key is empty or absent, -2 if incorrect data type
 */
int globalConfig::validateParam(const YAML::Node& a_BaseNode,
		const std::string& a_sKey,
		const enum eDataType a_eDataType)
{
	int iRet = 0;
	try
	{
		if(!(a_BaseNode[a_sKey])  || a_BaseNode[a_sKey].as<std::string>() == "")
		{
			DO_LOG_WARN(a_sKey + " is not present or empty !!");
			DO_LOG_WARN("setting it to default !!");
			iRet = -1;
		}
		else
		{
			switch (a_eDataType)
			{
				case DT_BOOL:
				{
					a_BaseNode[a_sKey].as<bool>();
				}
				break;
				case DT_INTEGER:
				{
					a_BaseNode[a_sKey].as<int>();
				}
				break;
				case DT_UNSIGNED_INT:
				{
					a_BaseNode[a_sKey].as<unsigned>();
				}
				break;
				case DT_STRING:
				{
					a_BaseNode[a_sKey].as<std::string>();
				}
				break;
				case DT_MAP:
				{
					a_BaseNode[a_sKey].as<std::map<string,string>>();
				}
				break;
				default:
				{
					DO_LOG_ERROR("Invalid data type::" + to_string(a_eDataType));
					iRet = -3;
				}
				break;
			}
		}
	}
	catch (YAML::Exception &e)
	{
		DO_LOG_ERROR( a_sKey +
				" field data type is invalid, setting it to default, exception :: " +	std::string(e.what()));
		iRet = -2;
	}
	return iRet;
}
