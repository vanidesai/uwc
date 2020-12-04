/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/
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
