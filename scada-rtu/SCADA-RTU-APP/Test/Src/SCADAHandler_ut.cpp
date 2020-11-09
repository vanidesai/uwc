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

void TargetFunctionCaller( std::vector<stRefForSparkPlugAction> stRefActionVec, bool& bRes )
{
	std::cout<<"<<<<<<<<<<<<<<<<<<Target calling start"<<std::endl;
	//CIntMqttHandler::instance().disconnect();
	bRes = CSCADAHandler::instance().prepareSparkPlugMsg(stRefActionVec);
	std::cout<<"<<<<<<<<<<<<<<<<<<Target called"<<std::endl;
}

void PostSem_semIntMQTTConnLost()
{
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout<<"<<<<<<<<<<<<<<<<<<Before Semaphore"<<std::endl;
	CSCADAHandler::instance().signalIntMQTTConnLostThread();
	std::cout<<"<<<<<<<<<<<<<<<<<<After Semaphore"<<std::endl;
}

/***************************************************************/


TEST_F(SCADAHandler_ut, prepareSparkPlugMsg_DeathMsg)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	//CIntMqttHandler::instance().disconnect();

	/*CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
				"Death/A/B",
				"{\"metrics\": [{\"name\":\"UtData01\", \"dataType\":\"Uint8\", \"value\": \"0x00\"}],\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
				stRefActionVec);*/

	CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
					"Data/A/B",
					"{\"metrics\": [{\"name\":\"UtData01\", \"dataType\":\"boolean\", \"value\": true}],\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
					stRefActionVec);

	bool Bool_Res_Local = true;
	std::thread TestTarget( TargetFunctionCaller, stRefActionVec, std::ref(Bool_Res_Local) );
	std::thread TestHelper( PostSem_semIntMQTTConnLost );

	TestTarget.join();
	TestHelper.join();

	EXPECT_EQ( true, Bool_Res_Local );
}

TEST_F(SCADAHandler_ut, prepareSparkPlugMsg_DataMsg)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	//CIntMqttHandler::instance().disconnect();

	/*CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
				"Death/A/B",
				"{\"metrics\": [{\"name\":\"UtData01\", \"dataType\":\"Uint8\", \"value\": \"0x00\"}],\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
				stRefActionVec);*/

	CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
					"Data/A/B",
					"{\"metrics\": [{\"name\":\"UtData02\", \"dataType\":\"boolean\", \"value\": true}],\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
					stRefActionVec);

	bool Bool_Res_Local = true;
	std::thread TestTarget( TargetFunctionCaller, stRefActionVec, std::ref(Bool_Res_Local) );
	std::thread TestHelper( PostSem_semIntMQTTConnLost );

	TestTarget.join();
	TestHelper.join();

	EXPECT_EQ( true, Bool_Res_Local );
}

