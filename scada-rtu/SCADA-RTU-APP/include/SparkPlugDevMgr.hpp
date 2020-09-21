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
#include "NetworkInfo.hpp"

extern "C"
{
#include "cjson/cJSON.h"
}

using namespace std;

class CSparkPlugDevManager
{
	devSparkplugMap_t m_mapSparkPlugDev;
	CVendorAppList m_objVendorAppList;
	std::mutex m_mutexDevList;

	CSparkPlugDevManager()
	{
		;
	}
	;

	CMetric parseRealDeviceUpdateMsg(std::string a_sPayLoad);

	metricMap_t parseVendorAppBirthMessage(std::string a_sPayLoad);
	bool processMetric(CMetric &a_oMetric, cJSON *a_cjArrayElemMetric);
	bool processMetricFromUpdateMsg(CMetric &a_oMetric, cJSON *a_cjRoot);
	bool processDCMDMetric(CMetric &a_oMetric, org_eclipse_tahu_protobuf_Payload_Metric& a_sparkplugMetric);

	uint64_t parseVendorAppDeathMessage(std::string& a_sPayLoad);

	void processDeathMsg(std::string a_sAppName,
			std::string a_sPayLoad,
			std::vector<stRefForSparkPlugAction> &a_stRefActionVec);
	void processBirthMsg(std::string a_sAppName, std::string a_sSubDev,
			std::string a_sPayLoad,
			std::vector<stRefForSparkPlugAction> &a_stRefActionVec);
	void processDataMsg(std::string a_sAppName, std::string a_sSubDev,
			std::string a_sPayLoad,
			std::vector<stRefForSparkPlugAction> &a_stRefActionVec);

	void processUpdateMsg(std::string a_sDeviceName,
			std::string a_sSubDev, std::string a_sPayLoad,
			std::vector<stRefForSparkPlugAction> &a_stRefActionVec);


public:
	static CSparkPlugDevManager& getInstance();

	bool getTopicParts(std::string a_sTopic,
			std::vector<std::string> &a_vsTopicParts, const string& a_delimeter);

	bool processInternalMQTTMsg(std::string a_sTopic, std::string a_sPayLoad,
			std::vector<stRefForSparkPlugAction> &a_stRefAction);

	bool processExternalMQTTMsg(std::string a_sTopic, org_eclipse_tahu_protobuf_Payload& a_payload,
			std::vector<stRefForSparkPlugAction> &a_stRefAction);

	void printRefActions(std::vector<stRefForSparkPlugAction> &a_stRefActionVec);

	bool addRealDevices();

	bool prepareDBirthMessage(org_eclipse_tahu_protobuf_Payload& a_rTahuPayload, std::string a_sDevName);
	bool setMsgPublishedStatus(eDevStatus a_enStatus, std::string a_sDevName);

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
