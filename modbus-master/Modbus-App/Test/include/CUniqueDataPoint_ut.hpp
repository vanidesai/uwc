/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#ifndef __CUNIQUEDATAPOINT_H
#define __CUNIQUEDATAPOINT_H

#include <gtest/gtest.h>
#include <string.h>
#include "NetworkInfo.hpp"
#include "yaml-cpp/eventhandler.h"
#include "yaml-cpp/yaml.h"
#include "ConfigManager.hpp"
#include "utils/YamlUtil.hpp"


class CUniqueDataPoint_ut : public ::testing::Test {
protected:
	virtual void SetUp();
	virtual void TearDown();

public:
	YAML::Node							baseNode;
	network_info::CWellSiteInfo			CWellSiteInfo_obj;
	network_info::CWellSiteInfo			CWellSiteInfo_obj_CUniqueDataPoint;
	network_info::CWellSiteDevInfo		CWellSiteDevInfo_obj;
	network_info::CWellSiteDevInfo		CWellSiteDevInfo_obj_CUniqueDataPoint;
	network_info::CDataPoint			CDataPoint_obj;
	network_info::CDataPoint			CDataPoint_obj_CUniqueDataPoint;

	void PrintError(string e)
	{
		cout<<"ERROR HERE";
		cout<<endl<<"======================================="<<endl;
		cout<<e;
		cout<<endl<<"======================================="<<endl;
	}
};





#endif /*__CUNIQUEDATAPOINT_H */
