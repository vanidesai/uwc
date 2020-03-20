/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../include/CEISMsgbusHandler_ut.hpp"


#include <gtest/internal/gtest-internal.h>
#include <cstdlib>
#include <iostream>
#include <string>

config_t config_t_obj;

//#include "gmock/gmock.h"

using namespace std;


void CEISMsgbusHandler_ut::SetUp() {
	// Setup code
}

void CEISMsgbusHandler_ut::TearDown() {
	// TearDown code
}

#if 0 //CONTAINER
TEST_F(CEISMsgbusHandler_ut, prepareCommonContext_Success_pub)
{

	bool bRetVal = false;

	CEISMsgbusHandler::Instance().cleanup();

	bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext("pub");
	EXPECT_EQ(true, bRetVal);

	CEISMsgbusHandler::Instance().cleanup();


}
#endif

#if 0 //CONTAINER
TEST_F(CEISMsgbusHandler_ut, prepareCommonContext_Success_sub)
{
	bool bRetVal = false;

	bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext("sub");

	EXPECT_EQ(true, bRetVal);

}
#endif

// Topic is other that "pub" and "sub"
TEST_F(CEISMsgbusHandler_ut, prepareCommonContext_InvTopic) {

	bool bRetVal = true;

	bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext("other");
	EXPECT_EQ(false, bRetVal);
}



TEST_F(CEISMsgbusHandler_ut, getCTX_ValidTopic) {

	stZmqContext msgbusContext;
	string topic = "Modbus_TCP_Master_ReadResponse"; //topic exists
	bool retVal = CEISMsgbusHandler::Instance().getCTX(topic, msgbusContext);

	EXPECT_EQ(true, retVal);
}


TEST_F(CEISMsgbusHandler_ut, getCTX_InvTopic) {

	bool retVal = true;

	stZmqContext msgbusContext;
	string topic = "InvalidTopic"; //topic does not exist
	retVal = CEISMsgbusHandler::Instance().getCTX(topic, msgbusContext);

	EXPECT_EQ(false, retVal);
}


TEST_F(CEISMsgbusHandler_ut, getPubCTX_ValTopic) {

	stZmqPubContext pubContext;
	string topic = "MQTT_Export_ReadRequest"; //topic exists
	bool retVal = CEISMsgbusHandler::Instance().getPubCTX(topic, pubContext);

	EXPECT_EQ(true, retVal);
}


TEST_F(CEISMsgbusHandler_ut, getPubCTX_InvTopic) {

	bool retVal = true;

	stZmqPubContext pubContext;
	string topic = "InvalidTopic"; //topic does not exist
	retVal = CEISMsgbusHandler::Instance().getPubCTX(topic, pubContext);

	EXPECT_EQ(false, retVal);
}

TEST_F(CEISMsgbusHandler_ut, getSubCTX_ValTopic) {

	stZmqSubContext subContext;
	string topic = "Modbus_TCP_Master_WriteResponse"; //topic exists
	bool retVal = CEISMsgbusHandler::Instance().getSubCTX(topic, subContext);

	EXPECT_EQ(true, retVal);
}

TEST_F(CEISMsgbusHandler_ut, getSubCTX_InvTopic) {

	bool retVal = true;

	stZmqSubContext subContext;
	string topic = "InvalidTopic"; //topic does not exist
	retVal = CEISMsgbusHandler::Instance().getSubCTX(topic, subContext);

	EXPECT_EQ(false, retVal);
}

TEST_F(CEISMsgbusHandler_ut, removeSubCTX_SuccRemoves) {

	bool retVal = false;

	stZmqSubContext subContext;
	string topic = "Modbus_TCP_Master_PolledData"; //topic exists

	retVal = CEISMsgbusHandler::Instance().getSubCTX(topic, subContext);

	if(retVal)
	{
		stZmqContext msgbusContext;
		bool retVal = CEISMsgbusHandler::Instance().getCTX(topic, msgbusContext);

		if(retVal)
		{
			CEISMsgbusHandler::Instance().removeSubCTX(topic, msgbusContext);

			bool retVal = CEISMsgbusHandler::Instance().getSubCTX(topic, subContext);
			EXPECT_EQ(false, retVal);
		}
		else
		{
			cout<<endl<<"###################################################"<<endl;
			cout<<"Error in getCTX()"<<endl;
			cout<<"Skipping this test case";
			cout<<endl<<"###################################################"<<endl;
		}
	}
	else
	{
		cout<<endl<<"###################################################"<<endl;
		cout<<"Error in getSubCTX()"<<endl;
		cout<<"Skipping this test case";
		cout<<endl<<"###################################################"<<endl;
	}
}

#if 0 //CONTAINER
TEST_F(CEISMsgbusHandler_ut, insertSubCTX__) {

	recv_ctx_t* sub_ctx = NULL;

	bool retVal = false;

	string topic_type = "sub";

	stZmqSubContext subContext;
	string topic = "PL0_flowmeter1"; //topic exists


	/* Insert subscriber context */
	config_t* config = CfgManager::Instance().getEnvConfig().get_messagebus_config(topic, topic_type);
	void* ctx = msgbus_initialize(config);
	msgbus_subscriber_new(ctx, topic.c_str(), NULL, &sub_ctx);

	subContext.m_pContext = sub_ctx;
	retVal = CEISMsgbusHandler::Instance().insertSubCTX(topic, subContext);

	/* Get subscriber context */
	retVal = CEISMsgbusHandler::Instance().getSubCTX(topic, subContext);

	cout<<endl<<"#####################################################"<<endl;
	cout<<"retVal: "<<retVal;
	cout<<endl<<"#####################################################"<<endl;

	if(retVal) {
		bool isInserted = CEISMsgbusHandler::Instance().insertSubCTX(topic, subContext);
		EXPECT_EQ(true, isInserted);
	}
}
#endif


TEST_F(CEISMsgbusHandler_ut, 11_manatory_param) {

	stZmqPubContext pubContext;
	string topic = "MQTT_Export_ReadRequest"; //topic exists
	bool retVal = CEISMsgbusHandler::Instance().getPubCTX(topic, pubContext);
	if(retVal) {

		stZmqContext msgbusContext;
		bool retVal = CEISMsgbusHandler::Instance().getCTX(topic, msgbusContext);
		if(retVal) {
			CEISMsgbusHandler::Instance().removePubCTX(topic, msgbusContext);
		}
	}
	else
	{
		cout<<topic<<" not found";
	}
}

#if 0 //CONTAINER
TEST_F(CEISMsgbusHandler_ut, 12_manatory_param) {

	try {

		CEISMsgbusHandler::Instance().cleanup();
		EXPECT_EQ(true, true);

	}catch(exception &ex) {

		std::cout << __func__ << " cleanup test for EIS msg bus\n";
		EXPECT_EQ(true, false);
	}
}
#endif

TEST_F(CEISMsgbusHandler_ut, 13_manatory_param) {

	stZmqContext context;
	string topic = "PL0_flowmeter1"; //topic exists
	bool retVal = CEISMsgbusHandler::Instance().getCTX(topic, context);

	if(retVal) {
		bool isInserted = CEISMsgbusHandler::Instance().insertCTX(topic, context);
		EXPECT_EQ(true, isInserted);
	}
	else{
		cout<<topic<<" not found in context map";
	}
}

#if 0 //CONTAINER
TEST_F(CEISMsgbusHandler_ut, 14_manatory_param) {

	bool retVal;
	stZmqContext context;
	string topic = "Test_Topic";
	setenv("Test_Topic_cfg", "zmq_tcp,127.0.0.1:5097", 1);
	string topicType = "pub";

	CEISMsgbusHandler::Instance().cleanup();

	/*****************Insert context**********************************/
	config_t* config = CfgManager::Instance().getEnvClient()->get_messagebus_config(
				CfgManager::Instance().getConfigClient(),
				topic.c_str(), topicType.c_str());


	void* msgbus_ctx = msgbus_initialize(config);

	context.m_pContext = msgbus_ctx;
	CEISMsgbusHandler::Instance().insertCTX(topic, context);
	/****************************************************************/

	//Removing context
	CEISMsgbusHandler::Instance().removeCTX(topic);


	retVal = CEISMsgbusHandler::Instance().getCTX(topic, context);
	EXPECT_EQ(false, retVal); // getCTX should return false after context removal

}
#endif

TEST_F(CEISMsgbusHandler_ut, RemoveCtxt) {

	bool retVal = true;
	stZmqContext context;
	string topic = "Insert_Topic_UT";

	context.m_pContext = NULL;
	if( false == CEISMsgbusHandler::Instance().getCTX(topic, context) )
	{
		if( true == CEISMsgbusHandler::Instance().insertCTX(topic, context) )
		{
			if( true == CEISMsgbusHandler::Instance().getCTX(topic, context) )
			{
				CEISMsgbusHandler::Instance().removeCTX(topic);
				retVal = CEISMsgbusHandler::Instance().getCTX(topic, context);
			}
		}
	}

	EXPECT_EQ(false, retVal);
}

