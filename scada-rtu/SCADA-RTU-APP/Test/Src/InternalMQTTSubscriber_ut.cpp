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

TEST_F(InternalMQTTSubscriber_ut, prepareCJSONMsg_EmptyVector)
{
	bool result = CIntMqttHandler::instance().prepareCJSONMsg(stRefActionVec);
	EXPECT_EQ(true, result);
}

