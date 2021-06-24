/********************************************************************************
* Copyright (c) 2021 Intel Corporation.

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*********************************************************************************/


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
 * Test case to check if postMsgsToWriteOnMQTT() Thread function reads requests from queue filled up by MQTT and send data to EII
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
 * Test case to check if analyzeControlLoopData() Thread function reads requests from queue filled up by MQTT and send data to EII
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


