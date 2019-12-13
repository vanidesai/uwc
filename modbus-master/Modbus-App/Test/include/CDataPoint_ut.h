/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/
#ifndef INCLUDE_CDATAPOINT_A_UT_H_
#define INCLUDE_CDATAPOINT_A_UT_H_

#include <stdbool.h>
#include "gtest/gtest.h"
#include "NetworkInfo.hpp"
#include "ZmqHandler.hpp"
#include "PeriodicReadFeature.hpp"
#include "PublishJson.hpp"
#include "yaml-cpp/eventhandler.h"
#include "yaml-cpp/yaml.h"
#include "ConfigManager.hpp"
#include "utils/YamlUtil.hpp"


class CDataPoint_ut : public ::testing::Test {
protected:
	virtual void SetUp();
	virtual void TearDown();

public:
	YAML::Node baseNode;
	std::string id;
	network_info::CDataPoint CDataPoint_obj;
	network_info::stDataPointAddress m_Address;
	network_info::stPollingData m_PollingData;
	network_info::eEndPointType eEndPoint_obj;
};


#endif /* INCLUDE_CDATAPOINT_A_UT_H_ */
