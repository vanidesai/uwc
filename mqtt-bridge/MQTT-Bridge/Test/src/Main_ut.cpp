/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/


#include "../include/Main_ut.hpp"


using namespace std;

void Main_ut::SetUp() {
	// Setup code
}

void Main_ut::TearDown() {
	// TearDown code
}

std::atomic<bool> g_shouldStop_ut(false);
sem_t g_semaphoreRespProcess_ut;

/********************************************************/
/*	Helper Functions	*/

/**
 * Helper function to set g_shouldStop as true and call postMsgstoMQTT()
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
void TargetCaller_postMsgstoMQTT()
{
	g_shouldStop = true;
	postMsgstoMQTT();
	for (auto &th : g_vThreads)
	{
		th.detach();
	}
}

/**
 * Test case to check if processMsgToSendOnEIS()function Process message received from MQTT and send it on EIS for valid topic
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Main_ut, processMsgToSendOnEIS_ValidTopic)
{
	mqtt::const_message_ptr recvdMsg = mqtt::make_message(
			"{\"topic\": \"MQTT_Export_RdReq\"}",
			"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}");

	CMessageObject Temp(recvdMsg);
	std::string Topic = "MQTT_Export_RdReq";
	processMsgToSendOnEIS(Temp, Topic);

}

/**
 * Test case to check if processMsg()function do not Process message received from EIS and send for publishing on MQTT for Null msg and returns false
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Main_ut, processMsg_NULLMsg)
{
	string topic = "MQTT_Export_RdReq";
	CMQTTPublishHandler mqttPublisher(EnvironmentInfo::getInstance().getDataFromEnvMap("MQTT_URL_FOR_EXPORT").c_str(), topic.c_str(), 0);
	bool RetVal = processMsg(NULL, mqttPublisher);
	EXPECT_EQ(false, RetVal);
}

/**
 * Test case to check if processMsg()function do not Process message received from EIS and send for publishing on MQTT if topic is present in zmq and returns false
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Main_ut, processMsg_TopicNotPresentInZMQmsg)
{
	msg_envelope_t *msg = NULL;

	msg_envelope_elem_body_t* ptVersion = msgbus_msg_envelope_new_string("2.0");
	msg_envelope_elem_body_t* ptDriverSeq = msgbus_msg_envelope_new_string("TestStr");
	msg_envelope_elem_body_t* ptTopic = msgbus_msg_envelope_new_string("");

	msg = msgbus_msg_envelope_new(CT_JSON);
	msgbus_msg_envelope_put(msg, "version", ptVersion);
	msgbus_msg_envelope_put(msg, "driver_seq", ptDriverSeq);
	msgbus_msg_envelope_put(msg, "topic", ptTopic);

	string topic = "MQTT_Export_RdReq";
	CMQTTPublishHandler mqttPublisher(EnvironmentInfo::getInstance().getDataFromEnvMap("MQTT_URL_FOR_EXPORT").c_str(), topic.c_str(), 0);

	bool realTime = true;
	bool IsRead = true;
	set_thread_priority_for_eis(realTime, IsRead);


	bool RetVal = processMsg(msg, mqttPublisher);

	EXPECT_EQ(false, RetVal);
}

/**
 * Test case to check if set_thread_priority_for_eis()function Set thread priority for threads that send messages from MQTT-Export to EIS for realTime = true and IsRead = true
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Main_ut, set_thread_priority_for_eis_RTRead)
{
	bool realTime = true;
	bool IsRead = true;
	set_thread_priority_for_eis(realTime, IsRead);
}

/**
 * Test case to check if set_thread_priority_for_eis()function Set thread priority for threads that send messages from MQTT-Export to EIS for realTime = true and IsRead = false
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Main_ut, set_thread_priority_for_eis_RTWrite)
{
	bool realTime = true;
	bool IsRead = false;
	set_thread_priority_for_eis(realTime, IsRead);

}

/**
 * Test case to check if set_thread_priority_for_eis()function Set thread priority for threads that send messages from MQTT-Export to EIS for realTime = false and IsRead = true
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Main_ut, set_thread_priority_for_eis_NonRTRead)
{
	bool realTime = false;
	bool IsRead = true;
	set_thread_priority_for_eis(realTime, IsRead);

}

/**
 * Test case to check if set_thread_priority_for_eis()function Set thread priority for threads that send messages from MQTT-Export to EIS for realTime = false and IsRead = false
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Main_ut, set_thread_priority_for_eis_NonRTWrite)
{
	bool realTime = false;
	bool IsRead = false;

	set_thread_priority_for_eis(realTime, IsRead);

}

/**
 * Test case to check code doesnt hang in getRTRead() function
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Main_ut, postMsgsToEIS_RTRead)
{
	g_shouldStop = true;
	postMsgsToEIS(QMgr::getRTRead());
}

/**
 * Test case to check code doesnt hang in getRTWrite() function
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Main_ut, postMsgsToEIS_RTWrite)
{
	g_shouldStop = true;
	postMsgsToEIS(QMgr::getRTWrite());
}

/**
 * Test case to check if getRead()function return reference of on-demand read operation instance successfully
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Main_ut, postMsgsToEIS_Read)
{
	g_shouldStop = true;
	postMsgsToEIS(QMgr::getRead());
}

/**
 * Test case to check if getWrite()function return reference of on-demand write operation instance successfully
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Main_ut, postMsgsToEIS_Write)
{
	g_shouldStop = true;
	postMsgsToEIS(QMgr::getWrite());
}

/**
 * Test case to check if postMsgstoMQTT() works fine
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Main_ut, postMsgstoMQTT)
{
	std::thread Thread_TargetCaller_postMsgstoMQTT( TargetCaller_postMsgstoMQTT );
	std::this_thread::sleep_for(std::chrono::seconds(10));
	Thread_TargetCaller_postMsgstoMQTT.join();
}