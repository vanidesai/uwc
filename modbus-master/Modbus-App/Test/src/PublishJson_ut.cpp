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

#if 0 //Shifted to common library
TEST_F(PublishJson_ut, publishJson_NULL_msgCTX)
{
	try
	{

		setenv("WRITE_RESPONSE_TOPIC", "PL_0", 1);
		std::string topic = std::getenv("WRITE_RESPONSE_TOPIC");
		zmq_handler::getCTX(topic);

		std::string sUsec{""};
		uint8_t valv = PublishJsonHandler::instance().publishJson(sUsec, NULL, topic);

		EXPECT_EQ(false, valv);

	}
	catch(std::exception &e)
	{

		EXPECT_EQ("map::at", (string)e.what());
	}
}
#endif

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

#if 0 //Shifted to common library
/*** Test:PublishJson_ut::publishJson_valid_arguments and pub topic***/
/* Null msg */

TEST_F(PublishJson_ut, publishJson_null_msg)
{

	std::string topic = "Modbus-TCP-Master_ReadResponse";
	std::string topicType = "pub";
	msg_envelope_t *msg = NULL;



	try
	{
		msg = msgbus_msg_envelope_new(CT_JSON);

		std::string sUsec{""};
		EXPECT_EQ(true, PublishJsonHandler::instance().publishJson(sUsec, msg, topic));

	}

	catch(std::exception &e)
	{
		EXPECT_EQ("map::at", (string)e.what());
	}
}
#endif

#if 0 //Shifted to common library
/***Test:PublishJson_ut::publishJson_pub() checks for functionality with NULL argument***/
/* NULL arguments */
TEST_F(PublishJson_ut, publishJson_pub)
{
	std::string sUsec{""};

	try
	{
		EXPECT_EQ(false, PublishJsonHandler::instance().publishJson(sUsec, NULL, "PL1_flowmeter2"));
	}

	catch(std::exception &e)
	{
		EXPECT_EQ("", e.what());
	}
}
#endif

TEST_F(PublishJson_ut, element_exist_inJSON)
{
	cJSON *root = NULL;
	std::string a_sKeyName = "datapoints" ;
	bool result = false;
	try
	{
		result = isElementExistInJson(root, a_sKeyName);
		EXPECT_EQ(false, result);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("", e.what());
	}
}
//#endif


TEST_F(PublishJson_ut, Set_Topic_null)
{
	bool result;
	std::string topic;
	result = PublishJsonHandler::instance().setTopicForOperation(topic);
	EXPECT_EQ(false, result);
}


TEST_F(PublishJson_ut, Set_Topic_ReadRes)
{
	bool result;
	std::string topic = "TCP1_RdResp";
	result = PublishJsonHandler::instance().setTopicForOperation(topic);
	EXPECT_EQ(true, result);
}

TEST_F(PublishJson_ut, Set_Topic_ReadRes_RT)
{
	bool result;
	std::string topic = "TCP1_RdResp_RT";
	result = PublishJsonHandler::instance().setTopicForOperation(topic);
	EXPECT_EQ(true, result);
}

TEST_F(PublishJson_ut, Set_Topic_WRITE_RES)
{
	bool result;
	std::string topic = "TCP1_WrResp";
	result = PublishJsonHandler::instance().setTopicForOperation(topic);
	EXPECT_EQ(true, result);
}

TEST_F(PublishJson_ut, Set_Topic_WRITE_RES_RT)
{
	bool result;
	std::string topic = "TCP1_WrResp_RT";
	result = PublishJsonHandler::instance().setTopicForOperation(topic);
	EXPECT_EQ(true, result);
}

TEST_F(PublishJson_ut, Set_Topic_PolledData)
{
	bool result;
	std::string topic = "TCP1_PolledData";
	result = PublishJsonHandler::instance().setTopicForOperation(topic);
	EXPECT_EQ(true, result);
}

TEST_F(PublishJson_ut, Set_Topic_PolledData_RT)
{
	bool result;
	std::string topic = "TCP1_PolledData_RT";
	result = PublishJsonHandler::instance().setTopicForOperation(topic);
	EXPECT_EQ(true, result);
}

#endif
