/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "EISPlBusHandler.hpp"
#include "KPIAppConfigMgr.hpp"
#include "CommonDataShare.hpp"
#include "EnvironmentVarHandler.hpp"
#include "Logger.hpp"
#include "Common.hpp"

// patterns to be used to find on-demand topic strings
// topic syntax -
// for non-RT topic for polling - <topic_name>__PolledData
// for RT topic read RT - <topic_name>__RdReq_RT
#define POLLING			 		"_PolledData"
#define POLLING_RT 				"_PolledData_RT"
#define WRITE_RESPONSE 			"_WrResp"
#define WRITE_RESPONSE_RT		"_WrResp_RT"

extern std::atomic<bool> g_stopThread;

/**
 * Create EIS msg bus context and topic context for publisher and subscriber both
 * @param None
 * @return true/false based on success/failure
 */
bool CEISPlBusHandler::initEISContext()
{
	bool retVal = true;
	try
	{
		// Initializing all the pub/sub topic base context for ZMQ
		const char* env_pubTopics = std::getenv("PubTopics");
		if (env_pubTopics != NULL)
		{
			DO_LOG_DEBUG("List of topic configured for Pub are :: "+ (std::string)(env_pubTopics));
			if (true != zmq_handler::prepareCommonContext("pub"))
			{
				DO_LOG_ERROR("Context creation failed for sub topic ");
				std::cout << __func__ << ":" << __LINE__ << " Error : Context creation failed for sub topic" <<  std::endl;
				retVal = false;
			}
		}
		else
		{
			DO_LOG_ERROR("could not find PubTopics in environment variables");
			std::cout << __func__ << ":" << __LINE__ << " Error : could not find PubTopics in environment variables" <<  std::endl;
			retVal = false;
		}

		const char* env_subTopics = std::getenv("SubTopics");
		if(env_subTopics != NULL)
		{
			DO_LOG_DEBUG("List of topic configured for Sub are :: "+ (std::string)(env_subTopics));
			if (true != zmq_handler::prepareCommonContext("sub"))
			{
				DO_LOG_ERROR("Context creation failed for sub topic");
				std::cout << __func__ << ":" << __LINE__ << " Error : Context creation failed for sub topic" <<  std::endl;
				retVal = false;
			}
			else
			{
				CcommonEnvManager::Instance().splitString(static_cast<std::string>(env_subTopics),',');
			}
		}
		else
		{
			DO_LOG_ERROR("could not find SubTopics in environment variables");
			std::cout << __func__ << ":" << __LINE__ << " Error : could not find SubTopics in environment variables" <<  std::endl;
			retVal = false;
		}
	}
	catch (exception &ex)
	{
		DO_LOG_ERROR((std::string)ex.what());
	}

	return retVal;
}

/**
 * Process message received from EIS for control loop operations
 * @param msg	:[in] actual message
 * @param a_bIsPollMsg :[in] true = polling msg, false = write response
 * returns true/false based on success/failure
 */
bool CEISPlBusHandler::processMsg(msg_envelope_t *msg, CQueueHandler &a_rQ,
		const std::function<bool(const std::string &)> &a_fPointListChecker)
{
	bool bRetVal = false;
	try
	{
		if(msg == NULL)
		{
			DO_LOG_ERROR("Received NULL msg in msgbus_recv_wait");
			return false;
		}

		msg_envelope_elem_body_t* data;
		msgbus_ret_t msgRet = msgbus_msg_envelope_get(msg, "data_topic", &data);
		if(msgRet != MSG_SUCCESS)
		{
			DO_LOG_ERROR("topic key not present in zmq message");
			return false;
		}
		std::string sRcvdTopic{data->body.string};
		if(false == a_fPointListChecker(sRcvdTopic))
		{
			DO_LOG_DEBUG(sRcvdTopic + ": This is not a control loop topic. Ignored.")
			return false;
		}

		msg_envelope_serialized_part_t *parts = NULL;
		int num_parts = msgbus_msg_envelope_serialize(msg, &parts);
		if (num_parts <= 0)
		{
			DO_LOG_ERROR("Failed to serialize message");
		}
		else if(NULL != parts)
		{
			if(NULL != parts[0].bytes)
			{
				string sMsgBody(parts[0].bytes);
				CMessageObject oMsg{sRcvdTopic, sMsgBody};
				a_rQ.pushMsg(oMsg);
				bRetVal = true;
			}
		}
		else
		{
			DO_LOG_ERROR("NULL pointer received");
			bRetVal = false;
		}
		if(NULL != parts)
		{
			msgbus_msg_envelope_serialize_destroy(parts, num_parts);
			parts = NULL;
		}
	}
	catch (exception &ex)
	{
		DO_LOG_ERROR((std::string)ex.what());
	}

	return bRetVal;
}

/**
 * Thread function to listen on EIS and send data to MQTT
 * @param sTopic :[in] topic to listen onto
 * @param context :[in] msg bus context
 * @param subContext :[in] sub context
 * @param a_bIsPolling :[in] operation type this thread needs to perform
 * @return None
 */
void CEISPlBusHandler::listenOnEIS(string sTopic, zmq_handler::stZmqContext context, 
			zmq_handler::stZmqSubContext subContext, bool a_bIsPolling)
{
	if(context.m_pContext == NULL || subContext.sub_ctx == NULL)
	{
		std::cout << "Context is null. Cannot start listening on EIS for topic : " << sTopic << endl;
		DO_LOG_ERROR("Context is null. Cannot start listening on EIS for topic : " + sTopic);
		return;
	}

	std::function<bool(const std::string &)> fPointListChecker = 
		std::bind(&CControlLoopMapper::isControlLoopPollPoint,
			&CKPIAppConfig::getInstance().getControlLoopMapper(), std::placeholders::_1);
	std::reference_wrapper<CQueueHandler> refQ{QMgr::PollMsgQ()};
	if(false == a_bIsPolling)
	{
		refQ = QMgr::WriteRespMsgQ();
		fPointListChecker = 
			std::bind(&CControlLoopMapper::isControlLoopWrRspPoint, 
				&CKPIAppConfig::getInstance().getControlLoopMapper(), std::placeholders::_1);
	}

	void *msgbus_ctx = context.m_pContext;
	recv_ctx_t *sub_ctx = subContext.sub_ctx;

	DO_LOG_INFO("ZMQ listening for topic : " + sTopic);

	while ((false == g_stopThread.load()) && (msgbus_ctx != NULL) && (sub_ctx != NULL))
	{
		try
		{
			msg_envelope_t *msg = NULL;
			msgbus_ret_t ret = msgbus_recv_wait(msgbus_ctx, sub_ctx, &msg);
			if (ret != MSG_SUCCESS)
			{
				// Interrupt is an acceptable error
				if (ret == MSG_ERR_EINTR)
				{
					DO_LOG_ERROR("received MSG_ERR_EINT");
					//break;
				}
				DO_LOG_ERROR("Failed to receive message errno: " + std::to_string(ret));
				continue;
			}
			
			if(NULL != msg)
			{
				// process ZMQ message 
				processMsg(msg, refQ, fPointListChecker);
				
				msgbus_msg_envelope_destroy(msg);
				msg = NULL;
			}
		}
		catch (exception &ex)
		{
			DO_LOG_ERROR((std::string)ex.what() + " for topic : " + sTopic);
		}
	}//while ends

	DO_LOG_DEBUG("exited !!");
}

/**
 * publish message to EIS
 * @param a_sEisMsg :[in] message to publish on EIS
 * @param a_sEisTopic :[in] eis topic
 * @return true/false based on success/failure
 */
bool publishEISMsg(std::string a_sEisMsg, std::string &a_sEisTopic)
{
	// Creating message to be published
	msg_envelope_t *msg = NULL;
	cJSON *root = NULL;

	try
	{
		msg = msgbus_msg_envelope_new(CT_JSON);
		if(msg == NULL)
		{
			return false;
		}
		//parse from root element
		root = cJSON_Parse(a_sEisMsg.c_str());
		if (NULL == root)
		{
			if(msg != NULL)
			{
				msgbus_msg_envelope_destroy(msg);
			}
			return false;
		}

		cJSON *device = root->child;
		while (device)
		{
			if(cJSON_IsString(device))
			{
				msg_envelope_elem_body_t *value = msgbus_msg_envelope_new_string(
						device->valuestring);
				if(value != NULL)
				{
					msgbus_msg_envelope_put(msg, device->string, value);
				}
			}
			else
			{
				throw string("Invalid JSON");
			}
			device = device->next;
		}

		if (root)
		{
			cJSON_Delete(root);
			root = NULL;
		}

		//add time stamp before publishing msg on EIS
		std::string strTsReceived{""};
		bool bRet = true;
		if(true == zmq_handler::publishJson(strTsReceived, msg, a_sEisTopic, "tsMsgPublishOnEIS"))
		{
			bRet = true;
		}
		else
		{
			DO_LOG_ERROR("Failed to publish write msg on EIS: " + a_sEisMsg);
			bRet = false;
		}

		if(msg != NULL)
			msgbus_msg_envelope_destroy(msg);
		msg = NULL;

		return bRet;
	}
	catch(string& strException)
	{
		DO_LOG_ERROR("publishEISMsg error1::" + strException);
	}
	catch(exception &ex)
	{
		DO_LOG_ERROR("publishEISMsg error2::" + std::string(ex.what()));
	}

	if(msg != NULL)
	{
		msgbus_msg_envelope_destroy(msg);
	}
	if (root)
	{
		cJSON_Delete(root);
	}

	return false;
}

/**
 * publish message to EIS
 * @param a_sMsg :[in] message to publish on EIS
 * @return true/false based on success/failure
 */
bool CEISPlBusHandler::publishWriteMsg(const std::string &a_sMsg)
{
	try
	{
		std::string eisTopic = "";
		if(true == CKPIAppConfig::getInstance().isRTModeForWriteOp())
		{
			eisTopic.assign(EnvironmentInfo::getInstance().getDataFromEnvMap("WriteRequest_RT"));
		}
		else
		{
			eisTopic.assign(EnvironmentInfo::getInstance().getDataFromEnvMap("WriteRequest"));
		}		
		return publishEISMsg(a_sMsg, eisTopic);
	}
	catch (exception &ex)
	{
		DO_LOG_ERROR(ex.what());
	}
	return false;
}

/**
 * Configures EIS listeners based on configurations
 * @param a_bIsPollingRT :[in] tells whether RT or Non-RT polling topics to be scanned
 * @param a_bIsWrOpRT :[in] tells whether RT or Non-RT write response topics to be scanned
 * @return None
 */
void CEISPlBusHandler::configEISListerners(bool a_bIsPollingRT, bool a_bIsWrOpRT)
{
	try
	{
		// get sub topic list
		std::vector<std::string> vFullTopics = CcommonEnvManager::Instance().getTopicList();
		std::string sPollPattern{POLLING_RT}, sWrOpPattern{WRITE_RESPONSE_RT};

		if(false == a_bIsPollingRT)
		{
			sPollPattern.assign(POLLING);
		}
		if(false == a_bIsWrOpRT)
		{
			sWrOpPattern.assign(WRITE_RESPONSE);
		}

		for (auto &sTopic : vFullTopics)
		{
			if(sTopic.empty())
			{
				continue;
			}

			bool bIsPolling = true;
			// Pattern matching
			if(std::string::npos != sTopic.find(sPollPattern, 
				sTopic.length() - sPollPattern.length() ))
			{
				// This topic is for polling
				bIsPolling = true;
			}
			else if(std::string::npos != sTopic.find(sWrOpPattern,
				sTopic.length() - sWrOpPattern.length() ))
			{
				// This topic is for write response processing 
				bIsPolling = false;
			}
			else
			{
				// Not matching with pattern. 
				DO_LOG_INFO(sTopic + ": Topic will not be monitored on ZMQ");
				continue;
			}
			DO_LOG_INFO(sTopic + ": Topic will be monitored on ZMQ");
			
			zmq_handler::stZmqContext& context = zmq_handler::getCTX(sTopic);
			zmq_handler::stZmqSubContext& subContext = zmq_handler::getSubCTX(sTopic);

			m_vThreads.push_back(
				std::thread(&CEISPlBusHandler::listenOnEIS, this, sTopic, context, subContext, bIsPolling));
		}
	}
	catch(const std::exception& e)
	{
		DO_LOG_ERROR(e.what());
	}
}

/**
 * Stops EIS listener threads
 * @return None
 */
void CEISPlBusHandler::stopEISListeners()
{
	try
	{
		for (auto &itr : m_vThreads)
		{
			if (itr.joinable())
			{
				itr.join();
			}
		}
	}
	catch(const std::exception& e)
	{
		DO_LOG_ERROR(e.what());
	}
}
