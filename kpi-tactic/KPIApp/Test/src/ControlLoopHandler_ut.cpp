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

#include "../include/ControlLoopHandler_ut.hpp"

void ControlLoopHandler_ut::SetUp()
{
	// Setup code
}

void ControlLoopHandler_ut::TearDown()
{
	// TearDown code
}

/**
 * Test case to check if DummyAnalysisMsg() function posts a dummy analysis message in case of error successfully
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ControlLoopHandler_ut, DummyAnalysisMsg)
{
	std::string AppSeq = "12-34";
	CControlLoopOp_obj.postDummyAnalysisMsg("AppSeq", "WrReqInitFailed");
}

/**
 * Test case to check if postDummyAnalysisMsg() function posts a dummy analysis message in case of error successfully
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ControlLoopHandler_ut, PostDummyanalysisMsg)
{
	std::string AppSeq = "1234";
	CControlLoopOp_obj.postDummyAnalysisMsg(stPollWrData_obj, AppSeq, "WrReqInitFailed");
}

/**
 * Test case to check if isPresent() function's  behavior
 * @param :[in] None
 * @param :[out] None
 * @return bool
 */
TEST_F(ControlLoopHandler_ut, present_check)
{
	std::string sID = "site-id";
	std::string sLastWrSeqVal{""};
	bool result = CMapOfReqMapper::getInstace().isPresent(sID, sLastWrSeqVal);
	EXPECT_EQ(0, result);
}
/**
 * Test case to check if triggerControlLoops() function fails to receive  a polling message and returns false on failure
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ControlLoopHandler_ut, triggerLoops_false)
{
	std::string Point = "3454";
	//CControlLoopMapper CControlLoopMapper_obj;
	bool RetVal = CKPIAppConfig::getInstance().getControlLoopMapper().triggerControlLoops(Point, recvdMsg);
	EXPECT_EQ(true, RetVal);
}

/**
 * Test case to check if configControlLoopOps() function doesn't start control loop threads when control loop map is empty, and return true
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ControlLoopHandler_ut, ConfigLoops_EmptyContrlLoopMap)
{

    EXPECT_EQ( true, CKPIAppConfig::getInstance().getControlLoopMapper().configControlLoopOps(true) );
}

/**
 * Test case to check if configControlLoopOps() function Starts control loop threads successfully when control loop map is not empty
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ControlLoopHandler_ut, ConfigLoops_nonEmptyContrlLoopMap)
{
    bool RetVal = CKPIAppConfig::getInstance().parseYMLFile("ControlLoopConfig.yml");

    if( true == RetVal)
    {
        EXPECT_EQ( true, CKPIAppConfig::getInstance().getControlLoopMapper().configControlLoopOps(true) );

        g_stopThread.store(true);
        CKPIAppConfig::getInstance().getControlLoopMapper().stopControlLoopOps();
        g_stopThread.store(false);
    }
    else
    {
        std::cout << std::endl << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>UT Error: " << std::endl;
        std::cout << std::endl << "     ControlLoopHandler_ut.ConfigLoops_nonEmptyContrlLoopMap can't be tested, Control loop map is empty" << std::endl;
        EXPECT_EQ(true, RetVal);
    }
}

/**
 * Test case to check if stopControlLoopOps() function Stops control loop threads successfully and returns true on success
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ControlLoopHandler_ut, stopLoops)
{
	bool RetVal = CKPIAppConfig::getInstance().getControlLoopMapper().stopControlLoopOps();
	EXPECT_EQ(1, RetVal);
}

/**
 * Test case to check if stopThread() function Stops control loop threads successfully and returns true on success
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ControlLoopHandler_ut, StopThread)
{
	bool RetVal = CControlLoopOp_obj.stopThread();
	EXPECT_EQ(true, RetVal);
}

/**
 * Test case to check if isControlLoopPollPoint() function Checks whether given polling topic is a part of one of the control loops and returns false on failure
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ControlLoopHandler_ut, ControlLoopPolledPoint_false)
{
	std::string PollTopic = "KPIAPP_WrReq,KPIAPP_WrReq_RT";
	bool RetVal = CKPIAppConfig::getInstance().getControlLoopMapper().isControlLoopPollPoint(PollTopic);
	EXPECT_EQ(0, RetVal);
}

/**
 * Test case to check if isControlLoopPollPoint() function Checks whether given polling topic is a part of one of the control loops and returns false on failure
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ControlLoopHandler_ut, ControlLoopWtRspPoint_false)
{
	std::string WrRspTopic = " respTCP_WrResp";
	bool RetVal = CKPIAppConfig::getInstance().getControlLoopMapper().isControlLoopWrRspPoint(WrRspTopic);
	EXPECT_EQ(0, RetVal);
}

/**
 * Test case to check if publishWriteReq() function Checks whether given polling topic is a part of one of the control loops and returns false on failure
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ControlLoopHandler_ut, PublishWtRq_false)
{
	bool RetVal = CKPIAppConfig::getInstance().getControlLoopMapper().publishWriteReq(CControlLoopOp_obj, strMsg, recvdMsg);
	EXPECT_EQ(1, RetVal);
}

/**
 * Test case to check if destroySubCtx() function Destroys the sub contexts returns false on failure
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
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

/**
 * Test case to check if pushAnalysisMsg() function's behavior
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(ControlLoopHandler_ut, PushAnalysisMsg)
{
	struct stPollWrData a_stPollWrData;
	CMessageObject a_msgWrResp;
	CControlLoopMapper& oCtrlLoopMapper = CKPIAppConfig::getInstance().getControlLoopMapper();
	oCtrlLoopMapper.pushAnalysisMsg(a_stPollWrData, a_msgWrResp);

}
