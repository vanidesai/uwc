/********************************************************************************
* Copyright (c) 2021 Intel Corporation.

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*********************************************************************************/


#include "../include/EnvironmentVarHandler_ut.hpp"


void EnvironmentHandler_ut::SetUp()
{
	// Setup code
}

void EnvironmentHandler_ut::TearDown()
{
	// TearDown code
}

/**Test for readCommonEnvVariables()**/
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

/**Test for addDataToEnvMap()**/
TEST_F(EnvironmentHandler_ut, addDataToMap)
{
	std::string strKey = "TEST";
	std::string strVal = "TestVal";
	bool bRetVal = EnvironmentInfo::getInstance().addDataToEnvMap(strKey, strVal);
	EXPECT_TRUE(bRetVal);
}

/**Test for getDataFromEnvMap()**/
TEST_F(EnvironmentHandler_ut, getDataFromMap)
{
	std::string strKey = "TEST";
	std::string strVal = "TestVal";
	std::string bRetVal = EnvironmentInfo::getInstance().getDataFromEnvMap(strKey);
	EXPECT_EQ(strVal, bRetVal);
}

