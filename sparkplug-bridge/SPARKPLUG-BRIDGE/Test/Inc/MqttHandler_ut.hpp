/********************************************************************************
* Copyright (c) 2021 Intel Corporation.

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*********************************************************************************/

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
