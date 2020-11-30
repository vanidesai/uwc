/*
 * SparkPlugDevMgr_ut.hpp
 *
 *  Created on: 22-Oct-2020
 *      Author: user
 */

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
