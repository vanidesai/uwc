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

#ifndef TEST_INCLUDE_SCADASPARKPLUGDEV_UT_H_
#define TEST_INCLUDE_SCADASPARKPLUGDEV_UT_H_

#include <string.h>
#include "SparkPlugDevices.hpp"
#include <ctime>

#ifdef UNIT_TEST
#include <gtest/gtest.h>
#endif


class SparkPlugDevices_ut : public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();

public:
	bool Bool_Res = false;


	std::string YmlFile = "flowmeter_datapoints.yml";
	std::string DevName = "Device";
	network_info::CDataPointsYML CDataPointsYML_obj{YmlFile};
	network_info::CDeviceInfo CDeviceInfo_obj{YmlFile, DevName, CDataPointsYML_obj};


	network_info::CWellSiteInfo	CWellSiteInfo_obj;
		network_info::CWellSiteDevInfo CWellSiteDevInfo_obj{CDeviceInfo_obj};
		network_info::CDataPoint CDataPoint_obj;
		uint8_t a_uiFuncCode;
		uint32_t U32_code;
		bool bRet = true;
		std::string a_sSubDev = "device";
		std::string a_sSparkPlugName = "A";
		bool a_bIsVendorApp = false;
		std::vector<stRefForSparkPlugAction> stRefActionVec;

		org_eclipse_tahu_protobuf_Payload_Metric a_metric = { NULL, false, 0, true, get_current_timestamp(), true,
										METRIC_DATA_TYPE_UNKNOWN, false, 0, false, 0, false,
										true, false,
							org_eclipse_tahu_protobuf_Payload_MetaData_init_default,
										false,
												org_eclipse_tahu_protobuf_Payload_PropertySet_init_default,
										0,
										{ 0 } };
		std::string m_sSparkPlugName = "A";
		char *name = NULL;



		org_eclipse_tahu_protobuf_Payload_Metric SparkPlugmetric = {SparkPlugmetric.name = strdup(m_sSparkPlugName.c_str()) , false, 0, true, get_current_timestamp(), true,
										METRIC_DATA_TYPE_UNKNOWN, false, 0, false, 0, false,
										true, false,
							org_eclipse_tahu_protobuf_Payload_MetaData_init_default,
										false,
												org_eclipse_tahu_protobuf_Payload_PropertySet_init_default,
										0,
										{ 0 } };



		network_info::CUniqueDataPoint	CUniqueDataPoint_obj
		{
			"D1",
			CWellSiteInfo_obj,
			CWellSiteDevInfo_obj,
			CDataPoint_obj
		};

		org_eclipse_tahu_protobuf_Payload dbirth_payload = { true, get_current_timestamp(), 0, &a_metric,
				true, 0, NULL, NULL, NULL};

		//metricMap_t m_metrics;
		metricMapIf_t m_metrics;

		CSparkPlugDev CSparkPlugDev_obj{a_sSubDev, a_sSparkPlugName, a_bIsVendorApp};



};

#endif /* TEST_INCLUDE_SCADASPARKPLUGDEV_UT_H_ */
