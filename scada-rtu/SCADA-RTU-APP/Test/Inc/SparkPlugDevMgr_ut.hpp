/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/
#ifndef SPARKPLUGDEVMGR_UT_HPP_
#define SPARKPLUGDEVMGR_UT_HPP_

#include "SparkPlugDevices.hpp"
#include "SparkPlugDevMgr.hpp"
#include "Metric.hpp"
#include <ctime>

#ifdef UNIT_TEST
#include <gtest/gtest.h>
#endif


class SparkPlugDevMgr_ut : public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();

public:

	std::string DevName = "";
	org_eclipse_tahu_protobuf_Payload_Metric a_metric = { NULL, false, 0, true, get_current_timestamp(), true,
						METRIC_DATA_TYPE_UNKNOWN, false, 0, false, 0, false,
						true, false,
			org_eclipse_tahu_protobuf_Payload_MetaData_init_default,
						false,
								org_eclipse_tahu_protobuf_Payload_PropertySet_init_default,
						0,
						{ 0 } };
	std::vector<std::string> RetVec;
	org_eclipse_tahu_protobuf_Payload dbirth_payload;

};



#endif /* SPARKPLUGDEVMGR_UT_HPP_ */
