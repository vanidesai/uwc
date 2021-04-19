/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef TEST_INCLUDE_MQTTHANDLER_UT_H_
#define TEST_INCLUDE_MQTTHANDLER_UT_H_

#include <gtest/gtest.h>

//#include "MQTTHandler.hpp"
#include "Common.hpp"
#include "ConfigManager.hpp"


class MqttHandler_ut : public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();

public:

	/*mqtt::const_message_ptr msg =  mqtt::make_message(
			"{\"topic\": \"UT_writeRequest\"}",
			"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"0	\"}"
	);

	// Non-string value
	mqtt::const_message_ptr msg_PushInQ =  mqtt::make_message(
				"{\"topic\": \"MQTT/SCADA/RTU/UT_writeRequest\"}",
				"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"0	\"}"
		);*/

//	QMgr::stMqttMsg stNewMsg;

};

#endif //TEST_INCLUDE_MQTTHANDLER_UT_H_
