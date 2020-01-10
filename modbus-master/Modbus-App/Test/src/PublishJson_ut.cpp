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
// (msg_envelope_t* msg, void* msgbus_ctx, const std::string a_sTopic);

void PublishJson_ut::SetUp()
{
	// Setup code
}

void PublishJson_ut::TearDown()
{
	// TearDown code
}


/*************************************publishJson()********************************************/
//#if 0
/*** Test:PublishJson_ut::publishJson_functioncall() **/
TEST_F(PublishJson_ut, publishJson_functioncall)
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

/*** Test:PublishJson_ut::publishJson_sub() checks behaviour of function for sub topic***/
TEST_F(PublishJson_ut, publishJson_sub)
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

/***Test:PublishJson_ut::publishJson_NULL_msgCTX() Gives exception when msgCTX is NULL ***/

TEST_F(PublishJson_ut, publishJson_NULL_msgCTX)
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

/***TEST:PublishJson_ut::publishJson_all_notNULL() check when not a single argument is NULL***/
TEST_F(PublishJson_ut, publishJson_all_notNULL)
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
//#endif

/*** Test:PublishJson_ut::publishJson_valid_arguments and pub topic***/
/* Valid arguments */
TEST_F(PublishJson_ut, publishJson_valid_arguments)
{
	msg_envelope_t *msg = NULL;

	try
	{
		msg = msgbus_msg_envelope_new(CT_JSON);

		zmq_handler::prepareCommonContext("pub");

		zmq_handler::stZmqContext msgbus_ctx = zmq_handler::getCTX("PL1_flowmeter2");

		EXPECT_EQ(true, PublishJsonHandler::instance().publishJson(msg, msgbus_ctx.m_pContext, "PL1_flowmeter2"));

	}

	catch(std::exception &e)
	{
		EXPECT_EQ("", e.what());
	}
}


/***Test:PublishJson_ut::publishJson_pub() checks for functionality with NULL argument***/
/* NULL arguments */
TEST_F(PublishJson_ut, publishJson_pub)
{
	try
	{
		zmq_handler::prepareCommonContext("pub");

		zmq_handler::stZmqContext msgbus_ctx = zmq_handler::getCTX("PL1_flowmeter2");

		EXPECT_EQ(false, PublishJsonHandler::instance().publishJson(NULL, msgbus_ctx.m_pContext, "PL1_flowmeter2"));

	}

	catch(std::exception &e)
	{
		EXPECT_EQ("", e.what());
	}
}

/******************************initialize_message()******************************/
/***Test:PublishJson_ut::initialize_message() checks the behaviour of initialize_message() **/
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
