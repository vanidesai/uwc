/*
 * Mock_MQTT_EnvConfCaller.hpp
 *
 *  Created on: 30-Jan-2020
 *      Author: user
 */

#ifndef MOCK_MQTT_ENVCONFCALLER_HPP_
#define MOCK_MQTT_ENVCONFCALLER_HPP_

#include "MQTT_EnvConfigCaller.hpp"
#include "gmock/gmock.h"  // Brings in gMock.
#include "eis/utils/config.h"



class Mock_MQTT_EnvConfCaller: public MQTT_EnvConfCaller
{

public:
	MOCK_METHOD2(
			EnvConf_Caller_GetMessagebusConfig,config_t*(
					std::string& topic,
					std::string& topic_type));

	MOCK_METHOD1(
			Caller_MsgbusInitialize,void*(config_t* config));
};



#endif /* MOCK_MQTT_ENVCONFCALLER_HPP_ */
