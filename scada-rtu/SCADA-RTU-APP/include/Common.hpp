/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#ifndef INCLUDE_COMMON_HPP_
#define INCLUDE_COMMON_HPP_

#include <string>
#include <map>
#include <algorithm>
#include "ConfigManager.hpp"

using namespace std;

#define SPARKPLUG_TOPIC "spBv1.0/Sparkplug B Devices"

class CCommon
{
private:
	// Private constructor so that no objects can be created.
	CCommon();
	CCommon(const CCommon & obj){}
	CCommon& operator=(CCommon const&);

	std::string m_strAppName;
	std::string m_strMqttURL;
	std::string m_siteListFileName;
	bool m_devMode;

public:
	~CCommon();
	bool readCommonEnvVariables();
	bool readEnvVariable(const char *pEnvVarName, std::string &storeVal);

	/**
	 * Set application name
	 * @param strAppName :[in] Application name to set
	 * @return None
	 */
	void setStrAppName(const std::string &strAppName)
	{
		m_strAppName = strAppName;
	}

	/**
	 * Get application name
	 * @param None
	 * @return application name in string
	 */
	const std::string& getStrAppName() const
	{
		return m_strAppName;
	}

	/**
	 * Set MQTT Export URL to connect with MQTT broker
	 * @param strMqttExportURL
	 * @return None
	 */
	void setStrMqttURL(const std::string &strMqttURL)
	{
		m_strMqttURL = strMqttURL;
	}

	/**
	 * Get MQTT-Export broker connection URL
	 * @param None
	 * @return connection URL in string
	 */
	const std::string& getStrMqttURL() const
	{
		return m_strMqttURL;
	}

	/**
	 * Get single instance of this class
	 * @param None
	 * @return this instance of CCommon class
	 */
	static CCommon& getInstance()
	{
		static CCommon _self;
			return _self;
	}

	/**
	 * Check if application set for dev mode or not
	 * @param None
	 * @return true if set to devMode
	 * 			false if not set to devMode
	 */
	bool isDevMode() const
	{
		return m_devMode;
	}

	/**
	 * Set dev mode
	 * @param devMode :[in] value to set for dev_mode
	 * @return None
	 */
	void setDevMode(bool devMode)
	{
		m_devMode = devMode;
	}

	/**
	 * get site list from file name
	 * @return site list
	 */
	const std::string& getSiteListFileName() const
	{
		return m_siteListFileName;
	}

	/**
	 * set site list from file name
	 * @param siteListFileName	:[in] site list to set
	 */
	void setSiteListFileName(const std::string &siteListFileName)
	{
		m_siteListFileName = siteListFileName;
	}

	/**
	 * Return topic in sparkplug format to set in will message
	 * in mqtt subscriber
	 * @return death topic in string
	 */
	std::string getDeathTopic()
	{
		std::string topic(SPARKPLUG_TOPIC);
		topic.append("/NDEATH/" + getStrAppName());

		return topic;
	}

	/**
	 * Return topic in sparkplug format to send as node birth
	 * to SCADA
	 * @return nbirth topic in string
	 */
	std::string getNBirthTopic()
	{
		std::string topic(SPARKPLUG_TOPIC);
		topic.append("/NBIRTH/" + getStrAppName());

		return topic;
	}
};
#endif
