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
#include "../include/MQTTSubscribeHandler_ut.hpp"


void MQTTSubscribeHandler_ut::SetUp()
{
	// Setup code
}

void MQTTSubscribeHandler_ut::TearDown()
{
	// TearDown code
}

/**
 * Test case to check the behaviour of recvdMsg() when topic ends with "/read"
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(MQTTSubscribeHandler_ut, recvdMsg_readTopic)
{

	mqtt::const_message_ptr recvdMsg = mqtt::make_message(
				"MQTT_Export_RdReq/read",
				"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}");

	CMQTTHandler::instance().msgRcvd(recvdMsg);

}

/**
 * Test case to check the behaviour of recvdMsg() when topic ends with "/write"
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(MQTTSubscribeHandler_ut, recvdMsg_writeTopic)
{

	mqtt::const_message_ptr recvdMsg = mqtt::make_message(
				"MQTT_Export_WrReq/write",
				"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}");

	CMQTTHandler::instance().msgRcvd(recvdMsg);

}

/**
 * Test case to check the behaviour of recvdMsg() when topic does not match any pattern (/read or /write)
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(MQTTSubscribeHandler_ut, recvdMsg_InvalTopic)
{

	mqtt::const_message_ptr recvdMsg = mqtt::make_message(
				"MQTT_Export_WrReq/something_else",
				"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}");

	CMQTTHandler::instance().msgRcvd(recvdMsg);

}

/**
 * Test case to check the behaviour of recvdMsg() when payload is Non RT and write topic
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(MQTTSubscribeHandler_ut, recvdMsg_NonRTPayLoad_write)
{

	mqtt::const_message_ptr recvdMsg = mqtt::make_message(
					"MQTT_Export_WrReq/write",
					"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"0\"}");

	CMQTTHandler::instance().msgRcvd(recvdMsg);

}

/**
 * Test case to check the behaviour of recvdMsg() when payload is Non RT and read topic
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(MQTTSubscribeHandler_ut, recvdMsg_NonRTPayLoad_read)
{

	mqtt::const_message_ptr recvdMsg = mqtt::make_message(
					"MQTT_Export_RdReq/read",
					"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"0\"}");

	CMQTTHandler::instance().msgRcvd(recvdMsg);

}
