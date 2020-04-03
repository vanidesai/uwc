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
#include "../include/EISMsgbusHandler_ut.hpp"


using namespace std;

void Main_ut::SetUp() {
	// Setup code
}

void Main_ut::TearDown() {
	// TearDown code
}

std::atomic<bool> g_shouldStop_ut(false);
sem_t g_semaphoreRespProcess_ut;

/* Valid JSON */
TEST_F(Main_ut, parse_msg_ValidJson) {

	try
	{
		const char *json = "{\"topic\": \"PL0_iou_write\", \"command\": \" DValve\", \"value\": \"0x00\", \"app_seq\":\"1234\"}";

		std::string topicName = parse_msg(json);
		std::cout << __func__ << " topic name parsed : " << topicName << endl;
		EXPECT_EQ(topicName, "PL0_iou_write");

	}catch(exception &ex) {
		EXPECT_EQ("", "PL0_iou_write");
	}
}


/* Valid JSON; "tooic value object is empty */
TEST_F(Main_ut, parse_msg_NoTopic) {

	try
	{
		const char *json = "{\"command\": \" DValve\", \"value\": \"0x00\", \"app_seq\":\"1234\"}";

		std::string topicName = parse_msg(json);
		std::cout << __func__ << " topic name parsed : " << topicName << endl;
		EXPECT_EQ(topicName, "");

	}catch(exception &ex) {
		EXPECT_EQ("", "PL0_iou_write");
	}
}

TEST_F(Main_ut, parse_msg_EmptyTopic) {

	try
	{
		const char *json = "{\"topic\": 0, \"command\": \" DValve\", \"value\": \"0x00\", \"app_seq\":\"1234\"}";

		std::string topicName = parse_msg(json);
		std::cout << __func__ << " topic name parsed : " << topicName << endl;
		EXPECT_EQ(topicName, "");

	}catch(exception &ex) {
		EXPECT_EQ("", "PL0_iou_write");
	}
}

/* Invalid JSON */
TEST_F(Main_ut, parse_msg_InvalidJson) {

	try
	{
		const char *json = "Invalid";

		std::string topicName = parse_msg(json);
		std::cout << __func__ << " topic name parsed : " << topicName << endl;
		EXPECT_EQ("", topicName);

	}catch(exception &ex) {
		EXPECT_EQ("", "PL0_iou_write");
	}
}

TEST_F(Main_ut, addSrTopic_InvalidTopic)
{

	string Inval_Topic = "";
	string Inval_JsonText = "";

	bool bRes = addSrTopic(Inval_JsonText, Inval_Topic);

	EXPECT_EQ(false, bRes);


}

TEST_F(Main_ut, addSrTopic_ValidTopic)
{
	mqtt::const_message_ptr recvdMsg;
	int msgRequestType = 1;
	bool RetVal = false;

	mqtt::const_message_ptr msg = mqtt::make_message(
			"{\"topic\": \"MQTT_Export_ReadRequest\"}",
			"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}");

	QMgr::CQueueMgr CQueueMgr_obj(1, 1);

	if (true == CQueueMgr_obj.getSubMsgFromQ(msg))
	{
		string rcvdTopic = recvdMsg->get_topic();
		string strMsg = recvdMsg->get_payload();
		RetVal = addSrTopic(strMsg, rcvdTopic);

		EXPECT_EQ(true, RetVal);
	}
	else
	{
		cout<<endl<<"######################ERROR##############################"<<endl;
		cout<<"Failed to getSubMsgFromQ. "<<endl<<"Exiting this test case";
		cout<<endl<<"#########################################################"<<endl;
	}
}

TEST_F(Main_ut, processMsgToSendOnEIS_TopicEmpty)
{
	bool isRealtime = false;
	bool isRead = false;
	bool RetVal;
	mqtt::const_message_ptr recvdMsg = mqtt::make_message(
			"{\"topic\": \"MQTT_Export_ReadRequest\"}",
			"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}");;

	RetVal = processMsgToSendOnEIS(recvdMsg, isRead, isRealtime);

	EXPECT_EQ(false, RetVal);
}

TEST_F(Main_ut, processMsgToSendOnEIS_MsgEmpty)
{
	bool isRealtime = false;
	bool isRead = false;
	bool RetVal;
	mqtt::const_message_ptr recvdMsg = mqtt::make_message(
			"",
			"");

	RetVal = processMsgToSendOnEIS(recvdMsg, isRead, isRealtime);

	EXPECT_EQ(false, RetVal);
}

TEST_F(Main_ut, processMsgToSendOnEIS_RTWriteReq)
{
	bool isRealtime = true;
	bool isRead = false;
	bool RetVal;
	mqtt::const_message_ptr recvdMsg = mqtt::make_message(
			"{\"topic\": \"MQTT_Export_ReadRequest\"}",
			"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}");;

	RetVal = processMsgToSendOnEIS(recvdMsg, isRead, isRealtime);

	EXPECT_EQ(true, RetVal);
}

TEST_F(Main_ut, processMsgToSendOnEIS_RTReadReq)
{
	bool isRealtime = true;
	bool isRead = true;
	bool RetVal;
	mqtt::const_message_ptr recvdMsg = mqtt::make_message(
			"{\"topic\": \"MQTT_Export_ReadRequest\"}",
			"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}");;

	RetVal = processMsgToSendOnEIS(recvdMsg, isRead, isRealtime);

	EXPECT_EQ(true, RetVal);
}

TEST_F(Main_ut, processMsgToSendOnEIS_NonRTReadReq)
{
	bool isRealtime = false;
	bool isRead = true;
	bool RetVal;
	mqtt::const_message_ptr recvdMsg = mqtt::make_message(
			"{\"topic\": \"MQTT_Export_ReadRequest\"}",
			"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"0\"}");;

	RetVal = processMsgToSendOnEIS(recvdMsg, isRead, isRealtime);

	EXPECT_EQ(false, RetVal);
}

TEST_F(Main_ut, processMsgToSendOnEIS_EisTopicEmpty)
{
	bool isRealtime = true;
	bool isRead = false;
	bool RetVal;
	mqtt::const_message_ptr recvdMsg = mqtt::make_message(
			"{\"topic\": \"MQTT_Export_ReadRequest\"}",
			"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}");;

	CCommon::getInstance().setStrRTWriteRequest("");

	RetVal = processMsgToSendOnEIS(recvdMsg, isRead, isRealtime);

	EXPECT_EQ(false, RetVal);
}

TEST_F(Main_ut, processMsgToSendOnEIS_InvSrcTopic)
{
	bool isRealtime = true;
	bool isRead = false;
	bool RetVal;
	mqtt::const_message_ptr recvdMsg = mqtt::make_message(
			"{\"topic\": \"MQTT_Export_ReadRequest\"}",
			"Inv");

	CCommon::getInstance().setStrRTWriteRequest("SomeInvValue");

	RetVal = processMsgToSendOnEIS(recvdMsg, isRead, isRealtime);

	EXPECT_EQ(false, RetVal);
}

TEST_F(Main_ut, processMsg_NULLMsg)
{
	string topic = "MQTT_Export_RdReq";
	CMQTTPublishHandler mqttPublisher(CCommon::getInstance().getStrMqttExportURL().c_str(), topic.c_str(), 0);

	bool RetVal = processMsg(NULL, mqttPublisher);

	EXPECT_EQ(false, RetVal);
}

TEST_F(Main_ut, processMsg_EmptyTopic)
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
	CMQTTPublishHandler mqttPublisher(CCommon::getInstance().getStrMqttExportURL().c_str(), topic.c_str(), 0);


	bool RetVal = processMsg(msg, mqttPublisher);

	EXPECT_EQ(false, RetVal);
}


TEST_F(Main_ut, processMsg_ValArg)
{
	msg_envelope_t *msg = NULL;

	msg_envelope_elem_body_t* ptVersion = msgbus_msg_envelope_new_string("2.0");
	msg_envelope_elem_body_t* ptDriverSeq = msgbus_msg_envelope_new_string("TestStr");
	msg_envelope_elem_body_t* ptTopic = msgbus_msg_envelope_new_string("TestStr_Topic");

	msg = msgbus_msg_envelope_new(CT_JSON);
	msgbus_msg_envelope_put(msg, "version", ptVersion);
	msgbus_msg_envelope_put(msg, "driver_seq", ptDriverSeq);
	msgbus_msg_envelope_put(msg, "topic", ptTopic);

	string topic = "MQTT_Export_RdReq";
	CMQTTPublishHandler mqttPublisher(CCommon::getInstance().getStrMqttExportURL().c_str(), topic.c_str(), 0);


	bool RetVal = processMsg(msg, mqttPublisher);

	EXPECT_EQ(true, RetVal);
}

TEST_F(Main_ut, publishEISMsg_Suc)
{
	stZmqContext context;
	stZmqPubContext pubContext;
	string topicType;

	string Topic_pub = "MQTT_Export_WrReq";
	string Topic_sub = "TestTopic_sub";

	bool RetVal = false;


	/*****************publishEISMsg***************************/
	if( true == CEISMsgbusHandler::Instance().getCTX(Topic_pub, context) )
	{
		if( true == CEISMsgbusHandler::Instance().getPubCTX(Topic_pub, pubContext) )
		{
			//Test target
			RetVal = publishEISMsg(strMsg, context, pubContext);
		}
		else
		{
			cout<<endl<<"#############################################"<<endl;
			cout<<"Error in getPubCTX() of pubContext; Skipped test Main_ut.publishEISMsg_Suc";
			cout<<endl<<"#############################################"<<endl;
		}
	}
	else
	{
		cout<<endl<<"#############################################"<<endl;
		cout<<"Error in getCTX() of context; Skipped test Main_ut.publishEISMsg_Suc";
		cout<<endl<<"#############################################"<<endl;
	}

	EXPECT_EQ(true, RetVal);
}

TEST_F(Main_ut, publishEISMsg_InvEISMsg)
{
	stZmqContext context;
	stZmqPubContext pubContext;
	string topicType;
	string EisMsg_UT = "InvalMsg";

	string Topic_pub = "MQTT_Export_WrReq";
	string Topic_sub = "TestTopic_sub";

	bool RetVal = true;


	/*****************publishEISMsg***************************/
	if( true == CEISMsgbusHandler::Instance().getCTX(Topic_pub, context) )
	{
		if( true == CEISMsgbusHandler::Instance().getPubCTX(Topic_pub, pubContext) )
		{
			//Test target
			RetVal = publishEISMsg(EisMsg_UT, context, pubContext);
		}
		else
		{
			cout<<endl<<"#############################################"<<endl;
			cout<<"Error in getPubCTX() of pubContext; Skipped test Main_ut.publishEISMsg_Suc";
			cout<<endl<<"#############################################"<<endl;
		}
	}
	else
	{
		cout<<endl<<"#############################################"<<endl;
		cout<<"Error in getCTX() of context; Skipped test Main_ut.publishEISMsg_Suc";
		cout<<endl<<"#############################################"<<endl;
	}

	EXPECT_EQ(false, RetVal);
}

TEST_F(Main_ut, publishEISMsg_NULLArg)
{
	stZmqContext context;
	stZmqPubContext pubContext;
	bool RetVal = true;

	context.m_pContext = NULL;
	pubContext.m_pContext = NULL;
	RetVal = publishEISMsg(strMsg, context, pubContext);

	EXPECT_EQ(false, RetVal);


}

/*************Wrapper Function for publishEISMsg()*******************/
bool ut_g_threadFlag = false;

void publishEISMsg_TestWrapper(stZmqContext msgbus_ctx, stZmqPubContext pub_ctx)
{
	string EISMsg = "{ 	\"value\": \"0xFF00\", 	\"command\": \"Pointname\", 	\"app_seq\": \"1234\" }";

	while(false == ut_g_threadFlag){
		publishEISMsg(EISMsg, msgbus_ctx, pub_ctx);
		cout<<endl<<"[UT Debug]>>>>>>>>>>>>>>>>>>>>>> Publishing message to EIS.."<<endl;
	}
}
/********************************************************************/

TEST_F(Main_ut, postMsgstoMQTT)
{

	string topic = "MQTT_Export_WrReq";
	string topicType = "sub";
	string msg = "UT_msg2Publish";

	stZmqContext msgbus_ctx;
	stZmqPubContext pub_ctx;

	if( true == CEISMsgbusHandler::Instance().getCTX(topic, msgbus_ctx) )
	{
		if( true == CEISMsgbusHandler::Instance().getPubCTX(topic, pub_ctx) )
		{

			postMsgstoMQTT();
			g_shouldStop = true;

			for (auto &th : g_vThreads)
			{
				if( th.joinable() )
				{
					th.detach();
				}
			}
		}
		else
		{
			cout<<endl<<"#############################################"<<endl;
			cout<<"Error in getPubCTX";
			cout<<endl<<"#############################################"<<endl;
		}
	}
	else
	{
		cout<<endl<<"#############################################"<<endl;
		cout<<"Error in getCTX";
		cout<<endl<<"#############################################"<<endl;
	}

	EXPECT_EQ(1, 1); //Should not crash
}

TEST_F(Main_ut, set_thread_priority_for_eis_RTRead)
{
	bool	realTime = true;
	bool	IsRead = true;

	set_thread_priority_for_eis(realTime, IsRead);

}

TEST_F(Main_ut, set_thread_priority_for_eis_RTWrite)
{
	bool	realTime = true;
	bool	IsRead = false;

	set_thread_priority_for_eis(realTime, IsRead);

}

TEST_F(Main_ut, set_thread_priority_for_eis_NonRTRead)
{
	bool	realTime = false;
	bool	IsRead = true;

	set_thread_priority_for_eis(realTime, IsRead);

}

TEST_F(Main_ut, set_thread_priority_for_eis_NonRTWrite)
{
	bool	realTime = false;
	bool	IsRead = false;

	set_thread_priority_for_eis(realTime, IsRead);

}

// Check that code doesnt hang
TEST_F(Main_ut, postMsgsToEIS_InValArg)
{

	QMgr::CQueueMgr CQueueMgr_obj(1,1);


	postMsgsToEIS(CQueueMgr_obj);
}

#if 0
// Check that code doesnt hang
TEST_F(Main_ut, postMsgsToEIS_ValArg)
{
	QMgr::CQueueMgr CQueueMgr_obj(1,1);

	QMgr::CQueueMgr qMgr = std::ref(CQueueMgr_obj.getRTRead());


	postMsgsToEIS(qMgr);
}
#endif

// CEISMsgbusHandler::Instance().cleanup();
TEST_F(Main_ut, cleanup_Success)
{
	string topic = "MQTT_Export_WrReq";
	stZmqContext msgbus_ctx;

	if( true == CEISMsgbusHandler::Instance().getCTX(topic, msgbus_ctx) )
	{
		CEISMsgbusHandler::Instance().cleanup();
		bool RetVal = CEISMsgbusHandler::Instance().getCTX(topic, msgbus_ctx);

		EXPECT_EQ(false, RetVal);
	}
}
