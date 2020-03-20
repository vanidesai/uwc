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

