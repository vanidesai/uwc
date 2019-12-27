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
#include "BoostLogger.hpp"

/** Returns the single instance of this class
 *
 * @param  : nothing
 * @return : Object of this class
 */
CfgManager& CfgManager::Instance()
{
	static CfgManager _self;
	return _self;
}

/** ETCD callback on changing key
 *
 * @param key: changed key
 * @param value: changed value
 * @return: nothing
 */
void etcdOnChangeKeyCb(char* key, char * val)
{
	BOOST_LOG_SEV(lg, info) << __func__ << " Change key callback is called";
	printf("key: %s and value: %s\n", key, val);
}

/** ETCD callback on changing directory
 *
 * @param key: changed key
 * @param value: changed value
 * @return: nothing
 */
void etcdOnChangeDirCb(char* key, char * val)
{
	BOOST_LOG_SEV(lg, info) << __func__ << " ETCD :: Change dir callback is called";
	BOOST_LOG_SEV(lg, info) << __func__ << " Application is restarting to apply new changes from ETCD..";
	std::cout << __func__ << " Application is restarting to apply new changes from ETCD.."<<std::endl;
	std::cout << "New value to be apply is ::" << std::endl;
	printf("key: %s and value: %s\n", key, val);
	//exit(0);
	raise(SIGUSR1);
}

/** register callback for specific directory in ETCD
 *
 * @param key: name of the key
 * @return: nothing
 */
void CfgManager::registerCallbackOnChangeDir(char *dir)
{
	env_config.get_config_mgr_client()->register_watch_dir(dir, etcdOnChangeDirCb);
	BOOST_LOG_SEV(lg, info) << __func__ << " ETCD :: Callback is register for :: "<< dir;
	std::cout << " Callback is register for :: "<< dir <<std::endl;
}

/** Register callback for specific key in ETCD
 *
 * @param key: name of the key
 * @return: nothing
 */
void CfgManager::registerCallbackOnChangeKey(char *key)
{
	env_config.get_config_mgr_client()->register_watch_key(key, etcdOnChangeKeyCb);
	BOOST_LOG_SEV(lg, info) << __func__ << " ETCD :: Callback is register for :: "<< key;
	std::cout << " Callback is register for :: "<< key <<std::endl;
}

/** Returns the value from ETCD on specific key
 *
 * @param key: name of the key
 * @return: value from ETCD against given key
 */
char* CfgManager::getETCDValuebyKey(const char *key)
{
	char *etcdVal = NULL;
	if(key == NULL)
	{
		std::cout << "Key to be fetched from etcd is empty .." <<std::endl;
		BOOST_LOG_SEV(lg, info) << __func__ << " Key to be fetched from etcd is empty ..";
		return NULL;
	}
	std::string sActualKey(key);
	const char* pcAppName = std::getenv("AppName");
	if(NULL == pcAppName)
	{
		std::cout << "AppName Environment Variable is not set.." <<std::endl;
		BOOST_LOG_SEV(lg, info) << __func__ << " AppName Environment Variable is not set..";
		return NULL;
	}
	std::string AppName(pcAppName);
	std::string baseKey = "/" + AppName + DIR_PATH + sActualKey;
	etcdVal = env_config.get_config_mgr_client()->get_config(const_cast<char *>(baseKey.c_str()));
	return etcdVal;
}

/** Returns the client status of creation
 *
 * @param : Nothing
 * @return: true/false based on status
 */
bool CfgManager::IsClientCreated()
{
	config_mgr_t* cfgMangr = env_config.get_config_mgr_client();
	bool isClientCreated = (cfgMangr !=NULL)?true:false;
	return isClientCreated;
}
