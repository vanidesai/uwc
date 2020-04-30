/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include <eis/utils/json_config.h>

#include "EISMsgbusHandler.hpp"
#include "MQTTSubscribeHandler.hpp"
/**
 * namespace for EIS contexts
 */
namespace
{
	std::map<std::string, stZmqContext> g_mapContextMap;
	std::map<std::string, stZmqSubContext> g_mapSubContextMap;
	std::map<std::string, stZmqPubContext> g_mapPubContextMap;
}

//mutexes for EIS contexts
std::mutex __ctxMapLock;
std::mutex __PubctxMapLock;
std::mutex __SubctxMapLock;

/**
 * Prepare EIS contexts for msg bus and topics
 * @param topicType :[in] topic type sub/pub or create context
 * @return true/false based on success/failure
 */
bool CEISMsgbusHandler::prepareCommonContext(std::string topicType)
{
	msgbus_ret_t retVal = MSG_SUCCESS;
	bool retValue = false;
	recv_ctx_t* sub_ctx = NULL;

	if(!(topicType == "pub" || topicType == "sub"))
	{
		DO_LOG_ERROR("Invalid TopicType parameter ::" + topicType);
		std::cout << __func__ << ":" << __LINE__ << " Error : Invalid TopicType parameter" + topicType << std::endl;
		return retValue;
	}
	if(CfgManager::Instance().IsClientCreated())
	{
		std::vector<std::string> Topics;

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
			try
			{
			retValue = true;
			config_t* config = CfgManager::Instance().getEnvClient()->get_messagebus_config(CfgManager::Instance().getConfigClient(),
					topic.c_str(), topicType.c_str());
			if(config == NULL) {
				DO_LOG_ERROR("Failed to get publisher message bus config ::" + topic);
				std::cout << __func__ << ":" << __LINE__ << " Error : Failed to get publisher message bus config ::" + topic <<  std::endl;
				continue;
			}

			void* msgbus_ctx = msgbus_initialize(config);
			if(msgbus_ctx == NULL)
			{
				DO_LOG_ERROR("Failed to get message bus context with config for topic ::" + topic);
				std::cout << __func__ << ":" << __LINE__ << " Error : Failed to get message bus context with config for topic ::" + topic <<  std::endl;
				config_destroy(config);
				continue;
			}


			if(topicType == "pub")
			{
				//app will publish data to zmq
				publisher_ctx_t *pub_ctx = NULL;
				msgbus_ret_t ret = msgbus_publisher_new(msgbus_ctx, topic.c_str(), &pub_ctx);

				if(ret != MSG_SUCCESS)
				{
					DO_LOG_ERROR("Failed to initialize publisher errno: " + std::to_string(ret));
					std::cout << __func__ << ":" << __LINE__ << " Error : Failed to initialize publisher errno: " + std::to_string(ret) <<  std::endl;

					if(config != NULL)
						config_destroy(config);

					if(msgbus_ctx != NULL)
						msgbus_destroy(msgbus_ctx);

					continue;
				}

				// method to store msgbus_ctx as per the topic
				stZmqContext objTempCtx;
				objTempCtx.m_pContext = msgbus_ctx;

				if(false == insertCTX(topic, objTempCtx))
				{
					DO_LOG_ERROR("Failed to insert context for topic  : " + topic);
					std::cout << __func__ << ":" << __LINE__ << " Error : Failed to insert context for topic  : " + topic <<  std::endl;

					if(config != NULL)
						config_destroy(config);

					if(msgbus_ctx != NULL)
						msgbus_destroy(msgbus_ctx);

					continue; //continue with next topic
				}
				DO_LOG_DEBUG("Context created and stored for config for topic :: " + topic);

				stZmqPubContext objPubContext;
				objPubContext.m_pContext = pub_ctx;

				if(false == insertPubCTX(topic, objPubContext))
				{
					DO_LOG_ERROR("Failed to insert pub context for topic  : " + topic);
					std::cout << __func__ << ":" << __LINE__ << " Error : Failed to insert pub context for topic  : " + topic <<  std::endl;
					if(config != NULL)
						config_destroy(config);

					//remove msgbus context
					removeCTX(topic);
				}
			}
			else
			{
				std::size_t pos = topic.find('/');
				if (std::string::npos != pos)
				{
					std::string subTopic(topic.substr(pos + 1));

					DO_LOG_DEBUG("Cfg client is not connected");
					retVal = msgbus_subscriber_new(msgbus_ctx, subTopic.c_str(), NULL, &sub_ctx);
					if(retVal != MSG_SUCCESS)
					{
						DO_LOG_ERROR("Failed to create subscriber context. errno: " + std::to_string(retVal));
						std::cout << __func__ << ":" << __LINE__ << " Error : Failed to create subscriber context. errno: " + std::to_string(retVal) <<  std::endl;

						if(config != NULL)
							config_destroy(config);

						if(msgbus_ctx != NULL)
							msgbus_destroy(msgbus_ctx);

						continue;
					}
					else
					{
						std::cout << "Topic for ZMQ subscribe is :: "<< subTopic <<  endl;

						// add topic in list
						CEISMsgbusHandler::Instance().insertSubTopicInList(subTopic);

						// method to store msgbus_ctx as per the topic
						stZmqContext objTempCtx;
						objTempCtx.m_pContext = msgbus_ctx;

						if(false == insertCTX(subTopic, objTempCtx))
						{
							DO_LOG_ERROR("Failed to insert context for topic  : " + subTopic);
							std::cout << __func__ << ":" << __LINE__ << " Error : Failed to insert context for topic  : " + subTopic <<  std::endl;

							if(config != NULL)
								config_destroy(config);

							if(msgbus_ctx != NULL)
								msgbus_destroy(msgbus_ctx);

							continue; //continue with next topic
						}
						DO_LOG_DEBUG("Context created and stored for config for topic :: " + subTopic);

						stZmqSubContext objTempSubCtx;
						objTempSubCtx.m_pContext = sub_ctx;

						if(false == insertSubCTX(subTopic, objTempSubCtx))
						{
							DO_LOG_DEBUG("Failed to insert sub context for topic  : " + subTopic);
							if(config != NULL)
								config_destroy(config);
							//remove msgbus context
							removeCTX(subTopic);
						}
					}
				}
			}
			}
			catch(exception &e)
			{
				DO_LOG_FATAL((string)e.what() +
							" for topic:" +
							topic);
				std::cout << __func__ << ":" << __LINE__ << " Exception : ";
				cout << e.what() << " for topic:" << topic;
				cout<< std::endl;
				retValue = false;
			}

		}
	}
	else
	{
		DO_LOG_ERROR("Context creation failed !! config manager client is empty!! ");
		std::cout << "Context creation failed !! config manager client is empty!! " <<endl;
	}
	return retValue;
}

/**
 * Retrieve message bus context for a given topic
 * @param a_sTopic 		:[in] topic name to get context
 * @param msgbusContext :[out] message bus context
 * @return true/false based on success/failure
 */
bool CEISMsgbusHandler::getCTX(std::string a_sTopic, stZmqContext& msgbusContext)
{
	std::unique_lock<std::mutex> lck(__ctxMapLock);

	//return the request ID
	if( g_mapContextMap.end() != g_mapContextMap.find(a_sTopic))
	{
		msgbusContext = g_mapContextMap.at(a_sTopic);
		return true;
	}
	else
		return false;
}

/**
 * Insert message bus context in map with topic as key
 * @param a_sTopic 	:[in] topic name to insert msg bus context
 * @param ctxRef 	:[out] context reference
 * @return true/false based on success/failure
 */
bool CEISMsgbusHandler::insertCTX(std::string a_sTopic, stZmqContext ctxRef)
{
	try
	{
		std::unique_lock<std::mutex> lck(__ctxMapLock);

		// insert the data in map
		g_mapContextMap.insert(std::pair <std::string, stZmqContext> (a_sTopic, ctxRef));
		DO_LOG_DEBUG("End:");
	}
	catch(exception &ex)
	{
		DO_LOG_FATAL(ex.what());
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << ex.what() << std::endl;

		return false;
	}

	return true;
}

/**
 * Remove message bus context if any error occurs while creating pub/sub topic context
 * @param a_sTopic :[in] topic name for which msg bus context is to be removed
 * @return None
 */
void CEISMsgbusHandler::removeCTX(std::string a_sTopic)
{
	try
	{
		stZmqContext zmqCtx;
		if( getCTX(a_sTopic, zmqCtx))
		{

			std::unique_lock<std::mutex> lck(__ctxMapLock);
			g_mapContextMap.erase(a_sTopic);

			if(zmqCtx.m_pContext != NULL)
			{
				msgbus_destroy(zmqCtx.m_pContext);
			}
		}
		DO_LOG_DEBUG("destroyed contexts for topic : " + a_sTopic);
	}
	catch(exception &ex)
	{
		DO_LOG_FATAL(ex.what());
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << ex.what() << std::endl;
	}
}

/**
 * Retrieve sub topic context for the given topic
 * @param a_sTopic 		:[in] topic name to get sub context
 * @param subContext 	:[out] sub context
 * @return true/false based on success/failure
 */
bool CEISMsgbusHandler::getSubCTX(std::string a_sTopic, stZmqSubContext& subContext)
{
	std::unique_lock<std::mutex> lck(__SubctxMapLock);

	//return the context
	if( g_mapSubContextMap.end() != g_mapSubContextMap.find(a_sTopic))
	{
		subContext = g_mapSubContextMap.at(a_sTopic);
		return true;
	}
	else
		return false;
}

/**
 * Insert a sub context in a map with topic name as key
 * @param a_sTopic 	:[in] topic name to insert sub context
 * @param ctxRef	:[in] context reference
 * @return true/false based on success/failure
 */
bool CEISMsgbusHandler::insertSubCTX(std::string a_sTopic, stZmqSubContext ctxRef)
{
	bool bRet = true;
	try
	{
		std::unique_lock<std::mutex> lck(__SubctxMapLock);

		// insert the data
		g_mapSubContextMap.insert(std::pair <std::string, stZmqSubContext> (a_sTopic, ctxRef));
	}
	catch (exception &e)
	{
		DO_LOG_FATAL(e.what());
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
		bRet = false;
	}

	return bRet;
}

/**
 * Remove sub context for given topic from map
 * @param a_sTopic	:[in] topic name to remove sub context
 * @param zmqCtx	:[in] sub context to remove
 * @return None
 */
void CEISMsgbusHandler::removeSubCTX(std::string a_sTopic, stZmqContext& zmqCtx)
{
	try
	{
		stZmqSubContext zmqSubCtx;
		if( getSubCTX(a_sTopic, zmqSubCtx))
		{
			std::unique_lock<std::mutex> lck(__SubctxMapLock);

			g_mapSubContextMap.erase(a_sTopic);
			if((zmqCtx.m_pContext != NULL) && (zmqSubCtx.m_pContext != NULL))
			{
				msgbus_recv_ctx_destroy(zmqCtx.m_pContext, zmqSubCtx.m_pContext);
			}
		}
	}
	catch(exception &ex)
	{
		DO_LOG_FATAL(ex.what());
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << ex.what() << std::endl;
	}
}

/**
 * Retrieve pub context for a given topic from map
 * @param a_sTopic 		:[in] topic name to get pub context
 * @param pubContext	:[out] pub context
 * @return true/false based on success/failure
 */
bool CEISMsgbusHandler::getPubCTX(std::string a_sTopic, stZmqPubContext& pubContext)
{
	std::unique_lock<std::mutex> lck(__PubctxMapLock);
	// return the context
	if( g_mapPubContextMap.end() != g_mapPubContextMap.find(a_sTopic))
	{
		pubContext = g_mapPubContextMap.at(a_sTopic);
		return true;
	}
	else
		return false;
}

/**
 * Insert pub context in a map with topic name as key
 * @param a_sTopic	:[in] topic name to insert pub context
 * @param ctxRef	:[in] pub context to insert
 * @return true/false based on success/failure
 */
bool CEISMsgbusHandler::insertPubCTX(std::string a_sTopic, stZmqPubContext ctxRef)
{
	bool bRet = true;
	try
	{
		std::unique_lock<std::mutex> lck(__PubctxMapLock);

		//insert the data
		g_mapPubContextMap.insert(std::pair <std::string, stZmqPubContext> (a_sTopic, ctxRef));
	}
	catch (exception &e)
	{
		DO_LOG_FATAL(e.what());
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
		bRet = false;
	}

	return bRet;
}

/**
 * Removes pub context from map with given topic
 * @param a_sTopic	:[in] topic name to remove pub context
 * @param zmqCtx	:[in] pub context
 * @return None
 */
void CEISMsgbusHandler::removePubCTX(std::string a_sTopic, stZmqContext& zmqCtx)
{
	try
	{
		stZmqPubContext zmqPubCtx;
		if( getPubCTX(a_sTopic, zmqPubCtx))
		{
			std::unique_lock<std::mutex> lck(__PubctxMapLock);

			g_mapPubContextMap.erase(a_sTopic);
			if((zmqCtx.m_pContext != NULL) &&  (zmqPubCtx.m_pContext != NULL))
			{
				msgbus_publisher_destroy(zmqCtx.m_pContext, (publisher_ctx_t*) zmqPubCtx.m_pContext);
			}
		}
	}
	catch(exception &ex)
	{
		DO_LOG_FATAL(ex.what());
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << ex.what() << std::endl;
	}

}

/**
 * Default constructor
 */
CEISMsgbusHandler::CEISMsgbusHandler()
{

}

/**
 * Destroys all the message bus and topic contexts from map
 * @param None
 * @return None
 */
void CEISMsgbusHandler::cleanup()
{
	DO_LOG_DEBUG("Destroying all contexts ...");

	try
	{
		//iterate over contexts maps and destroy
		for (auto it = g_mapContextMap.cbegin(); it != g_mapContextMap.cend(); )
		{
			if (!it->first.empty())
			{
				DO_LOG_DEBUG("destroying context for topic : " +
							it->first);

				stZmqContext zmqCtx;
				if (getCTX(it->first, zmqCtx))
				{
					//check if it from sub or not
					removeSubCTX(it->first, zmqCtx);
					removePubCTX(it->first, zmqCtx);

					std::unique_lock<std::mutex> lck(__ctxMapLock);
					if (zmqCtx.m_pContext != NULL)
					{
						msgbus_destroy(zmqCtx.m_pContext);
						DO_LOG_DEBUG("destroyed contexts for topic : "
								+ it->first);
					}
				}

				std::unique_lock<std::mutex> lck(__ctxMapLock);
				it = g_mapContextMap.erase(it);
			}
			else
			{
				++it;
			}
		}

	}
	catch(exception &ex)
	{
		DO_LOG_FATAL(ex.what());
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << ex.what() << std::endl;
	}
	DO_LOG_DEBUG("Destroyed all contexts");
}

/**
 * Default destructor
 */
CEISMsgbusHandler::~CEISMsgbusHandler()
{

}
