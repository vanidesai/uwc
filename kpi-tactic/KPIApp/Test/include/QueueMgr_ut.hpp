/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef TEST_INCLUDE_QUEUEMGR_UT_HPP_
#define TEST_INCLUDE_QUEUEMGR_UT_HPP_

#include "gtest/gtest.h"
#include "Common.hpp"
#include "QueueMgr.hpp"
#include "QueueHandler.hpp"
#include "EISPlBusHandler.hpp"
#include "ControlLoopHandler.hpp"

class QueueMgr_ut : public ::testing::Test{
protected:
	virtual void SetUp();
	virtual void TearDown();
public:
	bool check = true;
	string subTopic = "TCP/TCP_WrResp";
	string Topic;
	string Msg;
	string strMsg = "{ 	\"value\": \"0xFF00\", 	\"command\": \"Pointname\", 	\"app_seq\": \"1234\" }";

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





};




#endif /* TEST_INCLUDE_QUEUEMGR_UT_HPP_ */
