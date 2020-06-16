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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <eis/config_manager/env_config.h>
#include <eis/config_manager/config_manager.h>
#include <iostream>
#include "yaml-cpp/yaml.h"
#include "Logger.hpp"
#include "Common.hpp"

#define GLOBAL_CONFIG_FILE_PATH "/opt/intel/eis/uwc_data/common_config/Global_Config.yml"
#define handle_error_en(en, msg) do { errno = en; perror(msg); } while (0)

/**
 * Config manager class
 */
class CfgManager {
public:

    /**
     * Get the single instance of this class
     * @param None
     * @return reference of this class
     */
	static CfgManager& Instance();

	bool IsClientCreated();

	/**
	 * Return client from EIS Env library
	 * @param None
	 * @return Configuration object
	 */
	const config_mgr_t* getConfigClient() const {
		return config_mgr_client;
	}

	/**
	 * Get client from EIS Config library
	 * @param None
	 * @return ENV client
	 */
	const env_config_t* getEnvClient() const {
		return env_config_client;
	}

private:

	// True for success and false for failure
	bool isClientCreated;

	// Local object for EIS ENV Manager
	env_config_t* env_config_client;

	// Local object for EIS Config Manager
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

/**
 * namespace for global configuration
 */
namespace globalConfig
{

// default values to be used for setting global configuration for all operations
#define DEFAULT_RETRIES 0
#define DEFAULT_QOS 0

/**
 * Enum of data types
 */
enum eDataType
{
	DT_BOOL,        //!< DT_BOOL
	DT_INTEGER,     //!< DT_INTEGER
	DT_UNSIGNED_INT,//!< DT_UNSIGNED_INT
	DT_STRING,      //!< DT_STRING
	DT_MAP          //!< DT_MAP
};

/** Function to validate key in YAML file
 *
 * @param baseNode :[in] YAML node to read from
 * @param a_sKey :[in] key to validate
 * @param a_eDataType :[in] data type
 * @return 0 if success, -1 if key is empty or absent, -2 if incorrect data type
 */
int validateParam(const YAML::Node& a_BaseNode,
		const std::string& a_sKey,
		const enum eDataType a_eDT);
}  // namespace globalConfig

#endif /* INCLUDE_CONFIGMANAGER_HPP_ */
