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
TEST_F(CMQTTHandler_ut, 0_manatory_param) {
	try
	{
		CMQTTHandler::instance();
		EXPECT_EQ(true, true);
	}catch(exception &ex) {
		EXPECT_EQ(true, false);
	}
}

//test 02
TEST_F(CMQTTHandler_ut, 1_manatory_param) {
	bool retVal = false;
	try
	{
		CMQTTHandler::instance();
		retVal = CMQTTHandler::instance().connect();
		EXPECT_EQ(retVal, true);
	}catch(exception &ex) {
		EXPECT_EQ(false, retVal);
	}
}

//test 02
TEST_F(CMQTTHandler_ut, 2_manatory_param) {
	bool retVal = false;
	try
	{
		mqtt::const_message_ptr recvdMsg;

		CTopicMapper::getInstance();
		CMQTTHandler::instance();

		system("mosquitto_pub -h localhost -t /iou/PL01/KeepAlive/write  -m \"{\"wellhead\": \"PL0\", \"command\": \" DValve\", \"value\": \"0x00\", \"app_seq\":\"123466666\"}\"");
		//publish MQTT message with subscribed topic
		retVal = CMQTTHandler::instance().getSubMsgFromQ(recvdMsg);

		CMQTTHandler::instance().cleanup();
		std::cout << __func__ << " retVal : " << retVal << std::endl;

		EXPECT_EQ(true, retVal);

	}catch(exception &ex) {
		EXPECT_EQ(false, retVal);
	}
}

//test 02
TEST_F(CMQTTHandler_ut, 3_manatory_param) {
	bool retVal = false;
	try
	{
		string topic = "PL01_iou_write";
		string g_msg = "{\"hello\": \"world\"}";

		CTopicMapper::getInstance();
		CEISMsgbusHandler::Instance();

		//publish MQTT message with subscribed topic
		retVal = CMQTTHandler::instance().publish(g_msg.c_str(), topic.c_str());

		CMQTTHandler::instance().cleanup();
		CEISMsgbusHandler::Instance().cleanup();

		retVal = true;

	}catch(exception &ex) {
		retVal = false;

	}

	std::cout << __func__ << " retVal : " << retVal << std::endl;
	EXPECT_EQ(true, retVal);
}


//test 02
TEST_F(CMQTTHandler_ut, 4_manatory_param) {
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

	std::cout << __func__ << " retVal : " << retVal << endl;
	EXPECT_EQ(true, retVal);
}
