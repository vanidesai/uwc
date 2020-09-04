/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../Inc/Publisher_ut.hpp"

void Publisher_ut::SetUp()
{
	// Setup code
}

void Publisher_ut::TearDown()
{
	// TearDown code
}

// publishMqttExportMsg: Successfully publish mqtt-export message
TEST_F(Publisher_ut, publishMqttExportMsg_Sucess)
{
	EXPECT_EQ( true, CPublisher::instance().publishIntMqttMsg(msg, topic) );

}
