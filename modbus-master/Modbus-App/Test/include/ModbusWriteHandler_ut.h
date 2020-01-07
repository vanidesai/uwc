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

#include <sys/msg.h>
#include <fstream>
#include <cstdlib>
#include <stdio.h>
#include "eis/msgbus/msgbus.h"
#include "eis/utils/json_config.h"
#include <semaphore.h>
#include "ModbusWriteHandler.hpp"


class ModbusWriteHandler_ut : public ::testing::Test {
protected:
	virtual void SetUp();
	virtual void TearDown();

public:
	const string topic = "MQTT-EXPORT/PL0_flowmeter1_write,MQTT-EXPORT/PL0_flowmeter2_write";
	string msg = "{ 	\"value\": \"0xFF00\", 	\"command\": \"Pointname\", 	\"app_seq\": \"1234\" }";

	string str = "data";
	uint8_t* target = NULL;
	string tCommand;
	string tValue = "0xFF00";
	RestMbusReqGeneric_t *stModbusRxPacket;
	unsigned char byte1;
	int i = 0;
	stWriteRequest writeReq;
	unsigned char  m_u8FunCode;
		MbusAPI_t stMbusApiPram = {};
	eMbusStackErrorCode eFunRetType = MBUS_STACK_NO_ERROR;
	eMbusStackErrorCode eFunRetType2;


};



#endif /* TEST_INCLUDE_MODBUSWRITEHANDLER_UT_H_ */
