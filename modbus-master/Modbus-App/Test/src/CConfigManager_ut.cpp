/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/
#if 0

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

/******************************CConfigManager::Instance()*****************************************/




/**********CConfigManager_ut::getInstance() should return Instance of class type CfgManager *******/

TEST_F(CConfigManager_ut, getInstance) {

	CfgManager::Instance();
	EXPECT_EQ(typeid(CfgManager), typeid(CfgManager::Instance()));
	//EXPECT_EQ("PL0", CWellSiteInfo_obj.getID());
}



/**********CConfigManager_ut::getETCDValuebyKey_NULL() should return Instance of class type CfgManager *****/
TEST_F(CConfigManager_ut, getETCDValuebyKey_NULL) {

	char *cEtcdValue  = CfgManager::Instance().getETCDValuebyKey("");
	EXPECT_EQ(*cEtcdValue, 0);

}


/*************CConfigManager_ut::getETCDValuebyKey_Val() should return Instance of class type CfgManager****/
TEST_F(CConfigManager_ut, getETCDValuebyKey_Val) {

	char *cEtcdValue  = CfgManager::Instance().getETCDValuebyKey("site_list.yaml");
	EXPECT_NE(NULL,cEtcdValue);

}


/*******CConfigManager_ut::IsClientCreated() should return Instance of class type CfgManager***********/
TEST_F(CConfigManager_ut, IsClientCreated) {

	EXPECT_TRUE(CfgManager::Instance().IsClientCreated());
}


/**********CConfigManager_ut::getEnvConfig_NULL() should return Instance of class type CfgManager*******/
TEST_F(CConfigManager_ut, getEnvConfig_NULL) {
	/// parse all the subtopics
	std::vector<std::string> stTopics = CfgManager::Instance().getEnvConfig().get_topics_from_env("");
	EXPECT_TRUE(stTopics.empty());
}

/*******CConfigManager_ut::getEnvConfig_Val() should return Instance of class type CfgManager***********/
TEST_F(CConfigManager_ut, getEnvConfig_Val) {
	/// parse all the subtopics
	std::vector<std::string> stTopics = CfgManager::Instance().getEnvConfig().get_topics_from_env("sub");
	EXPECT_FALSE(stTopics.empty());

}


TEST_F(CConfigManager_ut, etcdOnChangeKeyCb_Test) {

	Expected_output = "key: Test_Key and value: Test_Val\n";

	try
	{
		testing::internal::CaptureStdout();
		etcdOnChangeKeyCb("Test_Key", "Test_Val");
		std::string output = testing::internal::GetCapturedStdout();

		EXPECT_EQ(Expected_output, output);
	}

	catch(exception &e)
	{
		Test_Str = e.what();
		EXPECT_EQ("", Test_Str);
	}
}

/*TEST_F(CConfigManager_ut, etcdOnChangeDirCb_Test) {


    Expected_output = "etcdOnChangeDirCb Application is restarting to apply new changes from ETCD..\nNew value to be apply is ::\nkey: Test_Key and value: Test_Val\nPeriodic Timer is stopped successfully\n";
	try
	{
		testing::internal::CaptureStdout();
		etcdOnChangeDirCb("Test_Key", "Test_Val");
		std::string output = testing::internal::GetCapturedStdout();

		EXPECT_EQ(Expected_output, output);
	}

	catch(exception &e)
	{
		Test_Str = e.what();
		EXPECT_EQ("", Test_Str);
	}
}*/

TEST_F(CConfigManager_ut, registerCallbackOnChangeKey_Test) {

	Expected_output = " Callback is register for :: Test_Key\n";

	try
	{
		testing::internal::CaptureStdout();
		CfgManager::Instance().registerCallbackOnChangeKey("Test_Key");
		std::string output = testing::internal::GetCapturedStdout();

		EXPECT_EQ(Expected_output, output);
	}

	catch(exception &e)
	{
		Test_Str = e.what();
		EXPECT_EQ("", Test_Str);
	}
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


#endif
