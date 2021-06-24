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

#include "../include/Common_ut.hpp"
#include <string>
#include <vector>


using namespace std;

void Common_ut::SetUp()
{
	// Setup code
}

void Common_ut::TearDown()
{
	// TearDown code
}

/**
 * Test case to check if addTimestampsToMsg()function do not Add current time stamp in message payload for invalid msg and returns false
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Common_ut, addTimestampsToMsg_InvMsg)
{

	std::string TimeStamp;
	std::string message_Inv = "InvMsg";
	std::string Key = "RecMsg";
	bool RetVal = true;
	CCommon::getInstance().getCurrentTimestampsInString(TimeStamp);
	RetVal = CCommon::getInstance().addTimestampsToMsg(message_Inv, Key, TimeStamp);
	EXPECT_EQ(false, RetVal);

}

/**
 * Test case to check if addTimestampsToMsg()function Add current time stamp in message payload for valid msg and returns true on successs
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Common_ut, addTimestampsToMsg_KeyAddedSuccessfully)
{

	std::string TimeStamp;
	std::string message = "{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}";
	std::string Key = "timestamp_NotPresent";
	bool RetVal = false;
	CCommon::getInstance().getCurrentTimestampsInString(TimeStamp);
	RetVal = CCommon::getInstance().addTimestampsToMsg(message, Key, TimeStamp);

	EXPECT_EQ(true, RetVal);

}
