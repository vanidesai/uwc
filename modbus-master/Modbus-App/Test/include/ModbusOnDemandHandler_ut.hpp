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

#ifndef TEST_INCLUDE_MODBUSWRITEHANDLER_UT_HPP_
#define TEST_INCLUDE_MODBUSWRITEHANDLER_UT_HPP_

#include <stdbool.h>
#include "gtest/gtest.h"
#include "NetworkInfo.hpp"
#include "ZmqHandler.hpp"
#include "PeriodicReadFeature.hpp"
#include "PublishJson.hpp"
#include "yaml-cpp/eventhandler.h"
#include "yaml-cpp/yaml.h"
#include "ConfigManager.hpp"
#include "YamlUtil.hpp"

#include <sys/msg.h>
#include <fstream>
#include <cstdlib>
#include <stdio.h>
#include "eii/msgbus/msgbus.h"
#include "eii/utils/json_config.h"
#include <semaphore.h>
#include "ModbusOnDemandHandler.hpp"

class ModbusOnDemandHandler_ut : public ::testing::Test {
protected:
	virtual void SetUp();
	virtual void TearDown();

public:
	const string topic = "Modbus-TCP/PL0_flowmeter1_write,MQTT-EXPORT/PL0_flowmeter2_write";

	string msg =
			"{ 	\"value\": \"0xFF00\", 	\"command\": \"Flow\", 	\"app_seq\": \"1234\", \"tsMsgPublishOnEII\":\"2020-03-31 12:34:56\", \"tsMsgRcvdFromMQTT\":\"2020-03-13 12:34:56\",  \"wellhead\": \"PL0\",  \"version\": \"0.0.0.1\", \"sourcetopic\":\"/flowmeter/PL0/Flow/read\", \"timestamp\": \"2020-02-12 06:14:15\", \"usec\": \"1581488055204186\" }";

	string str = "data";
	uint8_t* target = NULL;
	string tCommand;
	string tValue = "0xFF00";
	unsigned char byte1;
	int i = 0;
	//stRequest writeReq;
	unsigned char  m_u8FunCode = 0;
	unsigned char u8FunCode = READ_COIL_STATUS;
	std::string strTopic = "";

	MbusAPI_t stMbusApiPram;

	eMbusAppErrorCode eFunRetType = APP_SUCCESS;

	string strCommand, strValue, strWellhead, strVersion, strSourceTopic;
	eMbusAppErrorCode eFunRetType2;

	stOnDemandRequest reqData;
	cJSON *root = NULL;

};



#endif /* TEST_INCLUDE_MODBUSWRITEHANDLER_UT_HPP_ */
