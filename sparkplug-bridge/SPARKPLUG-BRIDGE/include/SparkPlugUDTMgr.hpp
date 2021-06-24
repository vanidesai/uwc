/********************************************************************************
* Copyright (c) 2021 Intel Corporation.

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*********************************************************************************/

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
