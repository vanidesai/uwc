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

using namespace eis::config_manager;
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
		std::string a_sTopic,
		config_t *config)
{
	bool bRetVal = false;
	msgbus_ret_t retVal = MSG_SUCCESS;
	publisher_ctx_t* pub_ctx = NULL;
	recv_ctx_t* sub_ctx = NULL;

	if(NULL == msgbus_ctx || NULL == config || a_sTopic.empty())
	{
		DO_LOG_ERROR("NULL pointers received while creating context for topic ::" + a_sTopic);
		std::cout<<"[preparecontext]NULL pointers received while creating context for topic\n";
		return false;
	}

	if(a_bIsPub)
	{
		retVal = msgbus_publisher_new(msgbus_ctx, a_sTopic.c_str(), &pub_ctx);
	}
	else // else if sub
	{
		retVal = msgbus_subscriber_new(msgbus_ctx, a_sTopic.c_str(), NULL, &sub_ctx);
	}

	if(retVal != MSG_SUCCESS)
	{
		/// cleanup
		DO_LOG_ERROR("Failed to create publisher or subscriber for topic "+a_sTopic + " with error code:: "+std::to_string(retVal));
		std::cout << "ERROR:: Failed to create publisher or subscriber for topic : "<< a_sTopic<< " with error code:: "<< std::to_string(retVal)<<std::endl;
		return false;
	}
	else
	{
		bRetVal = true;
		std::cout<<"###################before insertCTX################ in prepareCntext()\n"<<a_sTopic.c_str()<<std::endl;
		stZmqContext objTempCtx{msgbus_ctx};
		zmq_handler::insertCTX(a_sTopic, objTempCtx);
		std::cout<<"###################After insertCTX################ in prepareCntext()\n";
		std::cout<<"$$$$$$$$$$$###########-a_bIsPub="<<a_bIsPub<<"######\n";
		if(a_bIsPub)
		{
			std::cout<<"################[prepareContext]: inside PUB if##########\n";
			stZmqPubContext objTempPubCtx;
			objTempPubCtx.m_pContext= pub_ctx;
			zmq_handler::insertPubCTX(a_sTopic, objTempPubCtx);
			std::cout<<"################[prepareContext]: inside PUB if after insertpub##########\n";
		}
		else
		{
			std::cout<<"################[prepareContext]: inside SUB else##########\n";
			stZmqSubContext objTempSubCtx;
			objTempSubCtx.sub_ctx= sub_ctx;
			zmq_handler::insertSubCTX(a_sTopic, objTempSubCtx);
			std::cout<<"################[prepareContext]: inside SUB else after insertSub##########\n";
		}
	}

	return bRetVal;

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

	PublisherCfg* pub_ctx;
	SubscriberCfg* sub_ctx;
	config_t* pub_config;
	config_t* sub_config;
	void* g_msgbus_ctx = NULL;

	if(!(topicType == "pub" || topicType == "sub"))
	{
		DO_LOG_ERROR("Invalid TopicType parameter ::" + topicType);
		return retValue;
	}

	fprintf(stderr,"############################## INSIED prepareCommonContext JUST BEFORE IF ELSE OF PUB SUB STARTS  ##############################");
	std::cout<<"############################## INSIDE prepareCommonContext JUST BEFORE IF ELSE OF PUB SUB STARTS ##############################"<<std::endl;

	// This check if EII cfgmgr is created or not
	if(CfgManager::Instance().IsClientCreated())
	{
		if(topicType == "pub") {
			int numPublishers = CfgManager::Instance().getEiiCfgMgr()->getNumPublishers();
			for(auto it =0; it<numPublishers; ++it) {
				std::cout<<"\nTEMP CHECK\n";
				pub_ctx = CfgManager::Instance().getEiiCfgMgr()->getPublisherByIndex(it);	// ngk
				pub_config = pub_ctx->getMsgBusConfig();
    			if (pub_config == NULL) {
        			DO_LOG_ERROR("Failed to get message bus config");
					fprintf(stderr,"############################## PUB PART Failed to get message bus config ##############################");
					std::cout<<"##############################PUB PART Failed to get message bus config##############################"<<std::endl;
        			return false;
    			}

				config_value_t* interface_value = pub_ctx->getInterfaceValue("Name");
				std::cout<<"###########################PUB PART interface_value is"<<interface_value->body.string<<"######################"<<std::endl;

				//LOG_INFO("Obtained interface value: %s", interface_value->body.string)

				g_msgbus_ctx = msgbus_initialize(pub_config);
    			if (g_msgbus_ctx == NULL) {
        			DO_LOG_ERROR("Failed to initialize message bus");
					fprintf(stderr,"##############################PUB PART Failed to initialize message bus##############################");
					std::cout<<"##############################PUB PART Failed to initialize message bus##############################"<<std::endl;
        			return false;
    			}

				fprintf(stderr,"##############################PUB PART After msgbus_initialize() but before getTopics() ##############################");
				std::cout<<"############################## PUB PART After msgbus_initialize() but before getTopics()##############################"<<std::endl;

				std::vector<std::string> topics = pub_ctx->getTopics();
				if(topics.empty()){
        			DO_LOG_ERROR("Failed to get topics");
					fprintf(stderr,"############################## PUB PART Failed to get topics ##############################");
					std::cout<<"############################## PUB PART Failed to get topics ##############################"<<std::endl;
        			return false;
    			}

				fprintf(stderr,"##############################JUST OUTSIDE PUB PART prepareContext() for loop ##############################");
				std::cout<<"##############################JUST OUTSIDE PUB PART prepareContext() for loop ##############################"<<std::endl;

				for (auto topic_it = 0; topic_it < topics.size(); topic_it++) {
					fprintf(stderr,"##############################JUST INSIDE PUB PART prepareContext() for loop ##############################");
				std::cout<<"##############################JUST INSIDE PUB PART prepareContext() for loop ##############################"<<std::endl;
     				std::string ind_topic = topics.at(topic_it);
					 DO_LOG_INFO("Topic for ZMQ Publish is :: " + ind_topic);
					prepareContext(true, g_msgbus_ctx, ind_topic, pub_config);
					fprintf(stderr,"############################## PUB PART after  prepareContext() call inside for loop ##############################");
				std::cout<<"############################## PUB PART after  prepareContext() call inside for loop ##############################"<<std::endl;
    			}
				
			}
		} else {  // else if its sub
			fprintf(stderr,"############################## SUB else PART before getNumSubscribers ##############################");
			std::cout<<"############################## SUB else PART before getNumSubscribers ##############################"<<std::endl;
			int numSubscribers = CfgManager::Instance().getEiiCfgMgr()->getNumSubscribers();
			fprintf(stderr,"############################## SUB else PART after getNumSubscribers ##############################");
			std::cout<<"############################## SUB else PART after getNumSubscribers ##############################"<<std::endl;	
			for(auto it =0; it<numSubscribers; ++it) {
			fprintf(stderr,"@@############################## SUB else PART before getSubscriberByIndex ##############################");
			std::cout<<"@@############################## SUB else PART before getSubscriberByIndex ##############################"<<std::endl;	
			sub_ctx = CfgManager::Instance().getEiiCfgMgr()->getSubscriberByIndex(it);
			fprintf(stderr,"@@############################## SUB else PART after getSubscriberByIndex ##############################");
			std::cout<<"@@############################## SUB else PART after getSubscriberByIndex ##############################"<<std::endl;		
			sub_config = sub_ctx->getMsgBusConfig();
    		if (sub_config == NULL) {
        		DO_LOG_ERROR("Failed to get message bus config");
				fprintf(stderr,"##############################Failed to get message bus config##############################");
				std::cout<<"##############################Failed to get message bus config##############################"<<std::endl;
        		return false;
    			}
				fprintf(stderr,"@@############################## SUB else PART after getMsgBusConfig() ##############################");
				std::cout<<"@@############################## SUB else PART after getMsgBusConfig() ##############################"<<std::endl;		
				g_msgbus_ctx = msgbus_initialize(sub_config);
    			if (g_msgbus_ctx == NULL) {
        			LOG_ERROR_0("Failed to initialize message bus");
					fprintf(stderr,"##############################Failed to initialize message bus##############################");
					std::cout<<"##############################Failed to initialize message bus##############################"<<std::endl;
        			return false;
    			}
				std::vector<std::string> topics = sub_ctx->getTopics();
				if(topics.empty()){
        			DO_LOG_ERROR("Failed to get topics");
					fprintf(stderr,"##############################Failed to get topics##############################");
					std::cout<<"##############################Failed to get topics##############################"<<std::endl;
        			return false;
    			}
				for (auto topic_it = 0; topic_it < topics.size(); topic_it++) {
     				std::string ind_topic = topics.at(topic_it);
					prepareContext(false, g_msgbus_ctx, ind_topic, sub_config);
    			}
			}
		}	// end of sub part else
	} // end of if eii configmgr created if() 
	else
	{
		DO_LOG_ERROR("EII Configmgr creation failed !! ");
		std::cout << "EII Configmgr creation failed !!  " <<endl;
	}
	DO_LOG_DEBUG("End: ");
	fprintf(stderr,"##############################Just before returning in end of preparecommoncontext##############################");
	std::cout<<"##############################Just before returning in end of preparecommoncontext##############################"<<retValue<<std::endl;
	return true;
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
	std:;cout<<"###########getSubCTX########## TOPIC IS="<<a_sTopic<<"###"<<std::endl;
	std::cout<<"######size of g_mapSubContextMap="<<g_mapSubContextMap.size()<<"######\n";
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
	std::cout<<"$$$$$$$$$$-TOPIC INSERTED IN insertSubCTX is"<<a_sTopic<<"$$$$$$$$$$$$$$$\n";
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
	std::cout<<"#############[getCTX]: a_stopic="<<a_sTopic<<"######No: of keys in a_stopic map is="<<g_mapContextMap.size()<<std::endl;
	
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
	catch (std::exception &e)
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

/**
 * Publish json
 * @param a_sUsec		:[out] USEC timestamp value at which a message is published
 * @param msg			:[in] message to publish
 * @param a_sTopic		:[in] topic on which to publish
 * @return 	true : on success,
 * 			false : on error
 */
bool zmq_handler::publishJson(std::string &a_sUsec, msg_envelope_t* msg, const std::string &a_sTopic, std::string a_sPubTimeField)
{
	if(NULL == msg)
	{
		DO_LOG_ERROR(": Failed to publish message - Input message is NULL");
		return false;
	}

	DO_LOG_DEBUG("msg to publish :: Topic :: " + a_sTopic);

	zmq_handler::stZmqContext& msgbus_ctx = zmq_handler::getCTX(a_sTopic);
	void* pub_ctx = zmq_handler::getPubCTX(a_sTopic).m_pContext;
	if((NULL == msgbus_ctx.m_pContext) || (NULL == pub_ctx))
	{
		DO_LOG_ERROR(": Failed to publish message - context is NULL: " + a_sTopic);
		return false;
	}

	msgbus_ret_t ret;

	{
		std::lock_guard<std::mutex> lock(msgbus_ctx.m_mutex);
		if(a_sPubTimeField.empty() == false)
		{
			auto p1 = std::chrono::system_clock::now();
			unsigned long uTime = (unsigned long)(std::chrono::duration_cast<std::chrono::microseconds>(p1.time_since_epoch()).count());
			a_sUsec = std::to_string(uTime);
			msg_envelope_elem_body_t* ptUsec = msgbus_msg_envelope_new_string(a_sUsec.c_str());
			if(NULL != ptUsec)
			{
				msgbus_msg_envelope_put(msg, a_sPubTimeField.c_str(), ptUsec);
			}
		}
		ret = msgbus_publisher_publish(msgbus_ctx.m_pContext, (publisher_ctx_t*)pub_ctx, msg);
	}

	if(ret != MSG_SUCCESS)
	{
		DO_LOG_ERROR(" Failed to publish message errno: " + std::to_string(ret));
		return false;
	}
	return true;
}

bool zmq_handler::returnAllTopics(std::string topicType, std::vector<std::string>& vecTopics) {
	int numPubsOrSubs;
	if(topicType == "pub") {
		numPubsOrSubs = CfgManager::Instance().getEiiCfgMgr()->getNumPublishers();
	} else if(topicType == "sub") {
		numPubsOrSubs = CfgManager::Instance().getEiiCfgMgr()->getNumSubscribers();
	}
	
	for(auto pub_or_sub_id=0; pub_or_sub_id<numPubsOrSubs; ++pub_or_sub_id) {
		std::vector<std::string> topics;
		if(topicType == "pub") {
			PublisherCfg* pub_ctx = CfgManager::Instance().getEiiCfgMgr()->getPublisherByIndex(pub_or_sub_id);
			topics = pub_ctx->getTopics();
		} else {
			SubscriberCfg* sub_ctx = CfgManager::Instance().getEiiCfgMgr()->getSubscriberByIndex(pub_or_sub_id);
			topics = sub_ctx->getTopics();
		}
		
		if(topics.empty()){
        DO_LOG_ERROR("Failed to get topics");
        return false;
    	}
		size_t numTopics = topics.size(); // num of topics in indivisual publisher or subscriber
		for(size_t indv_topic=0; indv_topic < numTopics; ++indv_topic) {
			vecTopics.push_back(topics[indv_topic]);
		}
		// vecTopics = TCP_PolledData, 
		// return true if everything goes well. 
		return true;
	}
}

size_t zmq_handler::getNumPubOrSub(std::string topicType) {
	size_t count = 0;
	if(topicType == "pub") {
		count = CfgManager::Instance().getEiiCfgMgr()->getNumPublishers();
	} else {
		count = CfgManager::Instance().getEiiCfgMgr()->getNumSubscribers();
	}
	return count;
}