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
		CLogger::getInstance().log(ERROR, LOGDETAILS("Invalid TopicType parameter ::" + topicType));
		std::cout << __func__ << ":" << __LINE__ << " Error : Invalid TopicType parameter" + topicType << std::endl;
		return retValue;
	}
	if(CfgManager::Instance().IsClientCreated())
	{
		std::vector<std::string> Topics;// = CfgManager::Instance().getEnvConfig().get_topics_from_env(topicType);

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
			CLogger::getInstance().log(ERROR, LOGDETAILS("topic list is empty"));
			cout << "topic list is empty" << endl;
			return false;
		}

		for (auto topic : Topics)
		{
			try
			{
			retValue = true;
			config_t* config = CfgManager::Instance().getEnvClient()->get_messagebus_config(CfgManager::Instance().getConfigClient(),
					topic.c_str(), topicType.c_str());
			if(config == NULL) {
				CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to get publisher message bus config ::" + topic));
				std::cout << __func__ << ":" << __LINE__ << " Error : Failed to get publisher message bus config ::" + topic <<  std::endl;
				continue;
			}

			void* msgbus_ctx = msgbus_initialize(config);
			if(msgbus_ctx == NULL)
			{
				CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to get message bus context with config for topic ::" + topic));
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
					CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to initialize publisher errno: " + std::to_string(ret)));
					std::cout << __func__ << ":" << __LINE__ << " Error : Failed to initialize publisher errno: " + std::to_string(ret) <<  std::endl;
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
					CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to insert context for topic  : " + topic));
					std::cout << __func__ << ":" << __LINE__ << " Error : Failed to insert context for topic  : " + topic <<  std::endl;
					if(msgbus_ctx != NULL)
						msgbus_destroy(msgbus_ctx);

					if(config != NULL)
						config_destroy(config);

					continue; //continue with next topic
				}
				CLogger::getInstance().log(DEBUG, LOGDETAILS("Context created and stored for config for topic :: " + topic));

				stZmqPubContext objPubContext;
				objPubContext.m_pContext = pub_ctx;

				if(false == insertPubCTX(topic, objPubContext))
				{
					CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to insert pub context for topic  : " + topic));
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

					CLogger::getInstance().log(DEBUG, LOGDETAILS("Cfg client is not connected"));
					retVal = msgbus_subscriber_new(msgbus_ctx, subTopic.c_str(), NULL, &sub_ctx);
					if(retVal != MSG_SUCCESS)
					{
						CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to create subscriber context. errno: " + std::to_string(retVal)));
						std::cout << __func__ << ":" << __LINE__ << " Error : Failed to create subscriber context. errno: " + std::to_string(retVal) <<  std::endl;
						if(msgbus_ctx != NULL)
							msgbus_destroy(msgbus_ctx);

						if(config != NULL)
							config_destroy(config);

						continue;
					}
					else
					{
						std::cout << "Topic for ZMQ subscribe is :: "<< subTopic <<  endl;

						/// add topic in list
						CEISMsgbusHandler::Instance().insertSubTopicInList(subTopic);

						// method to store msgbus_ctx as per the topic
						stZmqContext objTempCtx;
						objTempCtx.m_pContext = msgbus_ctx;

						if(false == insertCTX(subTopic, objTempCtx))
						{
							CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to insert context for topic  : " + subTopic));
							std::cout << __func__ << ":" << __LINE__ << " Error : Failed to insert context for topic  : " + subTopic <<  std::endl;
							if(msgbus_ctx != NULL)
								msgbus_destroy(msgbus_ctx);

							if(config != NULL)
								config_destroy(config);

							continue; //continue with next topic
						}
						CLogger::getInstance().log(DEBUG, LOGDETAILS("Context created and stored for config for topic :: " + subTopic));

						stZmqSubContext objTempSubCtx;
						objTempSubCtx.m_pContext = sub_ctx;

						if(false == insertSubCTX(subTopic, objTempSubCtx))
						{
							CLogger::getInstance().log(DEBUG, LOGDETAILS("Failed to insert sub context for topic  : " + subTopic));
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
				string temp = e.what();
				temp.append(" for topic:");
				temp.append(topic);
				CLogger::getInstance().log(FATAL, LOGDETAILS(temp));
				std::cout << __func__ << ":" << __LINE__ << " Exception : " << temp << std::endl;
				retValue = false;
			}

		}
	}
	else {
		CLogger::getInstance().log(DEBUG, LOGDETAILS("Failed to prepare contexts as config manager client is not connected"));
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
	CLogger::getInstance().log(DEBUG, LOGDETAILS(" Start: " + a_sTopic));
	try
	{
		std::unique_lock<std::mutex> lck(__ctxMapLock);

		/// insert the data in map
		g_mapContextMap.insert(std::pair <std::string, stZmqContext> (a_sTopic, ctxRef));
		CLogger::getInstance().log(DEBUG, LOGDETAILS("End:"));
	}
	catch(exception &ex)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(ex.what()));
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << ex.what() << std::endl;

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
		CLogger::getInstance().log(DEBUG, LOGDETAILS("destroyed contexts for topic : " + a_sTopic));
	}
	catch(exception &ex){
		CLogger::getInstance().log(FATAL, LOGDETAILS(ex.what()));
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << ex.what() << std::endl;
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
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
		bRet = false;
	}

	return bRet;
}

void CEISMsgbusHandler::removeSubCTX(std::string a_sTopic, stZmqContext& zmqCtx)
{
	try
	{
	stZmqSubContext zmqSubCtx;
	if( getSubCTX(a_sTopic, zmqSubCtx)) {

		std::unique_lock<std::mutex> lck(__SubctxMapLock);

			g_mapSubContextMap.erase(a_sTopic);
			if((zmqCtx.m_pContext != NULL) && (zmqSubCtx.m_pContext != NULL))
				msgbus_recv_ctx_destroy(zmqCtx.m_pContext, zmqSubCtx.m_pContext);
		}
	}catch(exception &ex) {
		CLogger::getInstance().log(FATAL, LOGDETAILS(ex.what()));
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << ex.what() << std::endl;
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
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
		bRet = false;
	}

	return bRet;
}

void CEISMsgbusHandler::removePubCTX(std::string a_sTopic, stZmqContext& zmqCtx)
{
	try
	{
		stZmqPubContext zmqPubCtx;
		if( getPubCTX(a_sTopic, zmqPubCtx)) {

			std::unique_lock<std::mutex> lck(__PubctxMapLock);

			g_mapPubContextMap.erase(a_sTopic);
			if((zmqCtx.m_pContext != NULL) &&  (zmqPubCtx.m_pContext != NULL))
				msgbus_publisher_destroy(zmqCtx.m_pContext, (publisher_ctx_t*) zmqPubCtx.m_pContext);
		}
	}catch(exception &ex) {
		CLogger::getInstance().log(FATAL, LOGDETAILS(ex.what()));
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << ex.what() << std::endl;
	}

}
/////////////////////////////////////////////

CEISMsgbusHandler::CEISMsgbusHandler(){

}


void CEISMsgbusHandler::cleanup() {
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Destroying all contexts ..."));

	try
	{
		//iterate over contexts maps and destroy
		for (auto it = g_mapContextMap.cbegin(); it != g_mapContextMap.cend();
				) {
			if (!it->first.empty()) {
				string temp = "destroying context for topic : ";
				temp.append(it->first);
				CLogger::getInstance().log(DEBUG, LOGDETAILS(temp));

				stZmqContext zmqCtx;
				if (getCTX(it->first, zmqCtx)) {

					//check if it from sub or not
					removeSubCTX(it->first, zmqCtx);
					removePubCTX(it->first, zmqCtx);

					std::unique_lock<std::mutex> lck(__ctxMapLock);
					if (zmqCtx.m_pContext != NULL) {
						msgbus_destroy(zmqCtx.m_pContext);
						CLogger::getInstance().log(DEBUG, LOGDETAILS("destroyed contexts for topic : "
								+ it->first));
					}
				}

				std::unique_lock<std::mutex> lck(__ctxMapLock);
				it = g_mapContextMap.erase(it);
			} else {
				++it;
			}
		}

	}catch(exception &ex) {
		CLogger::getInstance().log(FATAL, LOGDETAILS(ex.what()));
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << ex.what() << std::endl;

	}
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Destroyed all contexts"));

}

CEISMsgbusHandler::~CEISMsgbusHandler() {
	// TODO Auto-generated destructor stub
}
