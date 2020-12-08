/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../Inc/SparkPlugDevMgr_ut.hpp"

void SparkPlugDevMgr_ut::SetUp()
{
	// Setup code
}

void SparkPlugDevMgr_ut::TearDown()
{
	// TearDown code
}

/**
 * Test case to check prepareDBirthMessage() with Invalid device name
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(SparkPlugDevMgr_ut, prepareDBirthMessage_InvalidDevName)
{
	bool RetVal = CSparkPlugDevManager::getInstance().prepareDBirthMessage(dbirth_payload, DevName, true);
	EXPECT_EQ(false, RetVal);

}

/**
 * Test case to check getDeviceList() when spark plug device name map is not empty
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(SparkPlugDevMgr_ut, getDeviceList_NonEmptySprkPlgDevMAp)
{
	auto vDevList = CSparkPlugDevManager::getInstance().getDeviceList();

	//Returned vector should not be empty
	EXPECT_EQ(false, vDevList.empty());

}


