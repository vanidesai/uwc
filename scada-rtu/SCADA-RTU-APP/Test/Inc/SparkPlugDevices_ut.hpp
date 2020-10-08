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
	//	zmq_handler::stZmqContext a_BusContext;
		//zmq_handler::stZmqPubContext a_objPubContext;
		//CTimeRecord CTimeRecord_obj;
		uint8_t a_uiFuncCode;
		uint32_t U32_code;
		//std::vector<CRefDataForPolling> m_PolledPoints;
		//std::mutex m_2vectorMutex;
		bool bRet = true;

		network_info::CUniqueDataPoint	CUniqueDataPoint_obj
		{
			"D1",
			CWellSiteInfo_obj,
			CWellSiteDevInfo_obj,
			CDataPoint_obj
		};

};

#endif /* TEST_INCLUDE_SCADASPARKPLUGDEV_UT_H_ */
