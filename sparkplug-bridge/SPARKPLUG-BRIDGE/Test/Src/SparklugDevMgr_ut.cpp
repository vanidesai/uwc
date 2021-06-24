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

#include "../Inc/SparkPlugDevMgr_ut.hpp"

void SparkPlugDevMgr_ut::SetUp()
{
	// Setup code
}

void SparkPlugDevMgr_ut::TearDown()
{
	// TearDown code
}

/**
 * Test case to check prepareDBirthMessage() with Invalid device name
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(SparkPlugDevMgr_ut, prepareDBirthMessage_InvalidDevName)
{
	bool RetVal = CSparkPlugDevManager::getInstance().prepareDBirthMessage(dbirth_payload, DevName, true);
	EXPECT_EQ(false, RetVal);

}

/**
 * Test case to check getDeviceList() when spark plug device name map is not empty
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(SparkPlugDevMgr_ut, getDeviceList_NonEmptySprkPlgDevMAp)
{
	auto vDevList = CSparkPlugDevManager::getInstance().getDeviceList();

	//Returned vector should not be empty
	EXPECT_EQ(false, vDevList.empty());

}

/**
 * Test case to check processExternalMQTTMsg() with empty vector value
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(SparkPlugDevMgr_ut, ExternamMqttMsg)
{
	std::string topic = "/flowmeter/PL0/Flow/read";
	bool result = CSparkPlugDevManager::getInstance().processExternalMQTTMsg(topic, dbirth_payload, stRefActionVec);
	EXPECT_EQ(false, result);
}

TEST_F(SparkPlugDevMgr_ut, SetMsgPubStatus)
{
	bool result = CSparkPlugDevManager::getInstance().setMsgPublishedStatus(enDEVSTATUS_DOWN, "SparkPlug");
	EXPECT_EQ(true, result);
}


