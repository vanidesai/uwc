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
 * Prepare pub or sub context for ZMQ communication
 * @param a_bIsPub		:[in] flag to check for Pub or Sub
 * @param msgbus_ctx	:[in] Common message bus context used for zmq communication
 * @param a_sTopic		:[in] Topic for which pub or sub context needs to be created
 * @param config		:[in] Config instance used for zmq library
 * @return 	true : on success,
 * 			false : on error
 */
bool CEISMsgbusHandler::prepareContext(bool a_bIsPub,
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

		stZmqContext objTempCtx;
		objTempCtx.m_pContext = msgbus_ctx;
		if(!insertCTX(a_sTopic, objTempCtx))
		{
			DO_LOG_ERROR("Failed to insert context for topic  : " + a_sTopic);
			goto err;
		}

		if(a_bIsPub)
		{
			stZmqPubContext objTempPubCtx;
			objTempPubCtx.m_pContext= pub_ctx;
			if(!insertPubCTX(a_sTopic, objTempPubCtx))
			{
				DO_LOG_ERROR("Failed to insert pub context for topic  : " + a_sTopic);
				goto err;
			}
		}
		else
		{
			stZmqSubContext objTempSubCtx;
			objTempSubCtx.m_pContext= sub_ctx;
			if(!insertSubCTX(a_sTopic, objTempSubCtx))
			{
				DO_LOG_ERROR("Failed to insert sub context for topic  : " + a_sTopic);
			}
		}
	}

	return bRetVal;

err:
	// remove mgsbus context
	removeCTX(a_sTopic);

	/// free msg bus context
	if(msgbus_ctx != NULL)
	{
		msgbus_destroy(msgbus_ctx);
		msgbus_ctx = NULL;
	}
	if(NULL != pub_ctx && NULL != config)
	{
		msgbus_publisher_destroy(config, pub_ctx);
	}
	if(NULL != sub_ctx && NULL != config)
	{
		msgbus_recv_ctx_destroy(config, sub_ctx);
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
bool CEISMsgbusHandler::prepareCommonContext(std::string topicType)
{
	bool retValue = false;
	char **head = NULL;
	size_t topic_count = 0;
	string topic = "";

	if(!(topicType == "pub" || topicType == "sub"))
	{
		DO_LOG_ERROR("Invalid TopicType parameter ::" + topicType);
		std::cout << __func__ << ":" << __LINE__ << " Error : Invalid TopicType parameter" + topicType << std::endl;
		return retValue;
	}
	if(CfgManager::Instance().IsClientCreated())
	{
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
							CEISMsgbusHandler::Instance().insertSubTopicInList(subTopic);
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
						DO_LOG_DEBUG("destroyed contexts");
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
