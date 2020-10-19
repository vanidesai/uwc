/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef INCLUDE_COMMON_HPP_
#define INCLUDE_COMMON_HPP_

#include "Logger.hpp"
#include "EnvironmentVarHandler.hpp"
#include <string>
#include "QueueHandler1.hpp"

namespace commonUtilKPI
{
	std::string getValueofKeyFromJSONMsg(const std::string &a_sMsg, const std::string &a_sKey);
	void addFieldToMsg(std::string& a_sMsg, const std::string& a_sKey, const std::string& a_sVal, bool a_bIsLastField);
	bool createWriteRequest(std::string& a_sMsg, const std::string& a_sKey,
		const std::string& a_sWell, const std::string& a_sPoint,
		const std::string& a_sVal, const std::string& a_sRT,
		const std::string& a_sTopic, bool a_bIsMQTTModeOn);
	
	std::string createAnalysisMsg(CMessageObject &a_msgPoll, CMessageObject &a_msgWrResp);

	unsigned long get_micros(struct timespec ts);
	void getCurrentTimestampsInString(std::string &strCurTime);
	void getTimeParams(std::string &a_sTimeStamp, std::string &a_sUsec);

	void logAnalysisMsg(CMessageObject &a_msgPoll, CMessageObject &a_msgWrResp);
}

class commonSettings
{
	bool m_devMode;

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
