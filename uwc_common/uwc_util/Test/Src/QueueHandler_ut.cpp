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


