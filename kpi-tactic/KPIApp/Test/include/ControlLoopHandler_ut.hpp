/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/
#ifndef TEST_INCLUDE_CONTROLLOOPHANDLER_UT_HPP_
#define TEST_INCLUDE_CONTROLLOOPHANDLER_UT_HPP_


#include "ControlLoopHandler.hpp"
#include "gtest/gtest.h"
#include "KPIAppConfigMgr.hpp"
#include "CommonDataShare.hpp"
#include "ConfigManager.hpp"
#include "ZmqHandler.hpp"

extern std::atomic<bool> g_stopThread;

class ControlLoopHandler_ut : public ::testing::Test{
protected:
	virtual void SetUp();
	virtual void TearDown();
public:
	uint32_t uiId;
		std::string sId;
		std::string sPolledTopic;
		std::string sWritePointFullPath;
		std::string sWriteDevName;
		std::string sWriteWellheadName;
		std::string sWritePointName;
		uint32_t uiDelayMs;
		std::string sVal;

		CControlLoopOp CControlLoopOp_obj{uiId,
			                     	sPolledTopic,
									sWritePointFullPath,
									sWriteDevName,
									sWriteWellheadName,
									sWritePointName,
									uiDelayMs,
									sVal};
		std::string strMsg = "{ 	\"value\": \"0xFF00\", 	\"command\": \"Pointname\", 	\"app_seq\": \"1234\" }";
		CMessageObject recvdMsg;
		std::map<std::string, std::vector<CControlLoopOp>> m_oControlLoopMap;


};



#endif /* TEST_INCLUDE_CONTROLLOOPHANDLER_UT_HPP_ */
