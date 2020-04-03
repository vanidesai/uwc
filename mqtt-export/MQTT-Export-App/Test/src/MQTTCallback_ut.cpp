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


TEST_F(MQTTCallback_ut, connection_lost_causeNotEmpty_CSyncCallback)
{
	CSyncCallback CSyncCallback_obj;

	CSyncCallback_obj.connection_lost(cause);

}

TEST_F(MQTTCallback_ut, connection_lost_causeEmpty_CSyncCallback)
{
	CSyncCallback CSyncCallback_obj;

	CSyncCallback_obj.connection_lost("");

}

TEST_F(MQTTCallback_ut, message_arrived_ValMsg)
{
	CMQTTCallback CMQTTCallback_obj;

	mqtt::const_message_ptr msg =  mqtt::make_message(
				"{\"topic\": \"UT_writeRequest\"}",
				"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"0	\"}"
				);

	CMQTTCallback_obj.message_arrived(msg);

	//EXPECT: Code should not hang

}
