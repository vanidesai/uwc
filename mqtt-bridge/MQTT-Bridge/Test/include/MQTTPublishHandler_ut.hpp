/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef MQTTPUBLISHHANDLER_UT_H_
#define MQTTPUBLISHHANDLER_UT_H_

#include <gtest/gtest.h>
#include "MQTTPublishHandler.hpp"

#include "eis/utils/config.h"
#include "ZmqHandler.hpp"

class MQTTPublishHandler_ut : public ::testing::Test{

protected:
	virtual void SetUp();
	virtual void TearDown();

public:

	std::string EmptyTopic = "";
	std::string ValidTopic = "MQTT_Export_RdReq";
	std::string EmptyMsg = "";
	std::string ValidMsg = "{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}";

};


#endif /* MQTTPUBLISHHANDLER_UT_H_ */
