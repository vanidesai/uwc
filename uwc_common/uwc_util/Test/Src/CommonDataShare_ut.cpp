/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/


#include "../include/CommonDataShare_ut.hpp"

void CommonDataShare_ut::SetUp()
{
	// Setup code
}

void CommonDataShare_ut::TearDown()
{
	// TearDown code
}

/**Test for ShareToLibUwcCmnData()**/
TEST_F(CommonDataShare_ut, ShareToLibUwcCmnData)
{
	stCommonDataVal.m_sAppName = "uwc-util";
	stCommonDataVal.m_devMode = true;
	stCommonDataVal.m_isCommonDataInitialised = true;
	CcommonEnvManager::Instance().ShareToLibUwcCmnData(stCommonDataVal);
}

/**Test for getAppName()**/
TEST_F(CommonDataShare_ut, getAppName)
{
	std::string strAppNameToVerify="uwc-util";
	std::string strAppName="";
	strAppName = CcommonEnvManager::Instance().getAppName();
	EXPECT_EQ(strAppName, strAppNameToVerify);
}

/**Test for getDevMode()**/
TEST_F(CommonDataShare_ut, getDevMode)
{
	bool bDevMode;
	bDevMode = CcommonEnvManager::Instance().getDevMode();
	EXPECT_EQ(bDevMode, stCommonDataVal.m_devMode);
}

/**Test for getTopicList()**/
TEST_F(CommonDataShare_ut, splitStringTopicList)
{
	std::string strTemp = "uwc-util,uwc-util-nw";
	CcommonEnvManager::Instance().splitString(strTemp,',');

	std::vector<string> vtTopicList;
	vtTopicList = CcommonEnvManager::Instance().getTopicList();
}

/**Test for getTimeParams()**/
TEST_F(CommonDataShare_ut, getTimeParam)
{
	std::string strTimeStamp = "";
	std::string strUsec = "";
	CcommonEnvManager::Instance().getTimeParams(strTimeStamp, strUsec);
}
