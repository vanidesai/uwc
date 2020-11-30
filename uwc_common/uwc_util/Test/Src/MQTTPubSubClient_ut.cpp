/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../include/MQTTPubSubClient_ut.hpp"


void MQTTPubSubClient_ut::SetUp()
{
	// Setup code
}

void MQTTPubSubClient_ut::TearDown()
{
	// TearDown code
}

/**Test for CMQTTPubSubClient::publishMsg()**/
TEST_F(MQTTPubSubClient_ut, Pubmsg)
{
	std::string Topic = "TCP_WrReq";
	std::string Msg{""};
	int QOS = 1;
	mqtt::message_ptr pubmsg = mqtt::make_message(Topic, Msg, QOS, false);

	bool RetVal = CMQTTPubSubClient_obj.publishMsg(pubmsg);
	EXPECT_EQ(true, RetVal);
}

/**Test for CMQTTPubSubClient::connect() *
TEST_F(MQTTPubSubClient_ut, Connect)
{
	bool RetVal = CMQTTPubSubClient_obj.connect();
	EXPECT_EQ(true, RetVal);
}*/

/**Test for CMQTTPubSubClient::disconnect() **/
TEST_F(MQTTPubSubClient_ut, Discconect)
{
	bool RetVal = CMQTTPubSubClient_obj.disconnect();
	EXPECT_EQ(true, RetVal);
}

/**Test for CMQTTPubSubClient::disconnect() **/
TEST_F(MQTTPubSubClient_ut, Subscribe)
{
	std::string Topic = "TCP_WrResp";
	CMQTTPubSubClient_obj2.subscribe(Topic);
	//EXPECT_EQ(true, RetVal);
}

/**Test for MQTTBaseHandler::connected() **/
TEST_F(MQTTPubSubClient_ut, Connected)
{
	std::string cause;
	CMQTTBaseHandler_obj.connected(cause);
}

/**Test for MQTTBaseHandler::disconnected() **/
TEST_F(MQTTPubSubClient_ut, DisConnected)
{
	std::string cause;
	CMQTTBaseHandler_obj.disconnected(cause);
}

/**Test for MQTTBaseHandler::msgRcvd() **/
TEST_F(MQTTPubSubClient_ut, ReceiveMsg)
{
	std::string Topic = "TCP_WrReq";
	std::string Msg{""};
	int QOS = 1;
	mqtt::message_ptr pubmsg = mqtt::make_message(Topic, Msg, QOS, false);
	CMQTTBaseHandler_obj.msgRcvd(pubmsg);
}

/**Test for MQTTBaseHandler::isConnected() **/
TEST_F(MQTTPubSubClient_ut, IsConnected)
{
	bool RetVal = CMQTTBaseHandler_obj.isConnected();
	EXPECT_EQ(false, RetVal);
}

/**Test for MQTTBaseHandler::disconnect() **/
TEST_F(MQTTPubSubClient_ut, Disconnect)
{
	CMQTTBaseHandler_obj.disconnect();
	//EXPECT_EQ(false, RetVal);
}

/**Test for MQTTBaseHandler::publishMsg() **/
TEST_F(MQTTPubSubClient_ut, PublishMsg)
{
	std::string strMsg = "{ 	\"value\": \"0xFF00\", 	\"command\": \"Pointname\", 	\"app_seq\": \"1234\" }";
	std::string Topic = "TCP_WrReq";
	bool RetVal = CMQTTBaseHandler_obj.publishMsg(strMsg, Topic);
	EXPECT_EQ(true, RetVal);
}

