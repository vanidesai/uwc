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

#include "../include/MqttHandler_ut.hpp"


void MqttHandler_ut::SetUp()
{
	// Setup code
}

void MqttHandler_ut::TearDown()
{
	// TearDown code
}

/**
 * Test case to check if instance() function return Reference of this instance of this class
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
#if 1
TEST_F(MqttHandler_ut, instance_test)
{
	bool object = true;
	CMqttHandler::instance();
	EXPECT_EQ(true, object);
}

TEST_F(MqttHandler_ut, isConnected_false)
{
	bool RetVal = CMqttHandler::instance().isConnected();
	EXPECT_EQ(false, RetVal);
}

/**
 * Test case to check if publishMsg() function publishes the msg successfully when topic is given and returns true on success
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(MqttHandler_ut, publishMsg_true)
{
	bool RetVal = CMqttHandler::instance().publishMsg(strMsg, topic);
	EXPECT_EQ(true, RetVal);
}

/**
 * Test case to check if publishMsg() function do not publishes the msg successfully when topic is not given and returns false
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(MqttHandler_ut, publishMsg_false)
{
	bool RetVal = CMqttHandler::instance().publishMsg(strMsg, Topic);
	EXPECT_EQ(false, RetVal);
}

#endif
