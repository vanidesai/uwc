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

#include "../include/MQTTPublishHandler_ut.hpp"

void MQTTPublishHandler_ut::SetUp() {
	// Setup code
}

void MQTTPublishHandler_ut::TearDown() {
	// TearDown code
}

/**
 * Test case to check the behaviour of createNPubMsg() when topic is empty
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(MQTTPublishHandler_ut, createNPubMsg_EmptyTopic)
{
	CMQTTPublishHandler mqttPublisher_ut("tcp://127.0.0.1:11883", EmptyTopic, 1);
	EXPECT_EQ( false, mqttPublisher_ut.createNPubMsg(ValidMsg, EmptyTopic) );
}

/**
 * Test case to check the behaviour of createNPubMsg() when message is empty
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(MQTTPublishHandler_ut, createNPubMsg_EmptyMsg)
{
	CMQTTPublishHandler mqttPublisher_ut("tcp://mqtt_test_container:11883", EmptyTopic, 1);
	EXPECT_EQ( false, mqttPublisher_ut.createNPubMsg(EmptyMsg, ValidTopic) );
}

/**
 * Test case to check the behaviour of createNPubMsg() when topic and message both are valid
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(MQTTPublishHandler_ut, createNPubMsg_ValidTopic_ValidMsg)
{
	CMQTTPublishHandler mqttPublisher_ut("tcp://mqtt_test_container:11883", ValidTopic, 1);
	EXPECT_EQ( true, mqttPublisher_ut.createNPubMsg(ValidMsg, ValidTopic) );
}
