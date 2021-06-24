/********************************************************************************
* Copyright (c) 2021 Intel Corporation.

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*********************************************************************************/

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
	metricMapIf_t a_MetricList;
	CSparkPlugDev CSparkPlugDev_obj{"Dev01", "Dev_Name", false};

	metricMapIf_t oMetricMap = CSparkPlugDev_obj.processNewData(a_MetricList);

}

/**
 * Test case to check  function getCMDMsg() behaves as expected or not
 * @param :[in] None
 * @param :[out] None
 * @return : bool
 */
TEST_F(SparkPlugDevices_ut, CMDmsg)
{
	std::string topic = "/flowmeter/PL0/Flow/read";
	cJSON *metricArray = cJSON_CreateArray();
	bool result = CSparkPlugDev_obj.getCMDMsg(topic, m_metrics, metricArray);
	EXPECT_EQ(false, result);
}

/**
 * Test case to check checkMetric() function's behavior
 * @param :[in] None
 * @param :[out] None
 * @return: bool
 */
TEST_F(SparkPlugDevices_ut, MetricCheck)
{
	bool result = CSparkPlugDev_obj.checkMetric(SparkPlugmetric);
	EXPECT_EQ(false, result);
}

/**
 * Test case to check prepareDdataMsg() function's behavior with empty map
 * @param :[in] None
 * @param :[out] None
 * @return: bool
 */
TEST_F(SparkPlugDevices_ut, dDataMsg)
{
	bool result = CSparkPlugDev_obj.prepareDdataMsg(dbirth_payload, m_metrics);
	EXPECT_EQ(false, result);
}

TEST_F(SparkPlugDevices_ut, PrepDBorthMsg_NBIRTH_true)
{
	bool result = CSparkPlugDev_obj.prepareDBirthMessage(dbirth_payload, true);
	EXPECT_EQ(true, result);
}

TEST_F(SparkPlugDevices_ut, PrepDBorthMsg_NBIRTH_false)
{
	bool result = CSparkPlugDev_obj.prepareDBirthMessage(dbirth_payload, false);
	EXPECT_EQ(true, result);
}

