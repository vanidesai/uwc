/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/


#include "../include/PublishJson_ut.hpp"
//extern BOOLEAN PublishJsonHandler::publishJson
//						(msg_envelope_t* msg, void* msgbus_ctx, const std::string a_sTopic);

void PublishJson_ut::SetUp()
{
	// Setup code
}

void PublishJson_ut::TearDown()
{
	// TearDown code
}


/*************************************publishJson()********************************************/

TEST_F(PublishJson_ut, publishJson0)
{
	try
	{
		PublishJsonHandler::instance();

	}
	catch(std::exception &e)
	{
		EXPECT_EQ("map::at", (string)e.what());

	}
}

TEST_F(PublishJson_ut, publishJson1)
{
	try
	{
	setenv("WRITE_RESPONSE_TOPIC", "PL_0", 1);
		std::string topic = std::getenv("PL0_flowmeter1_write");
		//std::string topic = std::getenv("PL0_flowmeter1");
		if(getenv("SubTopics") != NULL)
		{
			bool bRes = zmq_handler::prepareCommonContext("sub");
		}


		zmq_handler::stZmqContext msgbus_ctx = zmq_handler::getCTX(topic);
		uint8_t valv = PublishJsonHandler::instance().publishJson(msg, msgbus_ctx.m_pContext, "PL0_flowmeter1");

	}
	catch(std::exception &e)
	{
		std::cout << __func__ << " *************** Exception : " << e.what() << std::endl;
		EXPECT_EQ("basic_string::_M_construct null not valid", (string)e.what());
	}
}


TEST_F(PublishJson_ut, publishJson2)
{
	try
	{
	setenv("WRITE_RESPONSE_TOPIC", "PL_0", 1);
	std::string topic = std::getenv("WRITE_RESPONSE_TOPIC");
	zmq_handler::stZmqContext msgbus_ctx = zmq_handler::getCTX(topic);
	uint8_t valv = PublishJsonHandler::instance().publishJson(NULL, NULL, topic);

	}
	catch(std::exception &e)
	{

		EXPECT_EQ("map::at", (string)e.what());
	}
}

TEST_F(PublishJson_ut, publishJson3)
{
	try
	{
	std::string sTopic("");
	const CRefDataForPolling& objReqData = CRequestInitiator::instance().getTxIDReqData(res.u16TransacID);

	uint8_t va =PublishJsonHandler::instance().publishJson(g_msg, objReqData.getBusContext().m_pContext,
						objReqData.getDataPoint().getID());

	}
	catch(std::exception &e)
	{

		EXPECT_EQ("map::at", (string)e.what());
	}
}



/******************************initialize_message()******************************/


TEST_F(PublishJson_ut, initialize_message)
{
	try
	{
	std::string strngMsg;
	strngMsg = "Publish";
	msg = PublishJsonHandler::instance().initialize_message(strngMsg);
	}
	catch(std::exception &e)
	{
		cout<<e.what()<<endl;
		EXPECT_EQ("", e.what());

	}


}






