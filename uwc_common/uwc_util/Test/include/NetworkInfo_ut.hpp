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

#ifndef TEST_INCLUDE_NETWORKINFO_UT_H_
#define TEST_INCLUDE_NETWORKINFO_UT_H_

#include <gtest/gtest.h>

#include "NetworkInfo.hpp"
//#include "Common.hpp"


class NetworkInfo_ut : public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();

public:
	bool Bool_Res = false;
	int Int_Res = 111;
	network_info::CDataPoint CDataPoint_obj;

	std::string YmlFile = "flowmeter_datapoints.yml";
	std::string DevName = "Device";
	network_info::CDataPointsYML CDataPointsYML_obj{YmlFile};
	network_info::CDeviceInfo CDeviceInfo_obj{YmlFile, DevName, CDataPointsYML_obj};

	network_info::CWellSiteDevInfo CWellSiteDevInfo_obj{CDeviceInfo_obj};
	network_info::CWellSiteInfo CWellSiteInfo_obj;

	/*YAML::Node a_oData = CommonUtils::loadYamlFile("Device_group1.yml");
	YAML::Node Node_IP_Blank = CommonUtils::loadYamlFile("Device_group1_UT.yml");
	YAML::Node WrongNode = CommonUtils::loadYamlFile("Devices_group_list.yml");*/


};












#endif //TEST_INCLUDE_NETWORKINFO_UT_H_
