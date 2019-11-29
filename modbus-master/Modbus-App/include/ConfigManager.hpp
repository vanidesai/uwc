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

#define APP_NAME getenv("AppName")
#define DIR_PATH "/config/UWC/YAML_Config/"

using namespace eis::config_manager;

class CfgManager {
public:

	void registerCallbackOnChangeDir(char *key);
	void registerCallbackOnChangeKet(char *key);
	static CfgManager& Instance();
	char* getETCDValuebyKey(const char *);
	bool IsClientCreated();

	const EnvConfig& getEnvConfig() const {
		return env_config;
	}

private:
	bool isClientCreated;
	EnvConfig env_config;

	CfgManager(){};
	CfgManager(CfgManager const&);             /// copy constructor is private
	CfgManager& operator=(CfgManager const&);  /// assignment operator is private
};

#endif /* INCLUDE_CONFIGMANAGER_HPP_ */
