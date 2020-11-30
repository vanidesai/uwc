/*
 * SparklugDevMgr_ut.cpp
 *
 *  Created on: 22-Oct-2020
 *      Author: user
 */

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


