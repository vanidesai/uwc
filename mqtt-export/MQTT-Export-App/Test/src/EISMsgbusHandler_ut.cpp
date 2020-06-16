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
TEST_F(EISMsgbusHandler_ut, prepareCommonContext_InvTopicType) {

	bool bRetVal = true;

	bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext("other");
	EXPECT_EQ(false, bRetVal);
}

#if 0 // In progress
TEST_F(EISMsgbusHandler_ut, prepareCommonContext_pub)
{

	bool bRetVal = false;

	const char* env_pubTopics = std::getenv("PubTopics");
	if (env_pubTopics != NULL)
	{
		bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext("sub");
		EXPECT_EQ(true, bRetVal);
	}
	else
	{
		cout<<endl<<"#############ERROR###################"<<endl;
		cout<<"Error in getenv(PubTopics).  Skippping this test case.";
		cout<<endl<<"#############ERROR###################"<<endl;
	}

}

TEST_F(EISMsgbusHandler_ut, prepareCommonContext_pub_Twice)
{

	bool bRetVal = false;

	CEISMsgbusHandler::Instance().cleanup();
	cout<<endl<<"[UT Debut]>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>cleanup()"<<endl;

	const char* env_pubTopics = std::getenv("PubTopics");

	cout<<endl<<"[UT Debut]>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>getenv()"<<endl;

	if (env_pubTopics != NULL)
	{
		bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext("sub");

		cout<<endl<<"[UT Debut]>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>prepareCommonContext()"<<endl;

		EXPECT_EQ(true, bRetVal);
	}
	else
	{
		cout<<endl<<"#############ERROR###################"<<endl;
		cout<<"Error in getenv(PubTopics).  Skippping this test case.";
		cout<<endl<<"#############ERROR###################"<<endl;
	}

}

TEST_F(EISMsgbusHandler_ut, prepareCommonContext_sub)
{

	bool bRetVal = true;

	bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext("sub");
	EXPECT_EQ(true, bRetVal);
}
#endif

TEST_F(EISMsgbusHandler_ut, getCTX_ValidTopic) {

	stZmqContext msgbusContext;
	string topic = "MQTT_Export_WrReq"; //topic exists
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
	string topic = "RTU_RdResp"; //topic exists
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

TEST_F(EISMsgbusHandler_ut, removeSubCTX_)
{

	bool retVal = false;

	stZmqSubContext subContext;
	string topic = "InvalidTopic"; //topic doesn't exist


	stZmqContext msgbusContext;
	CEISMsgbusHandler::Instance().removeSubCTX(topic, msgbusContext);

	retVal = CEISMsgbusHandler::Instance().getSubCTX(topic, subContext);
	EXPECT_EQ(false, retVal);
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


TEST_F(EISMsgbusHandler_ut, RemoveCtxt_CtxtNULL) {

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

TEST_F(EISMsgbusHandler_ut, prepareContext_NULLArg_msgbus_ctx)
{
	config_t config;

	bool Res = CEISMsgbusHandler::Instance().prepareContext(false,
											NULL,
											"TestStr",
											&config);

	EXPECT_EQ(false, Res);
}

TEST_F(EISMsgbusHandler_ut, prepareContext_TopicEmpty)
{
	void* msgbus_ctx;
	string topicType = "sub";

	char** ppcTopics = CfgManager::Instance().getEnvClient()->get_topics_from_env(topicType.c_str());

	config_t* config = CfgManager::Instance().getEnvClient()->get_messagebus_config(
								CfgManager::Instance().getConfigClient(),
								ppcTopics , 1, topicType.c_str());


	msgbus_ctx = msgbus_initialize(config);

	bool Res = CEISMsgbusHandler::Instance().prepareContext(false,
											msgbus_ctx,
											"",
											config);

	EXPECT_EQ(false, Res);
}

TEST_F(EISMsgbusHandler_ut, prepareContext_NULLArg_config)
{
	void* msgbus_ctx;
	string topicType = "sub";

	char** ppcTopics = CfgManager::Instance().getEnvClient()->get_topics_from_env(topicType.c_str());

	config_t* config = CfgManager::Instance().getEnvClient()->get_messagebus_config(
								CfgManager::Instance().getConfigClient(),
								ppcTopics , 1, topicType.c_str());


	msgbus_ctx = msgbus_initialize(config);

	bool Res = CEISMsgbusHandler::Instance().prepareContext(false,
											msgbus_ctx,
											"TestStr",
											NULL);

	EXPECT_EQ(false, Res);
}

TEST_F(EISMsgbusHandler_ut, prepareContext_SubFails)
{
	void* msgbus_ctx;
	string topicType = "sub";

	char** ppcTopics = CfgManager::Instance().getEnvClient()->get_topics_from_env(topicType.c_str());

	config_t* config = CfgManager::Instance().getEnvClient()->get_messagebus_config(
								CfgManager::Instance().getConfigClient(),
								ppcTopics , 1, topicType.c_str());


	msgbus_ctx = msgbus_initialize(config);

	bool Res = CEISMsgbusHandler::Instance().prepareContext(false,
											msgbus_ctx,
											"TestStr",
											config);

	EXPECT_EQ(false, Res);
}

TEST_F(EISMsgbusHandler_ut, prepareContext_PubFails)
{
	void* msgbus_ctx;
	string topicType = "pub";

	char** ppcTopics = CfgManager::Instance().getEnvClient()->get_topics_from_env(topicType.c_str());

	config_t* config = CfgManager::Instance().getEnvClient()->get_messagebus_config(
								CfgManager::Instance().getConfigClient(),
								ppcTopics , 1, topicType.c_str());


	msgbus_ctx = msgbus_initialize(config);


	bool Res = CEISMsgbusHandler::Instance().prepareContext(true,
											msgbus_ctx,
											"TestStr",
											config);

	EXPECT_EQ(false, Res);
}
