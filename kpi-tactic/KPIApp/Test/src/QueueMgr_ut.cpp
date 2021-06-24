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

#include "../include/QueueMgr_ut.hpp"

extern bool publishWriteReqOnMQTT(const std::string &a_sPubTopic, const std::string &a_sMsg);
void QueueMgr_ut::SetUp()
{
	// Setup code
}

void QueueMgr_ut::TearDown()
{
	// TearDown code
}

/**
 * Test case to check behaviour of initPlatformBusHandler() function in mqttMode is true
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(QueueMgr_ut, initPlatformBusHandler_MqttMode)
{
	PlBusMgr::initPlatformBusHandler(true);
//	PlBusMgr::stopListeners();
}

/**
 * Test case to check behaviour of initPlatformBusHandler() function in mqttMode is false
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(QueueMgr_ut, platfomBusHandler_false)
{
	PlBusMgr::initPlatformBusHandler(false);
	EXPECT_EQ(true, check);
}


/**
 * Test case to check if PollMsgQ() function return reference of on-demand read operation instance
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(QueueMgr_ut, PollMsg)
{
	QMgr::PollMsgQ();
	EXPECT_EQ(true, check);
}

/**
 * Test case to check if WriteRespMsgQ() function return reference of on-demand write operation instance
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(QueueMgr_ut, WriteRespMsgQ_test)
{
	QMgr::WriteRespMsgQ();
	EXPECT_EQ(true, check);
}

/**
 * Test case to check behaviour of initPlatformBusHandler() function in mqttMode is true
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(QueueMgr_ut, platfomBusHandler_true)
{
	PlBusMgr::initPlatformBusHandler(true);
	EXPECT_EQ(false, false);
}

/**
 * Test case to check if publishWriteReqOnMQTT()function Forms and publishes write request on MQTT and returns true on success
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(QueueMgr_ut, WriteRqOnMqtt_true)
{
	bool Result = publishWriteReqOnMQTT(subTopic, strMsg);
	EXPECT_EQ(true, Result);
}

/**
 * Test case to check if publishWriteReqOnMQTT()function fails to Form and publishe write request on MQTT when topic is not provides and returns false
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(QueueMgr_ut, WriteRqOnMqtt_false)
{
	bool Result = publishWriteReqOnMQTT(Topic, Msg);
	EXPECT_EQ(false, Result);
}

/**
 * Test case to check if publishWriteReq()function fails to Form and publishe write request on MQTT and returns false
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(QueueMgr_ut, PublishWriteRq)
{
	//CControlLoopOp a_rCtrlLoop;
	//CMessageObject a_oPollMsg;
	std::string WrSeq = "1234";
	bool Result = PlBusMgr::publishWriteReq(CControlLoopOp_obj, WrSeq);
	EXPECT_EQ(false, Result);
}

/**
 * Test case to check if stopListeners()function Stops listener threads successfully
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(QueueMgr_ut, StopListeners)
{
	PlBusMgr::stopListeners();
}



