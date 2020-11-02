/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../include/EISPlBusHandler_ut.hpp"

extern bool publishEISMsg(string eisMsg, zmq_handler::stZmqContext &context,
		zmq_handler::stZmqPubContext &pubContext);

extern vector<std::thread> g_vThreads;

void EISPlBusHandler_ut::SetUp()
{
	// Setup code
}

void EISPlBusHandler_ut::TearDown()
{
	// TearDown code
}

/***********************************************************/
/*	Helper function	*/
void initEISContext_Caller(bool& RetVal)
{
	CEISPlBusHandler CEISPlBusHandler_obj;
	RetVal = CEISPlBusHandler_obj.initEISContext();
}
/***********************************************************/
#if 0
TEST_F(EISPlBusHandler_ut, initEISContext)
{
	bool RetVal = false;

	std::thread Target_Caller( initEISContext_Caller, std::ref(RetVal) );
	Target_Caller.join();

	EXPECT_EQ(true, RetVal);

}
#endif

TEST_F(EISPlBusHandler_ut, EISListerners_true)
{
	bool a_bIsPollingRT = true;
	bool a_bIsWrOpRT = true;

	std::string topicType = "pub";


	CcommonEnvManager::Instance().addTopicToList("UTTopic1__PolledData_RT");

	char** ppcTopics = CfgManager::Instance().getEnvClient()->get_topics_from_env(topicType.c_str());

	config_t* config = CfgManager::Instance().getEnvClient()->get_messagebus_config(
								CfgManager::Instance().getConfigClient(),
								ppcTopics , 1, topicType.c_str());

	void* msgbus_ctx = msgbus_initialize(config);

	zmq_handler::stZmqContext Ctxt_obj(msgbus_ctx);
	zmq_handler::stZmqSubContext SubCtx_obj;
	zmq_handler::insertCTX("UTTopic1__PolledData_RT", Ctxt_obj);
//	zmq_handler::insertSubCTX("UTTopic1__PolledData_RT", SubCtx_obj);

	CEISPlBusHandler_obj.configEISListerners(a_bIsPollingRT, a_bIsWrOpRT);
//	std::this_thread::sleep_for(std::chrono::seconds(60));
//	CEISPlBusHandler_obj.stopEISListeners();
}

TEST_F(EISPlBusHandler_ut, EISListerners_false)
{
	bool a_bIsPollingRT = false;
	bool a_bIsWrOpRT = false;
	CEISPlBusHandler_obj.configEISListerners(a_bIsPollingRT, a_bIsWrOpRT);
}

TEST_F(EISPlBusHandler_ut, EISListerners_true_false)
{
	bool a_bIsPollingRT = true;
	bool a_bIsWrOpRT = false;
	CEISPlBusHandler_obj.configEISListerners(a_bIsPollingRT, a_bIsWrOpRT);
}

TEST_F(EISPlBusHandler_ut, EISListerners_false_true)
{
	bool a_bIsPollingRT = false;
	bool a_bIsWrOpRT = true;
	CEISPlBusHandler_obj.configEISListerners(a_bIsPollingRT, a_bIsWrOpRT);
}

TEST_F(EISPlBusHandler_ut, stopEISListener)
{
	CEISPlBusHandler_obj.stopEISListeners();
	EXPECT_EQ(1,1);
}

TEST_F(EISPlBusHandler_ut, PublishWriteMsg_false)
{
	bool RetVal = CEISPlBusHandler_obj.publishWriteMsg(strMsg);
	EXPECT_EQ(0, RetVal);
}

TEST_F(EISPlBusHandler_ut, EISMsg_ctx_Map_Empty)
{
	std::string eisTopic = "KPIAPP_WrReq";
	zmq_handler::stZmqPubContext objTempPubCtx;
	/*zmq_handler::stZmqContext context;
	void *m_pContext;
	void *msgbus_ctx = context.m_pContext;*/

	std::string topicType = "pub";
	//eisTopic.assign(EnvironmentInfo::getInstance().getDataFromEnvMap("WriteRequest_RT"));
	zmq_handler::stZmqPubContext pubctx;
	//config_t* config;
	//void* msgbus_ctx = msgbus_initialize(config);
	void* msgbus_ctx;
	try
	{
		zmq_handler::stZmqContext objTempCtx{msgbus_ctx};
		zmq_handler::insertCTX(eisTopic, objTempCtx);
		zmq_handler::stZmqContext& context = zmq_handler::getCTX(eisTopic);
		zmq_handler::insertPubCTX(eisTopic, objTempPubCtx);
		zmq_handler::stZmqPubContext& pubContext = zmq_handler::getPubCTX(eisTopic);
	    bool RetVal = publishEISMsg(strMsg, context, pubContext);
		EXPECT_EQ(true, RetVal);

	}
	catch(exception &e)
	{
		EXPECT_EQ(false, true);
	}

}

