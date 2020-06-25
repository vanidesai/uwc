/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include <vector>
#include "cjson/cJSON.h"
#include "Common.hpp"
#include "MQTTSubscribeHandler.hpp"

/**
 * constructor Initializes MQTT subscriber
 * @param strPlBusUrl :[in] MQTT broker URL
 * @return None
 */
CMQTTHandler::CMQTTHandler(std::string strPlBusUrl) :
		subscriber(strPlBusUrl, SUBSCRIBERID), subConfigState(MQTT_SUSCRIBER_CONNECT_STATE)
{
	try
	{
		//connect options for async subscriber
		mqtt::message willmsg("MQTTConfiguration", LWT_PAYLOAD, QOS, true);
		mqtt::will_options will(willmsg);
		conopts.set_will(will);
		conopts.set_keep_alive_interval(20);
		conopts.set_clean_session(true);
		conopts.set_automatic_reconnect(1, 10);

		// set the certificates if dev mode is false
		if(false == CCommon::getInstance().isDevMode())
		{
			mqtt::ssl_options sslopts;
			sslopts.set_trust_store("/run/secrets/ca_broker");
			sslopts.set_key_store("/run/secrets/client_cert");
			sslopts.set_private_key("/run/secrets/client_key");
			sslopts.set_enable_server_cert_auth(true);
			conopts.set_ssl(sslopts);
		}
		subscriber.set_callback(callback);

		connectSubscriber();
	}
	catch (const std::exception &e)
	{
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
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
	static string strPlBusUrl = CCommon::getInstance().getStrMqttExportURL();

	if(strPlBusUrl.empty())
	{
		std::cout << __func__ << ":" << __LINE__ << " Error : MQTT_URL_FOR_EXPORT Environment variable is not set" <<  std::endl;
		exit(EXIT_FAILURE);
	}

	static CMQTTHandler handler(strPlBusUrl.c_str());
	return handler;
}

/**
 * Get MQTT subscriber current state
 * @param None
 * @return subscriber current state
 */
Mqtt_Sub_Config_state_t CMQTTHandler::getMQTTSubConfigState()
{
	return subConfigState;
}

/**
 * Set MQTT subscriber state to given
 * @param tempConfigState :[in] subscriber state to set
 * @return None
 */
void CMQTTHandler::setMQTTSubConfigState(Mqtt_Sub_Config_state_t tempConfigState)
{
	subConfigState = tempConfigState;
}

/**
 * Subscribe with MQTT broker for topics for on-demand operations
 * @return true/false based on success/failure
 */
bool CMQTTHandler::subscribeToTopics()
{
	//get list of topics from topic mapper
	std::vector<std::string> vMqttEnvTopics;
	vMqttEnvTopics.push_back("mqtt_SubReadTopic");
	vMqttEnvTopics.push_back("mqtt_SubWriteTopic");

	std::vector<std::string> vMqttTopics;

	try
	{
		for (auto &envTopic : vMqttEnvTopics)
		{
			const char* env_pubWriteTopic = std::getenv(envTopic.c_str());
			if(env_pubWriteTopic == NULL)
			{
				std::cout << __func__ << ":" << __LINE__ << " Error : " + envTopic + " Environment Variable is not set" <<  std::endl;
				continue;
			}
			vMqttTopics.push_back(env_pubWriteTopic);
		}

		for (auto &topic : vMqttTopics)
		{
			if(! topic.empty())
			{
				cout << "Subscribing topic : " << topic << endl;
				subscriber.subscribe(topic, QOS, nullptr, listener);
			}
		}
		std::cout << __func__ << ":" << __LINE__ << "Subscribed topics with MQTT broker" << std::endl;
	}
	catch(exception &ex)
	{
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << ex.what() << std::endl;
		return false;
	}

	return true;
}

/**
 * Connect subscriber with MQTT broker
 * @return true/false based on success/failure
 */
bool CMQTTHandler::connectSubscriber()
{
	bool bFlag = true;
	try
	{
		subscriber.connect(conopts, nullptr, listener);

		// Wait for 2 seconds to get connected
		/*if (false == conntok->wait_for(2000))
		 {
		 CLogger::getInstance().log(DEBUG, LOGDETAILS("Error::Failed to connect to the platform bus ";
		 bFlag = false;
		 }*/
		std::cout << __func__ << ":" << __LINE__ << " Subscriber connected successfully with MQTT broker" << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
		bFlag = false;
	}

	return bFlag;
}

/**
* parse message to check if topic is of read or write
* @param topic	:[in] message from which to check read/write operation
* @param isWrite :[out] is it write operation or read (= ~write)
* @return true/false based on success/failure
*/
bool CMQTTHandler::getOperation(string &topic, bool &isWrite)
{
	//compare if request received for write or read
	if(topic.find("write") != std::string::npos)
	{
		isWrite = true;
	}
	else if(topic.find("read") != std::string::npos)
	{
		isWrite = false;
	}
	else
	{
		return false;
	}

	return true;
}

/**
* Fill the default real-time from global config
* @param isWrite :[in] operation type
* @return real-time value
*/
bool CMQTTHandler::fillDefaultRealtime(const bool isWrite)
{
	/* fill the default realtime from global configurations based on operation
	  if realtime flag is missing from received request */
	if(isWrite)
	{
		return globalConfig::CGlobalConfig::getInstance().
				getOpOnDemandWriteConfig().getDefaultRTConfig();
	}
	else
	{
		return globalConfig::CGlobalConfig::getInstance().
				getOpOnDemandReadConfig().getDefaultRTConfig();
	}
}

/**
* parse message to retrieve QOS and topic names
* @param json :[in] message from which to retrieve real-time
* @param isRealtime :[in] is it a message for real-time operation
* @param isWrite :[in] is it a message for write or read (= ~write) operation
* @return true/false based on success/failure
*/
bool CMQTTHandler::parseMQTTMsg(const char *json, bool &isRealtime, const bool isWrite)
{
	bool bRetVal = false;

	try
	{
		cJSON *root = cJSON_Parse(json);
		if (NULL == root)
		{
			return bRetVal;
		}

		if(! cJSON_HasObjectItem(root, "realtime"))
		{
			cout << "Message received from MQTT does not have key \"realtime\", request will be sent as non-RT" << endl;
			if (NULL != root)
				cJSON_Delete(root);

			isRealtime = fillDefaultRealtime(isWrite);
			return true;
		}

		char *crealtime = cJSON_GetObjectItem(root, "realtime")->valuestring;
		if (NULL == crealtime)
		{
			if (NULL != root)
			{
				cJSON_Delete(root);
			}
			isRealtime = fillDefaultRealtime(isWrite);
			return true;
		}
		else
		{
			if(strcmp(crealtime, "0") == 0)
			{
				isRealtime = false;
			}
			else if(strcmp(crealtime, "1") == 0)
			{
				isRealtime = true;
			}
			else
			{
				isRealtime = fillDefaultRealtime(isWrite);
			}
		}

		if (NULL != root)
			cJSON_Delete(root);

		bRetVal = true;
	}
	catch (std::exception &ex)
	{
		bRetVal = false;
	}

	return bRetVal;
}

/**
 * Push message in message queue to send on EIS
 * @param msg :[in] reference of message to push in queue
 * @return true/false based on success/failure
 */
bool CMQTTHandler::pushSubMsgInQ(mqtt::const_message_ptr msg)
{
	bool bRet = true;
	try
	{
		//add time stamp before publishing msg on EIS
		std::string strTsReceivedFromMQTT;
		CCommon::getInstance().getCurrentTimestampsInString(strTsReceivedFromMQTT);

		string topic = msg->get_topic();
		string payload = msg->get_payload();

		//this should be present in each incoming request
		if (payload == "") //will not be the case ever
		{
			return false;
		}

		bool isWrite = false;//toggle between read and write operation
		if(! getOperation(topic, isWrite))
		{
			return false;
		}

		//parse payload to check if realtime is true or false
		bool isRealTime = false;
		if( ! parseMQTTMsg(payload.c_str(), isRealTime, isWrite))
		{
			return false;
		}

		//add timestamp in msg and stored in queue
		CCommon::getInstance().addTimestampsToMsg(payload, "tsMsgRcvdFromMQTT", strTsReceivedFromMQTT);
		mqtt::const_message_ptr newMsg = mqtt::make_message(topic, payload);

		bRet = true;
	}
	catch (const std::exception &e)
	{
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
	conopts.set_automatic_reconnect(0);

	subscriber.disable_callbacks();

	if(subscriber.is_connected())
		subscriber.disconnect();
}

/**
 * Destructor
 */
CMQTTHandler::~CMQTTHandler()
{
}
