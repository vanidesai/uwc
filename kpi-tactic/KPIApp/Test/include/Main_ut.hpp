/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef TEST_INCLUDE_MAIN_UT_HPP_
#define TEST_INCLUDE_MAIN_UT_HPP_

#include "gtest/gtest.h"
#include "QueueHandler.hpp"
#include "QueueMgr.hpp"



class Main_ut : public ::testing::Test{
protected:
	virtual void SetUp();
	virtual void TearDown();
public:
	CQueueHandler CQueueHandler_obj;

};



#endif /* TEST_INCLUDE_MAIN_UT_HPP_ */
