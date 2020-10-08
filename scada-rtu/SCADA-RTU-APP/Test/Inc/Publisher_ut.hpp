/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef TEST_INCLUDE_PUBLISHER_UT_H_
#define TEST_INCLUDE_PUBLISHER_UT_H_


#include "Publisher.hpp"
#include <gtest/gtest.h>




class Publisher_ut : public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();

public:
	bool Bool_Res = false;
	std::string msg = "Test_Msg";
	std::string topic = "a/b/c";


};

#endif //TEST_INCLUDE_PUBLISHER_UT_H_
