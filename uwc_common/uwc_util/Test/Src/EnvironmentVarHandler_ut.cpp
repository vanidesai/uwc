/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/


#include "../include/EnvironmentVarHandler_ut.hpp"


void EnvironmentHandler_ut::SetUp()
{
	// Setup code
}

void EnvironmentHandler_ut::TearDown()
{
	// TearDown code
}

/*********In Logger functions there is nothing to check through the unit test cases
 therefore functions are just only called in the unit test cases for the seck of coverage of uncovered
 functions from Logger.cpp file.

 *********/


TEST_F(EnvironmentHandler_ut, readCommonVar)
{
	bool bRetVal = EnvironmentInfo::getInstance().readCommonEnvVariables(m_vecEnv);
	EXPECT_TRUE(bRetVal);

	m_vecEnv.push_back("TEST");
	bRetVal = EnvironmentInfo::getInstance().readCommonEnvVariables(m_vecEnv);
	EXPECT_FALSE(bRetVal);

	std::vector<std::string> vtTemp;
	bRetVal = EnvironmentInfo::getInstance().readCommonEnvVariables(vtTemp);
	EXPECT_FALSE(bRetVal);
}

TEST_F(EnvironmentHandler_ut, addDataToMap)
{
	std::string strKey = "TEST";
	std::string strVal = "TestVal";
	bool bRetVal = EnvironmentInfo::getInstance().addDataToEnvMap(strKey, strVal);
	EXPECT_TRUE(bRetVal);
}

TEST_F(EnvironmentHandler_ut, getDataFromMap)
{
	std::string strKey = "TEST";
	std::string strVal = "TestVal";
	std::string bRetVal = EnvironmentInfo::getInstance().getDataFromEnvMap(strKey);
	EXPECT_EQ(strVal, bRetVal);
}

