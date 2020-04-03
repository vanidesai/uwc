/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../include/EISMsgbusHandler_ut.hpp"


#include <gtest/internal/gtest-internal.h>
#include <cstdlib>
#include <iostream>
#include <string>

config_t config_t_obj;

//#include "gmock/gmock.h"

using namespace std;


void EISMsgbusHandler_ut::SetUp() {
	// Setup code
}

void EISMsgbusHandler_ut::TearDown() {
	// TearDown code
}

// Topic is other that "pub" and "sub"
TEST_F(EISMsgbusHandler_ut, prepareCommonContext_InvTopic) {

	bool bRetVal = true;

	bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext("other");
	EXPECT_EQ(false, bRetVal);
}

TEST_F(EISMsgbusHandler_ut, getCTX_ValidTopic) {

	stZmqContext msgbusContext;
	string topic = "TCP1_RdResp"; //topic exists
	bool retVal = CEISMsgbusHandler::Instance().getCTX(topic, msgbusContext);

	EXPECT_EQ(true, retVal);
}

TEST_F(EISMsgbusHandler_ut, getCTX_InvTopic) {

	bool retVal = true;

	stZmqContext msgbusContext;
	string topic = "InvalidTopic"; //topic does not exist
	retVal = CEISMsgbusHandler::Instance().getCTX(topic, msgbusContext);

	EXPECT_EQ(false, retVal);
}


TEST_F(EISMsgbusHandler_ut, getPubCTX_ValTopic) {

	stZmqPubContext pubContext;
	string topic = "MQTT_Export_RdReq"; //topic exists
	bool retVal = CEISMsgbusHandler::Instance().getPubCTX(topic, pubContext);

	EXPECT_EQ(true, retVal);
}

TEST_F(EISMsgbusHandler_ut, getPubCTX_InvTopic) {

	bool retVal = true;

	stZmqPubContext pubContext;
	string topic = "InvalidTopic"; //topic does not exist
	retVal = CEISMsgbusHandler::Instance().getPubCTX(topic, pubContext);

	EXPECT_EQ(false, retVal);
}

TEST_F(EISMsgbusHandler_ut, getSubCTX_ValTopic) {

	stZmqSubContext subContext;
	string topic = "TCP1_RdResp"; //topic exists
	bool retVal = CEISMsgbusHandler::Instance().getSubCTX(topic, subContext);

	EXPECT_EQ(true, retVal);
}

TEST_F(EISMsgbusHandler_ut, getSubCTX_InvTopic) {

	bool retVal = true;

	stZmqSubContext subContext;
	string topic = "InvalidTopic"; //topic does not exist
	retVal = CEISMsgbusHandler::Instance().getSubCTX(topic, subContext);

	EXPECT_EQ(false, retVal);
}

TEST_F(EISMsgbusHandler_ut, removeSubCTX_SuccRemoves) {

	bool retVal = false;

	stZmqSubContext subContext;
	string topic = "RTU1_WrResp_RT"; //topic exists

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

TEST_F(EISMsgbusHandler_ut, 11_manatory_param) {

	stZmqPubContext pubContext;
	string topic = "MQTT_Export_RdReq"; //topic exists
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

TEST_F(EISMsgbusHandler_ut, 13_manatory_param) {

	stZmqContext context;
	string topic = "MQTT_Export_RdReq"; //topic exists
	bool retVal = CEISMsgbusHandler::Instance().getCTX(topic, context);

	if(retVal) {
		bool isInserted = CEISMsgbusHandler::Instance().insertCTX(topic, context);
		EXPECT_EQ(true, isInserted);
	}
	else{
		cout<<topic<<" not found in context map";
	}
}


TEST_F(EISMsgbusHandler_ut, RemoveCtxt) {

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

