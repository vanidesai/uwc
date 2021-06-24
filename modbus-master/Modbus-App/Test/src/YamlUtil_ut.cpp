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

#include "../include/YamlUtil_ut.hpp"

void YamlUtil_ut::SetUp()
{
	// Setup code
}

void YamlUtil_ut::TearDown()
{
	// TearDown code
}



/**************************ConvertIPStringToCharArray()*************************************/


/***Test:YamlUtil_ut::ConvertIPStringToCharArray_ValidString() checks ConvertIPStringToCharArray()
 function with valid string*/

TEST_F(YamlUtil_ut, ConvertIPStringToCharArray_ValidString)
{
	try
	{
		CommonUtils::ConvertIPStringToCharArray(Test_Str, Arr);

		for( unsigned int i = 0; i < sizeof(Arr); i++)
		{
			EXPECT_EQ(ExpectedArr[i], Arr[i]);
		}
	}
	catch(std::exception &e)
	{
		Test_Str = e.what();
		EXPECT_EQ("", Test_Str);
	}
}

