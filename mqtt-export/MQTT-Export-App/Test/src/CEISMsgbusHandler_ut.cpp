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

TEST_F(CEISMsgbusHandler_ut, prepareCommonContext_Success_pub)
{

	bool bRetVal = false;

	string topic = "PL01_iou_write";
	//string topic = "MQTT_Export_ReadRequest,MQTT_Export_WriteRequest"
	string topic_type = "pub";


	bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext(topic_type);
	EXPECT_EQ(true, bRetVal);

}

TEST_F(CEISMsgbusHandler_ut, prepareCommonContext_Success_sub)
{
	bool bRetVal = false;

	string topic = "Modbus-TCP-Master/PL0_flowmeter1";
	string topic_type = "sub";

	setenv("SubTopics", "Modbus-TCP-Master/PL0_flowmeter1", 1);


	bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext(topic_type);

	setenv("SubTopics", "Modbus_TCP_Master/Modbus_TCP_Master_PolledData,Modbus_TCP_Master/Modbus_TCP_Master_ReadResponse,Modbus_TCP_Master/Modbus_TCP_Master_WriteResponse,Modbus_RTU_Master/Modbus_RTU_Master_PolledData,Modbus_RTU_Master/Modbus_RTU_Master_ReadResponse,Modbus_RTU_Master/Modbus_RTU_Master_WriteResponse", 1);

	EXPECT_EQ(true, bRetVal);

}

// Topic is other that "pub" and "sub"
TEST_F(CEISMsgbusHandler_ut, prepareCommonContext_InvTopic) {

	bool bRetVal = true;

	bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext("other");
	EXPECT_EQ(false, bRetVal);
}



#if 0 // To be updated later
TEST_F(CEISMsgbusHandler_ut, 2_manatory_param) {

	stZmqContext msgbusContext;
	string topic = "PL01_iou_write"; //topic exists
	bool retVal = CEISMsgbusHandler::Instance().getCTX(topic, msgbusContext);

	EXPECT_EQ(true, retVal);
}
#endif

TEST_F(CEISMsgbusHandler_ut, 3_manatory_param) {

	bool retVal = true;

	stZmqContext msgbusContext;
	string topic = "PL01_iou_write_test"; //topic does not exist
	retVal = CEISMsgbusHandler::Instance().getCTX(topic, msgbusContext);

	EXPECT_EQ(false, retVal);
}

#if 0 // To be updated later
TEST_F(CEISMsgbusHandler_ut, 4_manatory_param) {

	stZmqPubContext pubContext;
	string topic = "PL01_iou_write"; //topic exists
	bool retVal = CEISMsgbusHandler::Instance().getPubCTX(topic, pubContext);

	EXPECT_EQ(true, retVal);
}
#endif

TEST_F(CEISMsgbusHandler_ut, getPubCTX_InvTopic) {

	bool retVal = true;

	stZmqPubContext pubContext;
	string topic = "PL01_iou_write_test"; //topic does not exist
	retVal = CEISMsgbusHandler::Instance().getPubCTX(topic, pubContext);

	EXPECT_EQ(false, retVal);
}

TEST_F(CEISMsgbusHandler_ut, getPubCTX_ValTopic)
{

	bool retVal = false;
	bool isInserted = false;

	stZmqPubContext pubContext;
	string topic = "PL01_iou_write"; //topic exists

	if( true == CEISMsgbusHandler::Instance().insertPubCTX(topic, pubContext) )
	{
		retVal = CEISMsgbusHandler::Instance().getPubCTX(topic, pubContext);
		EXPECT_EQ(true, retVal);
	}
	else
	{
		cout<<endl<<"[UT Msg]:  >>>>>>>>>>>>>Error in insertPubCTX() for topic "<<topic;
	}
}

#if 0 // To be updated later
TEST_F(CEISMsgbusHandler_ut, 6_manatory_param) {

	stZmqSubContext subContext;
	string topic = "PL0_flowmeter1"; //topic exists
	bool retVal = CEISMsgbusHandler::Instance().getSubCTX(topic, subContext);

	EXPECT_EQ(true, retVal);
}
#endif

TEST_F(CEISMsgbusHandler_ut, getSubCTX_InvTopic) {

	bool retVal = true;

	stZmqSubContext subContext;
	string topic = "PL0_flowmeter1_test"; //topic does not exist
	retVal = CEISMsgbusHandler::Instance().getSubCTX(topic, subContext);

	EXPECT_EQ(false, retVal);
}

TEST_F(CEISMsgbusHandler_ut, removeSubCTX_SuccRemoves) {

	bool retVal = false;

	stZmqSubContext subContext;
	string topic = "PL0_flowmeter1"; //topic exists

	retVal = CEISMsgbusHandler::Instance().getSubCTX(topic, subContext);

	if(retVal)
	{
		stZmqContext msgbusContext;
		bool retVal = CEISMsgbusHandler::Instance().getCTX(topic, msgbusContext);

		if(retVal)
		{
			CEISMsgbusHandler::Instance().removeSubCTX(topic, msgbusContext);
		}
		else
		{
			cout<<endl<<"###################################################"<<endl;
			cout<<"Error in getCTX()"<<endl;
			cout<<"Exiting ut..";
			cout<<endl<<"###################################################"<<endl;
			//exit(0);
		}
	}
	else
	{
		cout<<endl<<"###################################################"<<endl;
		cout<<"Error in getSubCTX()"<<endl;
		cout<<"Exiting ut..";
		cout<<endl<<"###################################################"<<endl;
		//exit(0);
	}

	//Should not hang
	EXPECT_EQ(1, 1); //Execution of this line means the code doesnt terminate unexpectedly
}

#if 0
/* ############################################################## */
/* ############################################################## */
TEST_F(CEISMsgbusHandler_ut, 8_manatory_param) {

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
/* ############################################################## */
/* ############################################################## */
#endif





TEST_F(CEISMsgbusHandler_ut, 11_manatory_param) {

	stZmqPubContext pubContext;
	string topic = "PL0_flowmeter1"; //topic exists
	bool retVal = CEISMsgbusHandler::Instance().getPubCTX(topic, pubContext);
	if(retVal) {

		stZmqContext msgbusContext;
		bool retVal = CEISMsgbusHandler::Instance().getCTX(topic, msgbusContext);
		if(retVal) {
			CEISMsgbusHandler::Instance().removePubCTX(topic, msgbusContext);
		}
	}
}


TEST_F(CEISMsgbusHandler_ut, 12_manatory_param) {

	try {

		CEISMsgbusHandler::Instance().cleanup();
		EXPECT_EQ(true, true);

	}catch(exception &ex) {

		std::cout << __func__ << " cleanup test for EIS msg bus\n";
		EXPECT_EQ(true, false);
	}
}

TEST_F(CEISMsgbusHandler_ut, 13_manatory_param) {

	stZmqContext context;
	string topic = "PL0_flowmeter1"; //topic exists
	bool retVal = CEISMsgbusHandler::Instance().getCTX(topic, context);

	if(retVal) {
		bool isInserted = CEISMsgbusHandler::Instance().insertCTX(topic, context);
		EXPECT_EQ(true, isInserted);
	}
}


TEST_F(CEISMsgbusHandler_ut, 14_manatory_param) {

	bool retVal;
	stZmqContext context;
	string topic = "PL0_flowmeter1"; //topic exists

	CEISMsgbusHandler::Instance().removeCTX(topic);

	retVal = CEISMsgbusHandler::Instance().getCTX(topic, context);
	EXPECT_EQ(false, retVal); // getCTX should return false after context removal

}


TEST_F(CEISMsgbusHandler_ut, RemoveCtxt) {

	bool retVal = false;
	stZmqContext context;
	string topic = "PL0_flowmeter1"; //topic exists

	/*	retVal = CEISMsgbusHandler::Instance(
			EnvConf_CallObj,
			ConCAller_CallObj,
			MsgBusCaller_CallObj).getCTX(topic, context);*/

	CEISMsgbusHandler::Instance().insertCTX("PL0_flowmeter1", context);


	CEISMsgbusHandler::Instance().removeCTX(topic);


	//EXPECT_EQ(true, retVal);

}

