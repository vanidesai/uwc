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
#include "Constants.hpp"
#include "Logger.hpp"
#include "ConfigManager.hpp"
#include "EnvironmentVarHandler.hpp"

using namespace std;
using namespace globalConfig;

#define SCADA_CONFIG_FILE_PATH "/opt/intel/eis/uwc_data/scada-rtu/scada_config.yml"
#define SCADA_SPARKPLUG_VERSION "2.0"

class CCommon
{
private:
	std::vector<string> m_vecEnv{"AppName", "INTERNAL_MQTT_URL", "DEV_MODE", "NETWORK_TYPE", "DEVICES_GROUP_LIST_FILE_NAME", "MY_APP_ID"};
	// Private constructor so that no objects can be created.
	CCommon();
	CCommon(const CCommon & obj) = delete;
	CCommon& operator=(CCommon const&) = delete;

	string m_strExtMqttURL;
	int m_nQos;
	string m_strNodeConfPath;
	string m_strGroupId;
	string m_strNodeName;
	bool m_bIsScadaTLS;

	void setScadaRTUIds();

	/**
	 * Return topic in sparkplug format to send as device data
	 * to SCADA
	 * @return data topic in string
	 */
	std::string getSparkPlugTopic(const std::string &a_sTopic)
	{
		std::string sTopic{SPARKPLUG_TOPIC + m_strGroupId + "/" + a_sTopic + "/" + m_strNodeName};
		return sTopic;
	}

public:
	~CCommon();

	/**
	 * load config required for scada-rtu container from scada_config.yml file
	 * @param None
	 * @return true/false based on condition
	 */
	bool loadYMLConfig();

	/**
	 * Set MQTT Export URL to connect with MQTT broker
	 * @param strMqttExportURL
	 * @return None
	 */
	void setExtMqttURL(const std::string &strMqttURL)
	{
		m_strExtMqttURL = strMqttURL;
	}

	/**
	 * Get MQTT-Export broker connection URL
	 * @param None
	 * @return connection URL in string
	 */
	const std::string& getExtMqttURL() const
	{
		return m_strExtMqttURL;
	}

	/**
	 * Set MQTT QOS to connect with MQTT broker
	 * @param nQos
	 * @return None
	 */
	void setMQTTQos(const int nQos)
	{
		switch(nQos)
		{
		case 0:
		case 1:
		case 2:
			m_nQos = nQos;
			break;
		default:
			m_nQos = 1;
			break;
		}
	}

	/**
	 * Get MQTT-Export broker connection QOS parameter
	 * @param None
	 * @return connection QOS in int
	 */
	const int getMQTTQos() const
	{
		return m_nQos;
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
	 * Return topic in sparkplug format to set in will message
	 * in mqtt subscriber
	 * @return death topic in string
	 */
	std::string getDeathTopic()
	{
		return getSparkPlugTopic(NDEATH);
	}

	/**
	 * Return topic in sparkplug format to send as node birth
	 * to SCADA
	 * @return nbirth topic in string
	 */
	std::string getNBirthTopic()
	{
		return getSparkPlugTopic(NBIRTH);
	}

	/**
	 * Return topic in sparkplug format to send as device birth
	 * to SCADA
	 * @return dbirth topic in string
	 */
	std::string getDBirthTopic()
	{
		return getSparkPlugTopic(DBIRTH);
	}

	/**
	 * Return topic in sparkplug format to send as device data
	 * to SCADA
	 * @return data topic in string
	 */
	std::string getDDataTopic()
	{
		return getSparkPlugTopic(DDATA);
	}

	/**
	 * Return topic in sparkplug format to send as device death
	 * to SCADA
	 * @return death topic in string
	 */
	std::string getDDeathTopic()
	{
		return getSparkPlugTopic(DDEATH);
	}

	/**
	 * Get group name of scada-rtu
	 * @return group name of scada-rtu in string
	 */
	std::string getGroupName()
	{
		return m_strGroupId;
	}

	/**
	 * Get edge node id of scada-rtu
	 * @return edge node id of scada-rtu in string
	 */
	std::string getNodeName()
	{
		return m_strNodeName;
	}

	/**
	 * Returns sparkplug version to be sent in message
	 * @return version string
	 */
	const std::string getVersion()
	{
		return std::string(SCADA_SPARKPLUG_VERSION);
	}

	/**
	 * Check if application set for TLS mode or not
	 * @param None
	 * @return true if set to TLS
	 * 			false if set to non-TLS
	 */
	bool isScadaTLS() const
	{
		return m_bIsScadaTLS;
	}

	/**
	 * Set TLS mode for scada
	 * @param a_bIsTLS :[in] value to set for TLS/Non-TLS
	 * @return None
	 */
	void setScadaTLS(bool a_bIsTLS)
	{
		m_bIsScadaTLS = a_bIsTLS;
	}

	bool getTopicParts(std::string a_sTopic, std::vector<std::string> &a_vsTopicParts, const string& a_delimeter);

};
#endif
