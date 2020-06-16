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
#include <functional>
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

// lamda function to return true if given key matches the given pattern
std::function<bool(std::string, std::string)> regExFun = [](std::string a_sTopic, std::string a_sKeyToFind) ->bool {
	if(std::string::npos != a_sTopic.find(a_sKeyToFind.c_str(),
			a_sTopic.length() - std::string(a_sKeyToFind).length(),
			std::string(a_sKeyToFind).length()))
	{
		return true;
	}
	else
	{
		return false;
	}
};

/**
 * Set the topic name as per specified patterns
 * @param a_sTopic		:[in] Topic for which pub or sub context needs to be created
 * @return 	true : on success,
 * 			false : on error
 */
bool zmq_handler::setTopicForOperation(std::string a_sTopic)
{
	bool bRet = true;
	if(regExFun(a_sTopic, READ_RES))
	{
		PublishJsonHandler::instance().setSReadResponseTopic(a_sTopic);
		cout << "read Res topic = " << a_sTopic << endl;
		DO_LOG_INFO("read res topic = " + a_sTopic);
	}
	else if(regExFun(a_sTopic, READ_RES_RT))
	{
		PublishJsonHandler::instance().setSReadResponseTopicRT(a_sTopic);
		cout << "read Res RT topic = " << a_sTopic << endl;
		DO_LOG_INFO("read res RT topic = " + a_sTopic);
	}
	else if(regExFun(a_sTopic, WRITE_RES))
	{
		PublishJsonHandler::instance().setSWriteResponseTopic(a_sTopic);
		cout << "write res topic = " << a_sTopic << endl;
		DO_LOG_INFO("write res topic = " + a_sTopic);
	}
	else if(regExFun(a_sTopic, WRITE_RES_RT))
	{
		PublishJsonHandler::instance().setSWriteResponseTopicRT(a_sTopic);
		cout << "write res RT topic = " << a_sTopic << endl;
		DO_LOG_INFO("write res RT topic = " + a_sTopic);
	}
	else if(regExFun(a_sTopic, POLLDATA))
	{
		PublishJsonHandler::instance().setPolledDataTopic(a_sTopic);
		cout << "poll topic = " << a_sTopic << endl;
		DO_LOG_INFO("poll topic = " + a_sTopic);
	}
	else if(regExFun(a_sTopic, POLLDATA_RT))
	{
		PublishJsonHandler::instance().setPolledDataTopicRT(a_sTopic);
		cout << "poll topic RT = " << a_sTopic << endl;
		DO_LOG_INFO("poll topic RT = " + a_sTopic);
	}
	else if(regExFun(a_sTopic, READ_REQ) or
			regExFun(a_sTopic, READ_REQ_RT) or
			regExFun(a_sTopic, WRITE_REQ) or
			regExFun(a_sTopic, WRITE_REQ_RT))
	{
		// Do nothing it just to check the correct pattern
	}
	else
	{
		DO_LOG_ERROR("Invalid topic name in SubTopics/PubTopics. hence ignoring :: " + a_sTopic);
		cout << "Invalid topic name in SubTopics/PubTopics. hence ignoring :: " << a_sTopic << endl;
		DO_LOG_ERROR("Kindly specify correct topic name as per specification :: " + a_sTopic);
		cout << "Kindly specify correct topic name as per specification :: " << a_sTopic << endl;
		bRet = false;
	}
	return bRet;
}
/**
 * Prepare pub or sub context for ZMQ communication
 * @param a_bIsPub		:[in] flag to check for Pub or Sub
 * @param msgbus_ctx	:[in] Common message bus context used for zmq communication
 * @param a_sTopic		:[in] Topic for which pub or sub context needs to be created
 * @param config		:[in] Config instance used for zmq library
 * @return 	true : on success,
 * 			false : on error
 */
bool zmq_handler::prepareContext(bool a_bIsPub,
		void* msgbus_ctx,
		string a_sTopic,
		config_t *config)
{
	bool bRetVal = false;
	msgbus_ret_t retVal = MSG_SUCCESS;
	publisher_ctx_t* pub_ctx = NULL;
	recv_ctx_t* sub_ctx = NULL;

	if(NULL == msgbus_ctx || NULL == config || a_sTopic.empty())
	{
		DO_LOG_ERROR("NULL pointers received while creating context for topic ::" + a_sTopic);
		goto err;
	}

	if(a_bIsPub)
	{
		retVal = msgbus_publisher_new(msgbus_ctx, a_sTopic.c_str(), &pub_ctx);
	}
	else
	{
		retVal = msgbus_subscriber_new(msgbus_ctx, a_sTopic.c_str(), NULL, &sub_ctx);
	}

	if(retVal != MSG_SUCCESS)
	{
		/// cleanup
		DO_LOG_ERROR("Failed to create publisher or subscriber for topic "+a_sTopic + " with error code:: "+std::to_string(retVal));
		cout << "ERROR:: Failed to create publisher or subscriber for topic : "<< a_sTopic<< " with error code:: "<< std::to_string(retVal)<<endl;
		goto err;
	}
	else
	{
		bRetVal = true;
		if(!setTopicForOperation(a_sTopic))
		{
			goto err;
		}

		stZmqContext objTempCtx{msgbus_ctx};
		zmq_handler::insertCTX(a_sTopic, objTempCtx);

		if(a_bIsPub)
		{
			stZmqPubContext objTempPubCtx;
			objTempPubCtx.m_pContext= pub_ctx;
			zmq_handler::insertPubCTX(a_sTopic, objTempPubCtx);
		}
		else
		{
			stZmqSubContext objTempSubCtx;
			objTempSubCtx.sub_ctx= sub_ctx;
			PublishJsonHandler::instance().insertSubTopicInList(a_sTopic);
			zmq_handler::insertSubCTX(a_sTopic, objTempSubCtx);
		}
	}

	return bRetVal;

err:
	// remove mgsbus context
	removeCTX(a_sTopic);

	if(NULL != pub_ctx && NULL != config)
	{
		msgbus_publisher_destroy(config, pub_ctx);
	}
	if(NULL != sub_ctx && NULL != config)
	{
		msgbus_recv_ctx_destroy(config, sub_ctx);
	}
	/// free msg bus context
	if(msgbus_ctx != NULL)
	{
		msgbus_destroy(msgbus_ctx);
		msgbus_ctx = NULL;
	}

	return false;
}

/**
 * Prepare all EIS contexts for zmq communications based on topic configured in
 * SubTopics or PubTopics section from docker-compose.yml file
 * Following is the sequence of context creation
 * 	1. Get the topic from SubTopics/PubTopics section
 * 	2. Create msgbus config
 * 	3. Create the msgbus context based on msgbus config
 * 	4. Once msgbus context is successful then create pub and sub context for zmq publisher/subscriber
 *
 * @param topicType	:[in] topic type to create context for, value is either "sub" or "pub"
 * @return 	true : on success,
 * 			false : on error
 */
bool zmq_handler::prepareCommonContext(std::string topicType)
{
	DO_LOG_DEBUG("Start:");
	bool retValue = false;
	size_t topic_count = 0;
	char **head = NULL;
	string topic = "";

	if(!(topicType == "pub" || topicType == "sub"))
	{
		DO_LOG_ERROR("Invalid TopicType parameter ::" + topicType);
		return retValue;
	}
	if(CfgManager::Instance().IsClientCreated())
	{
		/// parse all the topics
		char** ppcTopics = CfgManager::Instance().getEnvClient()->get_topics_from_env(topicType.c_str());

		if(NULL != ppcTopics)
		{
			// store the head node for string array
			head = ppcTopics;
			while (*ppcTopics != NULL)
			{
				++topic_count;
				topic = *ppcTopics;

				try
				{
					retValue = true;
					config_t* config = CfgManager::Instance().getEnvClient()->get_messagebus_config(
							CfgManager::Instance().getConfigClient(),
							ppcTopics , topic_count, topicType.c_str());
					if(config == NULL)
					{
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
						std::cout << "Topic for ZMQ Publish is :: "<< topic <<  endl;
						prepareContext(true, msgbus_ctx, topic, config);
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
							//PublishJsonHandler::instance().insertSubTopicInList(subTopic);
							prepareContext(false, msgbus_ctx, subTopic, config);
						}
					}
				}
				catch(exception &e)
				{
					DO_LOG_FATAL("Exception occurred for topic :" + topic + " with exception code:: " + e.what());
					std::cout << __func__ << ":" << __LINE__ << "Exception occurred for topic :" + topic +
							" with exception code:: " + e.what() << std::endl;
					retValue = false;
				}

				DO_LOG_INFO("Context created and stored for config for topic :: " + topic);

				// free used data
				if(*ppcTopics)
				{
					free (*ppcTopics);
					*ppcTopics = NULL;
				}

				// go to next topic
				ppcTopics++;
			}

			// free base pointer
			if(head)
			{
				free(head);
				head = NULL;
			}
		}
		else
		{
			DO_LOG_ERROR("topic list is empty");
			cout << "topic list is empty" << endl;
			return false;
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
void zmq_handler::insertCTX(std::string a_sTopic, stZmqContext& ctxRef)
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
