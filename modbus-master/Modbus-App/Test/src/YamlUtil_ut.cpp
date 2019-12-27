/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

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

/* Valid string */
TEST_F(YamlUtil_ut, ConvertIPStringToCharArray_ValidString)
{
	try
	{
		CommonUtils::ConvertIPStringToCharArray(Test_Str, Arr);

		for( int i = 0; i < sizeof(Arr); i++)
		{
			EXPECT_EQ(ExpectedArr[i], Arr[i]);
		}
	}
	catch( exception &e)
	{
		Test_Str = e.what();
		EXPECT_EQ("", Test_Str);
	}
}
