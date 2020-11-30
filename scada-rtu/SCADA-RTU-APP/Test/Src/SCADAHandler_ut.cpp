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

/************************Helper functions************************/
/**
 * Helper function to call prepareSparkPlugMsg from thread
 * @param :[in] Vector: Reference action vector
 * @param :[out] boolean: Output result
 * @return None
 */
void TargetFunctionCaller( std::vector<stRefForSparkPlugAction> stRefActionVec, bool& bRes )
{
	bRes = CSCADAHandler::instance().prepareSparkPlugMsg(stRefActionVec);
}

/**
 * Helper function to call signalIntMQTTConnLostThread()
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
void PostSem_semIntMQTTConnLost()
{
	std::this_thread::sleep_for(std::chrono::seconds(1));
	CSCADAHandler::instance().signalIntMQTTConnLostThread();
}

/***************************************************************/

/**
 * Test case to check prepareSparkPlugMsg() with Death message
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(SCADAHandler_ut, prepareSparkPlugMsg_DeathMsg)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
					"Data/A/B",
					"{\"metrics\": [{\"name\":\"UtData01\", \"dataType\":\"boolean\", \"value\": true}],\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
					stRefActionVec);

	bool Bool_Res_Local = true;
	std::thread TestHelper( PostSem_semIntMQTTConnLost );
	std::thread TestTarget( TargetFunctionCaller, stRefActionVec, std::ref(Bool_Res_Local) );

	TestTarget.join();
	TestHelper.join();

	EXPECT_EQ( true, Bool_Res_Local );
}

/**
 * Test case to check prepareSparkPlugMsg() with Data message
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(SCADAHandler_ut, prepareSparkPlugMsg_DataMsg)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
					"Data/A/B",
					"{\"metrics\": [{\"name\":\"UtData02\", \"dataType\":\"boolean\", \"value\": true}],\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
					stRefActionVec);

	bool Bool_Res_Local = true;
	std::thread TestHelper( PostSem_semIntMQTTConnLost );
	std::thread TestTarget( TargetFunctionCaller, stRefActionVec, std::ref(Bool_Res_Local) );

	TestTarget.join();
	TestHelper.join();

	EXPECT_EQ( true, Bool_Res_Local );
}

