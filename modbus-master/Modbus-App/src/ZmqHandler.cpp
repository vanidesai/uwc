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
std::mutex __appSeqMapLock;

// Unnamed namespace to define globals
namespace
{
	std::map<std::string, stZmqContext> g_mapContextMap;
	std::map<std::string, stZmqSubContext> g_mapSubContextMap;
	std::map<std::string, stZmqPubContext> g_mapPubContextMap;

	std::map<unsigned short, stOnDemandRequest> g_mapAppSeq;
}

void zmq_handler::getTimeParams(std::string &a_sTimeStamp, std::string &a_sUsec)
{
	a_sTimeStamp.clear();
	a_sUsec.clear();

	const auto p1 = std::chrono::system_clock::now();

	std::time_t rawtime = std::chrono::system_clock::to_time_t(p1);
	std::tm* timeinfo = std::gmtime(&rawtime);
	if(NULL == timeinfo)
	{
		return;
	}
	char buffer [80];

	std::strftime(buffer,80,"%Y-%m-%d %H:%M:%S",timeinfo);
	a_sTimeStamp.insert(0, buffer);

	{
		std::stringstream ss;
		ss << std::chrono::duration_cast<std::chrono::microseconds>(p1.time_since_epoch()).count();
		a_sUsec.insert(0, ss.str());
	}
}

bool zmq_handler::prepareCommonContext(std::string topicType)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start:"));
	msgbus_ret_t retVal = MSG_SUCCESS;
	bool retValue = false;
	recv_ctx_t* sub_ctx = NULL;
	publisher_ctx_t* pub_ctx = NULL;

	if(!(topicType != "pub" || topicType != "sub"))
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS("Invalid TopicType parameter ::" + topicType));
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
			CLogger::getInstance().log(ERROR, LOGDETAILS("topic list is empty"));
			cout << "topic list is empty" << endl;
			return false;
		}

		for (auto topic : Topics)
		{
			retValue = true;
			config_t* config = CfgManager::Instance().getEnvClient()->get_messagebus_config(CfgManager::Instance().getConfigClient(), topic.c_str(), topicType.c_str());
			if(config == NULL) {
				CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to get publisher message bus config ::" + topic));
				continue;
			}

			void* msgbus_ctx = msgbus_initialize(config);
			if(msgbus_ctx == NULL)
			{
				CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to get message bus context with config for topic ::" + topic));
				config_destroy(config);
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
					CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to initialize publisher errno: " + std::to_string(retVal)));
					zmq_handler::removeCTX(topic);
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
						std::cout <<__func__ << "Failed to create subscriber context. errno: "<< retVal << std::endl;
						zmq_handler::removeCTX(subTopic);
					}
					else
					{
						stZmqSubContext objTempSubCtx;
						objTempSubCtx.sub_ctx= sub_ctx;
						zmq_handler::insertSubCTX(subTopic, objTempSubCtx);
					}
				}
			}
			CLogger::getInstance().log(INFO, LOGDETAILS("Context created and stored for config for topic :: " + topic));

		}
	}
	else
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS("Context creation failed !! config manager client is empty!! "));
		std::cout << "Context creation failed !! config manager client is empty!! " <<endl;
	}
	CLogger::getInstance().log(DEBUG, LOGDETAILS("End: "));

	return retValue;
}

stZmqSubContext& zmq_handler::getSubCTX(std::string a_sTopic)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start: " + a_sTopic));
	std::unique_lock<std::mutex> lck(__SubctxMapLock);

	/// return the request ID
	return g_mapSubContextMap.at(a_sTopic);
}

void zmq_handler::insertSubCTX(std::string a_sTopic, stZmqSubContext ctxRef)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start: " + a_sTopic));
	std::unique_lock<std::mutex> lck(__SubctxMapLock);

	/// insert the data in map
	g_mapSubContextMap.insert(std::pair <std::string, stZmqSubContext> (a_sTopic, ctxRef));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("End: "));
}

void zmq_handler::removeSubCTX(std::string a_sTopic)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start: " + a_sTopic));
	std::unique_lock<std::mutex> lck(__SubctxMapLock);

	g_mapSubContextMap.erase(a_sTopic);
	CLogger::getInstance().log(DEBUG, LOGDETAILS("End:"));
}

stZmqContext& zmq_handler::getCTX(std::string a_sTopic)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start: " + a_sTopic));
	std::unique_lock<std::mutex> lck(__ctxMapLock);

	/// return the request ID
	return g_mapContextMap.at(a_sTopic);
}

void zmq_handler::insertCTX(std::string a_sTopic, stZmqContext ctxRef)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start: " + a_sTopic));
	std::unique_lock<std::mutex> lck(__ctxMapLock);

	/// insert the data in map
	g_mapContextMap.insert(std::pair <std::string, stZmqContext> (a_sTopic, ctxRef));
	CLogger::getInstance().log(DEBUG, LOGDETAILS("End: "));
}

void zmq_handler::removeCTX(std::string a_sTopic)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start: " + a_sTopic));
	std::unique_lock<std::mutex> lck(__ctxMapLock);

	g_mapContextMap.erase(a_sTopic);
	CLogger::getInstance().log(DEBUG, LOGDETAILS("End:"));
}

stZmqPubContext& zmq_handler::getPubCTX(std::string a_sTopic)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start: " + a_sTopic));
	std::unique_lock<std::mutex> lck(__PubctxMapLock);

	CLogger::getInstance().log(DEBUG, LOGDETAILS("End: "));

	/// return the context
	return g_mapPubContextMap.at(a_sTopic);
}

bool zmq_handler::insertPubCTX(std::string a_sTopic, stZmqPubContext ctxRef)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start: "));
	bool bRet = true;
	try
	{
		std::unique_lock<std::mutex> lck(__PubctxMapLock);

		/// insert the data
		g_mapPubContextMap.insert(std::pair <std::string, stZmqPubContext> (a_sTopic, ctxRef));
	}
	catch (exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		bRet = false;
	}
	CLogger::getInstance().log(DEBUG, LOGDETAILS("End: "));

	return bRet;
}

void zmq_handler::removePubCTX(std::string a_sTopic)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start: " + a_sTopic));
	std::unique_lock<std::mutex> lck(__PubctxMapLock);
	g_mapPubContextMap.erase(a_sTopic);
	CLogger::getInstance().log(DEBUG, LOGDETAILS("End: "));
}

bool zmq_handler::getOnDemandReqData(unsigned short seqno, stOnDemandRequest& reqData)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start: " + std::to_string(seqno)));
	bool bRet = true;
	try
	{
		std::unique_lock<std::mutex> lck(__appSeqMapLock);

		/// return the context
		reqData = g_mapAppSeq.at(seqno);
	}
	catch(exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		bRet = false;
	}
	CLogger::getInstance().log(DEBUG, LOGDETAILS("End: "));

	return bRet;
}

bool zmq_handler::insertOnDemandReqData(unsigned short seqno, stOnDemandRequest reqData)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start: "));
	bool bRet = true;
	try
	{
		std::unique_lock<std::mutex> lck(__appSeqMapLock);

		/// insert the data
		g_mapAppSeq.insert(std::pair <unsigned short, stOnDemandRequest> (seqno, reqData));
	}
	catch (exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		bRet = false;
	}
	CLogger::getInstance().log(DEBUG, LOGDETAILS("End: "));

	return bRet;
}

void zmq_handler::removeOnDemandReqData(unsigned short seqno)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start: " + std::to_string(seqno)));
	std::unique_lock<std::mutex> lck(__appSeqMapLock);
	g_mapAppSeq.erase(seqno);
	CLogger::getInstance().log(DEBUG, LOGDETAILS("End: "));
}

std::string zmq_handler::swapConversion(std::vector<unsigned char> vt, bool a_bIsByteSwap, bool a_bIsWordSwap)
{
	auto numbytes = vt.size();
	if(0 == numbytes)
	{
		return NULL;
	}

	auto iPosByte1 = 1, iPosByte2 = 0;
	auto iPosWord1 = 0, iPosWord2 = 1;

	if(true == a_bIsByteSwap)
	{
		iPosByte1 = 0; iPosByte2 = 1;
	}
	if(true == a_bIsWordSwap)
	{
		iPosWord1 = 1; iPosWord2 = 0;
	}

	static const char* digits = "0123456789ABCDEF";
	std::string sVal(numbytes*2+2,'0');
	int i = 0;
	sVal[i++] = '0'; sVal[i++] = 'x';
	int iCurPos = 0;
	while(numbytes)
	{
		if(numbytes >= 4)
		{
			auto byte1 = vt[iCurPos + iPosWord1*2 + iPosByte1];
			auto byte2 = vt[iCurPos + iPosWord1*2 + iPosByte2];
			auto byte3 = vt[iCurPos + iPosWord2*2 + iPosByte1];
			auto byte4 = vt[iCurPos + iPosWord2*2 + iPosByte2];
			numbytes = numbytes - 4;
			iCurPos = iCurPos + 4;

			sVal[i] = digits[(byte1 >> 4) & 0x0F];
			sVal[i+1] = digits[byte1 & 0x0F];
			i += 2;

			sVal[i] = digits[(byte2 >> 4) & 0x0F];
			sVal[i+1] = digits[byte2 & 0x0F];
			i += 2;

			sVal[i] = digits[(byte3 >> 4) & 0x0F];
			sVal[i+1] = digits[byte3 & 0x0F];
			i += 2;

			sVal[i] = digits[(byte4 >> 4) & 0x0F];
			sVal[i+1] = digits[byte4 & 0x0F];
			i += 2;
		}
		else if(numbytes >= 2)
		{
			auto byte1 = vt[iCurPos + iPosByte1];
			auto byte2 = vt[iCurPos + iPosByte2];
			numbytes = numbytes - 2;
			iCurPos = iCurPos + 2;

			sVal[i] = digits[(byte1 >> 4) & 0x0F];
			sVal[i+1] = digits[byte1 & 0x0F];
			i += 2;

			sVal[i] = digits[(byte2 >> 4) & 0x0F];
			sVal[i+1] = digits[byte2 & 0x0F];
			i += 2;
		}
		else
		{
			auto byte1 = vt[iCurPos];
			--numbytes;
			++iCurPos;

			sVal[i] = digits[(byte1 >> 4) & 0x0F];
			sVal[i+1] = digits[byte1 & 0x0F];
			i += 2;
		}
	}
	return sVal;
}
