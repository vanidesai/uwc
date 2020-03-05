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


extern int hex2bin(const std::string &src, int iOpLen, uint8_t* target);
extern int char2int(char input);


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
	try
	{
		eFunRetType = modWriteHandler::Instance().jsonParserForWrite(topic, msg, stMbusApiPram, m_u8FunCode);
		//EXPECT_EQ(MBUS_STACK_NO_ERROR, eFunRetType);
		EXPECT_EQ(MBUS_JSON_APP_ERROR_EXCEPTION_RISE, eFunRetType);
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
	std::string Topic = "Topic_Inv";
	std::string Msg = "Msg_Inv";

	try
	{
		eFunRetType = modWriteHandler::Instance().jsonParserForWrite(Topic, Msg, stMbusApiPram, m_u8FunCode);
		//EXPECT_EQ(MBUS_JSON_APP_ERROR_INVALID_INPUT_PARAMETER, eFunRetType);
		EXPECT_EQ(MBUS_JSON_APP_ERROR_EXCEPTION_RISE, eFunRetType);
	}

	catch( exception &e)
	{
		EXPECT_EQ("", e.what());

	}
}


//#if 0
/*** Test: ModbusWriteHandler_ut::jsonParserForWrite_ValidTopicMsg***/
/** jsonParserForWrite() is called in this test with all correct input parameters.**/
/* Valid topic and msg */

/*TEST_F(ModbusWriteHandler_ut, jsonParserForWrite_ValidTopicMsg)
{
	try
	{
		modWriteHandler::Instance().jsonParserForWrite(topic, msg, stMbusApiPram, m_u8FunCode);
	}
	catch( exception &e)
	{
		cout<<endl<<"#####################ValidTopicMsg###############################"<<endl;
		cout<<e.what();
		cout<<endl<<"#################################################################"<<endl;
	}
}*/

/****** Test case to be updated by Aamir *******/
/***Test:ModbusWriteHandler_ut::jsonParserForWrite_InvalidTopicMsg***/

/* Invalid topic/msg */
/*TEST_F(ModbusWriteHandler_ut, jsonParserForWrite_InvalidTopicMsg)
{
	std::string Topic = "Topic_Inv";
	std::string Msg = "Msg_Inv";

	try
	{
		modWriteHandler::Instance().jsonParserForWrite(Topic, Msg, stMbusApiPram, m_u8FunCode);

	}

	catch( exception &e)
	{

		EXPECT_EQ("", e.what());

	}
}*/


/*** Test:ModbusWriteHandler_ut::modWriteHandler_getInstance() Check the instance type returned by function ***/

TEST_F(ModbusWriteHandler_ut, modWriteHandler_getInstance)
{
	EXPECT_EQ(typeid(modWriteHandler), typeid(modWriteHandler::Instance()));
}


//test 03:: Check the instance type returned by function

/*TEST_F(ModbusWriteHandler_ut, jsonParserForWrite)
{

	//RestMbusReqGeneric_t *pstModbusRxPacket = 0xFF00;
	//RestMbusReqGeneric_t *pstModbusRxPacket = NULL;
	RestMbusReqGeneric_t Rest_Modbus_obj;
	RestMbusReqGeneric_t *pstModbusRxPacket = &Rest_Modbus_obj;
	pstModbusRxPacket = new RestMbusReqGeneric_t();
	pstModbusRxPacket->m_stReqData.m_pu8Data = NULL;
	pstModbusRxPacket->m_u16ReffId = 1;
	if(pstModbusRxPacket == NULL) {
		std::cout << __func__ << " pstModbusRxPacket == NULL" << std::endl;
		EXPECT_EQ(true, false);
	}
	else
		try
	{
			EXPECT_EQ(typeid(MbusStackErrorCode), typeid(modWriteHandler::Instance().jsonParserForWrite(writeReq.m_strTopic, writeReq.m_strMsg, stMbusApiPram, m_u8FunCode)));

	}
	catch(std::exception &e)
	{

		std::cout<<e.what()<<endl;
		EXPECT_EQ("", e.what());
	}

	if(pstModbusRxPacket != NULL)
		delete pstModbusRxPacket;


}*/

/**Test::ModbusWriteHandler_ut::createWriteListner_test()
    checks the behaviour of the createWriteListener() function for valid topic andtype of topic**/

TEST_F(ModbusWriteHandler_ut, createWriteListner_test)
{
	/*setenv("WRITE_RESPONSE_TOPIC", "PL_0", 1);

	std::string topic = std::getenv("WRITE_RESPONSE_TOPIC");*/

	std::string sTopic = "MQTT-Export/PL0_flowmeter1_write";
	std::string topictype = "sub";
	try
	{

		modWriteHandler::Instance().createWriteListener();

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
	try
	{

		modWriteHandler::Instance().subscribeDeviceListener(topic);

	}
	catch(std::exception &e)
	{

		std::cout<<e.what()<<endl;
		EXPECT_EQ("",(string)e.what());
	}

}


TEST_F(ModbusWriteHandler_ut, hex_to_bin)
{
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


/*
TEST_F(ModbusWriteHandler_ut, init_write_handler_thread)
{


	try
	{
		modWriteHandler::Instance().initWriteHandlerThreads();
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("",(string)e.what());
	}

}*/



/*TEST_F(ModbusWriteHandler_ut, Json_parse)
{
	try
	{
		eFunRetType =modWriteHandler::Instance().jsonParserForWrite(writeReq.m_strTopic, writeReq.m_strMsg, stMbusApiPram, m_u8FunCode);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("", e.what());

	}
}*/


//#endif

#include "ConfigManager.hpp"
/*************** for Point2************************
TEST_F(ModbusWriteHandler_ut, thread_init_point2)
{
	//modWriteHandler::Instance().initWriteHandlerThreads();
	// sem_post(&modWriteHandler::Instance().semaphoreWriteReq);



	//******************** As we are not getting the msg from zmq
	//so we are calling the publishJson() function to get the msg************


	//modWriteHandler::Instance().subscribeDeviceListener("PL1_flowmeter2");

	msg_envelope_t *msg = NULL;
	std::string strngMsg;
	strngMsg = "Publish";
	msg = PublishJsonHandler::instance().initialize_message(strngMsg);

	try
	{

sem_t semaphoreWriteReq;
int ok = sem_init(&semaphoreWriteReq, 0, 0);
if (ok == -1) {
std::cout << "*******Could not create unnamed write semaphore\n";
//return false;
}
//return true;

sem_post(&semaphoreWriteReq);

		//eFunRetType =
		modWriteHandler::Instance().initWriteHandlerThreads();
		modWriteHandler::Instance().createWriteListener();

		//msg = msgbus_msg_envelope_new(CT_JSON);
		***************************************
		// Creating message to be published
		msg_envelope_elem_body_t* msgCommand = msgbus_msg_envelope_new_string("Point2");
		msg_envelope_elem_body_t* msgAppSeq = msgbus_msg_envelope_new_string("1234");
		msg_envelope_elem_body_t* msgValue = msgbus_msg_envelope_new_string("0x1234");
		msg = msgbus_msg_envelope_new(CT_JSON);
		msgbus_msg_envelope_put(msg, "command", msgCommand);
		msgbus_msg_envelope_put(msg, "app_seq", msgAppSeq);
		msgbus_msg_envelope_put(msg, "value", msgValue);


		std::string topic = "PL0_flowmeter1_write";
		std::string topictype = "pub";


		//zmq_handler::prepareCommonContext("pub");
		***********************
		config_t* config = CfgManager::Instance().getEnvConfig().get_messagebus_config(topic, topictype);

		void* msgbus_ctx = msgbus_initialize(config);
		if(msgbus_ctx == NULL)
		{
			BOOST_LOG_SEV(lg, error) << __func__ << " Failed to get message bus context with config for topic ::" << topic;
			config_destroy(config);
			//continue;
			return;
		}

		msgbus_ret_t retVal = MSG_SUCCESS;
		publisher_ctx_t* g_pub_ctx;
		retVal = msgbus_publisher_new(msgbus_ctx, topic.c_str(), &g_pub_ctx);
		**********************

		//zmq_handler::stZmqContext msgbus_ctx = zmq_handler::getCTX("PL0_flowmeter1_write");

		PublishJsonHandler::instance().publishJson(msg, msgbus_ctx, "PL0_flowmeter1_write");

		msgCommand = msgbus_msg_envelope_new_string("Arrival");
		msgbus_msg_envelope_put(msg, "command", msgCommand);
		PublishJsonHandler::instance().publishJson(msg, msgbus_ctx, "PL0_flowmeter1_write");


	}

	catch(std::exception &e)
	{

		EXPECT_EQ("basic_string::_M_construct null not valid", (string)e.what());
	}


}*/

/*
******************* For Arrival******************
TEST_F(ModbusWriteHandler_ut, thread_init_Arrival)
{
	//modWriteHandler::Instance().initWriteHandlerThreads();
	// sem_post(&modWriteHandler::Instance().semaphoreWriteReq);



	//******************** As we are not getting the msg from zmq
	//so we are calling the publishJson() function to get the msg************


	//modWriteHandler::Instance().subscribeDeviceListener("PL1_flowmeter2");

	msg_envelope_t *msg = NULL;
	std::string strngMsg;
	strngMsg = "Publish";
	msg = PublishJsonHandler::instance().initialize_message(strngMsg);

	try
	{

sem_t semaphoreWriteReq;
int ok = sem_init(&semaphoreWriteReq, 0, 0);
if (ok == -1) {
std::cout << "*******Could not create unnamed write semaphore\n";
//return false;
}
//return true;

sem_post(&semaphoreWriteReq);

		//eFunRetType =
		modWriteHandler::Instance().initWriteHandlerThreads();
		modWriteHandler::Instance().createWriteListener();

		//msg = msgbus_msg_envelope_new(CT_JSON);
		***************************************
		// Creating message to be published
		msg_envelope_elem_body_t* msgCommand = msgbus_msg_envelope_new_string("Arrival");
		msg_envelope_elem_body_t* msgAppSeq = msgbus_msg_envelope_new_string("1234");
		msg_envelope_elem_body_t* msgValue = msgbus_msg_envelope_new_string("0x1234");
		msg = msgbus_msg_envelope_new(CT_JSON);
		msgbus_msg_envelope_put(msg, "command", msgCommand);
		msgbus_msg_envelope_put(msg, "app_seq", msgAppSeq);
		msgbus_msg_envelope_put(msg, "value", msgValue);


		std::string topic = "PL0_flowmeter1_write";
		std::string topictype = "pub";


		//zmq_handler::prepareCommonContext("pub");
		***********************
		config_t* config = CfgManager::Instance().getEnvConfig().get_messagebus_config(topic, topictype);

		void* msgbus_ctx = msgbus_initialize(config);
		if(msgbus_ctx == NULL)
		{
			BOOST_LOG_SEV(lg, error) << __func__ << " Failed to get message bus context with config for topic ::" << topic;
			config_destroy(config);
			//continue;
			return;
		}

		msgbus_ret_t retVal = MSG_SUCCESS;
		publisher_ctx_t* g_pub_ctx;
		retVal = msgbus_publisher_new(msgbus_ctx, topic.c_str(), &g_pub_ctx);
		**********************

		//zmq_handler::stZmqContext msgbus_ctx = zmq_handler::getCTX("PL0_flowmeter1_write");

		PublishJsonHandler::instance().publishJson(msg, msgbus_ctx, "PL0_flowmeter1_write");

		msgCommand = msgbus_msg_envelope_new_string("Arrival");
		msgbus_msg_envelope_put(msg, "command", msgCommand);
		PublishJsonHandler::instance().publishJson(msg, msgbus_ctx, "PL0_flowmeter1_write");


	}

	catch(std::exception &e)
	{

		EXPECT_EQ("basic_string::_M_construct null not valid", (string)e.what());
	}


}



/******************* For DValve******************/
TEST_F(ModbusWriteHandler_ut, thread_init_DValve)
{
	//modWriteHandler::Instance().initWriteHandlerThreads();
	// sem_post(&modWriteHandler::Instance().semaphoreWriteReq);



	//******************** As we are not getting the msg from zmq
	//so we are calling the publishJson() function to get the msg************


	//modWriteHandler::Instance().subscribeDeviceListener("PL1_flowmeter2");

	msg_envelope_t *msg = NULL;
	std::string strngMsg;
	strngMsg = "Publish";
	msg = PublishJsonHandler::instance().initialize_message(strngMsg);

	try
	{
		/*
sem_t semaphoreWriteReq;
int ok = sem_init(&semaphoreWriteReq, 0, 0);
if (ok == -1) {
std::cout << "*******Could not create unnamed write semaphore\n";
//return false;
}
//return true;

sem_post(&semaphoreWriteReq);*/

		//eFunRetType =
		modWriteHandler::Instance().initWriteHandlerThreads();
		modWriteHandler::Instance().createWriteListener();

		//msg = msgbus_msg_envelope_new(CT_JSON);
		/*****************************************/
		// Creating message to be published
		msg_envelope_elem_body_t* msgCommand = msgbus_msg_envelope_new_string("DValve");
		msg_envelope_elem_body_t* msgAppSeq = msgbus_msg_envelope_new_string("1234");
		msg_envelope_elem_body_t* msgValue = msgbus_msg_envelope_new_string("0x1234");
		msg = msgbus_msg_envelope_new(CT_JSON);
		msgbus_msg_envelope_put(msg, "command", msgCommand);
		msgbus_msg_envelope_put(msg, "app_seq", msgAppSeq);
		msgbus_msg_envelope_put(msg, "value", msgValue);


		std::string topic = "PL0_flowmeter1_write";
		std::string topictype = "pub";


		//zmq_handler::prepareCommonContext("pub");
		/*************************/
		config_t* config = CfgManager::Instance().getEnvConfig().get_messagebus_config(topic, topictype);

		void* msgbus_ctx = msgbus_initialize(config);
		if(msgbus_ctx == NULL)
		{

			config_destroy(config);
			//continue;
			return;
		}

		msgbus_ret_t retVal = MSG_SUCCESS;
		publisher_ctx_t* g_pub_ctx;
		retVal = msgbus_publisher_new(msgbus_ctx, topic.c_str(), &g_pub_ctx);
		/************************/

		//zmq_handler::stZmqContext msgbus_ctx = zmq_handler::getCTX("PL0_flowmeter1_write");

		PublishJsonHandler::instance().publishJson(msg, msgbus_ctx, "PL0_flowmeter1_write");

		msgCommand = msgbus_msg_envelope_new_string("Arrival");
		msgbus_msg_envelope_put(msg, "command", msgCommand);
		PublishJsonHandler::instance().publishJson(msg, msgbus_ctx, "PL0_flowmeter1_write");


	}

	catch(std::exception &e)
	{

		EXPECT_EQ("basic_string::_M_construct null not valid", (string)e.what());
	}


}





