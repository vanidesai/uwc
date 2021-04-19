/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

/*** SparkPlugUDTMgr.hpp handles operations for managing spark plug template definitions */

#ifndef SPARKPLUG_UDT_MGR_HPP_
#define SPARKPLUG_UDT_MGR_HPP_

#pragma once

#include <string>
#include <map>
#include "SparkPlugDevices.hpp"

extern "C"
{
#include "cjson/cJSON.h"
}

using udtMap_t = std::map<std::string, std::shared_ptr<CUDT>>; /** map with key as string and value as CUDT*/

/** Class to maintain spark plug template definitions 
	UDT: User Defined Template
	UDT is a synonym for Sparkplug Template
*/
class CSparkPlugUDTManager
{
	CSparkPlugDev m_oDummyDev;
	std::map<std::string, udtMap_t> m_mapUDT; /** reference of devSparkplugMap_t*/
	std::mutex m_mutexUDTList; /** mutext for UDT list*/

	/** default constructor*/
	CSparkPlugUDTManager() : m_oDummyDev{"Dummy", "UDTMgr"}
	{
	}

	bool addNewUDT(const std::string &a_sName, const std::string &a_sVersion, 
			std::shared_ptr<CUDT> &a_pUDT);
	
	bool addUDTDefToNbirth(org_eclipse_tahu_protobuf_Payload& a_rTahuPayload,
							std::shared_ptr<CUDT> &a_pUDT);

#ifdef UNIT_TEST
	void testSetUDTMap(std::map<std::string, udtMap_t> & a_mapUDT){
		m_mapUDT = a_mapUDT;
	}
#endif


public:
	static CSparkPlugUDTManager& getInstance()
	{
		static CSparkPlugUDTManager _self;
		return _self;
	}
	
	void processTemplateDef(std::string a_sPayLoad,
		std::vector<stRefForSparkPlugAction> &a_stRefActionVec);

	bool addUDTDefsToNbirth(org_eclipse_tahu_protobuf_Payload& a_rTahuPayload);

	std::shared_ptr<CUDT> isPresent(const std::string &a_sName, const std::string &a_sVersion);
#ifdef UNIT_TEST
	friend class SparkPlugUDTMgr_ut;
#endif

};

#endif
