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
#include "MQTTPublishHandler.hpp"
#include "ConfigManager.hpp"
#include "Logger.hpp"
#include "ZmqHandler.hpp"
#include "EnvironmentVarHandler.hpp"

#include "QueueMgr.hpp"


std::string parse_msg(const char *json);
extern bool initEIIContext();
extern void postMsgstoMQTT();
extern void signalHandler(int signal);
extern bool addSrTopic(std::string &json, std::string& topic);
extern void postMsgsToEII(QMgr::CQueueMgr& qMgr);
extern bool processMsg(msg_envelope_t *msg, CMQTTPublishHandler &mqttPublisher);
extern void processMsgToSendOnEII(CMessageObject &recvdMsg, const std::string a_sEiiTopic);
extern void getOperation(std::string topic, globalConfig::COperation& operation);

extern std::vector<std::thread> g_vThreads;;

extern void Temp_Function(std::string& topic);

extern void set_thread_priority_for_eii(bool& isRealtime, bool& isRead);

extern std::atomic<bool> g_shouldStop;


std::vector<std::thread> g_vThreads_UT;


class Main_ut : public ::testing::Test{

protected:
	virtual void SetUp();
	virtual void TearDown();

public:


};

#endif /* INCLUDE_MQTTEXPORTMAIN_UT_HPP_ */
