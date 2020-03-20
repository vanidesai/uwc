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



TEST_F(CConfigManager_ut , test)
{
	bool bRet = PublishJsonHandler::instance().isDevMode();
	if(bRet == true)
	{
		PublishJsonHandler::instance().setDevMode(false);
		bRet = CfgManager::Instance().IsClientCreated();
		EXPECT_EQ(1, bRet);
	}
}

TEST_F(CConfigManager_ut, getInstance)
{

	CfgManager::Instance();
	EXPECT_EQ(typeid(CfgManager), typeid(CfgManager::Instance()));
	//EXPECT_EQ("PL0", CWellSiteInfo_obj.getID());


}


TEST_F(CConfigManager_ut, IsClientCreated)
{

	EXPECT_TRUE(CfgManager::Instance().IsClientCreated());

}


#endif
