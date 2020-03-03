/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#ifndef __CWELLSITEINFO_H
#define __CWELLSITEINFO_H

#include <gtest/gtest.h>
#include <string.h>
#include "NetworkInfo.hpp"
#include "yaml-cpp/eventhandler.h"
#include "yaml-cpp/yaml.h"
#include "ConfigManager.hpp"
#include "utils/YamlUtil.hpp"



class CWellSiteInfo_ut : public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();

public:
	YAML::Node 										baseNode;
	network_info::CWellSiteInfo						CWellSiteInfo_obj;
	int												temp = 100;
	int												Res = 100;
	std::vector<network_info::CWellSiteDevInfo>		Device_List_ut;
	string											test_str = "BLANK";
};





#endif /*__CWELLSITEINFO_H */
