/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

/*** SparkPlugDevMgr.hpp handles operations for managing spark plug device like device birth msg,
 * deivce death, process birth, process death */

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

/** Class to maintain spark plug device operations*/
class CSparkPlugDevManager
{
	devSparkplugMap_t m_mapSparkPlugDev; /** reference of devSparkplugMap_t*/
	CVendorAppList m_objVendorAppList; /** object of class CVendorAppList*/
	std::mutex m_mutexDevList; /** mutext for device list*/

	/** default constructor*/
	CSparkPlugDevManager()
	{
	}

	metricMapIf_t parseVendorAppBirthDataMessage(std::string a_sPayLoad, bool a_bIsBirthMsg);
	bool processDCMDMetric(CSparkPlugDev& a_SPDev, CIfMetric &a_oIfMetric, org_eclipse_tahu_protobuf_Payload_Metric& a_sparkplugMetric);

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

	std::shared_ptr<CIfMetric> metricFactoryMethod(cJSON *a_cjArrayElemMetric);

public:
	static CSparkPlugDevManager& getInstance();

	bool getTopicParts(std::string a_sTopic,
			std::vector<std::string> &a_vsTopicParts, const string& a_delimeter);

	bool processInternalMQTTMsg(std::string a_sTopic, std::string a_sPayLoad,
			std::vector<stRefForSparkPlugAction> &a_stRefAction);

	bool processExternalMQTTMsg(std::string a_sTopic, org_eclipse_tahu_protobuf_Payload& a_payload,
			std::vector<stRefForSparkPlugAction> &a_stRefAction);

	void parseVendorAppMericData(metricMapIf_t &a_oMetricMap, cJSON *a_cjRoot, const std::string &a_sKey,
					bool a_bIsBirthMsg = false);

	bool addRealDevices();

	bool prepareDBirthMessage(org_eclipse_tahu_protobuf_Payload& a_rTahuPayload, std::string a_sDevName, bool a_bIsNBIRTHProcess);
	bool setMsgPublishedStatus(eDevStatus a_enStatus, std::string a_sDevName);

	std::vector<std::string> getDeviceList();

	std::shared_ptr<CIfMetric> metricFactoryMethod(const std::string &a_sName,
										uint32_t a_uiDataType);

	void print()
	{
		/*std::cout << "Device List:\n";
		for (auto &itr : m_mapSparkPlugDev)
		{
			itr.second.print();
		}*/
	}
#ifdef UNIT_TEST
	friend class Metric_ut;
#endif
};

#endif
