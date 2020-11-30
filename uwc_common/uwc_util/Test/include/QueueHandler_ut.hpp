/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#ifndef QUEUEHANDLER_UT_HPP_
#define QUEUEHANDLER_UT_HPP_

#include "QueueHandler.hpp"
#include <gtest/gtest.h>

class QueueHandler_ut : public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();

public:

	CQueueHandler CQueueHandler_obj;
	CMessageObject msg;


};



#endif /* QUEUEHANDLER_UT_HPP_ */
