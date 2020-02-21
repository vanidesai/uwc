/*************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 *************************************************************************************/

#ifndef MQTTCALLBACK_UT_HPP_
#define MQTTCALLBACK_UT_HPP_

#include "MQTTCallback.hpp"
#include "MQTTHandler.hpp"
#include <gtest/gtest.h>

class MQTTCallback_ut : public ::testing::Test{

protected:
	virtual void SetUp();
	virtual void TearDown();

public:
	CMQTTCallback CMQTTCallback_obj;
	string cause = "cause";
	string TestMsg = "Message_for_message_arrived()";
};



#endif /* MQTTCALLBACK_UT_HPP_ */
