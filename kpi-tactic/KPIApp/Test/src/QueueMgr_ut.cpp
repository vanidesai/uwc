/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

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
TEST_F(QueueMgr_ut, )
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
	CMessageObject a_oPollMsg;
	std::string WrSeq = "1234";
	bool Result = PlBusMgr::publishWriteReq(CControlLoopOp_obj, WrSeq, a_oPollMsg);
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



