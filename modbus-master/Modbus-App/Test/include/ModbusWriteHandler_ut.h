/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#ifndef TEST_INCLUDE_MODBUSWRITEHANDLER_UT_H_
#define TEST_INCLUDE_MODBUSWRITEHANDLER_UT_H_

#include <stdbool.h>
#include "gtest/gtest.h"
#include "NetworkInfo.hpp"
#include "ZmqHandler.hpp"
#include "PeriodicReadFeature.hpp"
#include "PublishJson.hpp"
#include "yaml-cpp/eventhandler.h"
#include "yaml-cpp/yaml.h"
#include "ConfigManager.hpp"
#include "utils/YamlUtil.hpp"

#include "ModbusWriteHandler.hpp"


class ModbusWriteHandler_ut : public ::testing::Test {
protected:
	virtual void SetUp();
	virtual void TearDown();

public:
	const string topic = "MQTT-EXPORT/PL0_flowmeter1_write,MQTT-EXPORT/PL0_flowmeter2_write";
	string msg = "{ 	\"value\": \"0xFF00\", 	\"command\": \"Pointname\", 	\"app_seq\": \"1234\" }";
	string str = "data";
};



#endif /* TEST_INCLUDE_MODBUSWRITEHANDLER_UT_H_ */
