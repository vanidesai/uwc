/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#ifndef MQTTPUBSUBCLIENT_UT_HPP_
#define MQTTPUBSUBCLIENT_UT_HPP_

#include <gtest/gtest.h>
#include "MQTTPubSubClient.hpp"
#include "Logger.hpp"
#include "CommonDataShare.hpp"
#include "EnvironmentVarHandler.hpp"
#define SUBSCRIBER_ID "_KPI_SUBSCRIBER"

class MQTTPubSubClient_ut : public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();

public:
	std::string strPlBusUrl = "";
	std::string ClientID = "";
		int iQOS = 1;
		CMQTTBaseHandler  CMQTTBaseHandler_obj{strPlBusUrl, EnvironmentInfo::getInstance().getDataFromEnvMap("AppName")+SUBSCRIBER_ID,
			iQOS, (false == CcommonEnvManager::Instance().getDevMode()),
			"/run/secrets/ca_broker", "/run/secrets/client_cert",
			"/run/secrets/client_key", "MQTTSubListener"};


		CMQTTPubSubClient CMQTTPubSubClient_obj{strPlBusUrl, ClientID,
				iQOS,
				(false == CcommonEnvManager::Instance().getDevMode()), "/run/secrets/ca_broker",
				"/run/secrets/client_cert", "/run/secrets/client_key",
				"MQTTSubListener"};
		CMQTTPubSubClient CMQTTPubSubClient_obj2{strPlBusUrl, ClientID,
						iQOS,
						true, "/run/secrets/ca_broker",
						"/run/secrets/client_cert", "/run/secrets/client_key",
						"MQTTSubListener"};



};



#endif /* MQTTPUBSUBCLIENT_UT_HPP_ */
