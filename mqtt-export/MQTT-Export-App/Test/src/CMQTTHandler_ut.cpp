/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../include/CMQTTHandler_ut.hpp"

#include <gtest/internal/gtest-internal.h>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

extern void postMsgsToEIS();

void CMQTTHandler_ut::SetUp() {
	// Setup code
}

void CMQTTHandler_ut::TearDown() {
	// TearDown code
}


//test 01
TEST_F(CMQTTHandler_ut, instance_ValMqtURL) {
	try
	{
		CMQTTHandler::instance();
		EXPECT_EQ(true, true);
	}catch(exception &ex) {
		EXPECT_EQ(true, false);
	}
}


#ifdef QUEUE_FAILED_PUBLISH_MESSAGES
TEST_F(CMQTTHandler_ut, pushMsgInQ)
{

	CMQTTHandler::instance().postPendingMsgs();
}
#endif


TEST_F(CMQTTHandler_ut, publish_ValMsg)
{
	string PubTpoic = "PubTopics";

	struct timespec tsMsgRcvd;
	timespec_get(&tsMsgRcvd, TIME_UTC);

	CMQTTPublishHandler CMQTTPublishHandler_obj("str1", "str2");

	bool retVal = CMQTTPublishHandler_obj.publish(
			"{\"topic\": \"PL0_iou_write\", \"command\": \" DValve\", \"value\": \"0x00\", \"app_seq\":\"1234\"}",
			PubTpoic.c_str(),
			0,
			tsMsgRcvd);

	EXPECT_EQ(false, retVal);
}


TEST_F(CMQTTHandler_ut, publish_InValMsg)
{
	string PubTpoic = "PubTopics";

	struct timespec tsMsgRcvd;
	timespec_get(&tsMsgRcvd, TIME_UTC);

	CMQTTPublishHandler CMQTTPublishHandler_obj("str1", "str2");

	bool retVal = CMQTTPublishHandler_obj.publish(
			"InvalidMsg",
			PubTpoic.c_str(),
			0,
			tsMsgRcvd);

	EXPECT_EQ(false, retVal);
}


TEST_F(CMQTTHandler_ut, publish_MsgEmpty)
{
	string PubTpoic = "";

	struct timespec tsMsgRcvd;
	timespec_get(&tsMsgRcvd, TIME_UTC);

	CMQTTPublishHandler CMQTTPublishHandler_obj("str1", "str2");

	bool retVal = CMQTTPublishHandler_obj.publish(
			"",
			PubTpoic.c_str(),
			0,
			tsMsgRcvd);

	EXPECT_EQ(false, retVal);
}

TEST_F(CMQTTHandler_ut, publish_TopicEmp)
{
	string PubTpoic = "";

	struct timespec tsMsgRcvd;
	timespec_get(&tsMsgRcvd, TIME_UTC);

	CMQTTPublishHandler CMQTTPublishHandler_obj("str1", "str2");

	bool retVal = CMQTTPublishHandler_obj.publish(
			"{\"topic\": \"PL0_iou_write\"}",
			PubTpoic.c_str(),
			0,
			tsMsgRcvd);

	EXPECT_EQ(false, retVal);
}

// How to confirm??
// Working on it..
TEST_F(CMQTTHandler_ut, cleanup_Success)
{
	CMQTTHandler::instance().cleanup();

}

TEST_F(CMQTTHandler_ut, cleanup_1_Success)
{
	CMQTTPublishHandler CMQTTPublishHandler_obj("str1", "str2");

	CMQTTPublishHandler_obj.cleanup();
}

TEST_F(CMQTTHandler_ut, getOperation_IsWrite)
{
	string topic = "write_toipic";
	bool isWrite = false;

	if( true == CMQTTHandler::instance().queueMgr.getOperation(topic, isWrite) )
	{
		EXPECT_EQ(true, isWrite);
	}

}

TEST_F(CMQTTHandler_ut, getOperation_IsNotRead)
{
	string topic = "read_toipic";
	bool isWrite = true;

	if( true == CMQTTHandler::instance().queueMgr.getOperation(topic, isWrite) )
	{
		EXPECT_EQ(false, isWrite);
	}

}

TEST_F(CMQTTHandler_ut, getOperation_InvalidTopic)
{
	string topic = "Other_toipic";
	bool isWrite = true;
	bool RetVal = true;

	RetVal = CMQTTHandler::instance().queueMgr.getOperation(topic, isWrite);

	EXPECT_EQ(false, RetVal);

}

TEST_F(CMQTTHandler_ut, parseMQTTMsg_Inv_Json)
{

	bool isRealTime = false;
	bool RetVal = false;

	RetVal = CMQTTHandler::instance().queueMgr.parseMQTTMsg("", isRealTime);

	EXPECT_EQ(false, RetVal);

}

TEST_F(CMQTTHandler_ut, parseMQTTMsg_NoRT)
{

	bool isRealTime = true;
	bool RetVal = false;

	string topic = "{\"topicname\": \"TestTopic\"}";

	RetVal = CMQTTHandler::instance().queueMgr.parseMQTTMsg(topic.c_str(), isRealTime);

	if( RetVal == true )
	{
		EXPECT_EQ(false, isRealTime);
	}
	else
	{
		EXPECT_EQ(true, RetVal);
	}

}

TEST_F(CMQTTHandler_ut, parseMQTTMsg_RT_OtherThan0_1)
{

	bool isRealTime = true;
	bool RetVal = false;

	string topic = "{\"realtime\": \"2\"}";

	RetVal = CMQTTHandler::instance().queueMgr.parseMQTTMsg(topic.c_str(), isRealTime);

	if( true == RetVal)
	{
		EXPECT_EQ(false, isRealTime);
	}
	else
	{
		EXPECT_EQ(true, RetVal);
	}

}

TEST_F(CMQTTHandler_ut, parseMQTTMsg_RT_0)
{

	bool isRealTime = true;
	bool RetVal = false;

	string topic = "{\"realtime\": \"0\"}";

	RetVal = CMQTTHandler::instance().queueMgr.parseMQTTMsg(topic.c_str(), isRealTime);

	if( true == RetVal)
	{
		EXPECT_EQ(false, isRealTime);
	}
	else
	{
		EXPECT_EQ(true, RetVal);
	}

}

TEST_F(CMQTTHandler_ut, parseMQTTMsg_RT_1)
{

	bool isRealTime = true;
	bool RetVal = false;

	string topic = "{\"realtime\": \"1\"}";

	RetVal = CMQTTHandler::instance().queueMgr.parseMQTTMsg(topic.c_str(), isRealTime);

	if( true == RetVal)
	{
		EXPECT_EQ(true, isRealTime);
	}
	else
	{
		EXPECT_EQ(true, RetVal);
	}

}

TEST_F(CMQTTHandler_ut, pushSubMsgInQ_ReqTyp3)
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

TEST_F(CMQTTHandler_ut, pushSubMsgInQ_ReqTyp1)
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

TEST_F(CMQTTHandler_ut, pushSubMsgInQ_ReqTyp2)
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

TEST_F(CMQTTHandler_ut, pushSubMsgInQ_ReqTyp0)
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

TEST_F(CMQTTHandler_ut, pushSubMsgInQ_OtherThanReadWrite)
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

TEST_F(CMQTTHandler_ut, pushSubMsgInQ_parseMQTTMsgFails)
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

//{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}
