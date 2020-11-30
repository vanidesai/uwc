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
#include <string>
#include <vector>


using namespace std;

void Common_ut::SetUp()
{
	// Setup code
}

void Common_ut::TearDown()
{
	// TearDown code
}

/**
 * Test case to check if addTimestampsToMsg()function do not Add current time stamp in message payload for invalid msg and returns false
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Common_ut, addTimestampsToMsg_InvMsg)
{

	std::string TimeStamp;
	std::string message_Inv = "InvMsg";
	std::string Key = "RecMsg";
	bool RetVal = true;
	CCommon::getInstance().getCurrentTimestampsInString(TimeStamp);
	RetVal = CCommon::getInstance().addTimestampsToMsg(message_Inv, Key, TimeStamp);
	EXPECT_EQ(false, RetVal);

}

/**
 * Test case to check if addTimestampsToMsg()function Add current time stamp in message payload for valid msg and returns true on successs
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Common_ut, addTimestampsToMsg_KeyAddedSuccessfully)
{

	std::string TimeStamp;
	std::string message = "{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}";
	std::string Key = "timestamp_NotPresent";
	bool RetVal = false;
	CCommon::getInstance().getCurrentTimestampsInString(TimeStamp);
	RetVal = CCommon::getInstance().addTimestampsToMsg(message, Key, TimeStamp);

	EXPECT_EQ(true, RetVal);

}
