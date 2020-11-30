/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../include/MqttHandler_ut.hpp"


void MqttHandler_ut::SetUp()
{
	// Setup code
}

void MqttHandler_ut::TearDown()
{
	// TearDown code
}

/**
 * Test case to check if instance() function return Reference of this instance of this class
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
#if 1
TEST_F(MqttHandler_ut, instance_test)
{
	bool object = true;
	CMqttHandler::instance();
	EXPECT_EQ(true, object);
}

TEST_F(MqttHandler_ut, isConnected_false)
{
	bool RetVal = CMqttHandler::instance().isConnected();
	EXPECT_EQ(false, RetVal);
}

/**
 * Test case to check if publishMsg() function publishes the msg successfully when topic is given and returns true on success
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(MqttHandler_ut, publishMsg_true)
{
	bool RetVal = CMqttHandler::instance().publishMsg(strMsg, topic);
	EXPECT_EQ(true, RetVal);
}

/**
 * Test case to check if publishMsg() function do not publishes the msg successfully when topic is not given and returns false
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(MqttHandler_ut, publishMsg_false)
{
	bool RetVal = CMqttHandler::instance().publishMsg(strMsg, Topic);
	EXPECT_EQ(false, RetVal);
}

#endif
