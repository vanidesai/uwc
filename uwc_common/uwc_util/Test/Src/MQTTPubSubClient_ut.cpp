/********************************************************************************
* Copyright (c) 2021 Intel Corporation.

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*********************************************************************************/

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

