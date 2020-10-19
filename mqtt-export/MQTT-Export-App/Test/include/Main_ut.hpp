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

#include "Common.hpp"
#include "MQTTSubscribeHandler.hpp"
#include "../include/MQTTPublishHandler.hpp"
#include "ConfigManager.hpp"
#include "Logger.hpp"
#include "ZmqHandler.hpp"

#include "QueueMgr.hpp"


std::string parse_msg(const char *json);
extern bool initEISContext();
extern void postMsgstoMQTT();
extern void signalHandler(int signal);
extern bool addSrTopic(string &json, string& topic);
extern void postMsgsToEIS(QMgr::CQueueMgr& qMgr);
extern bool processMsg(msg_envelope_t *msg, CMQTTPublishHandler &mqttPublisher);
extern bool processMsgToSendOnEIS(mqtt::const_message_ptr &recvdMsg, bool &isRead, bool &isRealtime);

extern vector<std::thread> g_vThreads;

extern void Temp_Function(string& topic);


/*extern void listenOnEIS(string topic, stZmqContext context,
		stZmqSubContext subContext);

extern bool publishEISMsg(string eisMsg, stZmqContext &context,
		stZmqPubContext &pubContext);*/

extern void set_thread_priority_for_eis(bool& isRealtime, bool& isRead);

extern std::atomic<bool> g_shouldStop;


vector<std::thread> g_vThreads_UT;


class Main_ut : public ::testing::Test{

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
