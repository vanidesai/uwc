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

using namespace zmq_handler;

std::mutex fileMutex;
std::mutex __ctxMapLock;
std::mutex __SubctxMapLock;
std::mutex __PubctxMapLock;
std::mutex __appSeqMapLock;
publisher_ctx_t* g_pub_ctx;

// Unnamed namespace to define globals
namespace
{
	std::map<std::string, stZmqContext> g_mapContextMap;
	std::map<std::string, stZmqSubContext> g_mapSubContextMap;
	std::map<std::string, stZmqPubContext> g_mapPubContextMap;

	std::map<unsigned short, std::string> g_mapAppSeq;
}

bool zmq_handler::prepareCommonContext(std::string topicType)
{
	BOOST_LOG_SEV(lg, debug) << __func__ << "Start: ";
	msgbus_ret_t retVal = MSG_SUCCESS;
	bool retValue = false;
	recv_ctx_t* sub_ctx = NULL;

	if(!(topicType != "pub" || topicType != "sub"))
	{
		BOOST_LOG_SEV(lg, error) << __func__ << " Invalid TopicType parameter ::" << topicType;
		return retValue;
	}
	if(CfgManager::Instance().IsClientCreated())
	{
		std::vector<std::string> Topics = CfgManager::Instance().getEnvConfig().get_topics_from_env(topicType);
		for (auto topic : Topics)
		{
			retValue = true;
			config_t* config = CfgManager::Instance().getEnvConfig().get_messagebus_config(topic, topicType);
			if(config == NULL) {
				BOOST_LOG_SEV(lg, error) << __func__ << " Failed to get publisher message bus config ::" << topic;
				continue;
			}

			void* msgbus_ctx = msgbus_initialize(config);
			if(msgbus_ctx == NULL)
			{
				BOOST_LOG_SEV(lg, error) << __func__ << " Failed to get message bus context with config for topic ::" << topic;
				config_destroy(config);
				continue;
			}

			/// method to store msgbus_ctx as per the topic
			stZmqContext objTempCtx;
			objTempCtx.m_pContext = msgbus_ctx;

			zmq_handler::insertCTX(topic, objTempCtx);
			if(topicType == "pub")
			{
				retVal = msgbus_publisher_new(msgbus_ctx, topic.c_str(), &g_pub_ctx);

				if(retVal != MSG_SUCCESS)
				{
					BOOST_LOG_SEV(lg, error) <<__func__ << "Failed to initialize publisher errno: "<< retVal;
				}
			}
			else
			{
				std::cout << __func__ << " Context created and stored for config for topic :: " << topic << std::endl;
				retVal = msgbus_subscriber_new(msgbus_ctx, topic.c_str(), NULL, &sub_ctx);
				if(retVal != MSG_SUCCESS)
				{
					std::cout <<__func__ << "Failed to create subscriber context. errno: "<< retVal << std::endl;
				}
				else
				{
					stZmqSubContext objTempSubCtx;
					objTempSubCtx.sub_ctx= sub_ctx;
					zmq_handler::insertSubCTX(topic, objTempSubCtx);
				}
			}
			BOOST_LOG_SEV(lg, info) << __func__ << " Context created and stored for config for topic :: " << topic;

		}
	}
	BOOST_LOG_SEV(lg, debug)<<__func__ << "End: ";

	return retValue;
}

stZmqSubContext& zmq_handler::getSubCTX(std::string a_sTopic)
{
	BOOST_LOG_SEV(lg, debug)<<__func__ << "Start: " << a_sTopic;
	std::unique_lock<std::mutex> lck(__SubctxMapLock);

	/// return the request ID
	return g_mapSubContextMap.at(a_sTopic);
}

void zmq_handler::insertSubCTX(std::string a_sTopic, stZmqSubContext ctxRef)
{
	BOOST_LOG_SEV(lg, debug)<<__func__ << "Start: " << a_sTopic;
	std::unique_lock<std::mutex> lck(__SubctxMapLock);

	/// insert the data in map
	g_mapSubContextMap.insert(std::pair <std::string, stZmqSubContext> (a_sTopic, ctxRef));
	BOOST_LOG_SEV(lg, debug)<<__func__ << "End: ";
}

void zmq_handler::removeSubCTX(std::string a_sTopic)
{
	BOOST_LOG_SEV(lg, debug)<<  __func__ << "Start: " << a_sTopic;
	std::unique_lock<std::mutex> lck(__SubctxMapLock);

	g_mapSubContextMap.erase(a_sTopic);
	BOOST_LOG_SEV(lg, debug)<<  __func__ << "End:";
}

stZmqContext& zmq_handler::getCTX(std::string a_sTopic)
{
	BOOST_LOG_SEV(lg, debug)<<__func__ << "Start: " << a_sTopic;
	std::unique_lock<std::mutex> lck(__ctxMapLock);

	/// return the request ID
	return g_mapContextMap.at(a_sTopic);
}

void zmq_handler::insertCTX(std::string a_sTopic, stZmqContext ctxRef)
{
	BOOST_LOG_SEV(lg, debug)<<__func__ << "Start: " << a_sTopic;
	std::unique_lock<std::mutex> lck(__ctxMapLock);

	/// insert the data in map
	g_mapContextMap.insert(std::pair <std::string, stZmqContext> (a_sTopic, ctxRef));
	BOOST_LOG_SEV(lg, debug)<<__func__ << "End: ";
}

void zmq_handler::removeCTX(std::string a_sTopic)
{
	BOOST_LOG_SEV(lg, debug)<<  __func__ << "Start: " << a_sTopic;
	std::unique_lock<std::mutex> lck(__ctxMapLock);

	g_mapContextMap.erase(a_sTopic);
	BOOST_LOG_SEV(lg, debug)<<  __func__ << "End:";
}

stZmqPubContext zmq_handler::getPubCTX(std::string a_sTopic)
{
	BOOST_LOG_SEV(lg, debug)<<  __func__ << "Start: " << a_sTopic;
	std::unique_lock<std::mutex> lck(__PubctxMapLock);

	BOOST_LOG_SEV(lg, debug)<<  __func__ << "End: ";

	/// return the context
	return g_mapPubContextMap.at(a_sTopic);
}

bool zmq_handler::insertPubCTX(std::string a_sTopic, stZmqPubContext ctxRef)
{
	BOOST_LOG_SEV(lg, debug)<<  __func__ << "Start: " ;
	bool bRet = true;
	try
	{
		std::unique_lock<std::mutex> lck(__PubctxMapLock);

		/// insert the data
		g_mapPubContextMap.insert(std::pair <std::string, stZmqPubContext> (a_sTopic, ctxRef));
	}
	catch (exception &e)
	{
		BOOST_LOG_SEV(lg, debug) << "Error::" << __func__ << "Exception is ::" << e.what();
		bRet = false;
	}
	BOOST_LOG_SEV(lg, debug)<<  __func__ << "End: ";

	return bRet;
}

void zmq_handler::removePubCTX(std::string a_sTopic)
{
	BOOST_LOG_SEV(lg, debug)<<  __func__ << "Start: " << a_sTopic;
	std::unique_lock<std::mutex> lck(__PubctxMapLock);
	g_mapPubContextMap.erase(a_sTopic);
	BOOST_LOG_SEV(lg, debug)<<  __func__ << "End: ";
}

std::string& zmq_handler::getAppSeq(unsigned short seqno)
{
	BOOST_LOG_SEV(lg, debug)<<  __func__ << "Start: " << seqno;
	std::unique_lock<std::mutex> lck(__appSeqMapLock);

	BOOST_LOG_SEV(lg, debug)<<  __func__ << "End: ";

	/// return the context
	return g_mapAppSeq.at(seqno);
}

bool zmq_handler::insertAppSeq(unsigned short seqno, std::string appseqno)
{
	BOOST_LOG_SEV(lg, debug)<<  __func__ << "Start: " ;
	bool bRet = true;
	try
	{
		std::unique_lock<std::mutex> lck(__appSeqMapLock);

		/// insert the data
		g_mapAppSeq.insert(std::pair <unsigned short, std::string> (seqno, appseqno));
	}
	catch (exception &e)
	{
		BOOST_LOG_SEV(lg, debug) << "Error::" << __func__ << "Exception is ::" << e.what();
		bRet = false;
	}
	BOOST_LOG_SEV(lg, debug)<<  __func__ << "End: ";

	return bRet;
}

void zmq_handler::removeAppSeq(unsigned short seqno)
{
	BOOST_LOG_SEV(lg, debug)<<  __func__ << "Start: " << seqno;
	std::unique_lock<std::mutex> lck(__appSeqMapLock);
	g_mapAppSeq.erase(seqno);
	BOOST_LOG_SEV(lg, debug)<<  __func__ << "End: ";
}
