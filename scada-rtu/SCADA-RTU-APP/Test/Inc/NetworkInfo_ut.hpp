/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef TEST_INCLUDE_NETWORKINFO_UT_H_
#define TEST_INCLUDE_NETWORKINFO_UT_H_

#include <gtest/gtest.h>

#include "NetworkInfo.hpp"
#include "Common.hpp"
#include "Publisher.hpp"


class NetworkInfo_ut : public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();

public:
	bool Bool_Res = false;
	int Int_Res = 111;
	network_info::CDataPoint CDataPoint_obj;
	network_info::CDeviceInfo CDeviceInfo_obj;

	network_info::CWellSiteDevInfo CWellSiteDevInfo_obj;
	network_info::CWellSiteInfo CWellSiteInfo_obj;

	/*YAML::Node a_oData = CommonUtils::loadYamlFile("Device_group1.yml");
	YAML::Node Node_IP_Blank = CommonUtils::loadYamlFile("Device_group1_UT.yml");
	YAML::Node WrongNode = CommonUtils::loadYamlFile("Devices_group_list.yml");*/


};












#endif //TEST_INCLUDE_NETWORKINFO_UT_H_
