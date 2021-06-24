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

/**Test for getTimeParams()**/
TEST_F(CommonDataShare_ut, getTimeParam)
{
	std::string strTimeStamp = "";
	std::string strUsec = "";
	CcommonEnvManager::Instance().getTimeParams(strTimeStamp, strUsec);
}
