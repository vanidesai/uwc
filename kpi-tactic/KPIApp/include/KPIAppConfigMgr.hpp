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

/*** KPIAppConfigMgr.hpp handles the configuration of kpi app */

#ifndef INCLUDE_KPIAPPCONFIGMGR_HPP_
#define INCLUDE_KPIAPPCONFIGMGR_HPP_

#include "ControlLoopHandler.hpp"

/** class for kpi app configuration*/
class CKPIAppConfig
{
	uint32_t m_uiExecTimeMin; /** Execution time in minutes*/
	bool m_bIsMQTTModeApp; /** mqtt mode on or not(true or false*/
	bool m_bIsRTModeForPolledPoints; /** RT mode for polled points(true or false) */
	bool m_bIsRTModeForWriteOp; /** RT mode for write operation(true or false) */

	CControlLoopMapper m_oCtrlLoopMap; /** object of class CControlLoopMapper */

	/** Default constructor*/
	CKPIAppConfig(): m_uiExecTimeMin{0}, m_bIsMQTTModeApp{false}, 
		m_bIsRTModeForPolledPoints{true}, m_bIsRTModeForWriteOp{true}, m_oCtrlLoopMap{}
	{}

	/** delete copy and move constructors and assign operators*/
	CKPIAppConfig(const CKPIAppConfig&) = delete;	 			// Copy construct
	CKPIAppConfig& operator=(const CKPIAppConfig&) = delete;	// Copy assign

public:
	static CKPIAppConfig& getInstance()
	{
		static CKPIAppConfig _self;
		return _self;
	}
	bool parseYMLFile(const std::string &a_sFileName);
	uint32_t getExecutionTime() {return m_uiExecTimeMin;}
	bool isMQTTModeOn() {return m_bIsMQTTModeApp;}
	bool isRTModeForPolledPoints() {return m_bIsRTModeForPolledPoints;}
	bool isRTModeForWriteOp() {return m_bIsRTModeForWriteOp;}

	CControlLoopMapper& getControlLoopMapper() {return m_oCtrlLoopMap;}
};

#endif 
