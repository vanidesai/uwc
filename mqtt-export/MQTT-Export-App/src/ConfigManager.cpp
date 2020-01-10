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

/** Returns the single instance of this class
 *
 * @param  : nothing
 * @return : object of this class
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
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Change key callback is called"));
	if(key == NULL || val == NULL) {
		CLogger::getInstance().log(DEBUG, LOGDETAILS("cannot restart application as key or value is null"));
		return;
	}
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Application is restarting to apply new changes from ETCD.."));
	string temp = "New value to be apply key:";
	temp.append(key);
	temp.append(":");
	temp.append(val);

	CLogger::getInstance().log(DEBUG, LOGDETAILS(temp));

	std::cout << __func__ << temp << endl;

	exit(0);
}

/** ETCD callback on changing directory
 *
 * @param key: changed key
 * @param value: changed value
 * @return: nothing
 */
void etcdOnChangeDirCb(char* key, char * val)
{
	std::cout<< __func__ << " ETCD :: Change dir callback is called";
	if(key == NULL || val == NULL) {
		CLogger::getInstance().log(DEBUG, LOGDETAILS("cannot restart application as key or value is null"));
		return;
	}
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Application is restarting to apply new changes from ETCD.."));
	string temp = "New value to be apply key:";
	temp.append(key);
	temp.append(":");
	temp.append(val);

	CLogger::getInstance().log(DEBUG, LOGDETAILS(temp));
	std::cout << __func__ << temp << endl;

	//raised to call clean up routine
	raise( SIGUSR1);
}

/** register callback for specific directory in ETCD
 *
 * @param key: name of the key
 * @return: nothing
 */
void CfgManager::registerCallbackOnChangeDir(char *dir)
{
	if(dir == NULL) {
		CLogger::getInstance().log(DEBUG, LOGDETAILS("dir is null, cannot register callback"));
		return;
	}
	env_config.get_config_mgr_client()->register_watch_dir(dir, etcdOnChangeDirCb);
	string temp = "ETCD :: Callback is register for :: ";
	temp.append(dir);
	CLogger::getInstance().log(DEBUG, LOGDETAILS(temp));

	temp = "Callback is register for :: ";
	temp.append(dir);
	CLogger::getInstance().log(DEBUG, LOGDETAILS(temp));
	std::cout << __func__ << temp << endl;
}

/** Register callback for specific key in ETCD
 *
 * @param key: name of the key
 * @return: nothing
 */
void CfgManager::registerCallbackOnChangeKey(char *key)
{
	if(key == NULL) {
		CLogger::getInstance().log(DEBUG, LOGDETAILS("key is null, cannot register callback"));
		return;
	}
	env_config.get_config_mgr_client()->register_watch_key(key, etcdOnChangeKeyCb);
	string temp = "ETCD :: Callback is register for :: ";
	temp.append(key);
	CLogger::getInstance().log(DEBUG, LOGDETAILS(temp));
	temp = "Callback is register for :: ";
	temp.append(key);
	CLogger::getInstance().log(DEBUG, LOGDETAILS(temp));
	std::cout << __func__ << temp << endl;
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
		CLogger::getInstance().log(DEBUG, LOGDETAILS("Key to be fetched from etcd is empty .."));
		return NULL;
	}
	std::string sActualKey(key);
	const char* env_appname = std::getenv("AppName");
	if(NULL == env_appname) {
		CLogger::getInstance().log(ERROR, LOGDETAILS("AppName Environment Variable is not set.."));
		return NULL;
	}
	std::string AppName(env_appname);

	if(AppName.empty())
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS("Environment variable AppName key does not have value"));
		return etcdVal;
	}

	std::string baseKey = "/" + AppName + "/" + sActualKey;

	//can this be null ? env_config.get_config_mgr_client()
	etcdVal = env_config.get_config_mgr_client()->get_config((char*)(baseKey.c_str()));
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
	(cfgMangr !=NULL)?m_isClientCreated=true:m_isClientCreated=false;

	return m_isClientCreated;
}
