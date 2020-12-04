/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

/*** SparkPlugDevices.hpp Maintains the spark plug device operations */

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

using metricMap_t = std::map<std::string, CMetric>; /** map with key as string and value as CMetric*/
using devSparkplugMap_t = std::map<std::string, CSparkPlugDev>; /** map with key as string and value as CSparkPlugDev*/
using var_dev_ref_t = std::variant<std::monostate, std::reference_wrapper<const network_info::CUniqueDataDevice>>; /**Type alias for metric value*/

/** Enumerator specifying device status*/
enum eDevStatus
{
	enDEVSTATUS_NONE, enDEVSTATUS_UP, enDEVSTATUS_DOWN
};

struct stRefForSparkPlugAction;

/** class holding spark plug device information*/
class CSparkPlugDev
{
	std::string m_sSubDev;/** subscriber device*/
	std::string m_sSparkPlugName;/**spark plug name*/
	bool m_bIsVendorApp;/** vendor app or not(true or false)*/
	metricMap_t m_mapMetrics;/** reference for metricMap_t*/
	std::atomic<eDevStatus> m_enLastStatetPublishedToSCADA;/** last state published to scada*/
	std::atomic<eDevStatus> m_enLastKnownStateFromDev; /** last state known from device*/
	uint64_t m_deathTimestamp; /** value for death timestamp*/
	var_dev_ref_t m_rDirectDevRef; /** direct device reference*/
	std::mutex m_mutexMetricList; /** mutex for metric list*/

	CSparkPlugDev& operator=(const CSparkPlugDev&) = delete;	/// assignmnet operator

	bool parseRealDeviceUpdateMsg(const std::string &a_sPayLoad, 
		std::string &a_sMetric, std::string &a_sValue, std::string &a_sStatus,
		uint64_t &a_usec, uint64_t &a_lastGoodUsec, uint32_t &a_error_code);

	bool validateRealDeviceUpdateData(
		const std::string &a_sValue, std::string a_sStatus,
		uint32_t a_error_code,
		bool &a_bIsGood, bool &a_bIsDeathCode);

public:
	/** constructor*/
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

	/** function to set death time*/
	void setDeathTime(uint64_t a_deviceDeathTimestamp)
	{
		m_deathTimestamp = a_deviceDeathTimestamp;
	}

	/** function to get death time*/
	uint64_t getDeathTime()
	{
		return m_deathTimestamp;
	}

	/** function to get subscriber device name*/
	std::string getSubDevName()
	{
		return m_sSubDev;
	}
	;

	/**function to get spark plug name*/
	std::string getSparkPlugName()
	{
		return m_sSparkPlugName;
	}
	;

	/**function to check metric*/
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

	/** function to set spark plug name*/
	void setSparkPlugName(std::string a_sVal)
	{
		m_sSparkPlugName = a_sVal;
	}
	;

	/** to set vendor app reference */
	void setVendorAppRef(CVendorApp &a_refVendorApp)
	{
	}
	;

	metricMap_t processNewData(metricMap_t a_MetricList);
	metricMap_t processNewBirthData(metricMap_t a_MetricList, bool &a_bIsOnlyValChange);
	
	/**To set published status */
	void setPublishedStatus(eDevStatus a_enStatus)
	{
		m_enLastStatetPublishedToSCADA.store(a_enStatus);
	}	

	/** To get last published device status*/
	eDevStatus getLastPublishedDevStatus()
	{
		return m_enLastStatetPublishedToSCADA.load();
	}

	/** function to set known device status*/
	void setKnownDevStatus(eDevStatus a_enStatus)
	{
		m_enLastKnownStateFromDev.store(a_enStatus);
	}	

	/** function to get last known device  status*/
	eDevStatus getLastKnownDevStatus()
	{
		return m_enLastKnownStateFromDev.load();
	}

	/** To check vendor app*/
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


/** Class for vendor App*/
class CVendorApp
{
	std::string m_sName; /** site name*/
	bool m_bIsDead; /** Is dead or not(true or false)*/
	std::map<std::string, std::reference_wrapper<CSparkPlugDev>> m_mapDevList; /** Map for device list*/
	std::mutex m_mutexDevList; /** mutex for device lst*/

	CVendorApp& operator=(const CVendorApp&) = delete;	/// assignmnet operator

public:
	/** constructor*/
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

	/** Function to add device*/
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

	/** Function to get device*/
	std::map<std::string, std::reference_wrapper<CSparkPlugDev>>& getDevices()
	{
		std::lock_guard<std::mutex> lck(m_mutexDevList);
		return m_mapDevList;
	}
	;

	/** Function to set death status*/
	void setDeathStatus(bool a_bIsDead)
	{
		m_bIsDead = a_bIsDead;
	}
	;

	/**Function to check dead or not*/
	bool isDead()
	{
		return m_bIsDead;
	}
	;
};


/** Class maintaining vendor app list*/
class CVendorAppList
{
	std::map<std::string, CVendorApp> m_mapVendorApps;/** map for vendor app*/
	std::mutex m_mutexAppList;/** mutex for app list*/

public:
	/**function to add device */
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

	/** function to get vendor app*/
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

/** Enumerator specifying msg action*/
enum eMsgAction
{
	enMSG_NONE, enMSG_BIRTH, enMSG_DEATH, enMSG_DATA, enMSG_DCMD_CMD, enMSG_DCMD_WRITE
};

/** Structure maintaing reference for sparkplug actions*/
struct stRefForSparkPlugAction
{
	std::reference_wrapper<CSparkPlugDev> m_refSparkPlugDev; /** wrapper for spark lug device*/
	eMsgAction m_enAction; /** reference for enum eMsgAction*/
	metricMap_t m_mapChangedMetrics; /** reference for metricMap_t*/

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
