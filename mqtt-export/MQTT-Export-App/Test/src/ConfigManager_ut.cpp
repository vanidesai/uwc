/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../include/ConfigManager_ut.hpp"


void ConfigManager_ut::SetUp()
{
	// Setup code
}

void ConfigManager_ut::TearDown()
{
	// TearDown code
}


// Thread priority = -1
TEST_F(ConfigManager_ut, set_thread_sched_param_ThreadPriorityMin1)
{

	globalConfig::COperation operation;


	globalConfig::set_thread_sched_param(operation,
											-1,
											0,
											true);

	//To verify the result, check log.

}


TEST_F(ConfigManager_ut, setDefaultConfig_OpTypePOLLING)
{
	globalConfig::setDefaultConfig(globalConfig::POLLING);

}

TEST_F(ConfigManager_ut, setDefaultConfig_OpTypeON_DEMAND_READ)
{
	globalConfig::setDefaultConfig(globalConfig::ON_DEMAND_READ);

}

TEST_F(ConfigManager_ut, setDefaultConfig_OpTypeON_DEMAND_WRITE)
{
	globalConfig::setDefaultConfig(globalConfig::ON_DEMAND_WRITE);

}
