/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "ZmqToMqtt.hpp"

#include "MQTTHandler.hpp"
#include <eis/utils/json_config.h>
#include "ConfigManager.hpp"

namespace
{
	std::map<std::string, stZmqContext> g_mapContextMap;
	std::map<std::string, stZmqSubContext> g_mapSubContextMap;
}

//std::mutex fileMutex;
std::mutex __ctxMapLock;
std::mutex __PubctxMapLock;

bool CZmqToMqtt::prepareCommonContext(std::string topicType)
{
	msgbus_ret_t retVal = MSG_SUCCESS;
	bool retValue = false;
	recv_ctx_t* sub_ctx = NULL;

	if(!(topicType != "pub" || topicType != "sub"))
	{
		std::cout << __func__ << " Invalid TopicType parameter ::" << topicType << std::endl;
		return retValue;
	}
	if(CfgManager::Instance().IsClientCreated())
	{
		std::vector<std::string> Topics = CfgManager::Instance().getEnvConfig().get_topics_from_env(topicType);
		for (auto& topic : Topics)
		{
			try
			{
			retValue = true;
			config_t* config = CfgManager::Instance().getEnvConfig().get_messagebus_config(topic, topicType);
			if(config == NULL) {
				std::cout << __func__ << " Failed to get publisher message bus config ::" << topic << std::endl;
				continue;
			}

			void* msgbus_ctx = msgbus_initialize(config);
			if(msgbus_ctx == NULL)
			{
				std::cout << __func__ << " Failed to get message bus context with config for topic ::" << topic << std::endl;
				config_destroy(config);
				continue;
			}

			/// method to store msgbus_ctx as per the topic
			stZmqContext objTempCtx;
			objTempCtx.m_pContext = msgbus_ctx;

			if(topicType == "pub")
			{
				// Future use when this app will publish data to zmq
			}
			else
			{
				CZmqToMqtt::insertCTX(topic, objTempCtx);

				std::cout << __func__ << " Context created and stored for config for topic :: " << topic << std::endl;
				retVal = msgbus_subscriber_new(msgbus_ctx, topic.c_str(), NULL, &sub_ctx);
				if(retVal != MSG_SUCCESS)
				{
					std::cout <<__func__ << "Failed to create subscriber context. errno: "<< retVal << std::endl;
				}
				else
				{
					//std::cout << __func__ << "Successfully started subscriber\n";
					stZmqSubContext objTempSubCtx;
					objTempSubCtx.m_pContext = sub_ctx;
					CZmqToMqtt::insertSubCTX(topic, objTempSubCtx);

				}
			}
			}
			catch(exception &e)
			{
				std::cout << __func__ << " Error::Exception is ::" << e.what() << " for topic:" << topic;
				//bRet = false;
			}

		}
	}
	return retValue;
}

stZmqContext& CZmqToMqtt::getCTX(std::string a_sTopic)
{
	//std::cout << __func__ << " Start: " << a_sTopic;
	std::unique_lock<std::mutex> lck(__ctxMapLock);

	/// return the request ID
	return g_mapContextMap.at(a_sTopic);
}

void CZmqToMqtt::insertCTX(std::string a_sTopic, stZmqContext ctxRef)
{
	std::cout << __func__ << " Start: " << a_sTopic << std::endl;
	std::unique_lock<std::mutex> lck(__ctxMapLock);

	/// insert the data in map
	g_mapContextMap.insert(std::pair <std::string, stZmqContext> (a_sTopic, ctxRef));
	std::cout<< "End: ";
}

void CZmqToMqtt::removeCTX(std::string a_sTopic)
{
	//std::cout<< "Start: " << a_sTopic << std::endl;
	std::unique_lock<std::mutex> lck(__ctxMapLock);

	g_mapContextMap.erase(a_sTopic);
	std::cout<< "End:";
}

stZmqSubContext CZmqToMqtt::getSubCTX(std::string a_sTopic)
{
	//std::cout<< "Start: " << a_sTopic << std::endl;
	std::unique_lock<std::mutex> lck(__PubctxMapLock);

	std::cout<< "End: ";

	/// return the context
	return g_mapSubContextMap.at(a_sTopic);
}

bool CZmqToMqtt::insertSubCTX(std::string a_sTopic, stZmqSubContext ctxRef)
{
	bool bRet = true;
	try
	{
		std::unique_lock<std::mutex> lck(__PubctxMapLock);

		/// insert the data
		g_mapSubContextMap.insert(std::pair <std::string, stZmqSubContext> (a_sTopic, ctxRef));
	}
	catch (exception &e)
	{
		std::cout << __func__ <<  ":Error::Exception is ::" << e.what() << std::endl;
		bRet = false;
	}

	return bRet;
}

void CZmqToMqtt::removeSubCTX(std::string a_sTopic)
{
	//std::cout<< __func__ << " Start: " << a_sTopic << std::endl;
	std::unique_lock<std::mutex> lck(__PubctxMapLock);
	g_mapSubContextMap.erase(a_sTopic);
	//std::cout<< __func__ << " End: " << std::endl;
}


CZmqToMqtt::CZmqToMqtt(){

}

CZmqToMqtt::~CZmqToMqtt() {
	// TODO Auto-generated destructor stub
}
