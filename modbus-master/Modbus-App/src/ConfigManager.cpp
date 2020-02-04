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
#include "Logger.hpp"

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

/** Returns the client status of creation
 *
 * @param : Nothing
 * @return: true/false based on status
 */
bool CfgManager::IsClientCreated()
{
	//config_mgr_t* cfgMangr = env_config.get_config_mgr_client();
	bool isClientCreated = (getConfigClient() !=NULL)?true:false;
	return isClientCreated;
}
