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

#if 1

extern bool isElementExistInJson(cJSON *root, std::string a_sKeyName);

void PublishJson_ut::SetUp()
{
	// Setup code
}

void PublishJson_ut::TearDown()
{
	// TearDown code
}

//#if  1
/*************************************publishJson()********************************************/
//#if 0
/*** Test:PublishJson_ut::publishJson_functioncall() **/
/*
TEST_F(PublishJson_ut, with_all_parameters)
{

	MsgbusManager msgbusMgr;

	zmq_handler::zmqHandler zmqhandler(msgbusMgr, CallerObj, CallerConfigobj);

	try{
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


		msgbus_ret_t retVal = MSG_SUCCESS;
		publisher_ctx_t* g_pub_ctx;

		zmq_handler::stZmqContext MsgbusCtx = zmqhandler.getCTX(topic);
		zmq_handler::stZmqPubContext pubCtx = zmqhandler.getPubCTX(topic);

		PublishJsonHandler::instance(msgbusMgr).publishJson(msg, MsgbusCtx.m_pContext, pubCtx.m_pContext, topic);
		msgCommand = msgbus_msg_envelope_new_string("Arrival");
		msgbus_msg_envelope_put(msg, "command", msgCommand);



	//	PublishJsonHandler::instance(msgbusMgr).publishJson(msg, MsgbusCtx.m_pContext, pubCtx.m_pContext, "PL0_flowmeter1_write");

	}
	catch(std::exception &e)
	{
		EXPECT_EQ("map::at", (string)e.what());
	}


}
*/

//#if 1
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
#if 0 //In progress
TEST_F(PublishJson_ut, publishJson_sub)
{
	try
	{
		std::string topic = "MQTT-Export/Modbus-TCP-Master_ReadRequest";

		if(getenv("SubTopics") != NULL)
		{
			bool bRes = zmq_handler::prepareCommonContext("sub");
		}
		bool bRes = zmq_handler::prepareCommonContext("sub");

		zmq_handler::stZmqContext MsgbusCtx = zmq_handler::getCTX(topic);
		zmq_handler::stZmqPubContext pubCtx = zmq_handler::getPubCTX(topic);

		uint8_t valv = PublishJsonHandler::instance().publishJson(msg, MsgbusCtx.m_pContext, pubCtx.m_pContext, "MQTT-Export/Modbus-TCP-Master_ReadRequest");

	}
	catch(std::exception &e)
	{

		EXPECT_EQ("map::at", (string)e.what());
	}
}

TEST_F(PublishJson_ut, prepareCommonCTX_pub)
{
	try
	{
		std::string topic = "TCP1_RdResp";

		bool bRes = zmq_handler::prepareCommonContext("pub");

		zmq_handler::stZmqContext MsgbusCtx = zmq_handler::getCTX(topic);
		zmq_handler::stZmqPubContext pubCtx = zmq_handler::getPubCTX(topic);

		uint8_t valv = PublishJsonHandler::instance().publishJson(msg, MsgbusCtx.m_pContext, pubCtx.m_pContext, "MQTT-Export/Modbus-TCP-Master_ReadRequest");

	}
	catch(std::exception &e)
	{

		EXPECT_EQ("map::at", (string)e.what());
	}
}
#endif

/***Test:PublishJson_ut::publishJson_NULL_msgCTX() Gives exception when msgCTX is NULL ***/

TEST_F(PublishJson_ut, publishJson_NULL_msgCTX)
{
	try
	{

		setenv("WRITE_RESPONSE_TOPIC", "PL_0", 1);
		std::string topic = std::getenv("WRITE_RESPONSE_TOPIC");
		zmq_handler::stZmqContext msgbus_ctx = zmq_handler::getCTX(topic);
		uint8_t valv = PublishJsonHandler::instance().publishJson(NULL, NULL, NULL, topic);

	}
	catch(std::exception &e)
	{

		EXPECT_EQ("map::at", (string)e.what());
	}
}

/***TEST:PublishJson_ut::publishJson_all_notNULL() check when not a single argument is NULL***/
/*TEST_F(PublishJson_ut, publishJson_all_notNULL)
{

	MsgbusManager msgbusMgr;
	zmq_handler::zmqHandler zmqhandler(msgbusMgr, CallerObj, CallerConfigobj);
	try
	{
		std::string sTopic("");
		const CRefDataForPolling& objReqData = CRequestInitiator::instance().getTxIDReqData(res.u16TransacID);

		uint8_t va =PublishJsonHandler::instance(msgbusMgr).publishJson(g_msg, objReqData.getBusContext().m_pContext,
				objReqData.getDataPoint().getID());

	}
	catch(std::exception &e)
	{

		EXPECT_EQ("map::at", (string)e.what());
	}
}*/
//#endif


/*** Test:PublishJson_ut::publishJson_valid_arguments and pub topic***/
/* Valid arguments */


#if 0 //In progress
TEST_F(PublishJson_ut, publishJson_valid_arguments)
{

	stZmqContext objTempCtx;
	stZmqPubContext objTempPubCtx;
	msg_envelope_t *msg = NULL;
	//std::string topic = "Modbus-TCP-Master_ReadResponse";
	std::string topic = "Mbus_TCP_RResp";
	std::string topicType = "pub";


	try
	{
		msg = msgbus_msg_envelope_new(CT_JSON);

		msg_envelope_elem_body_t* ptVersion = msgbus_msg_envelope_new_string("2.0");
		msgbus_msg_envelope_put(msg, "version", ptVersion);


		//objTempCtx = zmq_handler::getCTX("Modbus-TCP-Master_WriteResponse");
		//objTempPubCtx = zmq_handler::getPubCTX("Modbus-TCP-Master_WriteResponse");

		objTempCtx = zmq_handler::getCTX("Mbus_TCP_RResp");
		objTempPubCtx = zmq_handler::getPubCTX("Mbus_TCP_RResp");

		EXPECT_EQ(true, PublishJsonHandler::instance().publishJson(msg, objTempCtx.m_pContext, objTempPubCtx.m_pContext, topic));

	}

	catch(std::exception &e)
	{
		EXPECT_EQ("map::at",(string)e.what());
	}
}
#endif

/*** Test:PublishJson_ut::publishJson_valid_arguments and pub topic***/
/* Null msg */


TEST_F(PublishJson_ut, publishJson_null_msg)
{

	stZmqContext objTempCtx;
	stZmqPubContext objTempPubCtx;
	msg_envelope_t *msg = NULL;
	std::string topic = "Modbus-TCP-Master_ReadResponse";
	std::string topicType = "pub";


	try
	{
		msg = msgbus_msg_envelope_new(CT_JSON);

		objTempCtx = zmq_handler::getCTX("Modbus-TCP-Master_WriteResponse");
		objTempPubCtx = zmq_handler::getPubCTX("Modbus-TCP-Master_WriteResponse");

		//EXPECT_EQ(false, PublishJsonHandler::instance(msgbusMgr, msgbusEnvelope).publishJson(msg, objTempCtx.m_pContext, objTempPubCtx.m_pContext, topic));
		EXPECT_EQ(true, PublishJsonHandler::instance().publishJson(msg, objTempCtx.m_pContext, objTempPubCtx.m_pContext, topic));

	}

	catch(std::exception &e)
	{
		EXPECT_EQ("map::at", (string)e.what());
	}
}


/***Test:PublishJson_ut::publishJson_pub() checks for functionality with NULL argument***/
/* NULL arguments */
TEST_F(PublishJson_ut, publishJson_pub)
{
	try
	{
		EXPECT_EQ(false, PublishJsonHandler::instance().publishJson(NULL, NULL, NULL, "PL1_flowmeter2"));
	}

	catch(std::exception &e)
	{
		EXPECT_EQ("", e.what());
	}
}

TEST_F(PublishJson_ut, element_exist_inJSON)
{
	cJSON *root = NULL;
	std::string a_sKeyName = "datapoints" ;
	bool result = false;
	try
	{
		result = isElementExistInJson(root, a_sKeyName);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("", e.what());
	}
}
//#endif

#endif
