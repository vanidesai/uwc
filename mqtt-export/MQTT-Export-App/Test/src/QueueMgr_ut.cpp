/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../include/QueueMgr_ut.hpp"

void QueueMgr_ut::SetUp() {
	// Setup code
}

void QueueMgr_ut::TearDown() {
	// TearDown code
}

TEST_F(QueueMgr_ut, getSubMsgFromQ_write)
{
	mqtt::const_message_ptr msg = mqtt::make_message(
					"{\"topic\": \"MQTT_Export_ReadRequest\"}",
					"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}");


	QMgr::getWrite().getSubMsgFromQ(msg);

}

TEST_F(QueueMgr_ut, getSubMsgFromQ_read)
{
	mqtt::const_message_ptr msg = mqtt::make_message(
					"{\"topic\": \"MQTT_Export_ReadRequest\"}",
					"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}");


	QMgr::getRead().getSubMsgFromQ(msg);

}

TEST_F(QueueMgr_ut, getSubMsgFromQ_RTWrite)
{
	mqtt::const_message_ptr msg = mqtt::make_message(
					"{\"topic\": \"MQTT_Export_ReadRequest\"}",
					"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}");


	QMgr::getRTWrite().getSubMsgFromQ(msg);

}

TEST_F(QueueMgr_ut, getSubMsgFromQ_RTRead)
{
	mqtt::const_message_ptr msg = mqtt::make_message(
					"{\"topic\": \"MQTT_Export_ReadRequest\"}",
					"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}");


	QMgr::getRTRead().getSubMsgFromQ(msg);

}

TEST_F(QueueMgr_ut, isMsgArrived_NoSubMsgInQueue)
{
	mqtt::const_message_ptr msg = mqtt::make_message(
					"{\"topic\": \"MQTT_Export_ReadRequest\"}",
					"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}");


	bool RetVal = QMgr::getRTRead().isMsgArrived(msg);

	EXPECT_EQ(false, RetVal);

}
