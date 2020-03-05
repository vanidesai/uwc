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



#if 1

/**********CConfigManager_ut::getInstance() should return Instance of class type CfgManager *******/



TEST_F(CConfigManager_ut, getInstance) {

	CfgManager::Instance();
	EXPECT_EQ(typeid(CfgManager), typeid(CfgManager::Instance()));
	//EXPECT_EQ("PL0", CWellSiteInfo_obj.getID());
}


TEST_F(CConfigManager_ut, IsClientCreated)
{

	EXPECT_TRUE(CfgManager::Instance().IsClientCreated());

}



/*
*********CConfigManager_ut::getEnvConfig_NULL() should return Instance of class type CfgManager******
TEST_F(CConfigManager_ut, getEnvConfig_NULL)
{

	/// parse all the subtopics
	//std::vector<std::string> stTopics = CfgManager::Instance(CallerObj).getEnvConfig().get_topics_from_env("");
	char** stTopics =  CallerObj.EnvConf_Caller_GetTopicFromEnv("");
	//EXPECT_TRUE(stTopics.empty());
	EXPECT_EQ(stTopics, NULL);
}
*/

/*
******CConfigManager_ut::getEnvConfig_Val() should return Instance of class type CfgManager**********
TEST_F(CConfigManager_ut, getEnvConfig_Val)
{

	/// parse all the subtopics
	//std::vector<std::string> stTopics = CfgManager::Instance(CallerObj).getEnvConfig().get_topics_from_env("sub");
     char** stTopics =  CallerObj.EnvConf_Caller_GetTopicFromEnv("sub");
	//EXPECT_FALSE(stTopics.empty());
     EXPECT_NE("", NULL);

}
*/


#endif
