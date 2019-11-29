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

CfgManager& CfgManager::Instance()
{
	static CfgManager _self;
	return _self;
}

void etcdOnChangeKeyCb(char* key, char * val)
{
	std::cout << __func__ << " Change key callback is called\n";
	printf("key: %s and value: %s\n", key, val);
}

void etcdOnChangeDirCb(char* key, char * val)
{
	std::cout<< __func__ << " ETCD :: Change dir callback is called";
	std::cout << __func__ << " Application is restarting to apply new changes from ETCD..";
	std::cout << __func__ << " Application is restarting to apply new changes from ETCD.."<<std::endl;
	std::cout << "New value to be apply is ::" << std::endl;
	printf("key: %s and value: %s\n", key, val);
	exit(0);
}

void CfgManager::registerCallbackOnChangeDir(char *dir)
{
	env_config.get_config_mgr_client()->register_watch_dir(dir, etcdOnChangeDirCb);
	std::cout << __func__ << " ETCD :: Callback is register for :: "<< dir << std::endl;
	std::cout << " Callback is register for :: "<< dir <<std::endl;
}

void CfgManager::registerCallbackOnChangeKet(char *key)
{
	env_config.get_config_mgr_client()->register_watch_key(key, etcdOnChangeKeyCb);
	std::cout << __func__ << " ETCD :: Callback is register for :: "<< key << std::endl;
	std::cout << " Callback is register for :: "<< key <<std::endl;
}

char* CfgManager::getETCDValuebyKey(const char *key)
{
	char *etcdVal = NULL;
	if(key == NULL)
	{
		std::cout << "Key to be fetched from etcd is empty .." <<std::endl;
		std::cout << __func__ << " Key to be fetched from etcd is empty ..";
		return NULL;
	}
	std::string sActualKey(key);
	if(NULL == getenv("AppName"))
	{
		std::cout << "AppName Environment Variable is not set.." <<std::endl;
		std::cout << __func__ << " AppName Environment Variable is not set..";
		return NULL;
	}
	std::string AppName(APP_NAME);
	std::string baseKey = "/" + AppName + "/" +sActualKey;
	etcdVal = env_config.get_config_mgr_client()->get_config(baseKey.c_str());
	return etcdVal;
}

bool CfgManager::IsClientCreated()
{
	config_mgr_t* cfgMangr = env_config.get_config_mgr_client();
	(cfgMangr !=NULL)?isClientCreated=true:isClientCreated=false;
	return isClientCreated;
}
