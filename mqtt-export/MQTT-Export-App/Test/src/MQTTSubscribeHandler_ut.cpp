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
				"MQTT_Export_ReadRequest/read",
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
					"MQTT_Export_ReadRequest/read",
					"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"0\"}");

	CMQTTHandler::instance().msgRcvd(recvdMsg);

}
