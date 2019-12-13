/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/


#include "../include/CConfigManager_ut.h"
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

/******************************CConfigManager::Instance()*****************************************/

// TC001
//Test: Return of CConfigManager::Instance.
//CConfigManager::Instance() should return Instance of class type CfgManager
TEST_F(CConfigManager_ut, getInstance) {

	CfgManager::Instance();
	//EXPECT_EQ("PL0", CWellSiteInfo_obj.getID());
}

//Test: Return of CConfigManager::Instance.
//CConfigManager::Instance() should return Instance of class type CfgManager
TEST_F(CConfigManager_ut, getETCDValuebyKey_NULL) {

	char *cEtcdValue  = CfgManager::Instance().getETCDValuebyKey("");
	EXPECT_EQ(*cEtcdValue, 0);

}

//Test: Return of CConfigManager::Instance.
//CConfigManager::Instance() should return Instance of class type CfgManager
TEST_F(CConfigManager_ut, getETCDValuebyKey_Val) {

	char *cEtcdValue  = CfgManager::Instance().getETCDValuebyKey("site_list.yaml");
	EXPECT_NE(NULL,cEtcdValue);

}

//Test: Return of CConfigManager::Instance.
//CConfigManager::Instance() should return Instance of class type CfgManager
TEST_F(CConfigManager_ut, IsClientCreated) {

	EXPECT_TRUE(CfgManager::Instance().IsClientCreated());
}

//Test: Return of CConfigManager::Instance.
//CConfigManager::Instance() should return Instance of class type CfgManager
TEST_F(CConfigManager_ut, getEnvConfig_NULL) {
	/// parse all the subtopics
		std::vector<std::string> stTopics = CfgManager::Instance().getEnvConfig().get_topics_from_env("");
		EXPECT_TRUE(stTopics.empty());
}

//Test: Return of CConfigManager::Instance.
//CConfigManager::Instance() should return Instance of class type CfgManager
TEST_F(CConfigManager_ut, getEnvConfig_Val) {
	/// parse all the subtopics
		std::vector<std::string> stTopics = CfgManager::Instance().getEnvConfig().get_topics_from_env("sub");
		EXPECT_FALSE(stTopics.empty());

}

////Test: Return of CConfigManager::Instance.
////CConfigManager::Instance() should return Instance of class type CfgManager
//TEST_F(CConfigManager_ut, registerCallbackOnChangeDir) {
//
//	CfgManager::Instance().registerCallbackOnChangeDir();
//}
//
////Test: Return of CConfigManager::Instance.
////CConfigManager::Instance() should return Instance of class type CfgManager
//TEST_F(CConfigManager_ut, registerCallbackOnChangeKey) {
//
//	CfgManager::Instance().registerCallbackOnChangeKey();
//}

