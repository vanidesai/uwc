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
#include <eis/utils/json_config.h>

#include "EISMsgbusHandler.hpp"
#include "ConfigManager.hpp"

namespace
{
	std::map<std::string, stZmqContext> g_mapContextMap;
	std::map<std::string, stZmqSubContext> g_mapSubContextMap;
	std::map<std::string, stZmqPubContext> g_mapPubContextMap;
}

std::mutex __ctxMapLock;
std::mutex __PubctxMapLock;
std::mutex __SubctxMapLock;

bool CEISMsgbusHandler::prepareCommonContext(std::string topicType)
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


			if(topicType == "pub")
			{
				//app will publish data to zmq
				publisher_ctx_t *pub_ctx = NULL;
				msgbus_ret_t ret = msgbus_publisher_new(msgbus_ctx, topic.c_str(), &pub_ctx);

				if(ret != MSG_SUCCESS)
				{
					std::cout << __func__ << "Failed to initialize publisher errno: "<< ret << "\n";

					if(msgbus_ctx != NULL)
						msgbus_destroy(msgbus_ctx);

					if(config != NULL)
						config_destroy(config);

					continue;
				}

				// method to store msgbus_ctx as per the topic
				stZmqContext objTempCtx;
				objTempCtx.m_pContext = msgbus_ctx;

				if(false == insertCTX(topic, objTempCtx))
				{
					std::cout << __func__ << " Failed to insert context for topic  : " << topic << std::endl;

					if(msgbus_ctx != NULL)
						msgbus_destroy(msgbus_ctx);

					if(config != NULL)
						config_destroy(config);

					continue; //continue with next topic
				}
				std::cout << __func__ << " Context created and stored for config for topic :: " << topic << std::endl;

				stZmqPubContext objPubContext;
				objPubContext.m_pContext = pub_ctx;

				if(false == insertPubCTX(topic, objPubContext))
				{
					std::cout << __func__ << " Failed to insert pub context for topic  : " << topic << std::endl;
					if(config != NULL)
						config_destroy(config);

					//remove msgbus context
					removeCTX(topic);
				}
			}
			else
			{
				std::cout << __func__ << " Cfg client is not connected \n";
				retVal = msgbus_subscriber_new(msgbus_ctx, topic.c_str(), NULL, &sub_ctx);
				if(retVal != MSG_SUCCESS)
				{
					std::cout <<__func__ << "Failed to create subscriber context. errno: "<< retVal << std::endl;

					if(msgbus_ctx != NULL)
						msgbus_destroy(msgbus_ctx);

					if(config != NULL)
						config_destroy(config);

					continue;
				}
				else
				{
					// method to store msgbus_ctx as per the topic
					stZmqContext objTempCtx;
					objTempCtx.m_pContext = msgbus_ctx;

					if(false == insertCTX(topic, objTempCtx))
					{
						std::cout << __func__ << " Failed to insert context for topic  : " << topic << std::endl;
						if(msgbus_ctx != NULL)
							msgbus_destroy(msgbus_ctx);

						if(config != NULL)
							config_destroy(config);

						continue; //continue with next topic
					}
					std::cout << __func__ << " Context created and stored for config for topic :: " << topic << std::endl;

					stZmqSubContext objTempSubCtx;
					objTempSubCtx.m_pContext = sub_ctx;

					if(false == insertSubCTX(topic, objTempSubCtx))
					{
						std::cout << __func__ << " Failed to insert sub context for topic  : " << topic << std::endl;
						if(config != NULL)
							config_destroy(config);
						//remove msgbus context
						removeCTX(topic);
					}
				}
			}
			}
			catch(exception &e)
			{
				std::cout << __func__ << " Error::Exception is ::" << e.what() << " for topic:" << topic;
				retValue = false;
			}

		}
	}
	else {
		std::cout << __func__ << " Failed to prepare contexts as config manager client is not connected" << std::endl;
		retValue = false;
	}
	return retValue;
}

bool CEISMsgbusHandler::getCTX(std::string a_sTopic, stZmqContext& msgbusContext)
{
	std::unique_lock<std::mutex> lck(__ctxMapLock);

	//return the request ID
	if( g_mapContextMap.end() != g_mapContextMap.find(a_sTopic)) {
		msgbusContext = g_mapContextMap.at(a_sTopic);
		return true;
	}
	else
		return false;
}

bool CEISMsgbusHandler::insertCTX(std::string a_sTopic, stZmqContext ctxRef)
{
	std::cout << __func__ << " Start: " << a_sTopic << std::endl;
	try
	{
		std::unique_lock<std::mutex> lck(__ctxMapLock);

		/// insert the data in map
		g_mapContextMap.insert(std::pair <std::string, stZmqContext> (a_sTopic, ctxRef));
		std::cout<< "End: " << std::endl;
	}
	catch(exception &ex)
	{
		return false;
	}

	return true;
}

//gets called when error occurs while creating pub/sub topic context
void CEISMsgbusHandler::removeCTX(std::string a_sTopic)
{
	try{
		stZmqContext zmqCtx;
		if( getCTX(a_sTopic, zmqCtx)) {

			std::unique_lock<std::mutex> lck(__ctxMapLock);
			g_mapContextMap.erase(a_sTopic);

			if(zmqCtx.m_pContext != NULL) {

				msgbus_destroy(zmqCtx.m_pContext);
			}
		}
		std::cout << __func__ << " destroyed contexts for topic : " << a_sTopic << std::endl;
	}
	catch(exception &ex){
		std::cout << __func__ << " Exception : " << ex.what() << std::endl;
	}

}

bool CEISMsgbusHandler::getSubCTX(std::string a_sTopic, stZmqSubContext& subContext)
{
	std::unique_lock<std::mutex> lck(__SubctxMapLock);

	//return the context
	if( g_mapSubContextMap.end() != g_mapSubContextMap.find(a_sTopic)) {
		subContext = g_mapSubContextMap.at(a_sTopic);
		return true;
	}
	else
		return false;
}

bool CEISMsgbusHandler::insertSubCTX(std::string a_sTopic, stZmqSubContext ctxRef)
{
	bool bRet = true;
	try
	{
		std::unique_lock<std::mutex> lck(__SubctxMapLock);

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

void CEISMsgbusHandler::removeSubCTX(std::string a_sTopic, stZmqContext& zmqCtx)
{
	stZmqSubContext zmqSubCtx;
	if( getSubCTX(a_sTopic, zmqSubCtx)) {

		std::unique_lock<std::mutex> lck(__SubctxMapLock);

			g_mapSubContextMap.erase(a_sTopic);
			if((zmqCtx.m_pContext != NULL) && (zmqSubCtx.m_pContext != NULL))
				msgbus_recv_ctx_destroy(zmqCtx.m_pContext, zmqSubCtx.m_pContext);
		}
}

//////////////////sub topics//////////////////
bool CEISMsgbusHandler::getPubCTX(std::string a_sTopic, stZmqPubContext& pubContext)
{
	std::unique_lock<std::mutex> lck(__PubctxMapLock);
	// return the context
	if( g_mapPubContextMap.end() != g_mapPubContextMap.find(a_sTopic)) {
		pubContext = g_mapPubContextMap.at(a_sTopic);
		return true;
	}
	else
		return false;
}

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
		std::cout << __func__ <<  ":Error::Exception is ::" << e.what() << std::endl;
		bRet = false;
	}

	return bRet;
}

void CEISMsgbusHandler::removePubCTX(std::string a_sTopic, stZmqContext& zmqCtx)
{
		stZmqPubContext zmqPubCtx;
		if( getPubCTX(a_sTopic, zmqPubCtx)) {

			std::unique_lock<std::mutex> lck(__PubctxMapLock);

			g_mapPubContextMap.erase(a_sTopic);
			if((zmqCtx.m_pContext != NULL) &&  (zmqPubCtx.m_pContext != NULL))
				msgbus_publisher_destroy(zmqCtx.m_pContext, (publisher_ctx_t*) zmqPubCtx.m_pContext);
		}

}
/////////////////////////////////////////////

CEISMsgbusHandler::CEISMsgbusHandler(){

}


void CEISMsgbusHandler::cleanup() {
	std::cout << __func__ << " Destroying all contexts ...\n";

	try
	{
		//iterate over contexts maps and destroy
		for (auto it = g_mapContextMap.cbegin(); it != g_mapContextMap.cend();
				) {
			if (!it->first.empty()) {
				std::cout << __func__ << " destroying context for topic : "
						<< it->first << std::endl;

				stZmqContext zmqCtx;
				if (getCTX(it->first, zmqCtx)) {

					//check if it from sub or not
					removeSubCTX(it->first, zmqCtx);
					removePubCTX(it->first, zmqCtx);

					std::unique_lock<std::mutex> lck(__ctxMapLock);
					if (zmqCtx.m_pContext != NULL) {
						msgbus_destroy(zmqCtx.m_pContext);
						std::cout << __func__
								<< " destroyed contexts for topic : "
								<< it->first << std::endl;
					}
				}

				std::unique_lock<std::mutex> lck(__ctxMapLock);
				it = g_mapContextMap.erase(it);
			} else {
				++it;
			}
		}

	}catch(exception &ex) {
		std::cout << __func__ << " Exception : " << ex.what() << "\n";
	}
	std::cout << __func__ << " Destroyed all contexts !!\n";
}

CEISMsgbusHandler::~CEISMsgbusHandler() {
	// TODO Auto-generated destructor stub
}
