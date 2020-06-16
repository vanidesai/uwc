/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "MQTTHandler.hpp"
#include "Common.hpp"
#include "ConfigManager.hpp"

#define SUBSCRIBER_ID "SCADARTU_SUBSCRIBER"

int iMqttExpQOS = 0;

/**
 * Constructor Initializes MQTT publisher
 * @param strPlBusUrl :[in] MQTT broker URL
 * @param strClientID :[in] client ID with which to subscribe (this is topic name)
 * @param iQOS :[in] QOS value with which publisher will publish messages
 * @return None
 */
CMQTTHandler::CMQTTHandler(std::string strPlBusUrl, int iQOS):
		m_subscriber(strPlBusUrl, SUBSCRIBER_ID)
{
	try
	{
		m_QOS = iQOS;
		//connect options for sync publisher/client
		m_connOpts.set_keep_alive_interval(20);
		m_connOpts.set_clean_session(true);
		m_connOpts.set_automatic_reconnect(1, 10);

		m_subscriber.set_callback(m_mqttSubscriberCB);

		connect();

		DO_LOG_DEBUG("MQTT initialized successfully");
	}
	catch (const std::exception &e)
	{
		DO_LOG_FATAL(e.what());
	}
}

/**
 * Maintain single instance of this class
 * @param None
 * @return Reference of this instance of this class, if successful;
 * 			Application exits in case of failure
 */
CMQTTHandler& CMQTTHandler::instance()
{
	static string strPlBusUrl = CCommon::getInstance().getStrMqttURL();

	if(strPlBusUrl.empty())
	{
		DO_LOG_ERROR("MQTT_URL Environment variable is not set");
		std::cout << __func__ << ":" << __LINE__ << " Error : MQTT_URL Environment variable is not set" <<  std::endl;
		exit(EXIT_FAILURE);
	}

	static CMQTTHandler handler(strPlBusUrl.c_str(), iMqttExpQOS);
	return handler;
}


/**
 * MQTT publisher connects with MQTT broker
 * @param None
 * @return true/false based on success/failure
 */
bool CMQTTHandler::connect()
{

	bool bFlag = true;
	try
	{
		m_subscriber.connect(m_connOpts, nullptr, m_listener);

	    std::cout << __func__ << ":" << __LINE__ << " MQTT publisher & m_subscriber connected with MQTT broker" << std::endl;
	    DO_LOG_DEBUG("MQTT publisher & m_subscriber connected with MQTT broker");
	}
	catch (const std::exception &e)
	{
		DO_LOG_FATAL(e.what());
		std::cout << __func__ << ":" << __LINE__ << " Excception : MQTT publisher/ m_subscriber failed to connect with MQTT broker: " << e.what() << std::endl;

		bFlag = false;
	}
	return bFlag;
}

/**
 * Subscribe with MQTT broker for topics for on-demand operations
 * @return true/false based on success/failure
 */
bool CMQTTHandler::subscribeToTopics()
{
	//get list of topics from topic mapper
	std::vector<std::string> vMqttTopics;

	try
	{
		vMqttTopics.push_back("/+/+/+/readResponse");
		vMqttTopics.push_back("/+/+/+/writeResponse");
		for (auto &topic : vMqttTopics)
		{
			if(! topic.empty())
			{
				DO_LOG_DEBUG("MQTT handler subscribing topic : " + topic);
				std::cout << __func__ << ":" << __LINE__ << "MQTT handler subscribing topic : " << topic << endl;
				m_subscriber.subscribe(topic, m_QOS, nullptr, m_listener);
			}
		}

		std::cout << __func__ << ":" << __LINE__ << "MQTT handler subscribed topics with MQTT broker" << std::endl;
	}
	catch(exception &ex)
	{
		DO_LOG_FATAL(ex.what());
		std::cout << __func__ << ":" << __LINE__ << "CMQTTHandler Exception : " << ex.what() << std::endl;
		return false;
	}

	DO_LOG_DEBUG("MQTT handler subscribed topics with MQTT broker");

	return true;
}

/**
* parse message to retrieve QOS and topic names
* @param json :[in] message from which to retrieve real-time
* @param mqttMsgRecvd :[out] fill on the parsed information in this structure
* @return true/false based on success/failure
*/
bool CMQTTHandler::parseMsg(const char *json, QMgr::stMqttMsg& mqttMsgRecvd)
{
	bool bRetVal = false;

	cJSON *root = cJSON_Parse(json);
	try
	{
		if (NULL == root)
		{
			DO_LOG_ERROR("Message received from MQTT could not be parsed in json format");
			return bRetVal;
		}

		//json has needed values, fill in map
		cJSON *param = root->child;
		while(param)
		{
			if(cJSON_IsString(param))
			{
				DO_LOG_DEBUG((std::string)(param->string) + " : " + param->valuestring);

				mqttMsgRecvd.m_datapoint_key_val.insert(pair<string, string>((std::string)param->string, param->valuestring));
			}
			else
			{
				throw string("Invalid JSON");
			}
			param = param->next;
		}

		bRetVal = true;
	}
	catch (std::exception &ex)
	{
		DO_LOG_FATAL(ex.what());
		bRetVal = false;
	}

	if (NULL != root)
		cJSON_Delete(root);

	return bRetVal;
}

/**
 * Push message in message queue to send on EIS
 * @param msg :[in] reference of message to push in queue
 * @return true/false based on success/failure
 */
bool CMQTTHandler::pushMsgInQ(mqtt::const_message_ptr msg)
{
	bool bRet = true;
	try
	{
		QMgr::stMqttMsg stNewMsg;

		//fill up the data
		stNewMsg.m_mqttTopic = msg->get_topic();
		//parse information in key-value pair and store
		parseMsg(msg->get_payload().c_str(), stNewMsg);

		//parse device name
		//stNewMsg.m_strDataPoint = "/flowmeter/PL0/Flow";
		//stNewMsg.m_strDevice = "flowmeter";
		vector<string> splitTopic;

		string s = msg->get_topic();
		cout << "topic received : " << s << endl;
		size_t pos = 0;
		std::string delimiter = "/";
		std::string token;
		while ((pos = s.find(delimiter)) != std::string::npos)
		{
		    token = s.substr(0, pos);
		    //std::cout <<"*** " << token << std::endl;
		    splitTopic.push_back(token);
		    s.erase(0, pos + delimiter.length());
		}
/*		std::cout <<"*** device " << splitTopic[1] << std::endl;
		std::cout <<"*** site " << splitTopic[2] << std::endl;
		std::cout <<"*** data point " << splitTopic[3] << std::endl;*/

		stNewMsg.m_strDevice = splitTopic[1];
		stNewMsg.m_strSite = splitTopic[2];
		stNewMsg.m_strDataPoint = splitTopic[3];

		stNewMsg.m_mqttMsg = msg;

		QMgr::getMqtt().pushMsg(stNewMsg);

		DO_LOG_DEBUG("Pushed MQTT message in queue");
		bRet = true;
	}
	catch (const std::exception &e)
	{
		DO_LOG_FATAL(e.what());
		bRet = false;
	}
	return bRet;
}

/**
 * Clean up, destroy semaphores, disables callback, disconnect from MQTT broker
 * @param None
 * @return None
 */
void CMQTTHandler::cleanup()
{
	DO_LOG_DEBUG("Destroying CMQTTHandler instance ...");

	if(m_subscriber.is_connected())
	{
		m_subscriber.disconnect();
	}
	DO_LOG_DEBUG("Destroyed CMQTTHandler instance");
}

/**
 * Destructor
 */
CMQTTHandler::~CMQTTHandler()
{
}
