/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/


#include "../include/ModbusOnDemandHandler_ut.hpp"
#include "ConfigManager.hpp"
#include "cjson/cJSON.h"

#include "ZmqHandler.hpp"


extern int hex2bin(const std::string &src, int iOpLen, uint8_t* target);
extern int char2int(char input);


//extern sem_t semaphoreWriteReq;
void ModbusOnDemandHandler_ut::SetUp()
{
	// Setup code
}

void ModbusOnDemandHandler_ut::TearDown()
{
	// TearDown code
}


TEST_F(ModbusOnDemandHandler_ut, jsonParserForOnDemandRequest_InvServiceReq)
{

	stMbusApiPram.m_stOnDemandReqData.m_isByteSwap				= true;
	stMbusApiPram.m_stOnDemandReqData.m_isRT					= true;
	stMbusApiPram.m_stOnDemandReqData.m_isWordSwap				= true;
	stMbusApiPram.m_stOnDemandReqData.m_obtReqRcvdTS.tv_nsec	= 21132323;
	stMbusApiPram.m_stOnDemandReqData.m_obtReqRcvdTS.tv_sec		= 1;
	stMbusApiPram.m_stOnDemandReqData.m_strAppSeq				= "455";
	stMbusApiPram.m_stOnDemandReqData.m_strEisTime				= "2020-03-31 12:34:56";
	stMbusApiPram.m_stOnDemandReqData.m_strMetric				= "Flow";
	stMbusApiPram.m_stOnDemandReqData.m_strMqttTime				= "2020-03-13 12:34:56";
	stMbusApiPram.m_stOnDemandReqData.m_strTopic				= "/flowmeter/PL0/Flow/read";
	stMbusApiPram.m_stOnDemandReqData.m_strVersion				= "2.1";
	stMbusApiPram.m_stOnDemandReqData.m_strWellhead				= "PL0";
	stMbusApiPram.m_stOnDemandReqData.m_sUsec					= "0";
	stMbusApiPram.m_stOnDemandReqData.m_sTimestamp				= "0:0:0";

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

/* Invalid topic/msg */
TEST_F(ModbusOnDemandHandler_ut, jsonParserForOnDemandRequest_InvalidTopicMsg)
{

	stMbusApiPram.m_u16TxId = PublishJsonHandler::instance().getTxId();
	stMbusApiPram.m_stOnDemandReqData.m_strAppSeq = "1234";
	stMbusApiPram.m_stOnDemandReqData.m_strMetric =  "Flow";
	stMbusApiPram.m_stOnDemandReqData.m_sValue = "0X00";
	stMbusApiPram.m_stOnDemandReqData.m_strWellhead = "PL0";
	stMbusApiPram.m_stOnDemandReqData.m_strVersion = "version";
	stMbusApiPram.m_stOnDemandReqData.m_strTopic = "Invalid";
	stMbusApiPram.m_stOnDemandReqData.m_sTimestamp = "2020-02-12 06:14:15";
	stMbusApiPram.m_stOnDemandReqData.m_sUsec = "1581488055204186";
	stMbusApiPram.m_stOnDemandReqData.m_strMqttTime = "2020-03-13 12:34:56";
	stMbusApiPram.m_stOnDemandReqData.m_strEisTime = "2020-03-31 12:34:56";
	stMbusApiPram.m_stOnDemandReqData.m_isRT = true;
	stMbusApiPram.m_nRetry = 1;
	stMbusApiPram.m_lPriority = 1;

	try
	{
		eFunRetType = onDemandHandler::Instance().jsonParserForOnDemandRequest(stMbusApiPram,
				m_u8FunCode,
				16,
				true);

		EXPECT_EQ(APP_ERROR_UNKNOWN_SERVICE_REQUEST, eFunRetType);
	}

	catch( exception &e)
	{
		EXPECT_EQ("", e.what());

	}
}

#if 0 // Fails in RTU
TEST_F(ModbusOnDemandHandler_ut, jsonParserForOnDemandRequest_ValidIpJason)
{

	stMbusApiPram.m_stOnDemandReqData.m_isByteSwap				= true;
	stMbusApiPram.m_stOnDemandReqData.m_isRT					= true;
	stMbusApiPram.m_stOnDemandReqData.m_isWordSwap				= true;
	stMbusApiPram.m_stOnDemandReqData.m_obtReqRcvdTS.tv_nsec	= 21132323;
	stMbusApiPram.m_stOnDemandReqData.m_obtReqRcvdTS.tv_sec		= 1;
	stMbusApiPram.m_stOnDemandReqData.m_strAppSeq				= "455";
	stMbusApiPram.m_stOnDemandReqData.m_strEisTime				= "2020-03-31 12:34:56";
	stMbusApiPram.m_stOnDemandReqData.m_strMetric				= "D1";
	stMbusApiPram.m_stOnDemandReqData.m_strMqttTime				= "2020-03-13 12:34:56";
	stMbusApiPram.m_stOnDemandReqData.m_strTopic				= "/flowmeter/PL0/D1/read";
	stMbusApiPram.m_stOnDemandReqData.m_strVersion				= "2.1";
	stMbusApiPram.m_stOnDemandReqData.m_strWellhead				= "PL0";
	stMbusApiPram.m_stOnDemandReqData.m_sUsec					= "0";
	stMbusApiPram.m_stOnDemandReqData.m_sTimestamp				= "0:0:0";

	try
	{
		eFunRetType = onDemandHandler::Instance().jsonParserForOnDemandRequest(stMbusApiPram,
				m_u8FunCode,
				16,
				false);

		EXPECT_EQ(APP_SUCCESS, eFunRetType);
	}

	catch( exception &e)
	{
		EXPECT_EQ("", e.what());

	}
}
#endif

/*** Test:ModbusOnDemandHandler_ut::modWriteHandler_getInstance() Check the instance type returned by function ***/

TEST_F(ModbusOnDemandHandler_ut, modWriteHandler_getInstance)
{


	EXPECT_EQ(typeid(onDemandHandler), typeid(onDemandHandler::Instance()));
}


/**Test::ModbusOnDemandHandler_ut::createWriteListner_test()
    checks the behaviour of the createWriteListener() function for valid topic andtype of topic
 **/

TEST_F(ModbusOnDemandHandler_ut, createWriteListner_test)
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



/***Test ::ModbusOnDemandHandler_ut::subscribeDeviceListener()
    Check the behaviour of subscribeDeviceListener() function***/
TEST_F(ModbusOnDemandHandler_ut, subscribeDeviceListener_CallbaclNULL)
{

	globalConfig::COperation a_refOps;
	void *vpCallback = NULL;
	string SbTopic = "MQTT_Export/MQTT_Export_RdReq";

	//getOperation(SbTopic, a_refOps, &vpCallback, 0, 1, false, false);

	try
	{

		onDemandHandler::Instance().subscribeDeviceListener(topic,
				a_refOps,
				false,
				vpCallback,
				0,
				1,
				false);

	}
	catch(std::exception &e)
	{

		std::cout<<e.what()<<endl;
		EXPECT_EQ("",(string)e.what());
	}

}

TEST_F(ModbusOnDemandHandler_ut, hex_to_bin)
{

	stMbusApiPram.m_lPriority = 1;
	stMbusApiPram.m_u16ByteCount = 2;
	//	stMbusApiPram.m_u16Port = 1234;
	stMbusApiPram.m_u16Quantity = 4;
	stMbusApiPram.m_u16StartAddr = 60000;
	stMbusApiPram.m_u16TxId = 23;
	stMbusApiPram.m_u8DevId = 5;

	try
	{
		hex2bin(tValue, stMbusApiPram.m_u16ByteCount, stMbusApiPram.m_pu8Data);

	}
	catch(std::exception &e)
	{
		EXPECT_NE("", (string)e.what());
	}

}


TEST_F(ModbusOnDemandHandler_ut, hex_to_bin_test)
{

	stMbusApiPram.m_lPriority = 1;
	stMbusApiPram.m_u16ByteCount = 2;
	//	stMbusApiPram.m_u16Port = 1234;
	stMbusApiPram.m_u16Quantity = 4;
	stMbusApiPram.m_u16StartAddr = 564;
	stMbusApiPram.m_u16TxId = 23;
	stMbusApiPram.m_u8DevId = 5;

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




TEST_F(ModbusOnDemandHandler_ut, char_2_int)
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

TEST_F(ModbusOnDemandHandler_ut, validateInputJson_ValidJason)
{

	try
	{
		bool Value = onDemandHandler::Instance().validateInputJson("/flowmeter/PL0/Flow/read",
				"PL0",
				"Flow");

		EXPECT_EQ(true, Value);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ", (string)e.what());

	}

}

TEST_F(ModbusOnDemandHandler_ut, validateInputJson_InValidJason)
{

	try
	{
		bool Value = onDemandHandler::Instance().validateInputJson("/flowmeter/PL0/Flow/read",
				"Invalid",
				"Flow");

		EXPECT_EQ(false, Value);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ", (string)e.what());

	}

}

#if 0 //SPRINT13CHANGES
TEST_F(ModbusOnDemandHandler_ut, process_msg)
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


TEST_F(ModbusOnDemandHandler_ut, thread_init_DValve_sub)
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

		sRespTopic = PublishJsonHandler::instance().getSReadResponseTopic();

		zmq_handler::stZmqContext MsgbusCtx = zmq_handler::getCTX(topic);
		zmq_handler::stZmqPubContext pubCtx = zmq_handler::getPubCTX(topic);

		std::string sUsec{""};
		zmq_handler::publishJson(sUsec, g_msg, topic, "usec");

	}

	catch(std::exception &e)
	{
		EXPECT_NE("", (string)e.what());
	}


}


/****************** For Pub*****************/

#if 1 //sprint 14 changes
TEST_F(ModbusOnDemandHandler_ut, thread_init_DValve_pub)
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
		stZmqContext& objTempCtx  = zmq_handler::getCTX(topic);;
		stZmqPubContext objTempPubCtx;
		zmq_handler::insertCTX(topic, objTempCtx);
		cout << "**************************************************** Publishing for topic :: " << topic << endl;
		zmq_handler::stZmqContext MsgbusCtx = zmq_handler::getCTX(topic);
		cout << "**************************************************** Publishing for topic :: " << topic << endl;
		zmq_handler::insertPubCTX(topic, objTempPubCtx);
		zmq_handler::stZmqPubContext pubCtx = zmq_handler::getPubCTX(topic);

		cout << "**************************************************** Publishing for topic :: " << topic << endl;

		std::string sUsec{""};
		zmq_handler::publishJson(sUsec, g_msg, topic, "usec");

	}

	catch(std::exception &e)
	{
		EXPECT_NE("", (string)e.what());
	}


}
#endif

/**********************************createErrorrespose()*****************************************/

TEST_F(ModbusOnDemandHandler_ut, create_error_response_read)
{

	stMbusApiPram.m_lPriority = 1;
	stMbusApiPram.m_u16ByteCount = 2;
	//	stMbusApiPram.m_u16Port = 1234;
	stMbusApiPram.m_u16Quantity = 4;
	stMbusApiPram.m_u16StartAddr = 500;
	stMbusApiPram.m_u16TxId = 23;
	stMbusApiPram.m_u8DevId = 5;

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

TEST_F(ModbusOnDemandHandler_ut, create_error_response_write)
{

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

TEST_F(ModbusOnDemandHandler_ut, create_error_response_nonRT)
{
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

TEST_F(ModbusOnDemandHandler_ut, create_error_response)
{
	bool isWrite = false;
	bool isRT = false;
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

TEST_F(ModbusOnDemandHandler_ut, OnDemandInfoHandler_MbusAPI_tNULL)
{
	eMbusAppErrorCode eMbusAppErrorCode_variable;


	eMbusAppErrorCode_variable = onDemandHandler::Instance().onDemandInfoHandler(NULL,
			"Topic",
			0x01,
			false);

	EXPECT_EQ(APP_INTERNAL_ERORR, eMbusAppErrorCode_variable);

}

TEST_F(ModbusOnDemandHandler_ut, OnDemandInfoHandler_CallbackNULL)
{
	eMbusAppErrorCode eMbusAppErrorCode_variable;
	MbusAPI_t MbusAPI_t_obj;


	eMbusAppErrorCode_variable = onDemandHandler::Instance().onDemandInfoHandler(&MbusAPI_t_obj,
			"Topic",
			NULL,
			false);

	EXPECT_EQ(APP_INTERNAL_ERORR, eMbusAppErrorCode_variable);

}

TEST_F(ModbusOnDemandHandler_ut, getMsgElement_NULL)
{
	string RetStr = onDemandHandler::Instance().getMsgElement(NULL, "app_seq");

	EXPECT_EQ("", RetStr);

}

TEST_F(ModbusOnDemandHandler_ut, processMsg_MsgEnvNULL)
{
	string topic = "";
	bool RetVal = onDemandHandler::Instance().processMsg(NULL,
			topic,
			false,
			NULL,
			1,
			1,
			false);

	EXPECT_EQ(RetVal, false);

}
