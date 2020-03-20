/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef INCLUDE_MQTTEXPORTMAIN_UT_HPP_
#define INCLUDE_MQTTEXPORTMAIN_UT_HPP_

#include <gtest/gtest.h>

#include <iostream>
#include <thread>
#include <vector>
#include <iterator>
#include <assert.h>
#include <semaphore.h>
#include <csignal>

#include <gtest/internal/gtest-internal.h>
#include <cstdlib>
#include <iostream>
#include <string>

#include "EISMsgbusHandler.hpp"
#include "Common.hpp"
#include "MQTTSubscribeHandler.hpp"
#include "../include/MQTTPublishHandler.hpp"
#include "ConfigManager.hpp"
#include "Logger.hpp"


class MQTTExmportMain_ut : public ::testing::Test{

protected:
	virtual void SetUp();
	virtual void TearDown();

public:
	char* msg[2] = 	{
						"Message1",
						"/home/user/SVN/Intel_UWC/trunk/Technical/Sourcecode/mqtt-export/MQTT-Export-App/Test/src/JsonConfig.json"
						};

	string strMsg = "{ 	\"value\": \"0xFF00\", 	\"command\": \"Pointname\", 	\"app_seq\": \"1234\" }";


};

#endif /* INCLUDE_MQTTEXPORTMAIN_UT_HPP_ */
