/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../include/CMQTTHandler_ut.hpp"

#include <gtest/internal/gtest-internal.h>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

extern void postMsgsToEIS();

void CMQTTHandler_ut::SetUp() {
	// Setup code
}

void CMQTTHandler_ut::TearDown() {
	// TearDown code
}


//test 01
TEST_F(CMQTTHandler_ut, instance_ValMqtURL) {
	try
	{
		CMQTTHandler::instance();
		EXPECT_EQ(true, true);
	}catch(exception &ex) {
		EXPECT_EQ(true, false);
	}
}


#ifdef QUEUE_FAILED_PUBLISH_MESSAGES
TEST_F(CMQTTHandler_ut, pushMsgInQ)
{

	CMQTTHandler::instance().postPendingMsgs();
}
#endif


TEST_F(CMQTTHandler_ut, publish_ValMsg)
{
	string PubTpoic = "PubTopics";

	struct timespec tsMsgRcvd;
	timespec_get(&tsMsgRcvd, TIME_UTC);

	CMQTTPublishHandler CMQTTPublishHandler_obj("str1", "str2");

	bool retVal = CMQTTPublishHandler_obj.publish(
			"{\"topic\": \"PL0_iou_write\", \"command\": \" DValve\", \"value\": \"0x00\", \"app_seq\":\"1234\"}",
			PubTpoic.c_str(),
			0,
			tsMsgRcvd);

	EXPECT_EQ(false, retVal);
}


TEST_F(CMQTTHandler_ut, publish_InValMsg)
{
	string PubTpoic = "PubTopics";

	struct timespec tsMsgRcvd;
	timespec_get(&tsMsgRcvd, TIME_UTC);

	CMQTTPublishHandler CMQTTPublishHandler_obj("str1", "str2");

	bool retVal = CMQTTPublishHandler_obj.publish(
			"InvalidMsg",
			PubTpoic.c_str(),
			0,
			tsMsgRcvd);

	EXPECT_EQ(false, retVal);
}


TEST_F(CMQTTHandler_ut, publish_MsgEmpty)
{
	string PubTpoic = "";

	struct timespec tsMsgRcvd;
	timespec_get(&tsMsgRcvd, TIME_UTC);

	CMQTTPublishHandler CMQTTPublishHandler_obj("str1", "str2");

	bool retVal = CMQTTPublishHandler_obj.publish(
			"",
			PubTpoic.c_str(),
			0,
			tsMsgRcvd);

	EXPECT_EQ(false, retVal);
}

TEST_F(CMQTTHandler_ut, publish_TopicEmp)
{
	string PubTpoic = "";

	struct timespec tsMsgRcvd;
	timespec_get(&tsMsgRcvd, TIME_UTC);

	CMQTTPublishHandler CMQTTPublishHandler_obj("str1", "str2");

	bool retVal = CMQTTPublishHandler_obj.publish(
			"{\"topic\": \"PL0_iou_write\"}",
			PubTpoic.c_str(),
			0,
			tsMsgRcvd);

	EXPECT_EQ(false, retVal);
}

// How to confirm??
// Working on it..
TEST_F(CMQTTHandler_ut, cleanup_Success)
{
	CMQTTHandler::instance().cleanup();

}

TEST_F(CMQTTHandler_ut, cleanup_1_Success)
{
	CMQTTPublishHandler CMQTTPublishHandler_obj("str1", "str2");

	CMQTTPublishHandler_obj.cleanup();
}
