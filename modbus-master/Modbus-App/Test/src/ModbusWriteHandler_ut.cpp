/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/


#include "../include/ModbusWriteHandler_ut.hpp"
#include "ConfigManager.hpp"
#include "cjson/cJSON.h"

#include "ZmqHandler.hpp"




extern int hex2bin(const std::string &src, int iOpLen, uint8_t* target);
extern int char2int(char input);

//extern sem_t semaphoreWriteReq;
void ModbusWriteHandler_ut::SetUp()
{
	// Setup code
}

void ModbusWriteHandler_ut::TearDown()
{
	// TearDown code
}


TEST_F(ModbusWriteHandler_ut, jsonParserForWrite_InvServiceReq)
{

	stMbusApiPram.m_stOnDemandReqData.m_isByteSwap = true;
	stMbusApiPram.m_stOnDemandReqData.m_isRT = true;
	stMbusApiPram.m_stOnDemandReqData.m_isWordSwap = true;
	stMbusApiPram.m_stOnDemandReqData.m_obtReqRcvdTS.tv_nsec = 21132323;
	stMbusApiPram.m_stOnDemandReqData.m_obtReqRcvdTS.tv_sec = 1;
	stMbusApiPram.m_stOnDemandReqData.m_strAppSeq = "455";
	stMbusApiPram.m_stOnDemandReqData.m_strEisTime = "2020-03-31 12:34:56";
	stMbusApiPram.m_stOnDemandReqData.m_strMetric = "Test";
	stMbusApiPram.m_stOnDemandReqData.m_strMqttTime = "2020-03-13 12:34:56";
	stMbusApiPram.m_stOnDemandReqData.m_strTopic = "UnitTest";
	stMbusApiPram.m_stOnDemandReqData.m_strVersion ="2.1";
	stMbusApiPram.m_stOnDemandReqData.m_strWellhead = "test";

	bool isWrite = false;

	try
	{
		eFunRetType = onDemandHandler::Instance().jsonParserForOnDemandRequest(stMbusApiPram,
				m_u8FunCode,
				stMbusApiPram.m_u16TxId,
				isWrite);
		EXPECT_EQ(APP_ERROR_UNKNOWN_SERVICE_REQUEST, eFunRetType);
	}
	catch( exception &e)
	{
		EXPECT_EQ("", e.what());
	}

}

#if 0 //SPRINT13CHANGES
TEST_F(ModbusWriteHandler_ut, jsonParserForOnDemandRequest_Test)
{

	bool isWrite = true;


	reqData.m_isByteSwap = true;
	reqData.m_isWordSwap = false;
	reqData.m_obtReqRcvdTS.tv_nsec = 21132323;
	reqData.m_obtReqRcvdTS.tv_sec = 1;
	reqData.m_strAppSeq = "455";
	reqData.m_strMetric = "Test";
	reqData.m_strTopic = "kzdjfhdszh";
	reqData.m_strVersion = "2.1";
	reqData.m_strWellhead = "test";

	try
	{
		eFunRetType = onDemandHandler::Instance().jsonParserForOnDemandRequest(stMbusApiPram,
				m_u8FunCode,
				stMbusApiPram.m_u16TxId,
				isWrite);

		EXPECT_EQ(APP_SUCCESS, eFunRetType);

	}
	catch( exception &e)
	{
		EXPECT_EQ("", e.what());
	}
}
#endif

#if 0 //SPRINT13CHANGES
/* Invalid topic/msg */
TEST_F(ModbusWriteHandler_ut, jsonParserForWrite_InvalidTopicMsg)
{
	writeReq.m_strTopic = "Topic_Inv";
	writeReq.m_strMsg = "Msg_Inv";

	root = cJSON_Parse(writeReq.m_strMsg.c_str());

	cJSON *appseq = cJSON_GetObjectItem(root,"app_seq");
	cJSON *cmd=cJSON_GetObjectItem(root,"command");
	cJSON *value=cJSON_GetObjectItem(root,"value");
	cJSON *wellhead=cJSON_GetObjectItem(root,"wellhead");
	cJSON *version=cJSON_GetObjectItem(root,"version");
	cJSON *sourcetopic=cJSON_GetObjectItem(root,"sourcetopic");
	cJSON *timestamp=cJSON_GetObjectItem(root,"timestamp");
	cJSON *usec=cJSON_GetObjectItem(root,"usec");
	cJSON *mqttTime=cJSON_GetObjectItem(root,"tsMsgRcvdFromMQTT");
	cJSON *eisTime=cJSON_GetObjectItem(root,"tsMsgPublishOnEIS");
	void* ptrAppCallback = NULL;
	bool isWrite = true;

	try
	{

		//eFunRetType = modWriteHandler::Instance(msgbusMgr, CallerObj, msgbusEnvelope).jsonParserForOnDemandRequest(writeReq, stMbusApiPram, m_u8FunCode, stMbusApiPram.m_u16TxId);
		eFunRetType = onDemandHandler::Instance().jsonParserForOnDemandRequest(root, stMbusApiPram, m_u8FunCode, stMbusApiPram.m_u16TxId, isWrite, &ptrAppCallback);
		EXPECT_EQ(APP_ERROR_INVALID_INPUT_JSON, eFunRetType);
		//EXPECT_EQ(MBUS_APP_ERROR_UNKNOWN_SERVICE_REQUEST, eFunRetType);
	}

	catch( exception &e)
	{
		EXPECT_EQ("", e.what());

	}
}
#endif

/*** Test:ModbusWriteHandler_ut::modWriteHandler_getInstance() Check the instance type returned by function ***/

TEST_F(ModbusWriteHandler_ut, modWriteHandler_getInstance)
{


	EXPECT_EQ(typeid(onDemandHandler), typeid(onDemandHandler::Instance()));
}


//test 03:: Check the instance type returned by function

#if 0 //SPRINT13CHANGES
TEST_F(ModbusWriteHandler_ut, jsonParserForWrite)
{

	void* ptrAppCallback = NULL;
	//	pstModbusRxPacket->m_u16ReffId = 1;

	bool isWrite = false;

	root = cJSON_Parse(writeReq.m_strMsg.c_str());

	cJSON *appseq = cJSON_GetObjectItem(root,"app_seq");
	cJSON *cmd=cJSON_GetObjectItem(root,"command");
	cJSON *value=cJSON_GetObjectItem(root,"value");
	cJSON *wellhead=cJSON_GetObjectItem(root,"wellhead");
	cJSON *version=cJSON_GetObjectItem(root,"version");
	cJSON *sourcetopic=cJSON_GetObjectItem(root,"sourcetopic");
	cJSON *timestamp=cJSON_GetObjectItem(root,"timestamp");
	cJSON *usec=cJSON_GetObjectItem(root,"usec");
	cJSON *mqttTime=cJSON_GetObjectItem(root,"tsMsgRcvdFromMQTT");
	cJSON *eisTime=cJSON_GetObjectItem(root,"tsMsgPublishOnEIS");


	try
	{
		//EXPECT_EQ(typeid(MbusStackErrorCode), typeid(modWriteHandler::Instance(msgbusMgr, CallerObj, msgbusEnvelope).jsonParserForOnDemandRequest( writeReq, stMbusApiPram, m_u8FunCode, stMbusApiPram.m_u16TxId)));
		EXPECT_EQ(typeid(eFunRetType),
				typeid(onDemandHandler::Instance().jsonParserForOnDemandRequest(root,
						stMbusApiPram,
						m_u8FunCode,
						stMbusApiPram.m_u16TxId,
						isWrite,
						&ptrAppCallback)));

	}
	catch(std::exception &e)
	{

		std::cout<<e.what()<<endl;
		EXPECT_EQ("", e.what());
	}
}
#endif


/**Test::ModbusWriteHandler_ut::createWriteListner_test()
    checks the behaviour of the createWriteListener() function for valid topic andtype of topic
 **/

TEST_F(ModbusWriteHandler_ut, createWriteListner_test)
{

	/*setenv("WRITE_RESPONSE_TOPIC", "PL_0", 1);

	std::string topic = std::getenv("WRITE_RESPONSE_TOPIC");*/

	std::string sTopic = "MQTT-Export/PL0_flowmeter1_write";
	//std::string sTopic = "MQTT-Export/Modbus-TCP-Master_ReadRequest";
	std::string topictype = "sub";
	try
	{

		onDemandHandler::Instance().createOnDemandListener();

	}
	catch(std::exception &e)
	{

		cout<<e.what()<<endl;
		EXPECT_EQ("basic_string::_M_construct null not valid", (string)e.what());
	}
}



/***Test ::ModbusWriteHandler_ut::subscribeDeviceListener()
    Check the behaviour of subscribeDeviceListener() function***/
#if 0 //SPRINT13CHANGES
TEST_F(ModbusWriteHandler_ut, subscribeDeviceListener)
{

	msg_envelope_t *msg = 0xFF00;
	globalConfig::COperation a_refOps;
	try
	{

		onDemandHandler::Instance().subscribeDeviceListener(topic, a_refOps);

	}
	catch(std::exception &e)
	{

		std::cout<<e.what()<<endl;
		EXPECT_EQ("",(string)e.what());
	}

}
#endif

TEST_F(ModbusWriteHandler_ut, hex_to_bin)
{

	stMbusApiPram.m_lPriority = 1;
	//stMbusApiPram.m_pu8Data = NULL;
	stMbusApiPram.m_u16ByteCount = 2;
	stMbusApiPram.m_u16Port = 1234;
	stMbusApiPram.m_u16Quantity = 4;
	stMbusApiPram.m_u16StartAddr = 56436524;
	stMbusApiPram.m_u16TxId = 23;
	//	stMbusApiPram.m_u32mseTimeout = 45;
	stMbusApiPram.m_u8DevId = 5;
	stMbusApiPram.m_u8IpAddr[4];
	//stMbusApiPram.m_pu8Data = new uint8_t[stMbusApiPram.m_u16ByteCount]();
	stMbusApiPram.m_pu8Data[5];

	try
	{
		hex2bin(tValue, stMbusApiPram.m_u16ByteCount, stMbusApiPram.m_pu8Data);

	}
	catch(std::exception &e)
	{
		EXPECT_NE("", (string)e.what());
	}

}


TEST_F(ModbusWriteHandler_ut, hex_to_bin_test)
{

	stMbusApiPram.m_lPriority = 1;
	//stMbusApiPram.m_pu8Data = NULL;
	stMbusApiPram.m_u16ByteCount = 2;
	stMbusApiPram.m_u16Port = 1234;
	stMbusApiPram.m_u16Quantity = 4;
	stMbusApiPram.m_u16StartAddr = 56436524;
	stMbusApiPram.m_u16TxId = 23;
	//	stMbusApiPram.m_u32mseTimeout = 45;
	stMbusApiPram.m_u8DevId = 5;
	stMbusApiPram.m_u8IpAddr[4];
	//stMbusApiPram.m_pu8Data = new uint8_t[stMbusApiPram.m_u16ByteCount]();
	stMbusApiPram.m_pu8Data[5];

	try
	{
		int result = hex2bin("input", stMbusApiPram.m_u16ByteCount, stMbusApiPram.m_pu8Data);
		EXPECT_EQ(-1, result);

	}
	catch(std::exception &e)
	{
		EXPECT_NE("", (string)e.what());
	}

}




TEST_F(ModbusWriteHandler_ut, char_2_int)
{
	hex2bin(tValue, stMbusApiPram.m_u16ByteCount, stMbusApiPram.m_pu8Data);
	try
	{
		byte1 = char2int(tValue[i])*16 + char2int(tValue[i+1]);

	}
	catch(std::exception &e)
	{
		EXPECT_EQ("Invalid input string",(string)e.what());
	}
}

#if 0 //SPRINT13CHANGES
TEST_F(ModbusWriteHandler_ut, Validate_Json)
{
	writeReq.m_strTopic = topic;
	writeReq.m_strMsg = msg;

	cJSON *root = cJSON_Parse(writeReq.m_strMsg.c_str());
	cJSON *cmd=cJSON_GetObjectItem(root,"command");
	cJSON *wellhead=cJSON_GetObjectItem(root,"wellhead");
	cJSON *sourcetopic=cJSON_GetObjectItem(root,"sourcetopic");


	strCommand = cmd->valuestring;
	//strWellhead = wellhead->valuestring;
	strWellhead = "PL1";

	strSourceTopic = sourcetopic->valuestring;

	try
	{
		bool Value = onDemandHandler::Instance().validateInputJson(strSourceTopic, strWellhead, strCommand);
		EXPECT_EQ(0, Value);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ", (string)e.what());

	}

}
#endif

#if 0 //SPRINT13CHANGES
TEST_F(ModbusWriteHandler_ut, Validate_Json_catch)
{
	writeReq.m_strTopic = topic;
	writeReq.m_strMsg = msg;

	cJSON *root = cJSON_Parse(writeReq.m_strMsg.c_str());
	cJSON *cmd=cJSON_GetObjectItem(root,"command");
	cJSON *wellhead=cJSON_GetObjectItem(root," ");
	cJSON *sourcetopic=cJSON_GetObjectItem(root,"sourcetopic");


	strCommand = cmd->valuestring;
	//strWellhead = wellhead->valuestring;
	strWellhead = "PL1";

	strSourceTopic = sourcetopic->valuestring;

	try
	{
		bool Value = onDemandHandler::Instance().validateInputJson(strSourceTopic, strWellhead, strCommand);
		EXPECT_EQ(0, Value);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ", (string)e.what());

	}

}
#endif


#if 0 //SPRINT13CHANGES
TEST_F(ModbusWriteHandler_ut, process_msg)
{
	eMbusRequestType reqType = MBUS_REQUEST_NONE;
	msg_envelope_t *g_msg = NULL;
	msg_envelope_elem_body_t* msgCommand = msgbus_msg_envelope_new_string("DValve");
	msg_envelope_elem_body_t* msgAppSeq = msgbus_msg_envelope_new_string("1234");
	msg_envelope_elem_body_t* msgValue = msgbus_msg_envelope_new_string("0x1234");
	g_msg = msgbus_msg_envelope_new(CT_JSON);
	msgbus_msg_envelope_put(g_msg, "command", msgCommand);
	msgbus_msg_envelope_put(g_msg, "app_seq", msgAppSeq);
	msgbus_msg_envelope_put(g_msg, "value", msgValue);


	msg_envelope_elem_body_t* ptwellhead = msgbus_msg_envelope_new_string("PL0");
	msg_envelope_elem_body_t* ptcommand = msgbus_msg_envelope_new_string("Flow");
	msg_envelope_elem_body_t* ptvalue = msgbus_msg_envelope_new_string("0x00");
	msg_envelope_elem_body_t* pttimestamp = msgbus_msg_envelope_new_string("2019-09-20 12:34:56");
	msg_envelope_elem_body_t* ptusec = msgbus_msg_envelope_new_string("1571887474111145");
	msg_envelope_elem_body_t* ptversion = msgbus_msg_envelope_new_string("2.0");
	msg_envelope_elem_body_t* ptapp_seq = msgbus_msg_envelope_new_string("1234");
	msg_envelope_elem_body_t* ptsourcetopic = msgbus_msg_envelope_new_string("/flowmeter/PL0/Flow/read");

	msgbus_msg_envelope_put(g_msg, "wellhead", ptwellhead);
	msgbus_msg_envelope_put(g_msg, "command", ptcommand);
	msgbus_msg_envelope_put(g_msg, "value", ptvalue);
	msgbus_msg_envelope_put(g_msg, "timestamp", pttimestamp);
	msgbus_msg_envelope_put(g_msg, "usec", ptusec);
	msgbus_msg_envelope_put(g_msg, "version", ptversion);
	msgbus_msg_envelope_put(g_msg, "app_seq", ptapp_seq);
	msgbus_msg_envelope_put(g_msg, "sourcetopic", ptsourcetopic);
	bool result = onDemandHandler::Instance().processMsg(g_msg, "test");
	EXPECT_EQ(result, true);
}
#endif


/********************FOR Sub****************************/


TEST_F(ModbusWriteHandler_ut, thread_init_DValve_sub)
{

	std::string sRespTopic = "";

	//******************** As we are not getting the msg from zmq
	//so we are calling the publishJson() function to get the msg************

	msg_envelope_t *g_msg = NULL;
	std::string strngMsg= "Subscribe";
	//strngMsg = "Subscribe";
	//sem_post(&semaphoreWriteReq);
	try
	{


		msg_envelope_elem_body_t* msgCommand =msgbus_msg_envelope_new_string("DValve");
		msg_envelope_elem_body_t* msgAppSeq = msgbus_msg_envelope_new_string("1234");
		msg_envelope_elem_body_t* msgValue = msgbus_msg_envelope_new_string("0x1234");
		//g_msg = msgbus_msg_envelope_new(CT_JSON);
		g_msg = msgbus_msg_envelope_new(CT_JSON);
		msgbus_msg_envelope_put(g_msg, "command", msgCommand);
		msgbus_msg_envelope_put(g_msg, "app_seq", msgAppSeq);
		msgbus_msg_envelope_put(g_msg, "value", msgValue);

		msg_envelope_elem_body_t* ptwellhead = msgbus_msg_envelope_new_string("PL0");
		msg_envelope_elem_body_t* ptcommand = msgbus_msg_envelope_new_string("Flow");
		msg_envelope_elem_body_t* ptvalue = msgbus_msg_envelope_new_string("0x00");
		msg_envelope_elem_body_t* pttimestamp = msgbus_msg_envelope_new_string("2019-09-20 12:34:56");
		msg_envelope_elem_body_t* ptusec = msgbus_msg_envelope_new_string("1571887474111145");
		msg_envelope_elem_body_t* ptversion = msgbus_msg_envelope_new_string("2.0");
		msg_envelope_elem_body_t* ptapp_seq = msgbus_msg_envelope_new_string("1234");
		msg_envelope_elem_body_t* ptsourcetopic = msgbus_msg_envelope_new_string("/flowmeter/PL0/Flow/read");

		msgbus_msg_envelope_put(g_msg, "wellhead", ptwellhead);
		msgbus_msg_envelope_put(g_msg, "command", ptcommand);
		msgbus_msg_envelope_put(g_msg, "value", ptvalue);
		msgbus_msg_envelope_put(g_msg, "timestamp", pttimestamp);
		msgbus_msg_envelope_put(g_msg, "usec", ptusec);
		msgbus_msg_envelope_put(g_msg, "version", ptversion);
		msgbus_msg_envelope_put(g_msg, "app_seq", ptapp_seq);
		msgbus_msg_envelope_put(g_msg, "sourcetopic", ptsourcetopic);


		ptcommand = msgbus_msg_envelope_new_string("Arrival");
		msgbus_msg_envelope_put(g_msg, "command", ptcommand);


		std::string topic = "Modbus-TCP-Master_ReadRequest";
		std::string topictype = "sub";

		msgbus_ret_t retVal = MSG_SUCCESS;
		publisher_ctx_t* g_pub_ctx;

		sRespTopic = PublishJsonHandler::instance().getSReadResponseTopic();

		zmq_handler::stZmqContext MsgbusCtx = zmq_handler::getCTX(topic);
		zmq_handler::stZmqPubContext pubCtx = zmq_handler::getPubCTX(topic);

		PublishJsonHandler::instance().publishJson(g_msg, MsgbusCtx.m_pContext, pubCtx.m_pContext, topic);

	}

	catch(std::exception &e)
	{
		EXPECT_NE("", (string)e.what());
	}


}


/****************** For Pub*****************/

TEST_F(ModbusWriteHandler_ut, thread_init_DValve_pub)
{
	std::string sRespTopic = "";


	//sem_post(&semaphoreWriteReq);


	//******************** As we are not getting the msg from zmq
	//so we are calling the publishJson() function to get the msg************

	msg_envelope_t *g_msg = NULL;
	std::string strngMsg;
	strngMsg = "Publish";
	try
	{
		//***************** Check the below commented line once again

		//onDemandHandler::Instance().initWriteHandlerThreads();
		//TODO
		//modWriteHandler::Instance(msgbusMgr, CallerObj, msgbusEnvelope).createWriteListener();

		// Creating message to be published
		msg_envelope_elem_body_t* msgCommand = msgbus_msg_envelope_new_string("DValve");
		msg_envelope_elem_body_t* msgAppSeq = msgbus_msg_envelope_new_string("1234");
		msg_envelope_elem_body_t* msgValue = msgbus_msg_envelope_new_string("0x1234");
		g_msg = msgbus_msg_envelope_new(CT_JSON);
		msgbus_msg_envelope_put(g_msg, "command", msgCommand);
		msgbus_msg_envelope_put(g_msg, "app_seq", msgAppSeq);
		msgbus_msg_envelope_put(g_msg, "value", msgValue);


		msg_envelope_elem_body_t* ptwellhead = msgbus_msg_envelope_new_string("PL0");
		msg_envelope_elem_body_t* ptcommand = msgbus_msg_envelope_new_string("Flow");
		msg_envelope_elem_body_t* ptvalue = msgbus_msg_envelope_new_string("0x00");
		msg_envelope_elem_body_t* pttimestamp = msgbus_msg_envelope_new_string("2019-09-20 12:34:56");
		msg_envelope_elem_body_t* ptusec = msgbus_msg_envelope_new_string("1571887474111145");
		msg_envelope_elem_body_t* ptversion = msgbus_msg_envelope_new_string("2.0");
		msg_envelope_elem_body_t* ptapp_seq = msgbus_msg_envelope_new_string("1234");
		msg_envelope_elem_body_t* ptsourcetopic = msgbus_msg_envelope_new_string("/flowmeter/PL0/Flow/read");

		msgbus_msg_envelope_put(g_msg, "wellhead", ptwellhead);
		msgbus_msg_envelope_put(g_msg, "command", ptcommand);
		msgbus_msg_envelope_put(g_msg, "value", ptvalue);
		msgbus_msg_envelope_put(g_msg, "timestamp", pttimestamp);
		msgbus_msg_envelope_put(g_msg, "usec", ptusec);
		msgbus_msg_envelope_put(g_msg, "version", ptversion);
		msgbus_msg_envelope_put(g_msg, "app_seq", ptapp_seq);
		msgbus_msg_envelope_put(g_msg, "sourcetopic", ptsourcetopic);


		std::string topic = "Modbus-TCP-Master_ReadRequest";

		//sRespTopic = PublishJsonHandler::instance(msgbusMgr, msgbusEnvelope).getSReadResponseTopic();
		stZmqContext objTempCtx;
		stZmqPubContext objTempPubCtx;
		zmq_handler::insertCTX(topic, objTempCtx);
		cout << "**************************************************** Publishing for topic :: " << topic << endl;
		zmq_handler::stZmqContext MsgbusCtx = zmq_handler::getCTX(topic);
		cout << "**************************************************** Publishing for topic :: " << topic << endl;
		zmq_handler::insertPubCTX(topic, objTempPubCtx);
		zmq_handler::stZmqPubContext pubCtx = zmq_handler::getPubCTX(topic);

		cout << "**************************************************** Publishing for topic :: " << topic << endl;


		PublishJsonHandler::instance().publishJson(g_msg, MsgbusCtx.m_pContext, pubCtx.m_pContext, topic);

	}

	catch(std::exception &e)
	{
		EXPECT_NE("", (string)e.what());
	}


}


/**********************************createErrorrespose()*****************************************/

TEST_F(ModbusWriteHandler_ut, create_error_response_read)
{

	msg_envelope_t *msg = NULL;

	stMbusApiPram.m_lPriority = 1;
	//stMbusApiPram.m_pu8Data = NULL;
	stMbusApiPram.m_pu8Data[5];
	stMbusApiPram.m_u16ByteCount = 2;
	stMbusApiPram.m_u16Port = 1234;
	stMbusApiPram.m_u16Quantity = 4;
	stMbusApiPram.m_u16StartAddr = 56436524;
	stMbusApiPram.m_u16TxId = 23;
	//	stMbusApiPram.m_u32mseTimeout = 45;
	stMbusApiPram.m_u8DevId = 5;
	stMbusApiPram.m_u8IpAddr[4];

	bool isWrite = true;
	stOnDemandRequest reqData;
	reqData.m_isByteSwap = true;
	reqData.m_isRT = true;
	reqData.m_isWordSwap = false;
	reqData.m_obtReqRcvdTS.tv_nsec = 21132323;
	reqData.m_obtReqRcvdTS.tv_sec = 1;
	reqData.m_strAppSeq = "455";
	reqData.m_strMetric = "Test";
	reqData.m_strTopic = "kzdjfhdszh";
	reqData.m_strVersion = "2.1";
	reqData.m_strWellhead = "test";
	MbusAPI_t stMbusApiPram = {};

	try
	{

		bool output = common_Handler::insertReqData(stMbusApiPram.m_u16TxId, stMbusApiPram);
		onDemandHandler::Instance().createErrorResponse(eFunRetType, u8FunCode, stMbusApiPram.m_u16TxId, true, isWrite);
		EXPECT_EQ(1, output);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("Request not found in map", (string)e.what());
	}

}

TEST_F(ModbusWriteHandler_ut, create_error_response_write)
{

	msg_envelope_t *msg = NULL;
	bool isWrite = false;
	stOnDemandRequest reqData;
	reqData.m_isByteSwap = true;
	reqData.m_isWordSwap = false;
	reqData.m_obtReqRcvdTS.tv_nsec = 21132323;
	reqData.m_obtReqRcvdTS.tv_sec = 1;
	reqData.m_strAppSeq = "455";
	reqData.m_strMetric = "Test";
	reqData.m_strTopic = "kzdjfhdszh";
	reqData.m_strVersion = "2.1";
	reqData.m_strWellhead = "test";

	try
	{
		onDemandHandler::Instance().createErrorResponse(eFunRetType, m_u8FunCode, stMbusApiPram.m_u16TxId, true , isWrite);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("Request not found in map", (string)e.what());
	}

}

TEST_F(ModbusWriteHandler_ut, create_error_response_nonRT)
{

	msg_envelope_t *msg = NULL;
	bool isWrite = true;
	stOnDemandRequest reqData;
	reqData.m_isByteSwap = true;
	reqData.m_isWordSwap = false;
	reqData.m_obtReqRcvdTS.tv_nsec = 21132323;
	reqData.m_obtReqRcvdTS.tv_sec = 1;
	reqData.m_strAppSeq = "455";
	reqData.m_strMetric = "Test";
	reqData.m_strTopic = "kzdjfhdszh";
	reqData.m_strVersion = "2.1";
	reqData.m_strWellhead = "test";

	try
	{
		onDemandHandler::Instance().createErrorResponse(eFunRetType, m_u8FunCode, stMbusApiPram.m_u16TxId, false , isWrite);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("Request not found in map", (string)e.what());
	}

}

TEST_F(ModbusWriteHandler_ut, create_error_response)
{

	msg_envelope_t *msg = NULL;
	bool isWrite ;
	bool isRT;
	stOnDemandRequest reqData;
	reqData.m_isByteSwap = true;
	reqData.m_isWordSwap = false;
	reqData.m_obtReqRcvdTS.tv_nsec = 21132323;
	reqData.m_obtReqRcvdTS.tv_sec = 1;
	reqData.m_strAppSeq = "455";
	reqData.m_strMetric = "Test";
	reqData.m_strTopic = "kzdjfhdszh";
	reqData.m_strVersion = "2.1";
	reqData.m_strWellhead = "test";

	try
	{
		onDemandHandler::Instance().createErrorResponse(eFunRetType, m_u8FunCode, stMbusApiPram.m_u16TxId, isRT , isWrite);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("Request not found in map", (string)e.what());
	}

}

#if 0 //SPRINT13CHANGES
TEST_F(ModbusWriteHandler_ut, OnDemandInfoHandler_test)
{
	stRequest stRequestData;
	eMbusAppErrorCode eMbusAppErrorCode_variable;
	try
	{
		eMbusAppErrorCode_variable = onDemandHandler::Instance().onDemandInfoHandler(stRequestData);

		EXPECT_EQ(APP_ERROR_INVALID_INPUT_JSON, eMbusAppErrorCode_variable);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ", e.what());
	}

}
#endif
/*


TEST_F(ModbusWriteHandler_ut, ReqPriority_test)
{
	const globalConfig::COperation a_Ops;
	a_Ops.m_operationPriority = 4000;
	long variable = onDemandHandler::Instance().getReqPriority(a_Ops);
	cout<<"#############################################################################"<<endl;
	cout<<variable<<endl;
	cout<<"#############################################################################"<<endl;

}
 */

#if 0 //SPRINT13CHANGES
TEST_F(ModbusWriteHandler_ut, callBack_OnDemand_1stScenario)
{
	//void** ptrAppCallback = NULL;
	void *tt = NULL;
	void **ptrAppCallback = &tt;
	//void** ptrAppCallback;

	bool isRTFlag = true;
	bool isWriteFlag = true;
	MbusAPI_t stMbusApiPram = {};

	onDemandHandler::Instance().setCallbackforOnDemand(&ptrAppCallback, isRTFlag, isWriteFlag, stMbusApiPram);

}
#endif

#if 0 //SPRINT13CHANGES
TEST_F(ModbusWriteHandler_ut, callBack_OnDemand_2ndScenario)
{
	//void** ptrAppCallback = NULL;
	void *tt = NULL;
	void **ptrAppCallback = &tt;
	//void** ptrAppCallback;

	bool isRTFlag = false;
	bool isWriteFlag = true;
	MbusAPI_t stMbusApiPram = {};

	onDemandHandler::Instance().setCallbackforOnDemand(&ptrAppCallback, isRTFlag, isWriteFlag, stMbusApiPram);

}
#endif

