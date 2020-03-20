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

#include "Logger.hpp"
#include "Common.hpp"

class CfgManager {
public:

    /** Returns the single instance of this class
     *
     * @param  : nothing
     * @return : object of this class
     */
	static CfgManager& Instance();


    /** Returns the client status of creation
     *
     * @param : Nothing
     * @return: true/false based on status
     */
	bool IsClientCreated();

	/** Returns client from EIS Env library
	 *
	 * @param : Nothing
	 * @return: Configuration object
	 */
	const config_mgr_t* getConfigClient() const {
		return config_mgr_client;
	}

	/** Returns client from EIS Config library
	 *
	 * @param : Nothing
	 * @return: ENV object
	 */
	const env_config_t* getEnvClient() const {
		return env_config_client;
	}

private:

	/// True for success and false for failure
	bool isClientCreated;

	/// Local object for EIS ENV Manager
	env_config_t* env_config_client;

	/// Local object for EIS Config Manager
	config_mgr_t* config_mgr_client;

    /** Constructor
     */
	CfgManager();

    /** copy constructor is private
     */
	CfgManager(CfgManager const&);

    /** assignment operator is private
     */
	CfgManager& operator=(CfgManager const&);
};

#endif /* INCLUDE_CONFIGMANAGER_HPP_ */
