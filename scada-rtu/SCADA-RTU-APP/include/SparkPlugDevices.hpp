/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef SPARKPLUG_DEVICES_HPP_
#define SPARKPLUG_DEVICES_HPP_

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <variant>
#include <typeinfo>
#include <map>
#include <functional>
#include <mutex>

#include "Metric.hpp"
#include "NetworkInfo.hpp"
#include "Common.hpp"

extern "C"
{
#include "cjson/cJSON.h"
}

class CSparkPlugDev;
class CVendorApp;

using metricMap_t = std::map<std::string, CMetric>;
using devSparkplugMap_t = std::map<std::string, CSparkPlugDev>;
using var_dev_ref_t = std::variant<std::monostate, std::reference_wrapper<const network_info::CUniqueDataDevice>>; 

enum eDevStatus
{
	enDEVSTATUS_NONE, enDEVSTATUS_UP, enDEVSTATUS_DOWN
};

struct stRefForSparkPlugAction;

class CSparkPlugDev
{
	std::string m_sSubDev;
	std::string m_sSparkPlugName;
	bool m_bIsVendorApp;
	metricMap_t m_mapMetrics;
	std::atomic<eDevStatus> m_enLastStatetPublishedToSCADA;
	std::atomic<eDevStatus> m_enLastKnownStateFromDev;
	uint64_t m_deathTimestamp;
	var_dev_ref_t m_rDirectDevRef;
	std::mutex m_mutexMetricList;

	CSparkPlugDev& operator=(const CSparkPlugDev&) = delete;	/// assignmnet operator

	bool parseRealDeviceUpdateMsg(const std::string &a_sPayLoad, 
		std::string &a_sMetric, std::string &a_sValue, std::string &a_sStatus,
		uint64_t &a_usec, uint64_t &a_lastGoodUsec, uint32_t &a_error_code);

	bool validateRealDeviceUpdateData(
		const std::string &a_sValue, std::string a_sStatus,
		uint32_t a_error_code,
		bool &a_bIsGood, bool &a_bIsDeathCode);

public:
	CSparkPlugDev(std::string a_sSubDev, std::string a_sSparkPluName,
			bool a_bIsVendorApp = false) :
			m_sSubDev{ a_sSubDev }, m_sSparkPlugName{ a_sSparkPluName },
			m_bIsVendorApp{ a_bIsVendorApp }, 
			m_enLastStatetPublishedToSCADA{enDEVSTATUS_NONE},
			m_enLastKnownStateFromDev{enDEVSTATUS_NONE},
			m_deathTimestamp {0}, m_mutexMetricList{}
	{
		;
	}
	
	CSparkPlugDev(const network_info::CUniqueDataDevice &a_rUniqueDev, std::string a_sSparkPlugName) :
			m_sSubDev{ a_rUniqueDev.getWellSiteDev().getID() }, 
			m_sSparkPlugName{ a_sSparkPlugName },
			m_bIsVendorApp{ false },
			m_enLastStatetPublishedToSCADA{enDEVSTATUS_NONE},
			m_enLastKnownStateFromDev{enDEVSTATUS_NONE},
			m_deathTimestamp {0}, m_rDirectDevRef {a_rUniqueDev}, m_mutexMetricList{}
	{
		;
	}

	CSparkPlugDev(const CSparkPlugDev &a_refObj) :
			m_sSubDev{ a_refObj.m_sSubDev }, m_sSparkPlugName{ a_refObj.m_sSparkPlugName },
			m_bIsVendorApp{ a_refObj.m_bIsVendorApp }, m_mapMetrics{a_refObj.m_mapMetrics},
			m_enLastStatetPublishedToSCADA{enDEVSTATUS_NONE},
			m_enLastKnownStateFromDev{enDEVSTATUS_NONE},
			m_deathTimestamp {0}, 
			m_rDirectDevRef{a_refObj.m_rDirectDevRef}, m_mutexMetricList{}
	{
		;
	}

	void addMetric(const network_info::CUniqueDataPoint &a_rUniqueDataPoint);

	void setDeathTime(uint64_t a_deviceDeathTimestamp)
	{
		m_deathTimestamp = a_deviceDeathTimestamp;
	}

	uint64_t getDeathTime()
	{
		return m_deathTimestamp;
	}

	std::string getSubDevName()
	{
		return m_sSubDev;
	}
	;
	std::string getSparkPlugName()
	{
		return m_sSparkPlugName;
	}
	;

	bool checkMetric(std::string& strs)
	{
		bool flag = false;
		for(auto ele : m_mapMetrics)
		{
			if(strs == ele.second.getName())
			{
				flag = true;
				break;
			}
		}
		return flag;
	}

	void setSparkPlugName(std::string a_sVal)
	{
		m_sSparkPlugName = a_sVal;
	}
	;
	void setVendorAppRef(CVendorApp &a_refVendorApp)
	{
	}
	;

	metricMap_t processNewData(metricMap_t a_MetricList);
	metricMap_t processNewBirthData(metricMap_t a_MetricList, bool &a_bIsOnlyValChange);
	
	void setPublishedStatus(eDevStatus a_enStatus)
	{
		m_enLastStatetPublishedToSCADA.store(a_enStatus);
	}	
	eDevStatus getLastPublishedDevStatus()
	{
		return m_enLastStatetPublishedToSCADA.load();
	}
	void setKnownDevStatus(eDevStatus a_enStatus)
	{
		m_enLastKnownStateFromDev.store(a_enStatus);
	}	
	eDevStatus getLastKnownDevStatus()
	{
		return m_enLastKnownStateFromDev.load();
	}

	bool isVendorApp()
	{
		return m_bIsVendorApp;
	}
	;
	bool prepareDBirthMessage(org_eclipse_tahu_protobuf_Payload& a_rTahuPayload, bool a_bIsNBIRTHProcess);
	bool processRealDeviceUpdateMsg(const std::string a_sPayLoad, std::vector<stRefForSparkPlugAction> &a_stRefActionVec);

	void print()
	{
		/*std::cout << "Device: " << m_sSparkPlugName << ": Metric List:\n";
		for (auto &itr : m_mapMetrics)
		{
			itr.second.print();
		}*/
	}

	bool getWriteMsg(std::string& a_sTopic, cJSON *a_root, std::pair<const std::string,CMetric>& a_metric, const int& a_appSeqNo);
	bool getCMDMsg(std::string& a_sTopic, metricMap_t& m_metrics, cJSON *metricArray);
};

class CVendorApp
{
	std::string m_sName;
	bool m_bIsDead;
	std::map<std::string, std::reference_wrapper<CSparkPlugDev>> m_mapDevList;
	std::mutex m_mutexDevList;

	CVendorApp& operator=(const CVendorApp&) = delete;	/// assignmnet operator

public:
	CVendorApp(std::string a_sName) :
			m_sName(a_sName), m_bIsDead{ false }, m_mutexDevList{}
	{
		;
	}
	CVendorApp(const CVendorApp &a_refObj) :
			m_sName(a_refObj.m_sName), m_bIsDead{a_refObj.m_bIsDead},
			m_mapDevList{a_refObj.m_mapDevList}, m_mutexDevList{}
	{
		;
	}
	uint8_t addDevice(CSparkPlugDev &a_oDev)
	{
		std::lock_guard<std::mutex> lck(m_mutexDevList);
		std::string sDevName = a_oDev.getSparkPlugName();
		if (m_mapDevList.end() == m_mapDevList.find(sDevName))
		{
			m_mapDevList.emplace(sDevName, a_oDev);
			a_oDev.setVendorAppRef(*this);
			return 0;
		}
		return 1;
	}

	std::map<std::string, std::reference_wrapper<CSparkPlugDev>>& getDevices()
	{
		std::lock_guard<std::mutex> lck(m_mutexDevList);
		return m_mapDevList;
	}
	;

	void setDeathStatus(bool a_bIsDead)
	{
		m_bIsDead = a_bIsDead;
	}
	;
	bool isDead()
	{
		return m_bIsDead;
	}
	;
};

class CVendorAppList
{
	std::map<std::string, CVendorApp> m_mapVendorApps;
	std::mutex m_mutexAppList;

public:
	void addDevice(std::string a_sVendorApp, CSparkPlugDev &a_oDev)
	{
		std::lock_guard<std::mutex> lck(m_mutexAppList);
		if (m_mapVendorApps.end() == m_mapVendorApps.find(a_sVendorApp))
		{
			CVendorApp temp
			{ a_sVendorApp };
			m_mapVendorApps.emplace(a_sVendorApp, temp);
		}

		auto &oVendorApp = m_mapVendorApps.at(a_sVendorApp);
		oVendorApp.addDevice(a_oDev);
	}
	;

	CVendorApp* getVendorApp(std::string a_sVendorApp)
	{
		std::lock_guard<std::mutex> lck(m_mutexAppList);
		if (m_mapVendorApps.end() == m_mapVendorApps.find(a_sVendorApp))
		{
			return NULL;
		}
		auto &oVendorApp = m_mapVendorApps.at(a_sVendorApp);
		return &oVendorApp;
	}

};

enum eMsgAction
{
	enMSG_NONE, enMSG_BIRTH, enMSG_DEATH, enMSG_DATA, enMSG_DCMD_CMD, enMSG_DCMD_WRITE
};

struct stRefForSparkPlugAction
{
	std::reference_wrapper<CSparkPlugDev> m_refSparkPlugDev;
	eMsgAction m_enAction;
	metricMap_t m_mapChangedMetrics;

	stRefForSparkPlugAction(std::reference_wrapper<CSparkPlugDev> a_ref,
			eMsgAction a_enAction, metricMap_t a_mapMetrics) :
			m_refSparkPlugDev
			{ a_ref }, m_enAction
			{ a_enAction }, m_mapChangedMetrics
			{ a_mapMetrics }
	{
		;
	}
	;
};
#endif
