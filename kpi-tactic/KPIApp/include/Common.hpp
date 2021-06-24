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
/*** Common.hpp handles common functionalities like getting value from msg, adding field to msg, creating write request*/

#ifndef INCLUDE_COMMON_HPP_
#define INCLUDE_COMMON_HPP_

#include "Logger.hpp"
#include "EnvironmentVarHandler.hpp"
#include <string>
#include "QueueHandler.hpp"
#include "ControlLoopHandler.hpp"

/** namespace maintains functions for getting value from json, log analysis , creating write request*/
namespace commonUtilKPI
{
	std::string getValueofKeyFromJSONMsg(const std::string &a_sMsg, const std::string &a_sKey);
	void addFieldToMsg(std::string& a_sMsg, const std::string& a_sKey, const std::string& a_sVal, bool a_bIsLastField);
	bool createWriteRequest(std::string& a_sMsg, const std::string& a_sKey,
		const std::string& a_sWell, const std::string& a_sPoint,
		const std::string& a_sVal, const std::string& a_sRT,
		const std::string& a_sTopic, bool a_bIsMQTTModeOn);
	
	std::string createAnalysisMsg(struct stPollWrData &a_stPollWrData, CMessageObject &a_msgWrResp);

	unsigned long get_micros(struct timespec ts);
	void getCurrentTimestampsInString(std::string &strCurTime);
	void getTimeParams(std::string &a_sTimeStamp, std::string &a_sUsec);

	void logAnalysisMsg(struct stPollWrData &a_stPollWrData, CMessageObject &a_msgWrResp);
}

/** Class for common settings*/
class commonSettings
{
	bool m_devMode; /** dev mode or not(true or false)*/

	commonSettings(){}
	commonSettings(const commonSettings&) = delete;
	commonSettings& operator=(const commonSettings&) = delete;

public:
	static commonSettings& getInstance()
	{
		static commonSettings instance;
		return instance;
	}

	/**
	 * Check if application set for dev mode or not
	 * @param None
	 * @return true if set to devMode
	 * 			false if not set to devMode
	 */
	bool isDevMode() const {
		return m_devMode;
	}

	/**
	 * Set dev mode
	 * @param devMode :[in] value to set for dev_mode
	 * @return None
	 */
	void setDevMode(bool devMode) {
		m_devMode = devMode;
	}
};

#endif /* INCLUDE_COMMON_HPP_ */
