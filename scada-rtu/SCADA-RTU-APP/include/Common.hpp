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
#define MQTT_EXPORT_VERSION "2.0"

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
	char m_delimeter;
	string m_strNodeConfPath;
	string m_strGroupId;
	string m_strEdgeNodeID;
	bool m_devMode;
	bool m_bIsScadaTLS;

	void setScadaRTUIds();
	string getMACAddress(const string& a_strInterfaceName);

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
	 * Return topic in sparkplug format to set in will message
	 * in mqtt subscriber
	 * @return death topic in string
	 */
	std::string getDeathTopic()
	{
		std::string topic(SPARKPLUG_TOPIC);
		topic.append(m_strGroupId + "/");
		topic.append(NDEATH);
		topic.append("/" + getEdgeNodeID());

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
		topic.append(m_strGroupId + "/");
		topic.append(NBIRTH);
		topic.append("/" + getEdgeNodeID());

		return topic;
	}

	/**
	 * Return topic in sparkplug format to send as device birth
	 * to SCADA
	 * @return dbirth topic in string
	 */
	std::string getDBirthTopic()
	{
		std::string topic(SPARKPLUG_TOPIC);
		topic.append(m_strGroupId + "/");
		topic.append(DBIRTH);
		topic.append("/" + getEdgeNodeID());

		return topic;
	}

	/**
	 * Return topic in sparkplug format to send as device data
	 * to SCADA
	 * @return data topic in string
	 */
	std::string getDDataTopic()
	{
		std::string topic(SPARKPLUG_TOPIC);
		topic.append(m_strGroupId + "/");
		topic.append(DDATA);
		topic.append("/" + getEdgeNodeID());

		return topic;
	}

	/**
	 * Return topic in sparkplug format to send as device death
	 * to SCADA
	 * @return death topic in string
	 */
	std::string getDDeathTopic()
	{
		std::string topic(SPARKPLUG_TOPIC);
		topic.append(m_strGroupId + "/");
		topic.append(DDEATH);
		topic.append("/" + getEdgeNodeID());

		return topic;
	}

	/**
	 * Get edge node id of scada-rtu
	 * @return edge node id of scada-rtu in string
	 */
	std::string getEdgeNodeID()
	{
		return m_strEdgeNodeID;
	}

	const std::string getVersion()
	{
		return std::string(MQTT_EXPORT_VERSION);
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
