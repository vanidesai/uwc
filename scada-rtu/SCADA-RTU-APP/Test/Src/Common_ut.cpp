/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../Inc/Common_ut.hpp"

void Common_ut::SetUp()
{
	// Setup code
}

void Common_ut::TearDown()
{
	// TearDown code
}

/**
 * Test case to check if getTopicParts() splits the topic successfully
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Common_ut, getTopicParts_Test01)
{
	std::vector<std::string> TopicParts;
	CCommon::getInstance().getTopicParts("PartA_UT/PartB_UT/PartC_UT", TopicParts, "/");

	EXPECT_EQ("PartC_UT",TopicParts[2]);
}

/**
 * Test case to check if function getDatapointsQ() executes successfully
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Common_ut, getDataPointInQ)
{
	QMgr::getDatapointsQ();
	EXPECT_EQ(true, true);
}

/**
 * Test case to check if function getScadaSubQ() executes successfully
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Common_ut, ScadaSubQ)
{
	QMgr::getScadaSubQ();
	EXPECT_EQ(true, true);
}





