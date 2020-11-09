/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

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
#include "eis/msgbus/msgbus.h"
#include "eis/utils/json_config.h"
#include <semaphore.h>
#include "ModbusOnDemandHandler.hpp"

extern string onDemandHandler::getMsgElement(msg_envelope_t *a_Msg,
		string a_sKey);

class ModbusOnDemandHandler_ut : public ::testing::Test {
protected:
	virtual void SetUp();
	virtual void TearDown();

public:
	const string topic = "Modbus-TCP/PL0_flowmeter1_write,MQTT-EXPORT/PL0_flowmeter2_write";
	//const string topic = "Modbus-TCP-Master_ReadRequest";

	//string msg = "{ 	\"value\": \"0xFF00\", 	\"command\": \"Pointname\", 	\"app_seq\": \"1234\" }";

	//	string msg =
	//			"{ 	\"value\": \"0xFF00\", 	\"command\": \"AValve\", 	\"app_seq\": \"1234\", \"wellhead\": \"PL0\",  \"version\": \"0.0.0.1\", \"sourcetopic\":\"Response\", \"timestamp\": \"2020-02-12 06:14:15\", \"usec\": \"1581488055204186\" }";

	string msg =
			"{ 	\"value\": \"0xFF00\", 	\"command\": \"Flow\", 	\"app_seq\": \"1234\", \"tsMsgPublishOnEIS\":\"2020-03-31 12:34:56\", \"tsMsgRcvdFromMQTT\":\"2020-03-13 12:34:56\",  \"wellhead\": \"PL0\",  \"version\": \"0.0.0.1\", \"sourcetopic\":\"/flowmeter/PL0/Flow/read\", \"timestamp\": \"2020-02-12 06:14:15\", \"usec\": \"1581488055204186\" }";

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
	//sem_t semaphoreWriteReq;

	stOnDemandRequest reqData;
	cJSON *root = NULL;

	/*MsgbusManager msgbusMgr;
	EnvConf_Caller CallerObj;*/


};



#endif /* TEST_INCLUDE_MODBUSWRITEHANDLER_UT_HPP_ */
