/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/


#include "../include/CConfigManager_ut.hpp"

#include <iostream>
using namespace std;


void CConfigManager_ut::SetUp()
{
	// Setup code
}

void CConfigManager_ut::TearDown()
{
	// TearDown code
}


/**********CConfigManager_ut::getInstance() should return Instance of class type CfgManager *******/
TEST_F(CConfigManager_ut, getInstance)
{
	stUWCComnDataVal_t UWCDataToLib;
	UWCDataToLib.m_devMode = true;
	UWCDataToLib.m_sAppName = "Modbus_TCP_Master";
	UWCDataToLib.m_isCommonDataInitialised = true;

	CcommonEnvManager::Instance().ShareToLibUwcCmnData(UWCDataToLib);

	CfgManager::Instance();
	EXPECT_EQ(typeid(CfgManager), typeid(CfgManager::Instance()));
}

#if 0 //Instance for correct data has already been created
TEST_F(CConfigManager_ut , IsClientCreated_CriticalDataNotInitialised)
{

	stUWCComnDataVal_t UWCDataToLib;

	ImportFromApp::Instance().ShareToLibUwcCmnData(UWCDataToLib);

	bool bRet = CfgManager::Instance().IsClientCreated();

	EXPECT_EQ(false, bRet);
}
#endif

TEST_F(CConfigManager_ut, IsClientCreated_CriticalDataInitialised)
{
	stUWCComnDataVal_t UWCDataToLib;
	UWCDataToLib.m_devMode = true;
	UWCDataToLib.m_sAppName = "Modbus_TCP_Master";
	UWCDataToLib.m_isCommonDataInitialised = true;

	CcommonEnvManager::Instance().ShareToLibUwcCmnData(UWCDataToLib);

	EXPECT_TRUE(CfgManager::Instance().IsClientCreated());
}


//Nothing to check in this function , this function is here for code coverage point of view..
TEST_F(CConfigManager_ut, display_sched_attr_test)
{
	struct sched_param param;
	globalConfig::display_sched_attr(2, param);
}



TEST_F(CConfigManager_ut, display_thread_sched_attr_test)
{
	char msg;
	globalConfig::display_thread_sched_attr(&msg);

	// Please check log for test result
}

TEST_F(CConfigManager_ut, set_thread_param_true)
{
	globalConfig::COperation a_OpsInfo;
	globalConfig::eThreadScheduler a_eSched = globalConfig::threadScheduler::UNKNOWN;
	bool a_bIsOperation = true;
	globalConfig::set_thread_sched_param(a_OpsInfo, 1, a_eSched, a_bIsOperation);
}

TEST_F(CConfigManager_ut, set_thread_param_false)
{
	globalConfig::COperation a_OpsInfo;
	globalConfig::eThreadScheduler a_eSched = globalConfig::threadScheduler::UNKNOWN;
	bool a_bIsOperation = false;
	globalConfig::set_thread_sched_param(a_OpsInfo, 1, a_eSched, a_bIsOperation);
}

TEST_F(CConfigManager_ut, setDefaultconfig_polling)
{
	globalConfig::eOperationType a_eOpType = globalConfig::eOperationType::UNKNOWN_OPERATION;
	globalConfig::setDefaultConfig(a_eOpType);
}

TEST_F(CConfigManager_ut, loadGlobalConfigurations_GlobalKeyPressent)
{

	EXPECT_EQ( true, globalConfig::loadGlobalConfigurations() );
}

