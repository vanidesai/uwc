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
//#if 0



/******************************zmqHandler::getTimeParams()****************************************/

TEST_F(ZmqHandler_ut, getParams)
{
//	zmq_handler::zmqHandler zmqhandler();
	std::string strTimestamp, strUsec;
	try
	{
		common_Handler::getTimeParams(strTimestamp, strUsec);

	}
	catch(std::exception &e)
	{
		cout<<"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
		cout<<e.what()<<endl;
		cout<<"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"<<endl;
		EXPECT_EQ("", (string)e.what());
	}

}



//#if 1

/* This test is to check whether topic type is getting or not */
//************check for the Sub Topic************

#if 0//Commented because test is not running thorough the script

TEST_F(ZmqHandler_ut, prepare_test_sub)
{

	if(getenv("SubTopics") != NULL)
	{
		bool bRes = zmq_handler::prepareCommonContext("sub");
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
#endif


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

TEST_F(ZmqHandler_ut, gerReqData_test)
{
	MbusAPI_t reqData;
	bool result = common_Handler::getReqData(25, reqData);

	EXPECT_EQ(0, result);
}

TEST_F(ZmqHandler_ut, updateReqData_test)
{
	MbusAPI_t reqData;
	bool result = common_Handler::updateReqData(25, reqData);
	EXPECT_EQ(1, result);
}

TEST_F(ZmqHandler_ut, removeReqData_test)
{
	common_Handler::removeReqData(25);
	EXPECT_EQ(1, 1);
}

/****************************populatePollingRefData()
 *********This function test is to test the getCTX function******/

TEST_F(ZmqHandler_ut, getCTX)
{
	try
	{
		populatePollingRefData();

	}
	catch(std::exception &e)
	{
		std::cout << e.what() << std::endl;
		EXPECT_EQ("map::at", (string)e.what());
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
		EXPECT_NE(NULL, busCTX.m_pContext);

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
		EXPECT_NE(NULL, busCTX.m_pContext);

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
		EXPECT_NE(NULL, busSubCTX.sub_ctx);

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
	stZmqPubContext objTempPubCtx;
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


/**************************removeOnDemandReqData()************************************/

TEST_F(ZmqHandler_ut, removeAppCTX)
{
	try
	{
		common_Handler::removeReqData(1);
		EXPECT_EQ(1, 1);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("map::at", (string)e.what());
		cout<<e.what()<<endl;
	}
}

TEST_F(ZmqHandler_ut, buildNettest)
{
#ifdef MODBUS_STACK_TCPIP_ENABLED
	try
	{
		cout<<"****** Setting TCP mode ******"<<endl;
		network_info::buildNetworkInfo(true);
	}
	catch(std::exception &e)
	{
		cout<<"$$$$$"<<e.what()<<endl;
		EXPECT_EQ("",e.what());
	}
#else
	try
	{
		cout<<"****** Setting RTUs mode ******"<<endl;
		network_info::buildNetworkInfo(false);
	}
	catch(std::exception &e)
	{

		//cout<<"$$$$$"e.what()<<endl;
		EXPECT_EQ("",e.what());
	}
#endif

}



/***Test:ZmqHandler_ut::swap_test() checks the behaviour of the swapConversion() function for Byteswap is true ***/

TEST_F(ZmqHandler_ut, swap_Byte)
{
	std::vector<uint8_t> tempVt;
	int i = 2;	/// to ignore "0x" from datastring
	int iLen = stValue.length();
	while(i < iLen)
	{
		byte1 = char2int(stValue[i])*16 + char2int(stValue[i+1]);
		tempVt.push_back(byte1);
		i = i+2;
	}
	try
	{
		stValue  = common_Handler::swapConversion(tempVt,true, false);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("", (string)e.what());
	}
}



TEST_F(ZmqHandler_ut, swap_Byte_4Bytes)
{
	std::vector<uint8_t> tempVt;
	int i = 4;	/// to ignore "0x" from datastring
	int iLen = stValue.length();
	while(i < iLen)
	{
		byte1 = char2int(stValue[i])*16 + char2int(stValue[i+1]);
		tempVt.push_back(byte1);
		i = i+2;
	}
	try
	{
		stValue  = common_Handler::swapConversion(tempVt,true, false);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("", (string)e.what());
	}
}


/****ZmqHandler_ut::swap_Word() check the behaviour of the swapConversion() function for wordSwap is true  ***/

TEST_F(ZmqHandler_ut, swap_Word)
{
	std::vector<uint8_t> tempVt;
	int i = 2;	/// to ignore "0x" from datastring
	int iLen = TValue.length();
	while(i < iLen)
	{
		byte1 = char2int(TValue[i])*16 + char2int(TValue[i+1]);
		tempVt.push_back(byte1);
		i = i+2;
	}
	try
	{
		TValue  = common_Handler::swapConversion(tempVt, false, true);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("", (string)e.what());
	}
}


/*** Test:ZmqHandler_ut::swap_Word_byte() checks the behaviour of the swapConversion() function
     when both Wordswap and Byteswap are true */

TEST_F(ZmqHandler_ut, swap_Word_byte_true)
{

	std::vector<uint8_t> tempVt;
	int i = 2;	/// to ignore "0x" from datastring
	int iLen = TValue.length();
	while(i < iLen)
	{
		byte1 = char2int(TValue[i])*16 + char2int(TValue[i+1]);
		tempVt.push_back(byte1);
		i = i+2;
	}
	try
	{
		TValue  = common_Handler::swapConversion(tempVt, true, true);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("", (string)e.what());
	}
}

/*** Test:ZmqHandler_ut::swap_Word_byte() checks the behaviour of the swapConversion() function
     when both Wordswap and Byteswap are false */

TEST_F(ZmqHandler_ut, swap_Word_byte_false)
{

	std::vector<uint8_t> tempVt;
	int i = 2;	/// to ignore "0x" from datastring
	int iLen = TValue.length();
	while(i < iLen)
	{
		byte1 = char2int(TValue[i])*16 + char2int(TValue[i+1]);
		tempVt.push_back(byte1);
		i = i+2;
	}
	try
	{
		TValue  = common_Handler::swapConversion(tempVt, false, false);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("", (string)e.what());
	}
}
//#endif
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

