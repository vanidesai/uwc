/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#include "../include/QueueHandler_ut.hpp"

void QueueHandler_ut::SetUp()
{
	// Setup code
}

void QueueHandler_ut::TearDown()
{
	// TearDown code
}

/**Test for CQueueHandler::cleanup()**/
TEST_F(QueueHandler_ut, cleanUp)
{
	CQueueHandler_obj.cleanup();
}

/** Test for CQueueHandler::clear() to check if function Clears queue**/
TEST_F(QueueHandler_ut, Clr)
{
	CQueueHandler_obj.clear();
}

/** Test for CQueueHandler::pushMsg()**/
TEST_F(QueueHandler_ut, PushMsg)
{
	bool RetVal = CQueueHandler_obj.pushMsg(msg);
	EXPECT_EQ(true, RetVal);
}

/** Test for CQueueHandler::breakWaitOnQ()**/
TEST_F(QueueHandler_ut, BreakWait)
{
	bool RetVal = CQueueHandler_obj.breakWaitOnQ();
	EXPECT_EQ(true, RetVal);
}

/** Test for CQueueHandler::getSubMsgFromQ()**/
TEST_F(QueueHandler_ut, GetSubMsg)
{
	bool RetVal = CQueueHandler_obj.getSubMsgFromQ(msg);
	EXPECT_EQ(false, RetVal);
}


