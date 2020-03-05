/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#ifndef TEST_INCLUDE_CDEVICEINFO_UT_HPP_
#define TEST_INCLUDE_CDEVICEINFO_UT_HPP_

#include <stdbool.h>
#include "gtest/gtest.h"
#include "NetworkInfo.hpp"
#include "ZmqHandler.hpp"
#include "PeriodicReadFeature.hpp"
#include "PublishJson.hpp"
#include "ConfigManager.hpp"

class CDeviceInfo_ut : public ::testing::Test {
protected:
	virtual void SetUp();
	virtual void TearDown();
public:
	YAML::Node baseNode;

	network_info::CDataPoint datapoint_obj;
	network_info::CDeviceInfo Cdeviceinfo_obj;
};


#endif /* TEST_INCLUDE_CDEVICEINFO_UT_HPP_ */
