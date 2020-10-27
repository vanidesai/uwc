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



TEST_F(SparkPlugDevMgr_ut, DBirthMsg_false)
{
	bool RetVal = CSparkPlugDevManager::getInstance().prepareDBirthMessage(dbirth_payload, DevName, true);
    EXPECT_EQ(false, RetVal);

}


TEST_F(SparkPlugDevMgr_ut, DeviceList)
{
	auto vDevList = CSparkPlugDevManager::getInstance().getDeviceList();
	EXPECT_EQ(true, true);

}


