/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../include/MQTTSubscribeHandler_ut.hpp"

#include <gtest/internal/gtest-internal.h>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

extern void postMsgsToEIS();

void MQTTSubscribeHandler_ut::SetUp() {
	// Setup code
}

void MQTTSubscribeHandler_ut::TearDown() {
	// TearDown code
}

TEST_F(MQTTSubscribeHandler_ut, instance_ValMqtURL)
{
	try
	{
		CMQTTHandler::instance();
		EXPECT_EQ(true, true);
	}catch(exception &ex) {
		EXPECT_EQ(true, false);
	}
}

// How to confirm??
// Working on it..
TEST_F(MQTTSubscribeHandler_ut, cleanup_Success)
{
	CMQTTHandler::instance().cleanup();

}
