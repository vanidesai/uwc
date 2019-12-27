/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#ifndef INCLUDE_CONFIGMANAGER_HPP_
#define INCLUDE_CONFIGMANAGER_HPP_

#include<stdio.h>
#include <string.h>
#include<unistd.h>
#include <eis/config_manager/env_config.h>
#include "eis/config_manager/config_manager.h"
#include <iostream>

#define DIR_PATH "/config"

using namespace eis::config_manager;

class CfgManager {
public:

    /** register callback for specific directory in ETCD
     *
     * @param key: name of the key
     * @return: nothing
     */
	void registerCallbackOnChangeDir(char *key);

    /** Register callback for specific key in ETCD
     *
     * @param key: name of the key
     * @return: nothing
     */
	void registerCallbackOnChangeKey(char *key);

    /** Returns the single instance of this class
     *
     * @param  : nothing
     * @return : object of this class
     */
	static CfgManager& Instance();

    /** Returns the value from ETCD on specific key
     *
     * @param key: name of the key
     * @return: value from ETCD against given key
     */
	char* getETCDValuebyKey(const char *key);

    /** Returns the client status of creation
     *
     * @param : Nothing
     * @return: true/false based on status
     */
	bool IsClientCreated();

	/** Returns client from EIS Config library
	 *
	 * @param : Nothing
	 * @return: Configuration object
	 */
	/*const EnvConfig& getEnvConfig() const {
		return env_config;
	}*/

	EnvConfig& getEnvConfig() {
		return env_config;
	}

private:

	/// Local object for EIS Config Manager
	EnvConfig env_config;

    /** Constructor
     */
	CfgManager(){};

    /** copy constructor is private
     */
	CfgManager(CfgManager const&);

    /** assignment operator is private
     */
	CfgManager& operator=(CfgManager const&);
};

#endif /* INCLUDE_CONFIGMANAGER_HPP_ */
