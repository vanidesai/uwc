/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/


#include "../include/MQTTExportMain_ut.hpp"
#include "../include/CEISMsgbusHandler_ut.hpp"

using namespace std;

std::string parse_msg(const char *json, int& qos);
extern bool initEISContext();
extern void postMsgstoMQTT();
extern void signalHandler(int signal);
extern bool addSrTopic(string &json, string& topic);
extern void postMsgsToEIS();
extern vector<std::thread> g_vThreads;

extern void Temp_Function(string& topic);


void listenOnEIS(string topic, stZmqContext context,
		stZmqSubContext subContext);

bool publishEISMsg(string eisMsg, stZmqContext &context,
		stZmqPubContext &pubContext);

extern std::atomic<bool> g_shouldStop;
extern vector<std::thread> g_vThreads;


vector<std::thread> g_vThreads_UT;

void MQTTExmportMain_ut::SetUp() {
	// Setup code
}

void MQTTExmportMain_ut::TearDown() {
	// TearDown code
}

std::atomic<bool> g_shouldStop_ut(false);
sem_t g_semaphoreRespProcess_ut;

//#if 0
/*TEST_F(MQTTExmportMain_ut, PublisherTest)
{
	int ret;
	char* msg[2] = 	{
					"Message1",
					"/home/user/SVN/Intel_UWC/trunk/Technical/Sourcecode/mqtt-export/MQTT-Export-App/Test/src/JsonConfig.json"
					};

	ret = Publisher_Main(2, msg);

	cout<<endl<<"####################################################"<<endl;
	cout<<"Publisher_Main() returns "<<ret;
	cout<<endl<<"####################################################"<<endl;
}*/

///* on hold
#if 0
TEST_F(MQTTExmportMain_ut, initEISContext)
{
	bool RetVal = false;

	CTopicMapper::getInstance();

	RetVal = initEISContext();

	CEISMsgbusHandler::Instance().cleanup();

	EXPECT_EQ(true, RetVal);

}
#endif
//*/

/* Valid JSON */
TEST_F(MQTTExmportMain_ut, parse_msg_ValidJson) {

	try
	{
		const char *json = "{\"topic\": \"PL0_iou_write\", \"command\": \" DValve\", \"value\": \"0x00\", \"app_seq\":\"1234\"}";

		int qos = 0;
		std::string topicName = parse_msg(json, qos);
		std::cout << __func__ << " topic name parsed : " << topicName << endl;
		EXPECT_EQ(topicName, "PL0_iou_write");

	}catch(exception &ex) {
		EXPECT_EQ("", "PL0_iou_write");
	}
}


/* Valid JSON; "tooic value object is empty */
TEST_F(MQTTExmportMain_ut, parse_msg_EmptyTopic) {

	try
	{
		const char *json = "{\"command\": \" DValve\", \"value\": \"0x00\", \"app_seq\":\"1234\"}";

		int qos = 0;
		std::string topicName = parse_msg(json, qos);
		std::cout << __func__ << " topic name parsed : " << topicName << endl;
		EXPECT_EQ(topicName, "");

	}catch(exception &ex) {
		EXPECT_EQ("", "PL0_iou_write");
	}
}

/* Valid JSON; "qos present in JASON */
TEST_F(MQTTExmportMain_ut, parse_msg_QOSPresent) {

	try
	{
		const char *json = "{\"qos\": \"1\", \"topic\": \"PL0_iou_write\", \"command\": \" DValve\", \"value\": \"0x00\", \"app_seq\":\"1234\"}";

		int qos = 0;
		std::string topicName = parse_msg(json, qos);
		std::cout << __func__ << " topic name parsed : " << topicName << endl;
		//EXPECT_EQ(topicName, "");

	}catch(exception &ex) {
		EXPECT_EQ("", "PL0_iou_write");
	}
}

/* Valid JSON; using default qos */
TEST_F(MQTTExmportMain_ut, parse_msg_QOSDefault) {

	try
	{
		const char *json = "{\"qos\": \"5\", \"topic\": \"PL0_iou_write\", \"command\": \" DValve\", \"value\": \"0x00\", \"app_seq\":\"1234\"}";

		int qos = 0;
		std::string topicName = parse_msg(json, qos);
		std::cout << __func__ << " topic name parsed : " << topicName << endl;
		//EXPECT_EQ(topicName, "");

	}catch(exception &ex) {
		EXPECT_EQ("", "PL0_iou_write");
	}
}

/* Invalid JSON */
TEST_F(MQTTExmportMain_ut, parse_msg_InvalidJson) {

	try
	{
		const char *json = "Invalid";

		int qos = 0;
		std::string topicName = parse_msg(json, qos);
		std::cout << __func__ << " topic name parsed : " << topicName << endl;
		EXPECT_EQ("", topicName);

	}catch(exception &ex) {
		EXPECT_EQ("", "PL0_iou_write");
	}
}

//test 01
TEST_F(MQTTExmportMain_ut, 1_parse_msg) {
	std::string topicName;

	try
	{
		const char *json = "{\"topic1\": \"PL0_iou_write\", \"command\": \" DValve\", \"value\": \"0x00\", \"app_seq\":\"1234\"}";

		int qos = 0;
		topicName = parse_msg(json, qos);
		std::cout << __func__ << " topic name parsed : " << topicName << endl;
		EXPECT_EQ("", topicName);

	}catch(exception &ex) {
		EXPECT_EQ("", topicName);
	}
}


//test 01
/*TEST_F(MQTTExmportMain_ut, 3_postMsgstoMQTT) {

	try
	{
//		signalHandler(SIGUSR1);

	}
	catch(exception &ex)
	{
		// Application restarts
		EXPECT_EQ(true, true);
	}
}*/

/* Testing if addSrTopic() */
/* Un-Successful addition of sourcetopic key in payload */
TEST_F(MQTTExmportMain_ut, addSrTopic_InvalidTopic)
{

	mqtt::const_message_ptr recvdMsg;


	mqtt::const_message_ptr msg = mqtt::make_message("Topic_ut",
			"PL0_iou_write");
	CMQTTHandler::instance().pushSubMsgInQ(msg);

	std::string TcvdTopic;
	std::string StrMsg;

	while (false == g_shouldStop_ut.load())
	{

		if (false == CMQTTHandler::instance().getSubMsgFromQ(recvdMsg))
		{
			continue;
		}
		TcvdTopic = recvdMsg->get_topic();
		StrMsg = recvdMsg->get_payload();
		break;
	}

	try
	{
		bool bRes = addSrTopic(StrMsg, TcvdTopic);
		EXPECT_EQ(false, bRes);
	}
	catch(exception &ex)
	{
		//
	}
}

/* Successful addition of sourcetopic key in payload */
TEST_F(MQTTExmportMain_ut, addSrTopic_ValidTopic)
{

	mqtt::const_message_ptr recvdMsg;
	std::string StrMsg;
	std::string TcvdTopic;

	mqtt::const_message_ptr msg = mqtt::make_message(
			"{\"topic\":\"PL0_iou_write\",\"command\":\" DValve\",\"value\":\"0x00\",\"app_seq\":\"1234\"}",
			"Test_Source_Topic_ToBeAdded");

	CMQTTHandler::instance().pushSubMsgInQ(msg);

	while (false == g_shouldStop_ut.load())
	{
		if (false == CMQTTHandler::instance().getSubMsgFromQ(recvdMsg))
		{
			continue;
		}
		TcvdTopic = recvdMsg->get_topic();
		StrMsg = recvdMsg->get_payload();
		break;
	}

	//	cout<<endl<<"################Before####################"<<endl;
	//	cout<<"TcvdTopic: "<<TcvdTopic<<endl;
	//	cout<<endl<<"##########################################"<<endl;

	try
	{
		bool bRes = addSrTopic(TcvdTopic, StrMsg);

		//		cout<<endl<<"################After#####################"<<endl;
		//		cout<<"TcvdTopic: "<<TcvdTopic<<endl;
		//		cout<<endl<<"##########################################"<<endl;

		EXPECT_EQ(true, bRes);
	}
	catch(exception &ex)
	{

	}

}

#ifdef PROBLEMATIC
// No message is published..
TEST_F(MQTTExmportMain_ut, listenOnEIS_SuccussListens)
{
	string subTopic = "Modbus_TCP_Master_PolledData";

	stZmqContext context;
	stZmqSubContext subContext;

	bool ret;

	CEISMsgbusHandler::Instance().cleanup();
	if ( true == CEISMsgbusHandler::Instance().prepareCommonContext("pub") )
	{
		if ( true == CEISMsgbusHandler::Instance().prepareCommonContext("sub") )
		{

			if( true == CEISMsgbusHandler::Instance().getCTX(subTopic, context) )
			{
				if( true == CEISMsgbusHandler::Instance().getSubCTX(subTopic, subContext) )
				{
					try
					{
						g_vThreads.push_back(
								std::thread(listenOnEIS, subTopic, context, subContext));
					}
					catch(exception &e)
					{
						cout<<endl<<"FATAL ERROR#################################################"<<endl;
						EXPECT_EQ(1,0); //Test case fails;exception shouldn't have raised
					}
				}
			}
		}
	}
}
#endif

#if 0
TEST_F(MQTTExmportMain_ut, publishEISMsg_Suc)
{
	stZmqContext context;
	stZmqPubContext pubContext;

	string Topic = "MQTT_Export_ReadRequest";

	bool RetVal = false;

	CTopicMapper::getInstance();
	CMQTTHandler::instance();

	CEISMsgbusHandler::Instance().cleanup();

	if ( true == CEISMsgbusHandler::Instance().prepareCommonContext("pub") )
	{
		if ( true == CEISMsgbusHandler::Instance().prepareCommonContext("sub") )
		{
			if( true == CEISMsgbusHandler::Instance().getCTX(Topic, context) )
			{
				if( true == CEISMsgbusHandler::Instance().getPubCTX(Topic, pubContext) )
				{
					//Test target
					RetVal = publishEISMsg(strMsg, context, pubContext);
				}
				else
				{
					cout<<endl<<"#############################################"<<endl;
					cout<<"Error in getPubCTX() of pubContext; Skipped test MQTTExmportMain_ut.publishEISMsg_Suc";
					cout<<endl<<"#############################################"<<endl;
				}
			}
			else
			{
				cout<<endl<<"#############################################"<<endl;
				cout<<"Error in getCTX() of context; Skipped test MQTTExmportMain_ut.publishEISMsg_Suc";
				cout<<endl<<"#############################################"<<endl;
			}
		}
		else
		{
			cout<<endl<<"#############################################"<<endl;
			cout<<"Error in prepareCommonContext(\"sub\"); Skipped test MQTTExmportMain_ut.publishEISMsg_Suc";
			cout<<endl<<"#############################################"<<endl;
		}
	}
	else
	{
		cout<<endl<<"#############################################"<<endl;
		cout<<"Error in prepareCommonContext(\"pub\"); Skipped test MQTTExmportMain_ut.publishEISMsg_Suc";
		cout<<endl<<"#############################################"<<endl;
	}

	EXPECT_EQ(true, RetVal);
}
#endif

bool ut_g_threadFlag = false;
/*************Wrapper Function for publishEISMsg()*******************/
void publishEISMsg_TestWrapper(stZmqContext msgbus_ctx, stZmqPubContext pub_ctx)
{
	string EISMsg = "{ 	\"value\": \"0xFF00\", 	\"command\": \"Pointname\", 	\"app_seq\": \"1234\" }";

	while(false == ut_g_threadFlag){
		publishEISMsg(EISMsg, msgbus_ctx, pub_ctx);
		cout<<endl<<"[UT Debug]>>>>>>>>>>>>>>>>>>>>>> Publishing message to EIS.."<<endl;
	}
}
/********************************************************************/

#if 0
TEST_F(MQTTExmportMain_ut, postMsgstoMQTT)
{

	string topic = "MQTT_Export_ReadRequest";
	string topicType = "sub";
	string msg = "UT_msg2Publish";

	stZmqContext msgbus_ctx;
	stZmqPubContext pub_ctx;

#if 0
	/****************Insert context*********************************/
	config_t* config = CfgManager::Instance().getEnvClient()->get_messagebus_config(
			CfgManager::Instance().getConfigClient(),
			topic.c_str(), topicType.c_str());


	void* msgbus_ctx = msgbus_initialize(config);


	stZmqContext objTempCtx;
	objTempCtx.m_pContext = msgbus_ctx;

	CEISMsgbusHandler::Instance().insertCTX(topic, objTempCtx);
	/**************************************************************/



	/****************Insert sub-context****************************/
	recv_ctx_t* sub_ctx = NULL;
	std::size_t pos = topic.find('/');
	std::string subTopic(topic.substr(pos + 1));

	msgbus_subscriber_new(msgbus_ctx, subTopic.c_str(), NULL, &sub_ctx);

	stZmqSubContext objTempSubCtx;
	objTempSubCtx.m_pContext = NULL;
	/**************************************************************/

	/*
	cout<<endl<<"Debug(UT)########################################################"<<endl;
	cout<<"objTempCtx.m_pContext = "<<objTempCtx.m_pContext<<endl;
	cout<<"objTempSubCtx.m_pContext = "<<objTempSubCtx.m_pContext<<endl;
	cout<<"topic = "<<topic<<endl;
	cout<<"subTopic = "<<subTopic;
	cout<<endl<<"########################################################"<<endl;
	 */

#endif


	CTopicMapper::getInstance();
	CMQTTHandler::instance();


	CEISMsgbusHandler::Instance().cleanup();

	if ( true == CEISMsgbusHandler::Instance().prepareCommonContext("pub") )
	{
		if ( true == CEISMsgbusHandler::Instance().prepareCommonContext("sub") )
		{
			if( true == CEISMsgbusHandler::Instance().getCTX(topic, msgbus_ctx) )
			{
				if( true == CEISMsgbusHandler::Instance().getPubCTX(topic, pub_ctx) )
				{
//					msg_envelope_t *msg = msgbus_msg_envelope_new(CT_JSON);
//					msg_envelope_elem_body_t *value = msgbus_msg_envelope_new_string("UT_TestString");
//
//					msgbus_msg_envelope_put(msg, "Device_UT", value);
//
//					std::thread th_pub(&publishEISMsg_TestWrapper, msgbus_ctx, pub_ctx);
					postMsgstoMQTT();

//					ut_g_threadFlag = true;
//
//					th_pub.join();
//					for (auto &th : g_vThreads)
//					{
//						th.join();
//					}
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
		}
	}

	EXPECT_EQ(1, 1);
}
#endif

#if 0
TEST_F(MQTTExmportMain_ut, postMsgstoMQTT)
{

	string topic = "MQTT_Export_ReadRequest";
	string topicType = "sub";
	string msg = "UT_msg2Publish";

	stZmqContext msgbus_ctx;
	stZmqPubContext pub_ctx;


	CTopicMapper::getInstance();
	CMQTTHandler::instance();


	CEISMsgbusHandler::Instance().cleanup();

	bool RetTemp = initEISContext();
	cout<<endl<<"[UT Debug]>>>>>>>>>>>>>>>>>>>>>> initEISContext return: "<<RetTemp<<endl;

}
#endif

