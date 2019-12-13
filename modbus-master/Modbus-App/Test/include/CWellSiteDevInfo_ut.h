/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#ifndef __CWELLSITEDEVINFO_H
#define __CWELLSITEDEVINFO_H

#include <gtest/gtest.h>
#include <string.h>
#include "NetworkInfo.hpp"
#include "yaml-cpp/eventhandler.h"
#include "yaml-cpp/yaml.h"
#include "ConfigManager.hpp"
#include "utils/YamlUtil.hpp"

class CWellSiteDevInfo_ut : public ::testing::Test {
protected:
	virtual void SetUp();

	virtual void TearDown();

public:
	network_info::CWellSiteDevInfo					CWellSiteDevInfo_obj;
	string											tempStr = "BLANK";
	YAML::Node										baseNode;
	struct network_info::stModbusAddrInfo 			stModbusAddrInfo_obj;
	network_info::CDeviceInfo						CDeviceInfo_obj;

//	network_info::CWellSiteInfo objWellSite;
//	std::vector<std::string> g_sWellSiteFileList;
//	std::map<std::string,  CWellSiteInfo> g_mapYMLWellSite;
//	std::vector<CWellSiteInfo> oWellSiteList;
//	std::map<std::string, CWellSiteInfo>::iterator it;



};





#endif /*__CWELLSITEDEVINFO_H */
