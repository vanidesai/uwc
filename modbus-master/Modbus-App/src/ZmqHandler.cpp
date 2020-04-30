/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include <iostream>
#include <atomic>
#include <algorithm>
#include <map>
#include "ZmqHandler.hpp"
#include <mutex>
#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <fstream>
#include "ConfigManager.hpp"

#include "Logger.hpp"

using namespace zmq_handler;

std::mutex fileMutex;
std::mutex __ctxMapLock;
std::mutex __SubctxMapLock;
std::mutex __PubctxMapLock;

// Unnamed namespace to define globals
namespace
{
	std::map<std::string, stZmqContext> g_mapContextMap;
	std::map<std::string, stZmqSubContext> g_mapSubContextMap;
	std::map<std::string, stZmqPubContext> g_mapPubContextMap;
}

/**
 * Prepare EIS contexts for msgbus and topic
 * @param topicType	:[in] topic type to create context for
 * @return 	true : on success,
 * 			false : on error
 */
bool zmq_handler::prepareCommonContext(std::string topicType)
{
	DO_LOG_DEBUG("Start:");
	msgbus_ret_t retVal = MSG_SUCCESS;
	bool retValue = false;
	recv_ctx_t* sub_ctx = NULL;
	publisher_ctx_t* pub_ctx = NULL;

	if(!(topicType == "pub" || topicType == "sub"))
	{
		DO_LOG_ERROR("Invalid TopicType parameter ::" + topicType);
		return retValue;
	}
	if(CfgManager::Instance().IsClientCreated())
	{
		std::vector<std::string> Topics;// = CfgManager::Instance().getEnvConfig().get_topics_from_env(topicType);

		/// parse all the topics
		char** data = CfgManager::Instance().getEnvClient()->get_topics_from_env(topicType.c_str());

		if(NULL != data)
		{
			while (*data != NULL)
			{
				Topics.push_back(*data);
				data++;
			}
		}
		else
		{
			DO_LOG_ERROR("topic list is empty");
			cout << "topic list is empty" << endl;
			return false;
		}

		for (auto &topic : Topics)
		{
			retValue = true;
			config_t* config = CfgManager::Instance().getEnvClient()->get_messagebus_config(CfgManager::Instance().getConfigClient(), topic.c_str(), topicType.c_str());
			if(config == NULL) {
				DO_LOG_ERROR("Failed to get publisher message bus config ::" + topic);
				continue;
			}

			void* msgbus_ctx = msgbus_initialize(config);
			if(msgbus_ctx == NULL)
			{
				/// cleanup
				DO_LOG_ERROR("Failed to get message bus context with config for topic ::" + topic);

				/// free config context
				if(config != NULL)
				{
					config_destroy(config);
				}
				continue;
			}


			if(topicType == "pub")
			{
				/// method to store msgbus_ctx as per the topic
				stZmqContext objTempCtx;
				objTempCtx.m_pContext = msgbus_ctx;

				zmq_handler::insertCTX(topic, objTempCtx);

				retVal = msgbus_publisher_new(msgbus_ctx, topic.c_str(), &pub_ctx);

				if(retVal != MSG_SUCCESS)
				{
					/// cleanup
					DO_LOG_ERROR("Failed to initialize publisher errno: " + std::to_string(retVal));
					zmq_handler::removeCTX(topic);

					/// free config context
					if(config != NULL)
					{
						config_destroy(config);
					}
					/// free msg bus context
					if(msgbus_ctx != NULL)
					{
						msgbus_destroy(msgbus_ctx);
					}
					continue;
				}
				else
				{
					stZmqPubContext objTempPubCtx;
					objTempPubCtx.m_pContext= pub_ctx;
					zmq_handler::insertPubCTX(topic, objTempPubCtx);
				}
			}
			else
			{
				std::size_t pos = topic.find('/');
				if (std::string::npos != pos)
				{

					std::string subTopic(topic.substr(pos + 1));
					std::cout << __func__ << " Context created and stored for config for topic :: " << subTopic << std::endl;

					std::cout << "Topic for ZMQ subscribe is :: "<< subTopic <<  endl;

					/// add topic in list
					PublishJsonHandler::instance().insertSubTopicInList(subTopic);

					/// method to store msgbus_ctx as per the topic
					stZmqContext objTempCtx;
					objTempCtx.m_pContext = msgbus_ctx;

					zmq_handler::insertCTX(subTopic, objTempCtx);

					retVal = msgbus_subscriber_new(msgbus_ctx, subTopic.c_str(), NULL, &sub_ctx);
					if(retVal != MSG_SUCCESS)
					{
						/// cleanup
						std::cout <<__func__ << "Failed to create subscriber context. errno: "<< retVal << std::endl;
						zmq_handler::removeCTX(subTopic);

						/// free config context
						if(config != NULL)
						{
							config_destroy(config);
						}
						/// free msg bus context
						if(msgbus_ctx != NULL)
						{
							msgbus_destroy(msgbus_ctx);
						}
						continue;
					}
					else
					{
						stZmqSubContext objTempSubCtx;
						objTempSubCtx.sub_ctx= sub_ctx;
						zmq_handler::insertSubCTX(subTopic, objTempSubCtx);
					}
				}
			}
			DO_LOG_INFO("Context created and stored for config for topic :: " + topic);

		}
	}
	else
	{
		DO_LOG_ERROR("Context creation failed !! config manager client is empty!! ");
		std::cout << "Context creation failed !! config manager client is empty!! " <<endl;
	}
	DO_LOG_DEBUG("End: ");

	return retValue;
}

/**
 * Get sub context for topic
 * @param a_sTopic	:[in] topic to get sub context for
 * @return structure containing EIS contexts
 */
stZmqSubContext& zmq_handler::getSubCTX(std::string a_sTopic)
{
	DO_LOG_DEBUG("Start: " + a_sTopic);
	std::unique_lock<std::mutex> lck(__SubctxMapLock);

	/// return the request ID
	return g_mapSubContextMap.at(a_sTopic);
}

/**
 * Insert sub context
 * @param a_sTopic	:[in] insert context for topic
 * @param ctxRef	:[in] reference to context
 */
void zmq_handler::insertSubCTX(std::string a_sTopic, stZmqSubContext ctxRef)
{
	DO_LOG_DEBUG("Start: " + a_sTopic);
	std::unique_lock<std::mutex> lck(__SubctxMapLock);

	/// insert the data in map
	g_mapSubContextMap.insert(std::pair <std::string, stZmqSubContext> (a_sTopic, ctxRef));
	DO_LOG_DEBUG("End: ");
}

/**
 * Remove sub context
 * @param a_sTopic	:[in] remove sub context for topic
 */
void zmq_handler::removeSubCTX(std::string a_sTopic)
{
	DO_LOG_DEBUG("Start: " + a_sTopic);
	std::unique_lock<std::mutex> lck(__SubctxMapLock);

	g_mapSubContextMap.erase(a_sTopic);
	DO_LOG_DEBUG("End:");
}

/**
 * Get msgbus context for topic
 * @param a_sTopic :[in] topic for msgbus context
 * @return reference to structure containing contexts
 */
stZmqContext& zmq_handler::getCTX(std::string a_sTopic)
{
	DO_LOG_DEBUG("Start: " + a_sTopic);
	std::unique_lock<std::mutex> lck(__ctxMapLock);

	/// return the request ID
	return g_mapContextMap.at(a_sTopic);
}

/**
 * Insert msgbus context
 * @param a_sTopic	:[in] topic for which to insert msgbus context
 * @param ctxRef	:[in] msgbus context
 */
void zmq_handler::insertCTX(std::string a_sTopic, stZmqContext ctxRef)
{
	DO_LOG_DEBUG("Start: " + a_sTopic);
	std::unique_lock<std::mutex> lck(__ctxMapLock);

	/// insert the data in map
	g_mapContextMap.insert(std::pair <std::string, stZmqContext> (a_sTopic, ctxRef));
	DO_LOG_DEBUG("End: ");
}

/**
 * Remove msgbus context
 * @param a_sTopic	:[in] remove msgbus context for topic
 */
void zmq_handler::removeCTX(std::string a_sTopic)
{
	DO_LOG_DEBUG("Start: " + a_sTopic);
	std::unique_lock<std::mutex> lck(__ctxMapLock);

	g_mapContextMap.erase(a_sTopic);
	DO_LOG_DEBUG("End:");
}

/**
 * Get pub context
 * @param a_sTopic	:[in] topic for which to get pub context
 * @return reference to structure containing EIS contexts
 */
stZmqPubContext& zmq_handler::getPubCTX(std::string a_sTopic)
{
	DO_LOG_DEBUG("Start: " + a_sTopic);
	std::unique_lock<std::mutex> lck(__PubctxMapLock);

	DO_LOG_DEBUG("End: ");

	/// return the context
	return g_mapPubContextMap.at(a_sTopic);
}

/**
 * Insert pub contexts
 * @param a_sTopic	:[in] topic for which to insert pub context
 * @param ctxRef	:[in] context
 * @return 	true : on success,
 * 			false : on error
 */
bool zmq_handler::insertPubCTX(std::string a_sTopic, stZmqPubContext ctxRef)
{
	DO_LOG_DEBUG("Start: ");
	bool bRet = true;
	try
	{
		std::unique_lock<std::mutex> lck(__PubctxMapLock);

		/// insert the data
		g_mapPubContextMap.insert(std::pair <std::string, stZmqPubContext> (a_sTopic, ctxRef));
	}
	catch (exception &e)
	{
		DO_LOG_FATAL(e.what());
		bRet = false;
	}
	DO_LOG_DEBUG("End: ");

	return bRet;
}

/**
 * Remove pub context
 * @param a_sTopic	:[in] topic for which to remove context
 */
void zmq_handler::removePubCTX(std::string a_sTopic)
{
	DO_LOG_DEBUG("Start: " + a_sTopic);
	std::unique_lock<std::mutex> lck(__PubctxMapLock);
	g_mapPubContextMap.erase(a_sTopic);
	DO_LOG_DEBUG("End: ");
}
