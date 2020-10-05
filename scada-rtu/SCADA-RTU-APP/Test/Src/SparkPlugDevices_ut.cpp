/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../Inc/SparkPlugDevices_ut.hpp"

void SparkPlugDevices_ut::SetUp()
{
	// Setup code
}

void SparkPlugDevices_ut::TearDown()
{
	// TearDown code
}

/*TEST_F(SparkPlugDevices_ut, getWriteMsg_001)
{
	CSparkPlugDev CSparkPlugDev_obj{"Dev01", "Dev_Name", false};
	cJSON *root = NULL;
	metricMap_t m_metrics;
	std::string topic = "Topic";

	CSparkPlugDev_obj.getWriteMsg(topic, root, metricMap_t, 2);
}*/
