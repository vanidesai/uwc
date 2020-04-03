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



/*** Test: ModbusWriteHandler_ut::jsonParserForWrite_ValidTopicMsg***/
/** jsonParserForWrite() is called in this test with all correct input parameters.**/
/* Valid topic and msg */

TEST_F(ModbusWriteHandler_ut, jsonParserForWrite_ValidTopicMsg)
{
	writeReq.m_strTopic = topic;
	//	writeReq.m_strMsg = msg;
	writeReq.m_strMsg = msg;


	stMbusApiPram.m_lPriority = 1;
	stMbusApiPram.m_pu8Data = NULL;
	stMbusApiPram.m_u16ByteCount = 2;
	stMbusApiPram.m_u16Port = 1234;
	stMbusApiPram.m_u16Quantity = 4;
	stMbusApiPram.m_u16StartAddr = 56436524;
	stMbusApiPram.m_u16TxId = 23;
	stMbusApiPram.m_u32mseTimeout = 45;
	stMbusApiPram.m_u8DevId = 5;
	stMbusApiPram.m_u8IpAddr[4];

	reqData.m_isByteSwap = true;
	reqData.m_isWordSwap = false;
	reqData.m_obtReqRcvdTS.tv_nsec = 21132323;
	reqData.m_obtReqRcvdTS.tv_sec = 1;
	reqData.m_strAppSeq = "455";
	reqData.m_strMetric = "Test";
	reqData.m_strTopic = "kzdjfhdszh";
	reqData.m_strVersion = "2.1";
	reqData.m_strWellhead = "test";

	root = cJSON_Parse(writeReq.m_strMsg.c_str());

	cJSON *appseq = cJSON_GetObjectItem(root,"app_seq");
	cJSON *cmd=cJSON_GetObjectItem(root,"command");
	cJSON *value=cJSON_GetObjectItem(root,"value");
	cJSON *wellhead=cJSON_GetObjectItem(root,"wellhead");
	cJSON *version=cJSON_GetObjectItem(root,"version");
	cJSON *sourcetopic=cJSON_GetObjectItem(root,"sourcetopic");
	cJSON *timestamp=cJSON_GetObjectItem(root,"timestamp");
	cJSON *usec=cJSON_GetObjectItem(root,"usec");
	void* ptrAppCallback = NULL;
	bool isWrite = false;

	try
	{

		eFunRetType = onDemandHandler::Instance().jsonParserForOnDemandRequest(root, stMbusApiPram, m_u8FunCode, stMbusApiPram.m_u16TxId, isWrite, &ptrAppCallback);

		//EXPECT_EQ(MBUS_STACK_NO_ERROR, eFunRetType);
		//EXPECT_EQ(MBUS_JSON_APP_ERROR_INVALID_INPUT_PARAMETER, eFunRetType);
		EXPECT_EQ(MBUS_APP_ERROR_UNKNOWN_SERVICE_REQUEST, eFunRetType);


	}
	catch( exception &e)
	{
		EXPECT_EQ("", e.what());
	}

}


TEST_F(ModbusWriteHandler_ut, jsonParserForWrite_Test)
{
	writeReq.m_strTopic = topic;
	//	writeReq.m_strMsg = msg;
	writeReq.m_strMsg = msg;
	void* ptrAppCallback = NULL;

	//cJSON *root = cJSON_Parse(writeReq.m_strMsg.c_str());

	bool isWrite = true;
	stMbusApiPram.m_lPriority = 1;
	stMbusApiPram.m_pu8Data = NULL;
	stMbusApiPram.m_u16ByteCount = 2;
	stMbusApiPram.m_u16Port = 1234;
	stMbusApiPram.m_u16Quantity = 4;
	stMbusApiPram.m_u16StartAddr = 56436524;
	stMbusApiPram.m_u16TxId = 23;
	stMbusApiPram.m_u32mseTimeout = 45;
	stMbusApiPram.m_u8DevId = 5;
	stMbusApiPram.m_u8IpAddr[4];

	reqData.m_isByteSwap = true;
	reqData.m_isWordSwap = false;
	reqData.m_obtReqRcvdTS.tv_nsec = 21132323;
	reqData.m_obtReqRcvdTS.tv_sec = 1;
	reqData.m_strAppSeq = "455";
	reqData.m_strMetric = "Test";
	reqData.m_strTopic = "kzdjfhdszh";
	reqData.m_strVersion = "2.1";
	reqData.m_strWellhead = "test";

	root = cJSON_Parse(writeReq.m_strMsg.c_str());

	cJSON *appseq = cJSON_GetObjectItem(root,"app_seq");
	cJSON *cmd=cJSON_GetObjectItem(root,"command");
	cJSON *value=cJSON_GetObjectItem(root,"value");
	cJSON *wellhead=cJSON_GetObjectItem(root,"wellhead");
	cJSON *version=cJSON_GetObjectItem(root,"version");
	cJSON *sourcetopic=cJSON_GetObjectItem(root,"sourcetopic");
	cJSON *timestamp=cJSON_GetObjectItem(root,"timestamp");
	cJSON *usec=cJSON_GetObjectItem(root,"usec");


	try
	{

		eFunRetType = onDemandHandler::Instance().jsonParserForOnDemandRequest(root, stMbusApiPram, m_u8FunCode, stMbusApiPram.m_u16TxId, isWrite, &ptrAppCallback);

		//EXPECT_EQ(MBUS_STACK_NO_ERROR, eFunRetType);
		//EXPECT_EQ(MBUS_JSON_APP_ERROR_INVALID_INPUT_PARAMETER, eFunRetType);
		EXPECT_EQ(MBUS_APP_ERROR_UNKNOWN_SERVICE_REQUEST, eFunRetType);

	}
	catch( exception &e)
	{
		EXPECT_EQ("", e.what());
	}

}


/****** Test case to be updated by Aamir *******/
/***Test:ModbusWriteHandler_ut::jsonParserForWrite_InvalidTopicMsg***/

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
	void* ptrAppCallback = NULL;
	bool isWrite = true;

	try
	{

		//eFunRetType = modWriteHandler::Instance(msgbusMgr, CallerObj, msgbusEnvelope).jsonParserForOnDemandRequest(writeReq, stMbusApiPram, m_u8FunCode, stMbusApiPram.m_u16TxId);
		eFunRetType = onDemandHandler::Instance().jsonParserForOnDemandRequest(root, stMbusApiPram, m_u8FunCode, stMbusApiPram.m_u16TxId, isWrite, &ptrAppCallback);
		EXPECT_EQ(MBUS_JSON_APP_ERROR_INVALID_INPUT_PARAMETER, eFunRetType);
		//EXPECT_EQ(MBUS_APP_ERROR_UNKNOWN_SERVICE_REQUEST, eFunRetType);
	}

	catch( exception &e)
	{
		EXPECT_EQ("", e.what());

	}
}


/*** Test:ModbusWriteHandler_ut::modWriteHandler_getInstance() Check the instance type returned by function ***/

TEST_F(ModbusWriteHandler_ut, modWriteHandler_getInstance)
{


	EXPECT_EQ(typeid(onDemandHandler), typeid(onDemandHandler::Instance()));
}


//test 03:: Check the instance type returned by function

TEST_F(ModbusWriteHandler_ut, jsonParserForWrite)
{

	void* ptrAppCallback = NULL;
	//RestMbusReqGeneric_t *pstModbusRxPacket = 0xFF00;
	//RestMbusReqGeneric_t *pstModbusRxPacket = NULL;
	RestMbusReqGeneric_t Rest_Modbus_obj;
	RestMbusReqGeneric_t *pstModbusRxPacket = &Rest_Modbus_obj;
	pstModbusRxPacket = new RestMbusReqGeneric_t();
	pstModbusRxPacket->m_stReqData.m_pu8Data = NULL;
	pstModbusRxPacket->m_u16ReffId = 1;

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

	if(pstModbusRxPacket == NULL) {
		std::cout << __func__ << " pstModbusRxPacket == NULL" << std::endl;
		EXPECT_EQ(true, false);
	}
	else
		try
	{
			//EXPECT_EQ(typeid(MbusStackErrorCode), typeid(modWriteHandler::Instance(msgbusMgr, CallerObj, msgbusEnvelope).jsonParserForOnDemandRequest( writeReq, stMbusApiPram, m_u8FunCode, stMbusApiPram.m_u16TxId)));
			EXPECT_EQ(typeid(MbusStackErrorCode), typeid(onDemandHandler::Instance().jsonParserForOnDemandRequest(root, stMbusApiPram, m_u8FunCode, stMbusApiPram.m_u16TxId, isWrite, &ptrAppCallback)));

	}
	catch(std::exception &e)
	{

		std::cout<<e.what()<<endl;
		EXPECT_EQ("", e.what());
	}

	if(pstModbusRxPacket != NULL)
		delete pstModbusRxPacket;


}

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


TEST_F(ModbusWriteHandler_ut, hex_to_bin)
{

	stMbusApiPram.m_lPriority = 1;
	//stMbusApiPram.m_pu8Data = NULL;
	stMbusApiPram.m_u16ByteCount = 2;
	stMbusApiPram.m_u16Port = 1234;
	stMbusApiPram.m_u16Quantity = 4;
	stMbusApiPram.m_u16StartAddr = 56436524;
	stMbusApiPram.m_u16TxId = 23;
	stMbusApiPram.m_u32mseTimeout = 45;
	stMbusApiPram.m_u8DevId = 5;
	stMbusApiPram.m_u8IpAddr[4];
	stMbusApiPram.m_pu8Data = new uint8_t[stMbusApiPram.m_u16ByteCount]();

	try
	{
		hex2bin(tValue, stMbusApiPram.m_u16ByteCount, stMbusApiPram.m_pu8Data);

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

// NEED TO CHECK THE TEST


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

	//eFunRetType = modWriteHandler::Instance(msgbusMgr, CallerObj, msgbusEnvelope).jsonParserForOnDemandRequest(writeReq, stMbusApiPram, m_u8FunCode);
	try
	{
		bool Value = onDemandHandler::Instance().validateInputJson(strSourceTopic, strWellhead, strCommand);
	}
	catch(std::exception &e)
	{
		std::cout<<"##################################################################"<<endl;
		cout<<e.what()<<endl;
		std::cout<<"##################################################################"<<endl;

	}

}

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
		//Check the below commented line once again
		//onDemandHandler::Instance().initWriteHandlerThreads();
		//TODO
		//modWriteHandler::Instance(msgbusMgr, CallerObj, msgbusEnvelope).createWriteListener();

    //  bool result = modWriteHandler::Instance().processMsg(g_msg)
		//sem_t semaphoreWriteReq = onDemandHandler::Instance().getSemaphoreWriteReq();
		//sem_post(&onDemandHandler::Instance().getSemaphoreWriteReq());

		// Creating message to be published

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
	stMbusApiPram.m_pu8Data = NULL;
	stMbusApiPram.m_u16ByteCount = 2;
	stMbusApiPram.m_u16Port = 1234;
	stMbusApiPram.m_u16Quantity = 4;
	stMbusApiPram.m_u16StartAddr = 56436524;
	stMbusApiPram.m_u16TxId = 23;
	stMbusApiPram.m_u32mseTimeout = 45;
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

		common_Handler::insertReqData(stMbusApiPram.m_u16TxId, stMbusApiPram);
		onDemandHandler::Instance().createErrorResponse(eFunRetType, u8FunCode, stMbusApiPram.m_u16TxId, true, isWrite);
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
		onDemandHandler::Instance().createErrorResponse(eFunRetType, m_u8FunCode, stMbusApiPram.m_u16TxId, reqData.m_isRT, isWrite);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("Request not found in map", (string)e.what());
	}

}

/*

TEST_F(ModbusWriteHandler_ut, callBack_OnDemand_test)
{
	//void** ptrAppCallback = NULL;
	void** ptrAppCallback;
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
	 bool isRTFlag = true;
	 bool isWriteFlag = false;
	 MbusAPI_t stMbusApiPram;
	 onDemandHandler::Instance().setCallbackforOnDemand(&ptrAppCallback, isRTFlag, isWriteFlag, stMbusApiPram);
}
*/


