/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../include/KPIAppConfigMgr_ut.hpp"


void KPIAppConfigMgr_ut::SetUp()
{
	// Setup code
}

void KPIAppConfigMgr_ut::TearDown()
{
	// TearDown code
}

/**
 * Test case to check if parseYMLFile() function Parses configuration YML file for KPI App successfully and returns true on success
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(KPIAppConfigMgr_ut, ParsingYMl_true)
{

	bool RetVal = CKPIAppConfig::getInstance().parseYMLFile("iou_datapoints.yml");
	EXPECT_EQ(1, RetVal);
}

/**
 * Test case to check if parseYMLFile() function Parses configuration YML file for KPI App successfully and returns true on success
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(KPIAppConfigMgr_ut, ParsingYMl)
{

	bool RetVal = CKPIAppConfig::getInstance().parseYMLFile("ControlLoopConfig.yml");
	EXPECT_EQ(1, RetVal);
}

/**
 * Test case to check if parseYMLFile() function do not Parses wrong YML file for KPI App successfully and returns false
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(KPIAppConfigMgr_ut, ParsingYMl_false)
{
	std::string file = "coverage_command.txt";
	bool RetVal = CKPIAppConfig::getInstance().parseYMLFile(file);
	EXPECT_EQ(0, RetVal);
}


