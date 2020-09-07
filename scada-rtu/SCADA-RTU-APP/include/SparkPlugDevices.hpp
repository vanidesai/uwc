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

#include "Metric.hpp"

extern "C"
{
#include "cjson/cJSON.h"
}

using namespace std;

class CSparkPlugDev;
class CVendorApp;

using metricMap_t = std::map<std::string, CMetric>;
using devSparkplugMap_t = std::map<std::string, CSparkPlugDev>;


class CSparkPlugDev
{
	std::string m_sSubDev;
	std::string m_sSparkPlugName;
	bool m_bIsVendorApp;
	metricMap_t m_mapMetrics;
	bool m_bIsDead;

public:
	CSparkPlugDev(std::string a_sSubDev, std::string a_sSparkPluName,
			bool a_bIsVendorApp = false) :
			m_sSubDev
			{ a_sSubDev }, m_sSparkPlugName
			{ a_sSparkPluName }, m_bIsVendorApp
			{ a_bIsVendorApp }, m_bIsDead
			{ false }
	{
		;
	}
	;

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
	bool isVendorApp()
	{
		return m_bIsVendorApp;
	}
	;
	metricMap_t getMetrics()
	{
		return m_mapMetrics;
	}
	;

	void print()
	{
		/*std::cout << "Device: " << m_sSparkPlugName << ": Metric List:\n";
		for (auto &itr : m_mapMetrics)
		{
			itr.second.print();
		}*/
	}
};

class CVendorApp
{
	std::string m_sName;
	bool m_bIsDead;
	std::map<std::string, std::reference_wrapper<CSparkPlugDev>> m_mapDevList;

public:
	CVendorApp(std::string a_sName) :
			m_sName(a_sName), m_bIsDead
			{ false }
	{
		;
	}
	;
	uint8_t addDevice(CSparkPlugDev &a_oDev)
	{
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

public:
	void addDevice(std::string a_sVendorApp, CSparkPlugDev &a_oDev)
	{
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
	enMSG_BIRTH, enMSG_DEATH, enMSG_DATA,
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
