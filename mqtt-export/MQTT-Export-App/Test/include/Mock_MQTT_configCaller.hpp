/*
 * Mock_MQTT_configCaller.hpp
 *
 *  Created on: 03-Feb-2020
 *      Author: user
 */

#ifndef MOCK_MQTT_CONFIGCALLER_HPP_
#define MOCK_MQTT_CONFIGCALLER_HPP_

#include "MQTT_configCaller.hpp"

class Mock_MQTT_configCaller: public MQTT_configCaller
{
public:
	MOCK_METHOD1(
			Caller_config_destroy,void(config_t* config));
};



#endif /* MOCK_MQTT_CONFIGCALLER_HPP_ */
