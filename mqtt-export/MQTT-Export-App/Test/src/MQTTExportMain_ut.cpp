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
#include "../include/MsgPublisher_ut.hpp"


using namespace std;

std::string parse_msg(const char *json, int& qos);
extern bool initEISContext();
extern void postMsgstoMQTT();
extern void signalHandler(int signal);
extern bool addSrTopic(string &json, string& topic);
extern void postMsgsToEIS();
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

//test 01
/*TEST_F(MQTTExmportMain_ut, 2_initEISContext) {
	bool retVal = false;


	try
	{
		retVal = initEISContext();
	}

	catch(exception &ex)
	{
		retVal = false;
	}

	EXPECT_EQ(true, retVal);
}*/

TEST_F(MQTTExmportMain_ut, initEISContext)
{
	bool RetVal = false;

	CTopicMapper::getInstance();

	RetVal = initEISContext();

	EXPECT_EQ(false, RetVal);

}

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


/*// No message is published..
TEST_F(MQTTExmportMain_ut, listenOnEIS_SuccussListens)
{
	string subTopic = "Modbus-TCP-Master/PL0_flowmeter1";
	string pubTopic = "PL01_iou_write";
	stZmqContext context;
	stZmqSubContext subContext;

	bool ret;

	ret = CEISMsgbusHandler::Instance().getCTX(subTopic, context);
	ret = CEISMsgbusHandler::Instance().getSubCTX(subTopic, subContext);

	try
	{
		//Publisher_Main(2, msg);
		listenOnEIS(subTopic, context, subContext);

//		g_vThreads.push_back(
//		std::thread(&MQTT_Export::listenOnEIS, MQTT_Export_tempObj, subTopic, context, subContext));
	}
	catch(exception &e)
	{
		cout<<endl<<"FATAL ERROR#################################################"<<endl;
		EXPECT_EQ(1,0); //Test case fails;exception shouldn't have raised
	}

}
 */


#ifdef TEMP
TEST_F(MQTTExmportMain_ut, postMsgsToEIS)
{
	bool bRetVal = false;

	bool tempRes = false;
	string PTopic = "PL1_flowmeter2";
	string subTopic = "Modbus-TCP-Master/PL1_flowmeter2";
	string TopicType = "pub";
	/*config_t* config = CfgManager::Instance().getEnvConfig().get_messagebus_config(
			CfgManager::Instance().getConfigClient(),
			PTopic,
			TopicType);
	 */

	setenv("SubTopics", "Modbus-TCP-Master/PL1_flowmeter2", 1);


	config_t* config = CfgManager::Instance().getEnvClient()->get_messagebus_config
			(
					CfgManager::Instance().getConfigClient(),
					PTopic.c_str(),
					TopicType.c_str()
			);

	/* Insert context */
	void* msgbus_ctx = NULL;
	msgbus_ctx = msgbus_initialize(config);
	stZmqContext objTempCtx;
	objTempCtx.m_pContext = msgbus_ctx;
	cout<<endl<<"[UT Debug]: #######################################"<<endl;
	cout<<"msgbus_ctx = "<<msgbus_ctx;
	tempRes = CEISMsgbusHandler::Instance().insertCTX("PL0_flowmeter1", objTempCtx);

	if( true == tempRes)
	{
		cout<<endl<<"[UT Msg]:  >>>>>>>>>>>>>insertCTX() success for topic PL0_flowmeter1";

		/* Insert subcribe context */
		stZmqSubContext objTempSubCtx;
		recv_ctx_t* sub_ctx = NULL;
		msgbus_subscriber_new(msgbus_ctx, subTopic.c_str(), NULL, &sub_ctx);
		objTempSubCtx.m_pContext = sub_ctx;
		cout<<endl<<"[UT Debug]: #######################################"<<endl;
		cout<<"sub_ctx = "<<sub_ctx;
		tempRes = CEISMsgbusHandler::Instance().insertSubCTX("PL0_flowmeter1", objTempSubCtx);

		if( true == tempRes)
		{
			cout<<endl<<"[UT Msg]:  >>>>>>>>>>>>>insertSubCTX() success for subtopic "<<subTopic;
			cout<<endl<<"[UT Msg]:  >>>>>>>>>>>>>Posting message to MQTT..";

			/* Posting message to MQTT */
			postMsgstoMQTT();
		}
		else
		{
			cout<<endl<<"[UT Msg]:  >>>>>>>>>>>>>Error in insertSubCTX() for subtopic "<<subTopic;
		}

	}
	else
	{
		cout<<endl<<"[UT Msg]:  >>>>>>>>>>>>>Error in insertCTX() for topic PL0_flowmeter1";
	}

	setenv("SubTopics", "Modbus_TCP_Master/Modbus_TCP_Master_PolledData,Modbus_TCP_Master/Modbus_TCP_Master_ReadResponse,Modbus_TCP_Master/Modbus_TCP_Master_WriteResponse,Modbus_RTU_Master/Modbus_RTU_Master_PolledData,Modbus_RTU_Master/Modbus_RTU_Master_ReadResponse,Modbus_RTU_Master/Modbus_RTU_Master_WriteResponse", 1);
}
#endif

TEST_F(MQTTExmportMain_ut, postMsgstoMQTT)
{
	/* Posting message to MQTT */
	postMsgstoMQTT();

}

#ifdef INPROGRESS
TEST_F(MQTTExmportMain_ut, publishEISMsg_Suc)
{
	string eisTopic = "EIS_Topic";

	stZmqContext context;
	stZmqPubContext pubContext;

	string topic = "Modbus-TCP-Master/PL1_flowmeter2";
	string topicType = "sub";




	config_t* config = CfgManager::Instance().getEnvClient()->get_messagebus_config(
			CfgManager::Instance().getConfigClient(),
			topic.c_str(),
			topicType.c_str());

	void* msgbus_ctx = msgbus_initialize(config);
	context.m_pContext = msgbus_ctx;
	CEISMsgbusHandler::Instance().insertCTX(topic, context );




	publisher_ctx_t *pub_ctx = NULL;
	//msgbus_publisher_new(msgbus_ctx, topic.c_str(), &pub_ctx);

//	pubContext.m_pContext = pub_ctx;
	pubContext.m_pContext = msgbus_ctx;
	CEISMsgbusHandler::Instance().insertPubCTX(topic, pubContext);


	if( true == CEISMsgbusHandler::Instance().getCTX(topic, context) )
	{
		if( true == CEISMsgbusHandler::Instance().getPubCTX(topic, pubContext) )
		{
			cout<<endl<<"#############################################"<<endl;
			cout<<"context.m_pContext: "<<context.m_pContext;
			cout<<endl;
			cout<<"pubContext.m_pContext: "<<pubContext.m_pContext;
			cout<<endl<<"#############################################"<<endl;
			publishEISMsg(strMsg, context, pubContext);
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
#endif

