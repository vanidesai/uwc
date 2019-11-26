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

using namespace zmq_handler;

std::mutex fileMutex;
std::mutex __ctxMapLock;
std::mutex __PubctxMapLock;
publisher_ctx_t* g_pub_ctx;

// Unnamed namespace to define globals
namespace
{
	std::map<std::string, stZmqContext> g_mapContextMap;
	std::map<std::string, stZmqPubContext> g_mapPubContextMap;
}

void zmq_handler::startupPreparePubContext()
{
	BOOST_LOG_SEV(lg, debug) << __func__ << "Start: ";
	void* g_msgbus_ctx = NULL;
	std::string topic;

	do
	{
		char *tempTopic = std::getenv("PubTopics");
		if(NULL == tempTopic)
		{
			BOOST_LOG_SEV(lg, warning) << __func__ << "PUB_TOPIC environment is not available";
			break;
		}
		char* token;
		char* rest = tempTopic;

		BOOST_LOG_SEV(lg, info) << __func__ << "PUB_TOPIC value: " << tempTopic;

		while ((token = strtok_r(rest, ",", &rest)))
		{
			std::string tempstr(token);
			topic = tempstr;
			tempstr = tempstr + "_cfg";
			char *ch_ip_port = std::getenv(tempstr.c_str());
			if(NULL == ch_ip_port)
			{
				BOOST_LOG_SEV(lg, warning) << __func__ << tempstr << ": environment is not available";
				// go to next topic
				continue;
			}
			BOOST_LOG_SEV(lg, info) << __func__ << tempstr << " value: " << ch_ip_port;

			char *inrtoken = strtok(ch_ip_port, ",");
			cJSON *root, *ip_Port;
			root = cJSON_CreateObject();
			ip_Port = cJSON_CreateObject();
			cJSON_AddStringToObject(root, "type", inrtoken);
			bool isHost = true;
			while (inrtoken != NULL)
			{
				inrtoken = strtok(NULL, ":");
				if(isHost)
				{
					cJSON_AddStringToObject(ip_Port, "host", inrtoken);
					isHost = false;
				}
				else
				{
					if(inrtoken)
					{
						cJSON_AddNumberToObject(ip_Port, "port", atoi(inrtoken));
					}
				}
			}
			cJSON_AddItemToObject(root, "zmq_tcp_publish", ip_Port);

			char *ptr_chJson = NULL;
			ptr_chJson = cJSON_Print(root);

			if(NULL != root)
				cJSON_Delete(root);

			std::string filePath(std::getenv("CONFIG_FILE_PATH"));
			filePath = filePath + "tcp_socket.json";
			BOOST_LOG_SEV(lg, info) << __func__ << " filePath for JSON config temp storage: " << filePath;
			std::ofstream myfile (filePath);
			if (myfile.is_open())
			{
				myfile << ptr_chJson;
				myfile.close();
			}
			else std::cout << "Unable to open file";

			config_t* config = msgbus_json_config_new(filePath.c_str());
			if(config == NULL) {
				BOOST_LOG_SEV(lg, error) << __func__ << "Failed to load JSON configuration " << ch_ip_port << " for " << tempstr;
				continue;
			}

			g_msgbus_ctx = msgbus_initialize(config);
			if(g_msgbus_ctx == NULL)
			{
				BOOST_LOG_SEV(lg, error) << __func__ << "Failed to get message bus context with config " << ch_ip_port << " for " << tempstr;
				continue;
			}

			/// method to store msgbus_ctx as per the topic
			stZmqContext objTempCtx;
			objTempCtx.m_pContext = g_msgbus_ctx;
			zmq_handler::insertCTX(topic, objTempCtx);

			msgbus_ret_t ret;

			BOOST_LOG_SEV(lg, info) << __func__ << "Context created and stored for config " << ch_ip_port << " for " << tempstr;
			ret = msgbus_publisher_new(g_msgbus_ctx,topic.c_str(), &g_pub_ctx);

			if(ret != MSG_SUCCESS)
			{
				BOOST_LOG_SEV(lg, error) <<__func__ << "Failed to initialize publisher errno: "<< ret;
			}
		}
	} while(0);
	BOOST_LOG_SEV(lg, debug)<<__func__ << "End: ";
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
