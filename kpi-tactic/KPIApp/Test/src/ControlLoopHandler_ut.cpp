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
/*
TEST_F(ControlLoopHandler_ut, Thread)
{
	bool RetVal = CControlLoopOp_obj.startThread();
}
*/

TEST_F(ControlLoopHandler_ut, DummyAnalysisMsg)
{
	std::string AppSeq = "1234";
	CControlLoopOp_obj.postDummyAnalysisMsg("AppSeq", "WrReqInitFailed");
}

TEST_F(ControlLoopHandler_ut, triggerLoops_false)
{
	std::string Point = "3454";
	//CControlLoopMapper CControlLoopMapper_obj;
	bool RetVal = CKPIAppConfig::getInstance().getControlLoopMapper().triggerControlLoops(Point, recvdMsg);
	EXPECT_EQ(false, RetVal);
}

TEST_F(ControlLoopHandler_ut, ConfigLoops_true)
{
	/*std::string strPolledTopic{""};
		std::string sPollKey{strPolledTopic + "/update"};
		std::vector<CControlLoopOp> oTemp;
		m_oControlLoopMap.emplace(sPollKey, oTemp);*/
	//bool RetVal = CKPIAppConfig::getInstance().getControlLoopMapper().configControlLoopOps(false);
	bool RetVal = CKPIAppConfig::getInstance().getControlLoopMapper().configControlLoopOps(CKPIAppConfig::getInstance().isRTModeForWriteOp());
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


TEST_F(ControlLoopHandler_ut, PublishWtRq_false)
{
	bool RetVal = CKPIAppConfig::getInstance().getControlLoopMapper().publishWriteReq(CControlLoopOp_obj, strMsg, recvdMsg);
	EXPECT_EQ(0, RetVal);
}



TEST_F(ControlLoopHandler_ut, destroyCtx)
{

	std::string topicType = "sub";
	zmq_handler::stZmqSubContext objTempSubCtx;
	void* msgbus_ctx;
	std::string Topic = "TCP_WrResp";
	//std::string Topic = "TCP_RdResp";
	/*CcommonEnvManager::Instance().addTopicToList("TCP/TCP_WrResp_RT");

	char** ppcTopics = CfgManager::Instance().getEnvClient()->get_topics_from_env(topicType.c_str());

	config_t* config = CfgManager::Instance().getEnvClient()->get_messagebus_config(
									CfgManager::Instance().getConfigClient(),
									ppcTopics , 1, topicType.c_str());

	void* msgbus_ctx = msgbus_initialize(config);

	zmq_handler::stZmqContext Ctxt_obj(msgbus_ctx);
	zmq_handler::stZmqSubContext SubCtx_obj;
	zmq_handler::insertCTX("UTTopic1__PolledData_RT", Ctxt_obj);*/
	CcommonEnvManager::Instance().addTopicToList("TCP_WrResp");
	/*zmq_handler::stZmqContext objTempCtx{msgbus_ctx};
	zmq_handler::insertCTX(Topic, objTempCtx);
	zmq_handler::stZmqContext& context = zmq_handler::getCTX(Topic);
	zmq_handler::insertSubCTX(Topic, objTempSubCtx);
	zmq_handler::stZmqSubContext& subContext = zmq_handler::getSubCTX(Topic);*/

	//std::vector<std::string> vFullTopics = {"TCP1/TCP1_PolledData", "TCP1/TCP1_RdResp", "TCP1/TCP1_WrResp"};

	bool RetVal = CKPIAppConfig::getInstance().getControlLoopMapper().destroySubCtx();
	//EXPECT_EQ(true, RetVal);
	EXPECT_EQ(false, RetVal);

}


