/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../include/Common_ut.hpp"


void Common_ut::SetUp()
{
	// Setup code
}

void Common_ut::TearDown()
{
	// TearDown code
}

/**
 * Test case to check swapConversion() when input vector is empty
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Common_ut, swapConversion_EmptyVector)
{
	try
	{
		std::string TestStr = common_Handler::swapConversion(EmptyVec, false, false);
	}
	catch(std::exception& e)
	{
		EXPECT_STRNE("", e.what());
	}
}

/**
 * Test case to check swapConversion() when its neither byte swap nor word swap
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Common_ut, swapConversion_BytSwpAndWordSwapIsFalse)
{

	Vec.push_back(10);
	Vec.push_back(11);
	Vec.push_back(12);

	std::string TestStr = common_Handler::swapConversion(Vec, false, false);

	EXPECT_STREQ("0x0B0A0C", TestStr.c_str());
}

/**
 * Test case to check swapConversion() when vector size is >= 4
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Common_ut, swapConversion_VecSize5)
{

	Vec.push_back(10);
	Vec.push_back(11);
	Vec.push_back(12);
	Vec.push_back(13);
	Vec.push_back(14);

	std::string TestStr = common_Handler::swapConversion(Vec, false, false);

	EXPECT_STREQ("0x0B0A0D0C0E", TestStr.c_str());
}

