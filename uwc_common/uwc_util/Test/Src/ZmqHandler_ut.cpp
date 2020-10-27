/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#include "../include/ZmqHandler_ut.hpp"

extern int char2int(char input);



void ZmqHandler_ut::SetUp()
{
	// Setup code
}

void ZmqHandler_ut::TearDown()
{
	// TearDown code
}


/* This test is to check whether topic type is getting or not */
//************check for the topic is other than pub or sub in PubTopic************
TEST_F(ZmqHandler_ut, prepare_test_other1)
{

	if(getenv("PubTopics") != NULL)
	{
		bool bRes = zmq_handler::prepareCommonContext("Other");

		if(!bRes)
		{

			retValue =bRes;

			EXPECT_EQ(false, retValue);
		}
		else{
			retValue =bRes;

			EXPECT_EQ(true, retValue);
		}
	}

}


TEST_F(ZmqHandler_ut, getCTX2)
{
	try
	{

		//zmq_handler::stZmqContext &busCTX = zmqhandler.getCTX("PL0_flowmeter1");
		zmq_handler::stZmqContext &busCTX = zmq_handler::getCTX("Modbus-TCP-Master_ReadRequest");
		EXPECT_EQ(NULL, busCTX.m_pContext);


	}
	catch(std::exception &e)
	{

		std::cout<<e.what()<<endl;
		EXPECT_EQ("map::at", (string)e.what());
	}
}

TEST_F(ZmqHandler_ut, getCTX3)
{
	try
	{
		//zmq_handler::stZmqContext &busCTX = zmqhandler.getCTX("PL1_flowmeter2");
		zmq_handler::stZmqContext &busCTX = zmq_handler::getCTX("Modbus-TCP-Master_PolledData");
		EXPECT_NE((void*)NULL, busCTX.m_pContext);

	}
	catch(std::exception &e)
	{

		std::cout<<e.what()<<endl;
		EXPECT_EQ("map::at", (string)e.what());

	}
}

TEST_F(ZmqHandler_ut, getCTX4)
{
	try
	{
		//zmq_handler::stZmqContext &busCTX = zmqhandler.getCTX("PL1_flowmeter2_write");
		zmq_handler::stZmqContext &busCTX = zmq_handler::getCTX("MQTT-Export/Modbus-TCP-Master_WriteRequest");
		EXPECT_NE((void*)NULL, busCTX.m_pContext);

	}
	catch(std::exception &e)
	{

		std::cout<<e.what()<<endl;
		EXPECT_EQ("map::at", (string)e.what());

	}
}



/***************************getSubCTX()***********************************/
//this test checks for correct context for correct sub topic accordingly
//and throws exceptions accordingly



TEST_F(ZmqHandler_ut, getSubCtx)
{

	try
	{

		zmq_handler::prepareCommonContext("fgdgrpub");

		busSubCTX = zmq_handler::getSubCTX("Modbus-TCP-Master_ReadRequest");
		//EXPECT_EQ(1,1);
		EXPECT_NE((void*)NULL, busSubCTX.sub_ctx);

	}
	catch(std::exception &e)
	{
		cout<<e.what()<<endl;
		EXPECT_EQ("map::at", (string)e.what());
	}

}


/******************************************getPubCTX()***********************************/

TEST_F(ZmqHandler_ut, getPubCtx)
{
	zmq_handler::stZmqPubContext objTempPubCtx;
	std::string topic = "Modbus-TCP-Master_ReadRequest";

	try
	{

		zmq_handler::insertPubCTX(topic, objTempPubCtx);
		zmq_handler::getPubCTX(topic);
		busPubCTX = zmq_handler::getPubCTX("Modbus-TCP-Master_ReadRequest");

	}
	catch(std::exception &e)
	{

		//busPubCTX = zmqhandler.getPubCTX("PL0_flowmeter1");
		/*stZmqPubContext objTempPubCtx;
		zmqhandler.insertPubCTX(topic, objTempPubCtx);
		zmq_handler::stZmqPubContext pubCtx = zmqhandler.getPubCTX(topic);
		busPubCTX = zmqhandler.getPubCTX("Modbus-TCP-Master_ReadRequest");
		 */
		EXPECT_EQ("map::at", (string)e.what());

	}

}
/***************getPubCTX() and insertPubCTX() functions are working properly
 ***need to check******/
/***********************************insertPubCTX()***********************************/



TEST_F(ZmqHandler_ut, insertpub)
{
	try
	{
	//	bool bRes = zmq_handler::zmqHandler(msgbusMgr, CallerObj, CallerConfigobj, msgbusEnvelope).prepareCommonContext("sub");
		zmq_handler::insertPubCTX("Modbus-TCP-Master_PolledData", objPubContext);
		busPubCTX = zmq_handler::getPubCTX("Modbus-TCP-Master_PolledData");

	}
	catch(std::exception &e)
	{
		std::cout<<e.what()<<endl;
		//	zmqhandler.insertPubCTX("PL1_flowmeter2", objPubContext);
		//zmqhandler.insertPubCTX("Modbus-TCP-Master_PolledData", objPubContext);
		//busPubCTX = zmqhandler.getPubCTX("Modbus-TCP-Master_PolledData");
		EXPECT_EQ("map::at", (string)e.what());


	}
}




/*******************************removeCTX()***********************************************/

/*This test is to check the behaviour of the removeCTX() function************/

TEST_F(ZmqHandler_ut, removeCTX)
{
	try
	{
		// zmqhandler.removeCTX("PL0_flowmeter1");
		zmq_handler::removeCTX("MQTT-Export/Modbus-TCP-Master_WriteRequest");

	}
	catch(std::exception &e)
	{

		cout<<e.what()<<endl;
		EXPECT_EQ("map::at", (string)e.what());
	}
}

/*******************************removeSubCTX()***********************************/

TEST_F(ZmqHandler_ut, removeSubCTX)
{
	try
	{
		//zmqhandler.removeSubCTX("PL0_flowmeter1_write");
		zmq_handler::removeSubCTX("Modbus-TCP-Master_ReadRequest");

	}
	catch(std::exception &e)
	{
		cout<<e.what();
		EXPECT_EQ("map::at", (string)e.what());
	}
}

/****************************removePubCTX()************************************/

TEST_F(ZmqHandler_ut, removePubCTX)
{
	try
	{
		//zmqhandler.removePubCTX("PL0_flowmeter1_write");
		zmq_handler::removePubCTX("Modbus-TCP-Master_PolledData");
		EXPECT_EQ(1,1);
	}
	catch(std::exception &e)
	{
		std::cout<<e.what();
		EXPECT_EQ("map::at", (string)e.what());
	}
}

TEST_F(ZmqHandler_ut, isNumber_number)
{
	bool result = network_info::isNumber("123");
	EXPECT_EQ(1, result);
}

TEST_F(ZmqHandler_ut, isNumber_notNumber)
{
	bool result = network_info::isNumber("Test");
	EXPECT_EQ(0, result);
}

TEST_F(ZmqHandler_ut, prepareContext_NULLArg_msgbus_ctx)
{
	config_t config;

	bool Res = zmq_handler::prepareContext(false,
											NULL,
											"TestStr",
											&config);

	EXPECT_EQ(false, Res);
}

TEST_F(ZmqHandler_ut, prepareContext_TopicEmpty)
{
	void* msgbus_ctx;
	string topicType = "sub";

	char** ppcTopics = CfgManager::Instance().getEnvClient()->get_topics_from_env(topicType.c_str());

	config_t* config = CfgManager::Instance().getEnvClient()->get_messagebus_config(
								CfgManager::Instance().getConfigClient(),
								ppcTopics , 1, topicType.c_str());


	msgbus_ctx = msgbus_initialize(config);

	bool Res = zmq_handler::prepareContext(false,
											msgbus_ctx,
											"",
											config);

	EXPECT_EQ(false, Res);
}

TEST_F(ZmqHandler_ut, prepareContext_NULLArg_config)
{
	void* msgbus_ctx;
	string topicType = "sub";

	char** ppcTopics = CfgManager::Instance().getEnvClient()->get_topics_from_env(topicType.c_str());

	config_t* config = CfgManager::Instance().getEnvClient()->get_messagebus_config(
								CfgManager::Instance().getConfigClient(),
								ppcTopics , 1, topicType.c_str());


	msgbus_ctx = msgbus_initialize(config);

	bool Res = zmq_handler::prepareContext(false,
											msgbus_ctx,
											"TestStr",
											NULL);

	EXPECT_EQ(false, Res);
}

TEST_F(ZmqHandler_ut, prepareContext_SubFails)
{
	void* msgbus_ctx;
	string topicType = "sub";

	char** ppcTopics = CfgManager::Instance().getEnvClient()->get_topics_from_env(topicType.c_str());

	config_t* config = CfgManager::Instance().getEnvClient()->get_messagebus_config(
								CfgManager::Instance().getConfigClient(),
								ppcTopics , 1, topicType.c_str());


	msgbus_ctx = msgbus_initialize(config);

	bool Res = zmq_handler::prepareContext(false,
											msgbus_ctx,
											"TestStr",
											config);

	EXPECT_EQ(false, Res);
}

TEST_F(ZmqHandler_ut, prepareContext_PubFails)
{
	void* msgbus_ctx;
	string topicType = "pub";

	char** ppcTopics = CfgManager::Instance().getEnvClient()->get_topics_from_env(topicType.c_str());

	config_t* config = CfgManager::Instance().getEnvClient()->get_messagebus_config(
								CfgManager::Instance().getConfigClient(),
								ppcTopics , 1, topicType.c_str());


	msgbus_ctx = msgbus_initialize(config);

	bool Res = zmq_handler::prepareContext(true,
											msgbus_ctx,
											"TestStr",
											config);

	EXPECT_EQ(false, Res);
}

