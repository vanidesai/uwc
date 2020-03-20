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


//test 02
/*TEST_F(CMQTTHandler_ut, 1_manatory_param)
{
	bool retVal = false;
	try
	{
		CMQTTHandler::instance();
		retVal = CMQTTHandler::instance().connect();
		EXPECT_EQ(retVal, true);
	}catch(exception &ex) {
		EXPECT_EQ(false, retVal);
	}
}*/

//test 02
/*TEST_F(CMQTTHandler_ut, 2_manatory_param) {
	bool retVal = false;
	try
	{
		mqtt::const_message_ptr recvdMsg;

		CTopicMapper::getInstance();
		CMQTTHandler::instance();

		system("mosquitto_pub -h localhost -t /iou/PL01/KeepAlive/write  -m \"{\"wellhead\": \"PL0\", \"command\": \" DValve\", \"value\": \"0x00\", \"app_seq\":\"123466666\"}\"");
		//publish MQTT message with subscribed topic
		mqtt::const_message_ptr msg = {};
		CMQTTHandler::instance().pushSubMsgInQ(msg);
		retVal = CMQTTHandler::instance().getSubMsgFromQ(recvdMsg);

		CMQTTHandler::instance().cleanup();
		std::cout << __func__ << " retVal : " << retVal << std::endl;

		EXPECT_EQ(true, retVal);

	}catch(exception &ex) {
		EXPECT_EQ(true, retVal);
	}
}*/

//test 02
/*TEST_F(CMQTTHandler_ut, 3_manatory_param) {
	bool retVal = false;
	try
	{
		string topic = "PL01_iou_write";
		string g_msg = "{\"hello\": \"world\"}";

		CTopicMapper::getInstance();
		CEISMsgbusHandler::Instance();

		//publish MQTT message with subscribed topic
		retVal = CMQTTHandler::instance().publish(g_msg.c_str(), topic.c_str(), 0);

		CMQTTHandler::instance().cleanup();
		CEISMsgbusHandler::Instance().cleanup();

		retVal = true;

	}catch(exception &ex) {
		retVal = false;

	}

	std::cout << __func__ << " retVal : " << retVal << std::endl;
	EXPECT_EQ(true, retVal);
}*/


//test 02
/*TEST_F(CMQTTHandler_ut, 4_manatory_param) {
	bool retVal = false;
	try
	{
		CMQTTHandler::instance();
		CMQTTHandler::instance().cleanup();
		retVal = true;

	}catch(exception &ex) {

		std::cout << __func__ << " Exception :" << ex.what() << endl;
		retVal = false;
	}

	std::cout << __funcu__ << " retVal : " << retVal << endl;
	EXPECT_EQ(true, retVal);
}*/

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

#if 0 //Queue removed
TEST_F(CMQTTHandler_ut, getSubReadMsgFromQ_Fails)
{
	bool RetVal = true;
	mqtt::const_message_ptr recvdMsg;

	RetVal = CMQTTHandler::instance().getSubReadMsgFromQ(recvdMsg);
	EXPECT_EQ(false, RetVal);
}
#endif


#if 0 //Queue removed
TEST_F(CMQTTHandler_ut, getSubReadMsgFromQ_Success)
{
	bool RetVal = false;


	mqtt::const_message_ptr msg = mqtt::make_message(
			"{\"topic\": \"MQTT_Export_ReadRequest\"}",
	            "{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"0\"}");

	if( true == CMQTTHandler::instance().pushSubMsgInQ(msg) )
	{
		mqtt::const_message_ptr recvdMsg;

		RetVal = CMQTTHandler::instance().getSubReadMsgFromQ(recvdMsg);
		EXPECT_EQ(true, RetVal);
	}
	else
	{
		cout<<endl<<"#####################ERROR##########################"<<endl;
		cout<<"Could not push sub message in Q."<<endl<<"Exiting this test case";
		cout<<endl<<"####################################################"<<endl;

		EXPECT_EQ(true, RetVal);
	}

}
#endif

/*TEST_F(CMQTTHandler_ut, getOperation_)
{
	bool RetVal = false;


	mqtt::const_message_ptr msg = mqtt::make_message(
			"{\"topic\": \"MQTT_Export_WriteRequest\"}",
	            "{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"0\"}");

	CMQTTHandler::instance().pushSubMsgInQ(msg);


}*/

#if 0 //Queue removed
TEST_F(CMQTTHandler_ut, getSubWriteMsgFromQ_Fails)
{
	bool RetVal = true;
	mqtt::const_message_ptr recvdMsg;

	RetVal = CMQTTHandler::instance().getSubWriteMsgFromQ(recvdMsg);
	EXPECT_EQ(false, RetVal);
}
#endif

#if 0 //Queue removed
TEST_F(CMQTTHandler_ut, getSubRTWriteMsgFromQ_Fails)
{
	bool RetVal = true;
	mqtt::const_message_ptr recvdMsg;

	RetVal = CMQTTHandler::instance().getSubRTWriteMsgFromQ(recvdMsg);
	EXPECT_EQ(false, RetVal);
}
#endif

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
