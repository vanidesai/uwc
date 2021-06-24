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

/***QueueMgr.hpp is for queue managing operations*/

#ifndef QMGR_HPP_
#define QMGR_HPP_

#include <atomic>
#include <map>
#include <semaphore.h>
#include "mqtt/async_client.h"
#include "Common.hpp"
#include <queue>
#include <string>
#include "QueueHandler.hpp"

/**namespace for Queue manager*/
namespace QMgr
{

/** structure maintains mqtt msg related functionalities*/
struct stMqttMsg
{
	std::string m_strSite; /** site name*/
	std::string m_strDevice; /** device name*/
	std::string m_strDataPoint; /** data point name*/
	std::string m_mqttTopic; /** name of mqtt topic*/
	mqtt::const_message_ptr m_mqttMsg; /** mqtt msg*/
	std::map<std::string, std::string> m_datapoint_key_val;/** map for data point key value*/
	/**
	 * Set data point name on which to send message for SCADA
	 * @param a_sTopic :[in] data point name to set
	 * @return none
	 */
	void setDataPointName(std::string a_sDataPoint)
	{
		m_strDataPoint = a_sDataPoint;
	}

	/**
	 * Get data point name on which to send message for SCADA
	 * @param none
	 * @return data point name
	 */
	std::string setDataPointName()
	{
		return m_strDataPoint;
	}

	/**
	 * Set mqtt topic name on which mqtt has received message
	 * @param a_sMqttTopic :[in] mqtt topic name on which mqtt has received message
	 * @return none
	 */
	void setMqttTopic(std::string a_sMqttTopic)
	{
		m_mqttTopic = a_sMqttTopic;
	}

	/**
	 * Get mqtt topic name on which mqtt has received message
	 * @param none
	 * @return mqtt topic name on which mqtt has received message
	 */
	std::string getMqttTopic()
	{
		return m_mqttTopic;
	}

	/**
	 * Set mqtt-export message
	 * @param a_mqttMsg :[in] mqtt-export message
	 * @return none
	 */
	void setMqttExportMsg(mqtt::const_message_ptr a_mqttMsg)
	{
		m_mqttMsg = a_mqttMsg;
	}

	/**
	 * Get mqtt-export message
	 * @param none
	 * @return mqtt-export message
	 */
	mqtt::const_message_ptr& getMqttExportMsg()
	{
		return m_mqttMsg;
	}

	/**
	 * Insert keys and values received in MQTT payload in map
	 * @param key :[in] param name in payload
	 * @param value :[in] value of param in payload
	 * return none
	 */
	void insertParam(std::string key, std::string value)
	{
		m_datapoint_key_val.insert(std::pair<std::string, std::string> (key, value));
	}

	/**
	 * Get key-value pair received in MQTT payload in map depending on the index
	 * @param a_index :[in] key-value pair at index from the map
	 * @return pair of key and value
	 */
	std::pair<std::string, std::string> getParam(int a_index)
				{
		int i = 0;
		if( m_datapoint_key_val.size() > (size_t)a_index)
		{
			for( auto it = m_datapoint_key_val.begin(); it != m_datapoint_key_val.end(); ++it, ++i )
			{
				if(i == a_index)
				{
					return make_pair(it->first, it->second);
				}
			}
		}
		return std::make_pair("", "");
				}

	/**
	 * Get length of map formed with MQTT msg payload values
	 * @param none
	 * @return length of map
	 */
	int getLength()
	{
		return m_datapoint_key_val.size();
	}
};

/** functions to get on-demand operation instances*/
CQueueHandler& getDatapointsQ();
CQueueHandler& getScadaSubQ();
}
#endif
