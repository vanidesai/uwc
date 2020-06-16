/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef TEST_INCLUDE_MAIN_UT_H_
#define TEST_INCLUDE_MAIN_UT_H_

#include "Common.hpp"
#include "ConfigManager.hpp"
#include <iterator>
#include <vector>
#include <future>

#include "MQTTHandler.hpp"
#include "SCADAHandler.hpp"
#include "Publisher.hpp"

#ifdef UNIT_TEST
#include <gtest/gtest.h>
#endif

struct stFuture
{
	std::future<bool> m_fut;
	std::future_status m_bStatus;
};

extern void updateDataPoints(QMgr::CQueueMgr& a_qMgr);
extern bool processMsgToSendOnMqtt(QMgr::stMqttMsg a_mqttMsgToPublish);
extern bool postMsgsToSCADA(QMgr::CQueueMgr& a_qMgr);
extern bool isTaskComplete(stFuture &a_futures);
extern void timerThread(uint32_t interval);

extern vector<std::thread> g_vThreads;
extern std::atomic<bool> g_shouldStop;

class Main_ut : public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();

public:
	bool Bool_Res = false;
	QMgr::stMqttMsg MqttMsg;

	stFuture stTask;

};

#endif /* TEST_INCLUDE_MAIN_UT_H_ */
