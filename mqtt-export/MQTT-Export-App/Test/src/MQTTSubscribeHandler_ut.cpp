/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../include/MQTTSubscribeHandler_ut.hpp"

#include <gtest/internal/gtest-internal.h>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

extern void postMsgsToEIS();

void MQTTSubscribeHandler_ut::SetUp() {
	// Setup code
}

void MQTTSubscribeHandler_ut::TearDown() {
	// TearDown code
}

TEST_F(MQTTSubscribeHandler_ut, instance_ValMqtURL)
{
	try
	{
		CMQTTHandler::instance();
		EXPECT_EQ(true, true);
	}catch(exception &ex) {
		EXPECT_EQ(true, false);
	}
}

TEST_F(MQTTSubscribeHandler_ut, getOperation_IsWrite)
{
	string topic = "write_toipic";
	bool isWrite = false;

	if( true == CMQTTHandler::instance().getOperation(topic, isWrite) )
	{
		EXPECT_EQ(true, isWrite);
	}

}

TEST_F(MQTTSubscribeHandler_ut, getOperation_IsNotRead)
{
	string topic = "read_toipic";
	bool isWrite = true;

	if( true == CMQTTHandler::instance().getOperation(topic, isWrite) )
	{
		EXPECT_EQ(false, isWrite);
	}

}


TEST_F(MQTTSubscribeHandler_ut, getOperation_InvalidTopic)
{
	string topic = "Other_toipic";
	bool isWrite = true;
	bool RetVal = true;

	RetVal = CMQTTHandler::instance().getOperation(topic, isWrite);

	EXPECT_EQ(false, RetVal);

}

TEST_F(MQTTSubscribeHandler_ut, parseMQTTMsg_Inv_Json)
{

	bool isRealTime = false;
	bool isWrite = false;
	bool RetVal = false;

	RetVal = CMQTTHandler::instance().parseMQTTMsg("", isRealTime, isWrite);

	EXPECT_EQ(false, RetVal);

}

TEST_F(MQTTSubscribeHandler_ut, parseMQTTMsg_NoRT)
{

	bool isRealTime = true;
	bool isWrite = false;
	bool RetVal = false;

	string topic = "{\"topicname\": \"TestTopic\"}";

	RetVal = CMQTTHandler::instance().parseMQTTMsg(topic.c_str(), isRealTime, isWrite);

	if( RetVal == true )
	{
		EXPECT_EQ(false, isRealTime);
	}
	else
	{
		EXPECT_EQ(true, RetVal);
	}

}

TEST_F(MQTTSubscribeHandler_ut, parseMQTTMsg_RTisNULL)
{

	bool isRealTime = true;
	bool isWrite = false;
	bool RetVal = true;

	string topic = "{\"realtime\": 7}";

	RetVal = CMQTTHandler::instance().parseMQTTMsg(topic.c_str(), isRealTime, isWrite);
	if(true ==  RetVal)
	{
		EXPECT_EQ(false, isRealTime);
	}
	else
	{
		EXPECT_EQ(true, RetVal);
	}

}

TEST_F(MQTTSubscribeHandler_ut, parseMQTTMsg_RT_OtherThan0_1)
{

	bool isRealTime = true;
	bool isWrite = false;
	bool RetVal = false;

	string topic = "{\"realtime\": \"2\"}";

	RetVal = CMQTTHandler::instance().parseMQTTMsg(topic.c_str(), isRealTime, isWrite);

	if( true == RetVal)
	{
		EXPECT_EQ(false, isRealTime);
	}
	else
	{
		EXPECT_EQ(true, RetVal);
	}

}

TEST_F(MQTTSubscribeHandler_ut, parseMQTTMsg_RT_0)
{

	bool isRealTime = true;
	bool isWrite = false;
	bool RetVal = false;

	string topic = "{\"realtime\": \"0\"}";

	RetVal = CMQTTHandler::instance().parseMQTTMsg(topic.c_str(), isRealTime, isWrite);

	if( true == RetVal)
	{
		EXPECT_EQ(false, isRealTime);
	}
	else
	{
		EXPECT_EQ(true, RetVal);
	}

}

TEST_F(MQTTSubscribeHandler_ut, parseMQTTMsg_RT_1)
{

	bool isRealTime = true;
	bool IsWrite = false;
	bool RetVal = false;

	string topic = "{\"realtime\": \"1\"}";

	RetVal = CMQTTHandler::instance().parseMQTTMsg(topic.c_str(), isRealTime, IsWrite);

	if( true == RetVal)
	{
		EXPECT_EQ(true, isRealTime);
	}
	else
	{
		EXPECT_EQ(true, RetVal);
	}

}


TEST_F(MQTTSubscribeHandler_ut, pushSubMsgInQ_ReqTyp3)
{

	bool RetVal = true;
	mqtt::const_message_ptr msg;

	msg =  mqtt::make_message(
			"{\"topic\": \"UT_writeRequest\"}",
			"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}"
			);

	RetVal = CMQTTHandler::instance().pushSubMsgInQ(msg);
	EXPECT_EQ(true, RetVal);
}

TEST_F(MQTTSubscribeHandler_ut, pushSubMsgInQ_EmptyPayload)
{

	bool RetVal = true;
	mqtt::const_message_ptr msg;

	msg =  mqtt::make_message(
			"{\"topic\": \"UT_writeRequest\"}",
			""
			);

	RetVal = CMQTTHandler::instance().pushSubMsgInQ(msg);
	EXPECT_EQ(false, RetVal);
}

TEST_F(MQTTSubscribeHandler_ut, pushSubMsgInQ_ReqTyp1)
{

	bool RetVal = true;
	mqtt::const_message_ptr msg;

	msg =  mqtt::make_message(
			"{\"topic\": \"UT_readRequest\"}",
			"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}"
			);

	RetVal = CMQTTHandler::instance().pushSubMsgInQ(msg);
	EXPECT_EQ(true, RetVal);
}

TEST_F(MQTTSubscribeHandler_ut, pushSubMsgInQ_ReqTyp2)
{

	bool RetVal = true;
	mqtt::const_message_ptr msg;

	msg =  mqtt::make_message(
			"{\"topic\": \"UT_writeRequest\"}",
			"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"0	\"}"
			);

	RetVal = CMQTTHandler::instance().pushSubMsgInQ(msg);
	EXPECT_EQ(true, RetVal);
}

TEST_F(MQTTSubscribeHandler_ut, pushSubMsgInQ_ReqTyp0)
{

	bool RetVal = true;
	mqtt::const_message_ptr msg;

	msg =  mqtt::make_message(
			"{\"topic\": \"UT_readRequest\"}",
			"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"0	\"}"
			);

	RetVal = CMQTTHandler::instance().pushSubMsgInQ(msg);
	EXPECT_EQ(true, RetVal);
}

TEST_F(MQTTSubscribeHandler_ut, pushSubMsgInQ_OtherThanReadWrite)
{

	bool RetVal = true;
	mqtt::const_message_ptr msg;

	msg =  mqtt::make_message(
			"{\"topic\": \"other\"}",
			"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}"
			);

	RetVal = CMQTTHandler::instance().pushSubMsgInQ(msg);
	EXPECT_EQ(false, RetVal);
}

TEST_F(MQTTSubscribeHandler_ut, pushSubMsgInQ_parseMQTTMsgFails)
{

	bool RetVal = true;
	mqtt::const_message_ptr msg;

	msg =  mqtt::make_message(
			"{\"topic\": \"UT_writeRequest\"}",
			"{Invalid}"
			);

	RetVal = CMQTTHandler::instance().pushSubMsgInQ(msg);
	EXPECT_EQ(false, RetVal);
}

/*TEST_F(MQTTSubscribeHandler_ut, instance_MqttExportURL_Empty)
{

	CCommon::getInstance().setStrMqttExportURL("");

	CMQTTHandler::instance();

}*/

TEST_F(MQTTSubscribeHandler_ut, subscribeToTopics_Empty_SubReadTopic)
{
	unsetenv("mqtt_SubReadTopic");

	CMQTTCallback CMQTTCallback_obj;
	CMQTTCallback_obj.connected("Cause_UT");

	setenv("mqtt_SubReadTopic", "/+/+/+/read", 1);

	EXPECT_EQ(true, true);
	//Code doesnt hang

}

// How to confirm??
// Working on it..
TEST_F(MQTTSubscribeHandler_ut, cleanup_Success)
{
	CMQTTHandler::instance().cleanup();

}
