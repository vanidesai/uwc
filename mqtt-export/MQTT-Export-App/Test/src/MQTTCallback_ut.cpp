/*************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 *************************************************************************************/
#include "../include/MQTTCallback_ut.hpp"

void MQTTCallback_ut::SetUp() {
	// Setup code
}

void MQTTCallback_ut::TearDown() {
	// TearDown code
}


TEST_F(MQTTCallback_ut, connection_lost_causeNotEmpty)
{

	CMQTTCallback_obj.connection_lost(cause);

	/*
		Result of function connection_lost() cannot be tested.
		Reason:
		getMQTTConfigState() and getMQTTSubConfigState()
		(which returns ConfigState and subConfigState respectively),
		are private function and are not called from anywhere in app.
	 */
}

TEST_F(MQTTCallback_ut, message_arrived_SuccessfullAdditionInQ)
{

	mqtt::const_message_ptr msg = mqtt::make_message(TestMsg,
			"PL0_iou_write");
	mqtt::const_message_ptr recvdMsg;

	CMQTTCallback_obj.message_arrived(msg);

	/* Get message to check if msg was successfully pushed in queue */
	CMQTTHandler::instance().getSubMsgFromQ(recvdMsg);
	string rcvdTopic = recvdMsg->get_topic();

	EXPECT_EQ(TestMsg, rcvdTopic);
}

#if 0
TEST_F(MQTTCallback_ut, on_failure_Successful)
{

	/*
			Result of on_failure() cannot be tested.
			Reason:
			 It is non-public function and is not called from anywhere in app.
	 */

}
#endif
