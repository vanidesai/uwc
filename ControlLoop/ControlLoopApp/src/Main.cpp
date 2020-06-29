/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

//#include <iostream>
//#include <thread>
//#include <vector>
//#include <iterator>
//#include <semaphore.h>

#include "Common.hpp"
#include "QueueMgr.hpp"
#include "MQTTSubscribeHandler.hpp"
#include "EISMsgbusHandler.hpp"
#include "ConfigManager.hpp"
#include "Logger.hpp"
#include "MQTTPublishHandler.hpp"
#include "ConfigManager.hpp"

#ifdef UNIT_TEST
#include <gtest/gtest.h>
#endif

vector<std::thread> g_vThreads;

std::atomic<bool> g_shouldStop(false);

#define APP_VERSION "0.0.5.4"

// patterns to be used to find on-demand topic strings
// topic syntax -
// for non-RT topic for polling - <topic_name>__PolledData
// for RT topic read RT - <topic_name>__RdReq_RT
#define POLLING			 		"_PolledData"
#define POLLING_RT 				"_PolledData_RT"
#define READ_RESPONSE 			"_RdResp"
#define READ_RESPONSE_RT		"_RdResp_RT"
#define WRITE_RESPONSE 			"_WrResp"
#define WRITE_RESPONSE_RT		"_WrResp_RT"

/**
 * Get operation info from global config depending on the topic name
 * @param topic	:[in] topic for which to retrieve operation info
 * @param operation	:[out] operation info
 * @return none
 */
void getOperation(string topic, globalConfig::COperation& operation)
{
	if(std::string::npos != topic.find(POLLING_RT,
			topic.length() - std::string(POLLING_RT).length(),
			std::string(POLLING_RT).length()))
	{
		operation = globalConfig::CGlobalConfig::getInstance().getOpPollingOpConfig().getRTConfig();
	}
	else if(std::string::npos != topic.find(POLLING,
			topic.length() - std::string(POLLING).length(),
			std::string(POLLING).length()))
	{
		operation = globalConfig::CGlobalConfig::getInstance().getOpPollingOpConfig().getNonRTConfig();
	}
	else if(std::string::npos != topic.find(READ_RESPONSE_RT,
			topic.length() - std::string(READ_RESPONSE_RT).length(),
			std::string(READ_RESPONSE_RT).length()))
	{
		operation = globalConfig::CGlobalConfig::getInstance().getOpOnDemandReadConfig().getRTConfig();
	}
	else if(std::string::npos != topic.find(WRITE_RESPONSE_RT,
			topic.length() - std::string(WRITE_RESPONSE_RT).length(),
			std::string(WRITE_RESPONSE_RT).length()))
	{
		operation = globalConfig::CGlobalConfig::getInstance().getOpOnDemandWriteConfig().getRTConfig();
	}
	else if(std::string::npos != topic.find(READ_RESPONSE,
			topic.length() - std::string(READ_RESPONSE).length(),
			std::string(READ_RESPONSE).length()))
	{
		operation = globalConfig::CGlobalConfig::getInstance().getOpOnDemandReadConfig().getNonRTConfig();
	}
	else if(std::string::npos != topic.find(WRITE_RESPONSE,
			topic.length() - std::string(WRITE_RESPONSE).length(),
			std::string(WRITE_RESPONSE).length()))
	{
		operation = globalConfig::CGlobalConfig::getInstance().getOpOnDemandWriteConfig().getNonRTConfig();
	}
}

/**
 * Thread function to listen on EIS and send data to MQTT
 * @param topic :[in] topic to listen onto
 * @param context :[in] msg bus context
 * @param subContext :[in] sub context
 * @param operation :[in] operation type this thread needs to perform
 * @return None
 */
void listenOnEIS(string topic, stZmqContext context, stZmqSubContext subContext, globalConfig::COperation operation)
{
	globalConfig::set_thread_sched_param(operation);
	globalConfig::display_thread_sched_attr(topic + " listenOnEIS");

	if(context.m_pContext == NULL || subContext.m_pContext == NULL)
	{
		std::cout << "Cannot start listening on EIS for topic : " << topic << endl;
		return;
	}

	void *msgbus_ctx = context.m_pContext;
	recv_ctx_t *sub_ctx = subContext.m_pContext;

	QMgr::CEISMsgQMgr *ptrQ = NULL;
	if(std::string::npos != topic.find(POLLING_RT,
				topic.length() - std::string(POLLING_RT).length(),
				std::string(POLLING_RT).length()))
	{
		std::cout << topic << ": is supported. Will listen. \n";
		ptrQ = &(QMgr::PollMsgQ());
		operation = globalConfig::CGlobalConfig::getInstance().getOpPollingOpConfig().getRTConfig();
	}
	else if(std::string::npos != topic.find(WRITE_RESPONSE_RT,
			topic.length() - std::string(WRITE_RESPONSE_RT).length(),
			std::string(WRITE_RESPONSE_RT).length()))
	{
		std::cout << topic << ": is supported. Will listen. \n";
		ptrQ = &(QMgr::WriteRespMsgQ());
		operation = globalConfig::CGlobalConfig::getInstance().getOpPollingOpConfig().getRTConfig();
	}
	else
	{
		std::cout << topic << ": is not supported \n";
		return;
	}
	if(NULL == ptrQ)
	{
		std::cout << topic << ": Q Manager not found. Returning\n";
		return;
	}
	cout << "ZMQ listening for topic : " << topic << endl;

	while ((false == g_shouldStop.load()) && (msgbus_ctx != NULL) && (sub_ctx != NULL) && (NULL != ptrQ))
	{
		try
		{
			msg_envelope_t *msg = NULL;
			msgbus_ret_t ret;

			ret = msgbus_recv_wait(msgbus_ctx, sub_ctx, &msg);
			if (ret != MSG_SUCCESS)
			{
				// Interrupt is an acceptable error
				if (ret == MSG_ERR_EINTR)
				{
					//break;
				}
				continue;
			}
			
			if(msg != NULL)
			{
				struct timespec tsMsgRcvd;
				timespec_get(&tsMsgRcvd, TIME_UTC);
				ptrQ->pushMsg(msg, tsMsgRcvd);
			}
		}
		catch (exception &ex)
		{
			cout << (std::string)ex.what() << " for topic : " << topic << endl;
		}
	}//while ends
}

/**
 * publish message to EIS
 * @param eisMsg :[in] message to publish on EIS
 * @param context :[in] msg bus context
 * @param pubContext :[in] pub context
 * @return true/false based on success/failure
 */
bool publishEISMsg(string eisMsg, stZmqContext &context,
		stZmqPubContext &pubContext)
{
	bool retVal = false;

	if(context.m_pContext == NULL || pubContext.m_pContext == NULL)
	{
		return retVal;
	}

	// Creating message to be published
	msg_envelope_t *msg = msgbus_msg_envelope_new(CT_JSON);
	if(msg == NULL)
	{
		return retVal;
	}

	cJSON *root = NULL;

	try
	{
		//parse from root element
		root = cJSON_Parse(eisMsg.c_str());
		if (NULL == root)
		{
			if(msg != NULL)
			{
				msgbus_msg_envelope_destroy(msg);
			}

			return retVal;
		}

		cJSON *device = root->child;
		while (device)
		{

			if(cJSON_IsString(device))
			{
				cout << (std::string)(device->string) << " : " << device->valuestring << endl;

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
			// get and print key
			device = device->next;
		}

		if (root)
		{
			cJSON_Delete(root);
		}

		//add time stamp before publishing msg on EIS
		std::string strTsReceived;
		CCommon::getInstance().getCurrentTimestampsInString(strTsReceived);

		msg_envelope_elem_body_t* msgPublishOnEIS = msgbus_msg_envelope_new_string(strTsReceived.c_str());
		if(msgPublishOnEIS != NULL)
		{
			msgbus_msg_envelope_put(msg, "tsMsgPublishOnEIS", msgPublishOnEIS);
		}

		msgbus_publisher_publish(context.m_pContext,
				(publisher_ctx_t*) pubContext.m_pContext, msg);
		if(msg != NULL)
			msgbus_msg_envelope_destroy(msg);

		return true;
	}
	catch(string& strException)
	{
		std::cout << "publishEISMsg error1::" << strException << std::endl;
	}
	catch(exception &ex)
	{
		std::cout << "publishEISMsg error2::" << ex.what() << std::endl;
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
 * Set thread priority for threads that send messages from MQTT-Export to EIS
 * depending on read and real-time parameters
 * @param isRealtime :[in] is operation real-time or not
 * @param isRead :[in] is it read or write operation
 * @return None
 */
void set_thread_priority_for_eis(bool& isRealtime, bool& isRead)
{
	globalConfig::COperation operation;

	try
	{
		if(isRealtime)
		{
			if(isRead)
			{
				operation = globalConfig::CGlobalConfig::getInstance().getOpOnDemandReadConfig().getRTConfig();
			}
			else
			{
				operation = globalConfig::CGlobalConfig::getInstance().getOpOnDemandWriteConfig().getRTConfig();
			}
		}
		else
		{
			if(isRead)
			{
				operation = globalConfig::CGlobalConfig::getInstance().getOpOnDemandReadConfig().getNonRTConfig();
			}
			else
			{
				operation = globalConfig::CGlobalConfig::getInstance().getOpOnDemandWriteConfig().getNonRTConfig();
			}
		}
		globalConfig::set_thread_sched_param(operation);
		std::cout << "Set thread priorities for isRead: " << isRead << ", isRealtime : " << isRealtime << endl;
	}
	catch(exception &e)
	{
		cout << e.what() << endl;
	}
}

void getTimeParams(std::string &a_sTimeStamp, std::string &a_sUsec)
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

bool addFieldToMsg(std::string& a_sMsg, std::string a_sKey, std::string a_sVal, bool bIsLastField)
{
	a_sMsg = a_sMsg + '"' + a_sKey + '"' + ':' + '"' + a_sVal + '"';
	if(false == bIsLastField)
	{
		a_sMsg = a_sMsg + ',';
	}
	return true;
}

string createWrRTMsg(std::string& a_sMsg, QMgr::stEISMsg &a_stMsgArrived,
		std::string& a_sWell, std::string& a_sDev, std::string& a_sPoint)
{
	a_sMsg.clear();

	if(NULL == a_stMsgArrived.m_pEISMsg)
	{
		std::cout << "createWrRTMsg:: Received message is null\n";
		return "";
	}
	msg_envelope_elem_body_t* data;
	msgbus_ret_t msgRet = msgbus_msg_envelope_get(a_stMsgArrived.m_pEISMsg, "driver_seq", &data);
	if(msgRet != MSG_SUCCESS)
	{
		std::cout << "createWrRTMsg:: driver_seq key not present in zmq message\n";
		return "";
	}
	std::string sKey = data->body.string;
	std::string sSrcTopic("/" + a_sDev + "/" + a_sWell + "/" + a_sPoint + "/write");
	addFieldToMsg(a_sMsg, "app_seq", 		sKey, 		false);
	addFieldToMsg(a_sMsg, "sourcetopic", 	sSrcTopic, 	false);
	addFieldToMsg(a_sMsg, "wellhead", 		a_sWell, 	false);
	addFieldToMsg(a_sMsg, "command", 		a_sPoint, 	false);
	addFieldToMsg(a_sMsg, "value", 			"0x00", 	false);
	addFieldToMsg(a_sMsg, "version", 		"2.0", 		false);
	addFieldToMsg(a_sMsg, "realtime", 		"1", 		false);
	addFieldToMsg(a_sMsg, "tsMsgRcvdFromMQTT", (to_string(CCommon::getInstance().get_micros(a_stMsgArrived.m_stRcvdMsgTs))).c_str(), false);

	std::string sTs{""}, sUsec{""};
	getTimeParams(sTs, sUsec);

	addFieldToMsg(a_sMsg, "timestamp", 	sTs, 	false);
	addFieldToMsg(a_sMsg, "usec", 		sUsec, 	true);

	a_sMsg = "{" + a_sMsg + "}";

	return sKey;
}

/**
 *
 * Description
 * This function lets the current thread to sleep for an interval specified
 * with nanosecond precision. This adjusts the speed to complete previous send request.
 * The function gets the current time, adds the time equal to interframe delay and sleeps
 * for that much of time.
 *
 * @param lMilliseconds [in] long time duration in micro-seconds (interframe delay of current request)
 *
 * @return uint8_t [out] 0 if thread sleeps for calculated duration in nano-seconds
 * 						 -1 if function fails to calculate or sleep
 *
 */
int sleep_micros(long lMicroseconds)
{
	struct timespec ts;
	int rc = clock_gettime(CLOCK_MONOTONIC, &ts);
	if(0 != rc)
	{
		perror("Stack fatal error: Sleep function: clock_gettime failed: ");
		return -1;
	}
	unsigned long next_tick = (ts.tv_sec * 1000000000L + ts.tv_nsec) + lMicroseconds*1000;
	ts.tv_sec = next_tick / 1000000000L;
	ts.tv_nsec = next_tick % 1000000000L;
	do
	{
		rc = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL);
	} while(EINTR == rc);

	if(0 == rc)
	{
		return 0;
	}

	printf("Stack: Error in sleep function: %d\n", rc);
	return -1;
}

/**
 * Thread function to read requests from queue filled up by MQTT and send data to EIS
 * @param qMgr 	:[in] pointer to respective queue manager
 * @return None
 */
void postWriteMsgsToEIS(QMgr::CEISMsgQMgr& qMgr)
{
	//set priority to send msgs on EIS from MQTT-export (on-demand)
	//set_thread_priority_for_eis(isRealtime, isRead);

	globalConfig::set_thread_sched_param(globalConfig::CGlobalConfig::getInstance().getOpOnDemandWriteConfig().getRTConfig());
	globalConfig::display_thread_sched_attr("postWriteMsgsToEIS");

	try
	{
		std::string eisTopic(CCommon::getInstance().getStrRTWriteRequest());
		std::cout << "eisTopic:" << eisTopic << std::endl;
		//get EIS msg bus context
		stZmqContext context;
		if (!CEISMsgbusHandler::Instance().getCTX(eisTopic, context))
		{
			std::cout << eisTopic << ": context not found\n";
			return;
		}

		//will give topic context
		stZmqPubContext pubContext;
		if (!CEISMsgbusHandler::Instance().getPubCTX(eisTopic,
				pubContext))
		{
			std::cout << eisTopic << ": pub context not found\n";
			return;
		}

		const char *pcWRMSG_WELLHEAD = getenv("WRMSG_WELLHEAD");
		if(NULL == pcWRMSG_WELLHEAD)
		{
			std::cout << "Env WRMSG_WELLHEAD is not set\n";
			return;
		}
		const char *pcWRMSG_DEV = getenv("WRMSG_DEV");
		if(NULL == pcWRMSG_DEV)
		{
			std::cout << "Env WRMSG_DEV is not set\n";
			return;
		}
		const char *pcWRMSG_POINTS = getenv("WRMSG_POINTS");
		if(NULL == pcWRMSG_POINTS)
		{
			std::cout << "Env WRMSG_WELLHEAD is not set\n";
			return;
		}
		std::string sWell{pcWRMSG_WELLHEAD}, sDev{pcWRMSG_DEV}, sPoints{pcWRMSG_POINTS};

		std::cout << "All set to send RT write messages \n";

		while (false == g_shouldStop.load())
		{
			QMgr::stEISMsg recvdMsg;
			if(true == qMgr.waitForMsg(recvdMsg))
			{
				if(NULL == recvdMsg.m_pEISMsg)
				{
					continue;
				}
				std::string sMsg{""};
				std::string sKey = createWrRTMsg(sMsg, recvdMsg, sWell, sDev, sPoints);

				if(true == sKey.empty())
				{
					continue;
				}

				QMgr::CControlLoopMapper::getInstace().insertForTracking(sKey, recvdMsg);

				sleep_micros(CCommon::getInstance().getStrWRDelay());
				if(publishEISMsg(sMsg, context, pubContext))
				{
					std::cout << "Published EIS message : "	<< sMsg << " on topic :" << eisTopic << std::endl;
				}
				else
				{
					std::cout << "Failed to publish EIS message : "	<< sMsg << " on topic :" << eisTopic << std::endl;
					QMgr::stEISMsg msg;
					QMgr::CControlLoopMapper::getInstace().getForProcessing(sKey, msg);

					if(msg.m_pEISMsg != NULL)
					{
						msgbus_msg_envelope_destroy(msg.m_pEISMsg);
					}
				}
			}
		}
	}
	catch (const std::exception &e)
	{
	}
}

std::string createAnalysisMsg(QMgr::stEISMsg &a_msgPoll, QMgr::stEISMsg &a_msgWrResp)
{
	if((NULL == a_msgPoll.m_pEISMsg) || (NULL == a_msgWrResp.m_pEISMsg))
	{
		return "";
	}
	std::string sMsg{""};

	auto answer = [](std::string &a_sMsg, msg_envelope_t *a_pSrcMsgEIS, std::string a_sSrcKey, std::string a_sFinalKey, bool a_bIsLastField)
	{
		if(NULL != a_pSrcMsgEIS)
		{
			msg_envelope_elem_body_t* data;
			msgbus_ret_t msgRet = msgbus_msg_envelope_get(a_pSrcMsgEIS, a_sSrcKey.c_str(), &data);
			if(msgRet != MSG_SUCCESS)
			{
				//std::cout << "createAnalysisMsg::lambda: key not present in zmq : " << a_sSrcKey << std::endl;
			}
			else
			{
				std::string sData = data->body.string;
				addFieldToMsg(a_sMsg, a_sFinalKey, sData, a_bIsLastField);
			}
		}
		return a_sMsg;
	};

	// Process poll message
	answer(sMsg, a_msgPoll.m_pEISMsg,	"driver_seq",	"uniqueKey", 	false);
	answer(sMsg, a_msgPoll.m_pEISMsg,	"data_topic",		"pollTopic", 	false);
	answer(sMsg, a_msgPoll.m_pEISMsg, 	"status", 		"pollStatus", 	false);
	//answer(sMsg, a_msgPoll.m_pEISMsg, 	"retryCount",	"pollRetry", 	false);
	answer(sMsg, a_msgPoll.m_pEISMsg, 	"value", 		"pollValue", 	false);
	answer(sMsg, a_msgPoll.m_pEISMsg, 	"error_code",	"pollError", 	false);
	answer(sMsg, a_msgPoll.m_pEISMsg, 	"tsPollingTime", 		"tsPollingTime", 		false);
	answer(sMsg, a_msgPoll.m_pEISMsg, 	"reqRcvdInStack", 		"pollReqRcvdInStack", 	false);
	answer(sMsg, a_msgPoll.m_pEISMsg, 	"reqSentByStack", 		"pollReqSentByStack", 	false);
	answer(sMsg, a_msgPoll.m_pEISMsg, 	"respRcvdByStack", 		"pollRespRcvdByStack", 	false);
	answer(sMsg, a_msgPoll.m_pEISMsg, 	"respPostedByStack", 	"pollRespPostedByStack",false);
	answer(sMsg, a_msgPoll.m_pEISMsg, 	"usec",			 		"pollPostedToEIS",	 	false);

	// Process write-response message
	answer(sMsg, a_msgWrResp.m_pEISMsg, "tsMsgRcvdFromMQTT", 	"pollMsgRcvInApp",	false);
	answer(sMsg, a_msgWrResp.m_pEISMsg,	"data_topic",				"wrRspTopic", 	false);
	answer(sMsg, a_msgWrResp.m_pEISMsg, "status", 				"wrRspStatus", 	false);
	//answer(sMsg, a_msgWrResp.m_pEISMsg, "retryCount",			"wrRspRetry", 	false);
	answer(sMsg, a_msgWrResp.m_pEISMsg, "error_code",			"wrRspError", 	false);
	answer(sMsg, a_msgWrResp.m_pEISMsg, "tsMsgPublishOnEIS", 	"wrReqPublishOnEIS",false);
	answer(sMsg, a_msgWrResp.m_pEISMsg, "reqRcvdByApp", 		"wrReqRcvdByModbus",	false);
	answer(sMsg, a_msgWrResp.m_pEISMsg, "reqRcvdInStack", 		"wrReqRcvdInStack", 	false);
	answer(sMsg, a_msgWrResp.m_pEISMsg, "reqSentByStack", 		"wrReqSentByStack", 	false);
	answer(sMsg, a_msgWrResp.m_pEISMsg, "respRcvdByStack", 		"wrRespRcvdByStack", 	false);
	answer(sMsg, a_msgWrResp.m_pEISMsg, "respPostedByStack", 	"wrRespPostedByStack",	false);
	answer(sMsg, a_msgWrResp.m_pEISMsg, "usec",			 		"wrRespPostedToEIS",	false);

	// Add last timestamp
	addFieldToMsg(sMsg, "wrRespRcvInApp", (to_string(CCommon::getInstance().get_micros(a_msgWrResp.m_stRcvdMsgTs))).c_str(), true);

	sMsg = "{" + sMsg + "}";
	return sMsg;
}

/**
 * Thread function to read requests from queue filled up by MQTT and send data to EIS
 * @param qMgr 	:[in] pointer to respective queue manager
 * @return None
 */
void analyzeControlLoopData(QMgr::CEISMsgQMgr& qMgr)
{
	//set priority to send msgs on EIS from MQTT-export (on-demand)
	//set_thread_priority_for_eis(isRealtime, isRead);

	globalConfig::display_thread_sched_attr("analyzeControlLoopData");

	try
	{

		int qos = 1;
		//consider topic name as distinguishing factor for publisher
		std::string sMQTTTopic{"controlLoopData"};
		CMQTTPublishHandler mqttPublisher(CCommon::getInstance().getStrMqttExportURL().c_str(), sMQTTTopic, qos);

		std::cout << "All set to analyze control data\n";

		while (false == g_shouldStop.load())
		{
			QMgr::stEISMsg msgWrResp;
			if(true == qMgr.waitForMsg(msgWrResp))
			{
				if(NULL == msgWrResp.m_pEISMsg)
				{
					continue;
				}
				msg_envelope_elem_body_t* data;
				msgbus_ret_t msgRet = msgbus_msg_envelope_get(msgWrResp.m_pEISMsg, "app_seq", &data);
				if(msgRet != MSG_SUCCESS)
				{
					std::cout << "analyzeControlLoopData:: app_seq key not present in zmq message\n";
				}
				else
				{
					std::string sKey = data->body.string;
					std::cout << "analyzeControlLoopData:: app_seq = " << sKey << std::endl;
					QMgr::stEISMsg msgPoll;
					QMgr::CControlLoopMapper::getInstace().getForProcessing(sKey, msgPoll);

					std::string sCompleteMsg{""};
					if(NULL != msgPoll.m_pEISMsg)
					{
						sCompleteMsg = createAnalysisMsg(msgPoll, msgWrResp);
						// Delete poll message
						msgbus_msg_envelope_destroy(msgPoll.m_pEISMsg);
					}

					if(false == mqttPublisher.publish(sCompleteMsg, sMQTTTopic))
					{
						std::cout << "MQTT message not published\n";
					}
				}

				// Delete write response message
				msgbus_msg_envelope_destroy(msgWrResp.m_pEISMsg);
			}
		}
	}
	catch (const std::exception &e)
	{
		cout << "Exception occured while publishing data: "<<e.what() << endl;
	}
}

/**
 * Get EIS topic list, get corresponding message bus and topic contexts.
 * Spawn threads to listen to EIS messages, receive messages from EIS and publish them to MQTT
 * @param None
 * @return None
 */
void postMsgstoMQTT()
{
	// get sub topic list
	vector<string> vFullTopics = CEISMsgbusHandler::Instance().getSubTopicList();

	for (auto &topic : vFullTopics)
	{
		if(topic.empty())
		{
			continue;
		}

		stZmqContext context;

		if (!CEISMsgbusHandler::Instance().getCTX(topic, context))
		{
			continue;		//go to next topic
		}

		//will give topic context
		stZmqSubContext subContext;
		if (!CEISMsgbusHandler::Instance().getSubCTX(topic,
				subContext))
		{
			continue;		//go to next topic
		}

		//get operation depending on the topic
		globalConfig::COperation objOperation;
		getOperation(topic, objOperation);

		g_vThreads.push_back(
				std::thread(listenOnEIS, topic, context, subContext, objOperation));
	}
}

/**
 * Create EIS msg bus context and topic context for publisher and subscriber both
 * @param None
 * @return true/false based on success/failure
 */
bool initEISContext()
{
	bool retVal = true;

	// Initializing all the pub/sub topic base context for ZMQ
	const char* env_pubTopics = std::getenv("PubTopics");
	if (env_pubTopics != NULL)
	{
		cout << "List of topic configured for Pub are :: " << (std::string)(env_pubTopics) << endl;
		bool bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext(
				"pub");
		if (!bRetVal)
		{
			std::cout << __func__ << ":" << __LINE__ << " Error : Context creation failed for sub topic" <<  std::endl;
			retVal = false;
		}
	}
	else
	{
		std::cout << __func__ << ":" << __LINE__ << " Error : could not find PubTopics in environment variables" <<  std::endl;
		retVal = false;
	}

	const char* env_subTopics = std::getenv("SubTopics");
	if(env_subTopics != NULL)
	{
		cout << "List of topic configured for Sub are :: " << (std::string)(env_subTopics) << endl;
		bool bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext("sub");
		if (!bRetVal)
		{
			std::cout << __func__ << ":" << __LINE__ << " Error : Context creation failed for sub topic" <<  std::endl;
			retVal = false;
		}
	}
	else
	{
		std::cout << __func__ << ":" << __LINE__ << " Error : could not find SubTopics in environment variables" <<  std::endl;
		retVal = false;
	}

	return retVal;
}

/**
 * Main function of application
 * @param argc :[in] number of input parameters
 * @param argv :[in] input parameters
 * @return 	0/-1 based on success/failure
 */
int main(int argc, char *argv[])
{
	std::cout << __func__ << ":" << __LINE__ << " ------------- Starting Control Loop App Container -------------" << std::endl;

	try
	{
		//initialize CCommon class to get common variables
		string AppName = CCommon::getInstance().getStrAppName();
		if(AppName.empty())
		{
			std::cout << __func__ << ":" << __LINE__ << " Error : AppName Environment Variable is not set" <<  std::endl;
			return -1;
		}

		cout << "Control Loop App container app version is set to :: "+  std::string(APP_VERSION) << endl;

		// load global configuration for container real-time setting
		bool bRetVal = globalConfig::loadGlobalConfigurations();
		if(!bRetVal)
		{
			cout << "\nGlobal configuration is set with some default parameters\n\n";
		}
		else
		{
			cout << "\nGlobal configuration for container real-time is set successfully\n\n";
		}

		globalConfig::CPriorityMgr::getInstance();

		//read environment values from settings
		CCommon::getInstance();

		//Prepare MQTT for publishing & subscribing
		//subscribing to topics happens in callback of connect()
		//CMQTTHandler::instance();

		//Prepare ZMQ contexts for publishing & subscribing data
		initEISContext();

		g_vThreads.push_back(std::thread(postWriteMsgsToEIS, std::ref(QMgr::PollMsgQ())));
		g_vThreads.push_back(std::thread(analyzeControlLoopData, std::ref(QMgr::WriteRespMsgQ())));
		//Start listening on EIS & publishing to MQTT
		postMsgstoMQTT();

		for (auto &th : g_vThreads)
		{
			if (th.joinable())
			{
				th.join();
			}
		}

	}
	catch (std::exception &e)
	{
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
		return -1;
	}
	catch (...)
	{
		std::cout << __func__ << ":" << __LINE__ << "Exception : Unknown Exception Occurred. Exiting" << std::endl;
		return -1;
	}

	std::cout << __func__ << ":" << __LINE__ << " ------------- Exiting MQTT Export Container -------------" << std::endl;
	return 0;
}
