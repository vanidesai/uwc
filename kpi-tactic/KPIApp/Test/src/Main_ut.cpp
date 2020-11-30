/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/


#include "../include/Main_ut.hpp"

extern void postMsgsToWriteOnMQTT(CQueueHandler& qMgr);
extern void analyzeControlLoopData(CQueueHandler& qMgr);
extern std::vector<std::thread> g_vThreads;
extern std::atomic<bool> g_stopThread;

void Main_ut::SetUp()
{
	// Setup code
}

void Main_ut::TearDown()
{
	// TearDown code
}

/**********************Helper Functions**************************/
/**
 * Helper function to set global variable g_stopThread as true
 * @param :[in] None
 * @param :[out] g_shouldStop
 * @return None
 */
void CallHelper_sTopThreadTrue( mqtt::const_message_ptr& msg, CQueueHandler& CQueueHandler_obj )
{
	CQueueHandler_obj.pushMsg(msg);
	g_stopThread = true;
}

void TargetCaller_postMsgsToWriteOnMQTT(CQueueHandler& CQueueHandler_obj)
{
	postMsgsToWriteOnMQTT(CQueueHandler_obj);
}

void TargetCaller_analyzeControlLoopData(CQueueHandler& CQueueHandler_obj)
{
	analyzeControlLoopData(CQueueHandler_obj);
}

void CallHelper_sTopThreadTrue_analyzeControlLoopData( mqtt::const_message_ptr& msg, CQueueHandler& CQueueHandler_obj )
{
	CQueueHandler_obj.pushMsg(msg);
}

/**
 * Test case to check if postMsgsToWriteOnMQTT() Thread function reads requests from queue filled up by MQTT and send data to EIS
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Main_ut, PostMsgsToWriteOnMQTT_ControlLoopPollPoint_False)
{

	CQueueHandler CQueueHandler_obj;
	mqtt::const_message_ptr msg = mqtt::make_message("Death/A", "Msg_UT", 0, false);

	std::thread TestTarget( TargetCaller_postMsgsToWriteOnMQTT, std::ref(CQueueHandler_obj) );
	std::thread TestHelperStpThreadTrue( CallHelper_sTopThreadTrue, std::ref(msg), std::ref(CQueueHandler_obj) );

	TestTarget.join();
	TestHelperStpThreadTrue.join();

	g_stopThread = false;
}

/**
 * Test case to check if analyzeControlLoopData() Thread function reads requests from queue filled up by MQTT and send data to EIS
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Main_ut, analyzeControlLoopData_Test01)
{
	CQueueHandler CQueueHandler_obj;
	mqtt::const_message_ptr msg = mqtt::make_message("Death/A", "Msg_UT", 0, false);

	std::thread TestTarget_analyzeControlLoopData( TargetCaller_analyzeControlLoopData, std::ref(CQueueHandler_obj) );
	std::thread CallHelper_sTopThreadTrue_analyzeControlLoopData( CallHelper_sTopThreadTrue, std::ref(msg), std::ref(CQueueHandler_obj) );

	TestTarget_analyzeControlLoopData.join();
	CallHelper_sTopThreadTrue_analyzeControlLoopData.join();
}


