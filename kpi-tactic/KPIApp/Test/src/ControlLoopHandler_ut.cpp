/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../include/ControlLoopHandler_ut.hpp"

void ControlLoopHandler_ut::SetUp()
{
	// Setup code
}

void ControlLoopHandler_ut::TearDown()
{
	// TearDown code
}
/*Need to check
TEST_F(ControlLoopHandler_ut, DummyAnalysisMsg)
{
	std::string AppSeq = "1234";
	CControlLoopOp_obj.postDummyAnalysisMsg("AppSeq", "WrReqInitFailed");
}*/

TEST_F(ControlLoopHandler_ut, triggerLoops_false)
{
	std::string Point = "3454";
	//CControlLoopMapper CControlLoopMapper_obj;
	bool RetVal = CKPIAppConfig::getInstance().getControlLoopMapper().triggerControlLoops(Point, recvdMsg);
	EXPECT_EQ(false, RetVal);
}

TEST_F(ControlLoopHandler_ut, ConfigLoops_true)
{
	bool RetVal = CKPIAppConfig::getInstance().getControlLoopMapper().configControlLoopOps(true);
	EXPECT_EQ(true, RetVal);
}

TEST_F(ControlLoopHandler_ut, stopLoops)
{
	bool RetVal = CKPIAppConfig::getInstance().getControlLoopMapper().stopControlLoopOps();
	EXPECT_EQ(1, RetVal);
}

TEST_F(ControlLoopHandler_ut, StopThread)
{
	bool RetVal = CControlLoopOp_obj.stopThread();
	EXPECT_EQ(true, RetVal);
}

TEST_F(ControlLoopHandler_ut, ControlLoopPolledPoint_false)
{
	std::string PollTopic = "KPIAPP_WrReq,KPIAPP_WrReq_RT";
	bool RetVal = CKPIAppConfig::getInstance().getControlLoopMapper().isControlLoopPollPoint(PollTopic);
	EXPECT_EQ(0, RetVal);
}


TEST_F(ControlLoopHandler_ut, ControlLoopWtRspPoint_false)
{
	std::string WrRspTopic = " respTCP_WrResp";
	bool RetVal = CKPIAppConfig::getInstance().getControlLoopMapper().isControlLoopWrRspPoint(WrRspTopic);
	EXPECT_EQ(0, RetVal);
}
/*Need to check

TEST_F(ControlLoopHandler_ut, PublishWtRq_false)
{
	bool RetVal = CKPIAppConfig::getInstance().getControlLoopMapper().publishWriteReq(CControlLoopOp_obj, "", recvdMsg);
	EXPECT_EQ(0, RetVal);
}
*/


TEST_F(ControlLoopHandler_ut, destroyCtx)
{
	std::vector<std::string> vFullTopics = {"TCP1/TCP1_PolledData", "TCP1/TCP1_RdResp", "TCP1/TCP1_WrResp"};

	bool RetVal = CKPIAppConfig::getInstance().getControlLoopMapper().destroySubCtx();
	cout<<"###############################"<<endl;
	cout<<RetVal<<endl;
	cout<<"###############################"<<endl;

}


