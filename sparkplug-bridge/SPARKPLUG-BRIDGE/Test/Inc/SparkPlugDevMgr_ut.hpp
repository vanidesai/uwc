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
	std::vector<stRefForSparkPlugAction> stRefActionVec;

};



#endif /* SPARKPLUGDEVMGR_UT_HPP_ */
