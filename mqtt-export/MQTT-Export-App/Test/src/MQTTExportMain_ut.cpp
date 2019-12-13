/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include <gtest/internal/gtest-internal.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include "../include/MQTTExportMain_ut.hpp"

using namespace std;

extern std::string parse_msg(const char *json);
extern bool initEISContext();
extern void postMsgstoMQTT();
extern void signalHandler(int signal);

void MQTTExmportMain_ut::SetUp() {
	// Setup code
}

void MQTTExmportMain_ut::TearDown() {
	// TearDown code
}

//test 01
TEST_F(MQTTExmportMain_ut, 0_parse_msg) {
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

//test 01
TEST_F(MQTTExmportMain_ut, 1_parse_msg) {
	std::string topicName;
	try
	{
		const char *json = "{\"topic1\": \"PL0_iou_write\", \"command\": \" DValve\", \"value\": \"0x00\", \"app_seq\":\"1234\"}";

		topicName = parse_msg(json);
		std::cout << __func__ << " topic name parsed : " << topicName << endl;
		EXPECT_EQ("", topicName);

	}catch(exception &ex) {
		EXPECT_EQ("", topicName);
	}
}

//test 01
TEST_F(MQTTExmportMain_ut, 2_initEISContext) {
	bool retVal = false;
	try
	{
		retVal = initEISContext();

	}catch(exception &ex) {
		retVal = false;
	}

	EXPECT_EQ(true, retVal);
}

//test 01
TEST_F(MQTTExmportMain_ut, 3_postMsgstoMQTT) {

	try
	{
		signalHandler(SIGUSR1);

		EXPECT_EQ(true, true);

	}catch(exception &ex) {
		EXPECT_EQ(false, true);
	}
}
