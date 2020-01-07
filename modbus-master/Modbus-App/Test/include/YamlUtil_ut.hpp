/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef TEST_INCLUDE_YAMLUTIL_UT_HPP_
#define TEST_INCLUDE_YAMLUTIL_UT_HPP_

/* Include files */
#include "utils/YamlUtil.hpp"
#include<iostream>
#include "gtest/gtest.h"

class YamlUtil_ut : public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();

public:
	uint8_t	Arr[3];
	uint8_t	ExpectedArr[3] = {129, 130, 200};
	string	Test_Str = "129.130.200";

};


#endif /* TEST_INCLUDE_YAMLUTIL_UT_HPP_ */
