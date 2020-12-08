/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

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

	network_info::CWellSiteInfo			CWellSiteInfo_obj;
		network_info::CWellSiteDevInfo		CWellSiteDevInfo_obj;
		network_info::CDataPoint			CDataPoint_obj;
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

		network_info::CUniqueDataPoint	CUniqueDataPoint_obj
		{
			"D1",
			CWellSiteInfo_obj,
			CWellSiteDevInfo_obj,
			CDataPoint_obj
		};

		org_eclipse_tahu_protobuf_Payload dbirth_payload = { true, get_current_timestamp(), 0, &a_metric,
				true, 0, NULL, NULL, NULL};


		CSparkPlugDev CSparkPlugDev_obj{a_sSubDev, a_sSparkPlugName, a_bIsVendorApp};



};

#endif /* TEST_INCLUDE_SCADASPARKPLUGDEV_UT_H_ */
