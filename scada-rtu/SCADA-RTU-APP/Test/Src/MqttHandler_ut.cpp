/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../Inc/MqttHandler_ut.hpp"

void MqttHandler_ut::SetUp()
{
	// Setup code
}

void MqttHandler_ut::TearDown()
{
	// TearDown code
}

#if 0 // Function removed
// parseMsg: Valid json
TEST_F(MqttHandler_ut, parseMsg_ValJson)
{
	stNewMsg.m_mqttTopic = msg->get_topic();
	EXPECT_EQ( true, CMQTTHandler::instance().parseMsg(msg->get_payload().c_str(), stNewMsg) );
}

// parseMsg: Invalid json
TEST_F(MqttHandler_ut, parseMsg_InvJson)
{
	EXPECT_EQ( false, CMQTTHandler::instance().parseMsg(NULL, stNewMsg) );
}


// pushMsgInQ: Push success
TEST_F(MqttHandler_ut, pushMsgInQ_Succ)
{
	EXPECT_EQ( true, CMQTTHandler::instance().pushMsgInQ(msg_PushInQ) );
}


// pushMsgInQ: Successfully disconnects from mqtt
TEST_F(MqttHandler_ut, cleanup_Succ)
{
	CMQTTHandler::instance().cleanup();
}
#endif // Function removed















