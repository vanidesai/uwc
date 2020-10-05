/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../Inc/SCADAHandler_ut.hpp"

void SCADAHandler_ut::SetUp()
{
	// Setup code
}

void SCADAHandler_ut::TearDown()
{
	// TearDown code
}

TEST_F(SCADAHandler_ut, prepareSparkPlugMsg_001)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
				"Birth/SCADA-RTU/B",
				"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
				stRefActionVec);

	/*Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"Data/A/B",
			"{\"metrics\": [{\"name\":\"UT_UniqueName\"}],\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);*/

	CSCADAHandler::instance().prepareSparkPlugMsg(stRefActionVec);

	//EXPECT_EQ(true, Bool_Res);
}
