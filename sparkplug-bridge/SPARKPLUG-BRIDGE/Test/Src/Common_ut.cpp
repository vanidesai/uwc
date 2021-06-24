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

#include "../Inc/Common_ut.hpp"

void Common_ut::SetUp()
{
	// Setup code
}

void Common_ut::TearDown()
{
	// TearDown code
}

/**
 * Test case to check if getTopicParts() splits the topic successfully
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Common_ut, getTopicParts_Test01)
{
	std::vector<std::string> TopicParts;
	CCommon::getInstance().getTopicParts("PartA_UT/PartB_UT/PartC_UT", TopicParts, "/");

	EXPECT_EQ("PartC_UT",TopicParts[2]);
}

/**
 * Test case to check if function getDatapointsQ() executes successfully
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Common_ut, getDataPointInQ)
{
	QMgr::getDatapointsQ();
	EXPECT_EQ(true, true);
}

/**
 * Test case to check if function getScadaSubQ() executes successfully
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Common_ut, ScadaSubQ)
{
	QMgr::getScadaSubQ();
	EXPECT_EQ(true, true);
}





