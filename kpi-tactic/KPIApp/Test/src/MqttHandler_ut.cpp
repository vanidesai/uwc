/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../include/MqttHandler_ut.hpp"


void MqttHandler_ut::SetUp()
{
	// Setup code
}

void MqttHandler_ut::TearDown()
{
	// TearDown code
}

#if 1
TEST_F(MqttHandler_ut, instance_test)
{
	bool object = true;
	CMqttHandler::instance();
	EXPECT_EQ(true, object);
}
/*
TEST_F(MqttHandler_ut, diconnect_test)
{
	CMqttHandler::instance().connect();
	CMqttHandler::instance().disconnect();
	EXPECT_EQ(true, true);
}
*/

TEST_F(MqttHandler_ut, isConnected_false)
{
	bool RetVal = CMqttHandler::instance().isConnected();
	EXPECT_EQ(false, RetVal);
}

/*Private functions

TEST_F(MqttHandler_ut, pushMsg_true)
{
	mqtt::const_message_ptr msg = mqtt::make_message("Death/A", "Msg_UT", 0, false);
	bool RetVal = CMqttHandler::instance().pushMsgInQ(msg);
	EXPECT_EQ(true, RetVal);
}


TEST_F(MqttHandler_ut, pushMsg)
{
	mqtt::const_message_ptr msg = mqtt::make_message("", "", 0, true);
	bool RetVal = CMqttHandler::instance().pushMsgInQ(msg);
	EXPECT_EQ(true, RetVal);
}
*/



TEST_F(MqttHandler_ut, publishMsg_true)
{
	bool RetVal = CMqttHandler::instance().publishMsg(strMsg, topic);
	EXPECT_EQ(true, RetVal);
}


TEST_F(MqttHandler_ut, publishMsg_false)
{
	bool RetVal = CMqttHandler::instance().publishMsg(strMsg, Topic);
	EXPECT_EQ(false, RetVal);
}

#endif
