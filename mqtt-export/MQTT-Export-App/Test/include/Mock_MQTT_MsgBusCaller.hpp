/*
 * Mock_MQTT_MsgBusCaller.hpp
 *
 *  Created on: 03-Feb-2020
 *      Author: user
 */

#ifndef MOCK_MQTT_MSGBUSCALLER_HPP_
#define MOCK_MQTT_MSGBUSCALLER_HPP_

#include "MQTT_MsgBusCaller.hpp"
#include "gmock/gmock.h"  // Brings in gMock.

class Mock_MQTT_MsgBusCaller: public MQTT_MsgBusCaller
{

public:
	MOCK_METHOD1(
			Caller_MsgbusInitialize,void*(config_t* config));

	MOCK_METHOD3(
			Caller_MsgbusPublisherNew,
			msgbus_ret_t
			(void* ctx, const char* topic, publisher_ctx_t** pub_ctx));
};



#endif /* MOCK_MQTT_MSGBUSCALLER_HPP_ */
