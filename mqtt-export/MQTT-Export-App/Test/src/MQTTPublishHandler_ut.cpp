/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../include/MQTTPublishHandler_ut.hpp"

#include <gtest/internal/gtest-internal.h>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

extern void postMsgsToEIS();

void MQTTPublishHandler_ut::SetUp() {
	// Setup code
}

void MQTTPublishHandler_ut::TearDown() {
	// TearDown code
}


#ifdef QUEUE_FAILED_PUBLISH_MESSAGES
TEST_F(MQTTPublishHandler_ut, pushMsgInQ)
{

	CMQTTHandler::instance().postPendingMsgs();
}
#endif

#if 0 //SPRINT13CHANGES
TEST_F(MQTTPublishHandler_ut, publish_NotConnected)
{
	string PubTpoic = "PubTopics";


	struct timespec tsMsgRcvd;
	timespec_get(&tsMsgRcvd, TIME_UTC);
	string Message = "Message_ut";

	mqtt::const_message_ptr msg;


	CMQTTPublishHandler CMQTTPublishHandler_obj("PubID", "CliId", 0);

	bool retVal = CMQTTPublishHandler_obj.publish(
			Message,
			PubTpoic,
			tsMsgRcvd);

	EXPECT_EQ(false, retVal);
}
#endif


#if 0 //SPRINT13CHANGES
TEST_F(MQTTPublishHandler_ut, publish_MsgEmpty)
{
	string PubTpoic = "";


	struct timespec tsMsgRcvd;
	timespec_get(&tsMsgRcvd, TIME_UTC);
	string Message = "";

	mqtt::const_message_ptr msg;


	CMQTTPublishHandler CMQTTPublishHandler_obj("PubID", "CliId", 0);


	bool retVal = CMQTTPublishHandler_obj.publish(
			Message,
			PubTpoic,
			tsMsgRcvd);

	EXPECT_EQ(false, retVal);
}
#endif

#if 0 //SPRINT13CHANGES
TEST_F(MQTTPublishHandler_ut, publish_Called_moreThan1)
{
	string PubTpoic = "";


	struct timespec tsMsgRcvd;
	timespec_get(&tsMsgRcvd, TIME_UTC);
	string Message = "";

	mqtt::const_message_ptr msg;


	CMQTTPublishHandler CMQTTPublishHandler_obj("PubID", "CliId", 0);


	bool retVal = CMQTTPublishHandler_obj.publish(
			Message,
			PubTpoic,
			tsMsgRcvd);

	retVal = CMQTTPublishHandler_obj.publish(
				Message,
				PubTpoic,
				tsMsgRcvd);

	EXPECT_EQ(false, retVal);
}
#endif

#if 0 //SPRINT13CHANGES
TEST_F(MQTTPublishHandler_ut, publish_TopicEmp)
{
	string PubTpoic = "";


	struct timespec tsMsgRcvd;
	timespec_get(&tsMsgRcvd, TIME_UTC);
	string Message = "Message_ut";

	mqtt::const_message_ptr msg;


	CMQTTPublishHandler CMQTTPublishHandler_obj("PubID", "CliId", 0);


	bool retVal = CMQTTPublishHandler_obj.publish(
			Message,
			PubTpoic,
			tsMsgRcvd);

	EXPECT_EQ(false, retVal);
}
#endif

// How to confirm??
// Working on it..
TEST_F(MQTTPublishHandler_ut, cleanup_1_Success)
{
	CMQTTPublishHandler CMQTTPublishHandler_obj("PubID", "CliId", 0);

	CMQTTPublishHandler_obj.cleanup();
}

