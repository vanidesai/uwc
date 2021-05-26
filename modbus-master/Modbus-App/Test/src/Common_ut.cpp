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

TEST_F(Common_ut, HextoFloat_correct)
{
	std::string Value = "1234";
	float convertedVal = 6.53005e-42;
	float convertedValue = common_Handler::hexBytesToFloat(Value);
	EXPECT_EQ(convertedVal, convertedValue);
}

TEST_F(Common_ut, HextoFloat_EMpty)
{
	std::string Value = "";
	float convertedValue = common_Handler::hexBytesToFloat(Value);
	EXPECT_EQ(0, convertedValue);
}

TEST_F(Common_ut, HextoUShortInt)
{
	std::string Value = "12";
	unsigned short int covertedVal = 18;
	unsigned short int convertedValue = common_Handler::hexBytesToUShortInt(Value);
	EXPECT_EQ(covertedVal, convertedValue);
}

TEST_F(Common_ut, HextoUShortInt_empty)
{
	std::string Value = "";
	unsigned short int convertedValue = common_Handler::hexBytesToUShortInt(Value);
	EXPECT_EQ(0, convertedValue);
}

TEST_F(Common_ut, HextoShortInt_correct)
{
	std::string Value = "-255";
	unsigned short int convertedValue = common_Handler::hexBytesToShortInt(Value);
	std::cout<<"**********************"<<std::endl;
	std::cout<<convertedValue<<std::endl;
	std::cout<<"**********************"<<std::endl;
}

TEST_F(Common_ut, HextoShortInt_empty)
{
	std::string Value = "";
	unsigned short int convertedValue = common_Handler::hexBytesToShortInt(Value);
	std::cout<<"**********************"<<std::endl;
	std::cout<<convertedValue<<std::endl;
	std::cout<<"**********************"<<std::endl;
}

TEST_F(Common_ut, HextoBool_correct)
{
	std::string value = "0xff00";
	bool convertedValue = common_Handler::hexBytesToBool(value);
	std::cout<<"****************"<<std::endl;
	std::cout<<convertedValue<<std::endl;
	std::cout<<"****************"<<std::endl;
}

TEST_F(Common_ut, HextoBool_empty)
{
	std::string value = "";
	bool convertedValue = common_Handler::hexBytesToBool(value);
	std::cout<<"****************"<<std::endl;
	std::cout<<convertedValue<<std::endl;
	std::cout<<"****************"<<std::endl;
}

TEST_F(Common_ut, HextoString_correct)
{
	std::string value = "0x00A";
	std::string convertedValue = common_Handler::hexBytesToString(value);
	std::cout<<"****************"<<std::endl;
	std::cout<<convertedValue<<std::endl;
	std::cout<<"****************"<<std::endl;
}

TEST_F(Common_ut, HextoString_empty)
{
	std::string value = "";
	std::string convertedValue = common_Handler::hexBytesToString(value);
	std::cout<<"****************"<<std::endl;
	std::cout<<convertedValue<<std::endl;
	std::cout<<"****************"<<std::endl;
}

TEST_F(Common_ut, HxToInt_correct)
{
	std::string value = "0xffff";
	int convertedValue = common_Handler::hexBytesToInt(value);
	EXPECT_EQ(65535, convertedValue);
}

TEST_F(Common_ut, HxToInt_empty)
{
	std::string value = "";
	int convertedValue = common_Handler::hexBytesToInt(value);
	EXPECT_EQ(0, convertedValue);
}

TEST_F(Common_ut, HextoUnsignedInt_correct)
{
	std::string value = "0x00ff";
	unsigned int convertedValue = common_Handler::hexBytesToUnsignedInt(value);
	EXPECT_EQ(255, convertedValue);
}

TEST_F(Common_ut, HextoUnsignedInt_empty)
{
	std::string value = "";
	unsigned int convertedValue = common_Handler::hexBytesToUnsignedInt(value);
	EXPECT_EQ(0, convertedValue);
}




