/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/


#include "../Inc/InternalMQTTSubscriber_ut.hpp"

#include <iostream>
using namespace std;


void InternalMQTTSubscriber_ut::SetUp()
{
	// Setup code
}

void InternalMQTTSubscriber_ut::TearDown()
{
	// TearDown code
}

#if 0
//Nothing to check in this function , this function is here for code coverage point of view..
TEST_F(InternalMQTTSubscriber_ut, pushMsgInQ_Success)
{
	mqtt::const_message_ptr msg;

	CIntMqttHandler::instance().cleanup();
	EXPECT_EQ( true, CIntMqttHandler::instance().pushMsgInQ(msg) );
}

TEST_F(InternalMQTTSubscriber_ut, prepareCJSONMsg_test)
{
	bool result = CIntMqttHandler::instance().prepareCJSONMsg(stRefActionVec);
	EXPECT_EQ(true, result);
}
#endif
