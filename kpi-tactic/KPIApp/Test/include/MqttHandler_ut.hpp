/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef TEST_INCLUDE_MQTTHANDLER_UT_HPP_
#define TEST_INCLUDE_MQTTHANDLER_UT_HPP_



#include "gtest/gtest.h"
#include "Common.hpp"
#include "MqttHandler.hpp"


class MqttHandler_ut : public ::testing::Test{
protected:
	virtual void SetUp();
	virtual void TearDown();
public:
	CMessageObject recvdMsg{};
	std::string topic = ",KPIAPP_WrReq";
	std::string Topic;
	std::string strMsg = "{ 	\"value\": \"0xFF00\", 	\"command\": \"Pointname\", 	\"app_seq\": \"1234\" }";
	mqtt::const_message_ptr a_pMsg;
};


#endif /* TEST_INCLUDE_MQTTHANDLER_UT_HPP_ */
