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

#ifndef TEST_INCLUDE_YAMLUTIL_UT_HPP_
#define TEST_INCLUDE_YAMLUTIL_UT_HPP_

/* Include files */
#include "YamlUtil.hpp"
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
	std::string	Test_Str = "129.130.200";

};


#endif /* TEST_INCLUDE_YAMLUTIL_UT_HPP_ */
