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

/**
 * Test case to check processRealDeviceUpdateMsg() when a field is missing from payload
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(SparkPlugDevices_ut, processRealDeviceUpdateMsg_FieldMissingInPayload)
{
	CSparkPlugDev CSparkPlugDev_obj{"Dev01", "Dev_Name", false};

	std::string a_sPayLoad = "{\"metrics\": \"UtData02\", \"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}";
	std::vector<stRefForSparkPlugAction> a_stRefActionVec;
	EXPECT_EQ( false, CSparkPlugDev_obj.processRealDeviceUpdateMsg(a_sPayLoad, a_stRefActionVec) );

}

/**
 * Test case to check processRealDeviceUpdateMsg() when value is invalid in payload
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(SparkPlugDevices_ut, processRealDeviceUpdateMsg_WrongPayloadVal)
{
	CSparkPlugDev CSparkPlugDev_obj{"Dev01", "Dev_Name", false};

	std::string a_sPayLoad = "{\"metric\": \"UtData02\", \"status\": \"ON\", \"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}";
	std::vector<stRefForSparkPlugAction> a_stRefActionVec;
	EXPECT_EQ( false, CSparkPlugDev_obj.processRealDeviceUpdateMsg(a_sPayLoad, a_stRefActionVec) );

}

/**
 * Test case to check processRealDeviceUpdateMsg() when metric map is empty
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(SparkPlugDevices_ut, processRealDeviceUpdateMsg_MetricMapEmpty)
{
	CSparkPlugDev CSparkPlugDev_obj{"Dev01", "Dev_Name", false};

	std::string a_sPayLoad = "{\"metric\": \"UtData02\", \"status\": \"bad\", \"value\": \"0x00\", \"usec\": \"1571887474111145\", \"lastGoodUsec\": \"val1\", \"error_code\": \"2002\"}";
	std::vector<stRefForSparkPlugAction> a_stRefActionVec;
	EXPECT_EQ( false, CSparkPlugDev_obj.processRealDeviceUpdateMsg(a_sPayLoad, a_stRefActionVec) );

}

/**
 * Test case to check processRealDeviceUpdateMsg() when device map is empty
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(SparkPlugDevices_ut, processRealDeviceUpdateMsg_DeviceMapEmpty)
{
	CSparkPlugDev CSparkPlugDev_obj{"Dev01", "Dev_Name", false};

	std::string a_sPayLoad = "{\"metric\": \"UtData01\", \"status\": \"bad\", \"value\": \"0x00\", \"usec\": \"1571887474111145\", \"lastGoodUsec\": \"val1\", \"error_code\": \"2002\"}";
	std::vector<stRefForSparkPlugAction> a_stRefActionVec;
	EXPECT_EQ( false, CSparkPlugDev_obj.processRealDeviceUpdateMsg(a_sPayLoad, a_stRefActionVec) );

}

/**
 * Test case to check processRealDeviceUpdateMsg() when metric list is empty
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(SparkPlugDevices_ut, processNewData_EmptyMetricList)
{
	metricMap_t a_MetricList;
	CSparkPlugDev CSparkPlugDev_obj{"Dev01", "Dev_Name", false};

	metricMap_t oMetricMap = CSparkPlugDev_obj.processNewData(a_MetricList);

}
