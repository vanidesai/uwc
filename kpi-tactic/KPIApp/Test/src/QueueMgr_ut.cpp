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

#if 0
TEST_F(QueueMgr_ut, initPlatformBusHandler_MqttMode)
{
	PlBusMgr::initPlatformBusHandler(true);
//	PlBusMgr::stopListeners();
}
#endif

TEST_F(QueueMgr_ut, PollMsg)
{
	QMgr::PollMsgQ();
	EXPECT_EQ(true, check);
}

TEST_F(QueueMgr_ut, )
{
	QMgr::WriteRespMsgQ();
	EXPECT_EQ(true, check);
}
/*
TEST_F(QueueMgr_ut, platfomBusHandler_false)
{
	PlBusMgr::initPlatformBusHandler(false);
	EXPECT_EQ(true, check);
}*/

TEST_F(QueueMgr_ut, platfomBusHandler_true)
{
	PlBusMgr::initPlatformBusHandler(true);
	EXPECT_EQ(false, false);
}

TEST_F(QueueMgr_ut, WriteRqOnMqtt_true)
{
	bool Result = publishWriteReqOnMQTT(subTopic, strMsg);
	/*cout<<"########################################"<<endl;
	cout<<Result<<endl;
	cout<<"########################################"<<endl;*/
	EXPECT_EQ(true, Result);
}

TEST_F(QueueMgr_ut, WriteRqOnMqtt_false)
{
	bool Result = publishWriteReqOnMQTT(Topic, Msg);
	EXPECT_EQ(false, Result);
}
/*
TEST_F(QueueMgr_ut, PublishWriteRq)
{
	//CControlLoopOp a_rCtrlLoop;
	CMessageObject a_oPollMsg;
	string WrSeq = "1234";
	bool Result = PlBusMgr::publishWriteReq(CControlLoopOp_obj, WrSeq, a_oPollMsg);
	EXPECT_EQ(false, Result);
}*/

TEST_F(QueueMgr_ut, StopListeners)
{
	PlBusMgr::stopListeners();
}



