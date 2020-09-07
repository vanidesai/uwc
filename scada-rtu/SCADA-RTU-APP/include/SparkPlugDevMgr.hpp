/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef SPARKPLUG_DEV_MGR_HPP_
#define SPARKPLUG_DEV_MGR_HPP_

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <variant>
#include <typeinfo>
#include <map>
#include <functional>
#include "SparkPlugDevices.hpp"

extern "C"
{
#include "cjson/cJSON.h"
}

using namespace std;

class CSparkPlugDevManager
{
	devSparkplugMap_t m_mapSparkPlugDev;
	CVendorAppList m_objVendorAppList;

	CSparkPlugDevManager()
	{
		;
	}
	;

	metricMap_t parseVendorAppBirthMessage(std::string a_sPayLoad);
	bool processMetric(CMetric &a_oMetric, cJSON *a_cjName, cJSON *a_cjDatatype,
			cJSON *a_cjValue);
	bool getTopicParts(std::string a_sTopic,
			std::vector<std::string> &a_vsTopicParts);

	void processDeathMsg(std::string a_sAppName,
			std::vector<stRefForSparkPlugAction> &a_stRefActionVec);
	void processBirthMsg(std::string a_sAppName, std::string a_sSubDev,
			std::string a_sPayLoad,
			std::vector<stRefForSparkPlugAction> &a_stRefActionVec);
	void processDataMsg(std::string a_sAppName, std::string a_sSubDev,
			std::string a_sPayLoad,
			std::vector<stRefForSparkPlugAction> &a_stRefActionVec);

public:
	static CSparkPlugDevManager& getInstance();

	devSparkplugMap_t getDevList()
	{
		return m_mapSparkPlugDev;
	}
	;

	bool processInternalMQTTMsg(std::string a_sTopic, std::string a_sPayLoad,
			std::vector<stRefForSparkPlugAction> &a_stRefAction);

	void printRefActions(std::vector<stRefForSparkPlugAction> &a_stRefActionVec);

	void print()
	{
		/*std::cout << "Device List:\n";
		for (auto &itr : m_mapSparkPlugDev)
		{
			itr.second.print();
		}*/
	}
};

#endif
