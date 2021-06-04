/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/
#include "Common.hpp"
#include <cjson/cJSON.h>
#include "CommonDataShare.hpp"

/**
 * Get current time in micro seconds
 * @param ts :[in] timestamp to get microsecond value
 * @return time in micro seconds
 */
unsigned long commonUtilKPI::get_micros(struct timespec ts)
{
	return (unsigned long)ts.tv_sec * 1000000L + ts.tv_nsec/1000;
}

/**
 * Get current epoch time in string
 * @param strCurTime :[out] set current epoch time in string format
 * @return None
 */
void commonUtilKPI::getCurrentTimestampsInString(std::string &strCurTime)
{
	try
	{
		struct timespec tsMsgReceived;
		timespec_get(&tsMsgReceived, TIME_UTC);
		strCurTime = std::to_string(commonUtilKPI::get_micros(tsMsgReceived));
	}
	catch(std::exception &e)
	{
		DO_LOG_ERROR("Cannot get current time in string :: " + std::string(e.what()));
	}
}

/**
 * For logging control loop analysis data, a separate logger is used.
 * This function creates the analysis message and logs it into required logger.
 * @param a_stPollWrData[in]  polling and write data
 * @param a_msgWrResp	[in]  write response message
 * @return none
 */
void commonUtilKPI::logAnalysisMsg(struct stPollWrData &a_stPollWrData, CMessageObject &a_msgWrResp)
{
	std::string sMsg = commonUtilKPI::createAnalysisMsg(a_stPollWrData, a_msgWrResp);
	log4cpp::Category::getInstance(std::string("analysis")).info(sMsg);
}

/**
 * This function extracts the value of a key from given JSON payload 
 * @param a_sMsg 	:[in] JSON payload in string form
 * @param a_sKey 	:[in] Key name for which value is needed
 * @return string: extracted value
 */
std::string commonUtilKPI::getValueofKeyFromJSONMsg(const std::string &a_sMsg, const std::string &a_sKey)
{
	// 
	std::string sKeyToSearch{"\"" + a_sKey + "\""};
	std::string sValue{""};
	try
	{
		do
		{
			// Format in JSON is:
			// "Key":"value"
			auto keyLocation = a_sMsg.find(sKeyToSearch);
			if(std::string::npos == keyLocation)
			{
				break;
			}
			auto valSearchLoc = keyLocation + sKeyToSearch.length();
			valSearchLoc = a_sMsg.find(":", valSearchLoc);
			if(std::string::npos == valSearchLoc)
			{
				break;
			}
			valSearchLoc = a_sMsg.find("\"", valSearchLoc);
			if(std::string::npos == valSearchLoc)
			{
				break;
			}
			auto valEndQuoteLoc = a_sMsg.find("\"", valSearchLoc+1);
			if(std::string::npos != valEndQuoteLoc)
			{
				sValue.assign(a_sMsg.substr(valSearchLoc+1, valEndQuoteLoc-valSearchLoc-1));
				DO_LOG_DEBUG("Key: " + a_sKey + ", extracted value: " + sValue);
			}
		} while(0);
	}
	catch(const std::exception& e)
	{
		DO_LOG_ERROR(e.what());
	}
	return sValue;
}

/**
 * Add field to a JSON payload being formed
 * @param a_sMsg	[out] The message updated by this function
 * @param a_sKey	[in]  Key name to be added in JSON
 * @param a_sVal	[in]  value to be added in JSON
 * @param a_bIsLastField [in]  Indicates whether it is a last field in JSON
 * @return true/false based on success/failure
 */
void commonUtilKPI::addFieldToMsg(std::string& a_sMsg, const std::string& a_sKey, const std::string& a_sVal, bool a_bIsLastField)
{
	if(a_sKey.empty() || a_sVal.empty())
	{
		DO_LOG_ERROR("One or more parameters are empty, needed for adding to JSON message.");
		return ;
	}
	a_sMsg = a_sMsg + '"' + a_sKey + '"' + ':' + '"' + a_sVal + '"';
	if(false == a_bIsLastField)
	{
		a_sMsg = a_sMsg + ',';
	}		
};

/**
 * Function to create a write message to be sent as a part of control loop
 *
 * @param a_sMsg	[out] The write message created by this function
 * @param a_sKey	[in]  application sequence number
 * @param a_sWell	[in]  wellhead to be passed in JSON
 * @param a_sPoint	[in]  point name to be passed in JSON
 * @param a_sVal	[in]  value to be passed in JSON
 * @param a_sRT		[in]  RT status to be passed in JSON
 * @return true/false based on success/failure
 */
bool commonUtilKPI::createWriteRequest(std::string& a_sMsg, const std::string& a_sKey,
		const std::string& a_sWell, const std::string& a_sPoint,
		const std::string& a_sVal, const std::string& a_sRT,
		const std::string& a_sTopic, bool a_bIsMQTTModeOn)
{
	try
	{
		a_sMsg.clear();

		if(a_sKey.empty() || a_sWell.empty() || a_sPoint.empty() 
			|| a_sVal.empty() || a_sRT.empty() || a_sTopic.empty())
		{
			DO_LOG_ERROR("One or more parameters are empty, needed for creating a write request");	
			return false;
		}

		addFieldToMsg(a_sMsg, "app_seq", 		a_sKey,		false);
		addFieldToMsg(a_sMsg, "wellhead", 		a_sWell, 	false);
		addFieldToMsg(a_sMsg, "command", 		a_sPoint, 	false);
		addFieldToMsg(a_sMsg, "value", 			a_sVal, 	false);
		addFieldToMsg(a_sMsg, "version", 		"2.0", 		false);
		addFieldToMsg(a_sMsg, "realtime", 		a_sRT, 		false);
		
		std::string sTs{""}, sUsec{""};
		CcommonEnvManager::Instance().getTimeParams(sTs, sUsec);
		if(false == a_bIsMQTTModeOn)
		{
			addFieldToMsg(a_sMsg, "sourcetopic", 	a_sTopic, 	false);
			addFieldToMsg(a_sMsg, "tsMsgRcvdFromMQTT", sUsec, false);
		}

		addFieldToMsg(a_sMsg, "timestamp", 	sTs, 	false);
		addFieldToMsg(a_sMsg, "usec", 		sUsec, 	true);
		
		a_sMsg = "{" + a_sMsg + "}";
	}
	catch(const std::exception& e)
	{
		DO_LOG_ERROR(e.what());
	}

	return true;
}

/**
 * Function to create a analysis message for a control loop
 * @param a_stPollWrData[in]  polling and write data
 * @param a_msgWrResp	[in]  write response message
 * @return string: analysis message
 */
std::string commonUtilKPI::createAnalysisMsg(struct stPollWrData &a_stPollWrData, CMessageObject &a_msgWrResp)
{
	std::string sMsg{""};

	auto answer = [](std::string &a_sMsg, cJSON *a_pSrcMsgRoot, const std::string &a_sSrcKey, std::string a_sFinalKey, bool a_bIsLastField)
	{
		if(NULL != a_pSrcMsgRoot)
		{
			if(! cJSON_HasObjectItem(a_pSrcMsgRoot, a_sSrcKey.c_str()))
			{
				DO_LOG_DEBUG(a_sSrcKey + ": Key not found in message");
				return ;
			}

			char *pcVal = cJSON_GetObjectItem(a_pSrcMsgRoot, a_sSrcKey.c_str())->valuestring;
			if (NULL == pcVal)
			{
				DO_LOG_DEBUG(a_sSrcKey + ": Value for key not found in message");
				return ;
			}
			else
			{
				std::string sData{pcVal};
				addFieldToMsg(a_sMsg, a_sFinalKey, sData, a_bIsLastField);
			}
		}
	};

	try
	{
		cJSON *pRootPollMsg = cJSON_Parse(a_stPollWrData.m_oPollData.getStrMsg().c_str());
		if (NULL == pRootPollMsg)
		{
			DO_LOG_ERROR(a_stPollWrData.m_oPollData.getStrMsg() + ": Message could not be parsed in json format");
			return "";
		}

		// Process poll message
		answer(sMsg, pRootPollMsg,	"driver_seq",	"pollSeq",	 	false);
		answer(sMsg, pRootPollMsg,	"data_topic",	"pollTopic", 	false);
		answer(sMsg, pRootPollMsg,	"realtime",		"pollRT", 	false);
		answer(sMsg, pRootPollMsg, 	"status", 		"pollStatus", 	false);
		answer(sMsg, pRootPollMsg, 	"value", 		"pollValue", 	false);
		answer(sMsg, pRootPollMsg, 	"error_code",	"pollError", 	false);
		answer(sMsg, pRootPollMsg, 	"tsPollingTime", 		"tsPollingTime", 		false);
		answer(sMsg, pRootPollMsg, 	"reqRcvdInStack", 		"pollReqRcvdInStack", 	false);
		answer(sMsg, pRootPollMsg, 	"reqSentByStack", 		"pollReqSentByStack", 	false);
		answer(sMsg, pRootPollMsg, 	"respRcvdByStack", 		"pollRespRcvdByStack", 	false);
		answer(sMsg, pRootPollMsg, 	"respPostedByStack", 	"pollRespPostedByStack",false);
		answer(sMsg, pRootPollMsg, 	"usec",				 	"pollRespPostedToEII",false);
		answer(sMsg, pRootPollMsg, 	"tsMsgRcvdForProcessing","pollDataRcvdInExport",false);
		answer(sMsg, pRootPollMsg, 	"tsMsgReadyForPublish",	"pollDataPostedToMQTT",	false);

		cJSON_Delete(pRootPollMsg);
		pRootPollMsg = NULL;
		
		addFieldToMsg(sMsg, "pollDataRcvdInApp", (std::to_string(commonUtilKPI::get_micros(a_stPollWrData.m_oPollData.getTimestamp()))).c_str(), false);
		addFieldToMsg(sMsg, "wrReqCreation", (std::to_string(commonUtilKPI::get_micros(a_stPollWrData.m_tsStartWrReqCreate))).c_str(), false);

		cJSON *pRootWrRspMsg = cJSON_Parse(a_msgWrResp.getStrMsg().c_str());
		if (NULL == pRootWrRspMsg)
		{
			DO_LOG_ERROR(a_msgWrResp.getStrMsg() + ": Message could not be parsed in json format");
			return "";
		}
		
		// Process write-response message
		answer(sMsg, pRootWrRspMsg,	"app_seq",				"wrSeq",	 		false);
		answer(sMsg, pRootWrRspMsg,	"data_topic",			"wrRspTopic", 		false);
		answer(sMsg, pRootWrRspMsg,	"realtime",				"wrOpRT", 		false);
		answer(sMsg, pRootWrRspMsg, "status", 				"wrRspStatus", 		false);
		answer(sMsg, pRootWrRspMsg, "error_code",			"wrRspError", 		false);
		answer(sMsg, pRootWrRspMsg, "tsMsgRcvdFromMQTT", 	"wrReqRcvdInExport",	false);
		answer(sMsg, pRootWrRspMsg, "tsMsgPublishOnEII", 	"wrReqPublishOnEII",	false);
		answer(sMsg, pRootWrRspMsg, "reqRcvdByApp", 		"wrReqRcvdByModbus",	false);
		answer(sMsg, pRootWrRspMsg, "reqRcvdInStack", 		"wrReqRcvdInStack", 	false);
		answer(sMsg, pRootWrRspMsg, "reqSentByStack", 		"wrReqSentByStack", 	false);
		answer(sMsg, pRootWrRspMsg, "respRcvdByStack", 		"wrRespRcvdByStack", 	false);
		answer(sMsg, pRootWrRspMsg, "respPostedByStack", 	"wrRespPostedByStack",	false);
		answer(sMsg, pRootWrRspMsg, "usec",			 		"wrRespPostedToEII",	false);
		answer(sMsg, pRootWrRspMsg,	"tsMsgRcvdForProcessing","wrRespRcvdInExport",	false);
		answer(sMsg, pRootWrRspMsg,	"tsMsgReadyForPublish",	"wrRespPostedToMQTT",	false);

		cJSON_Delete(pRootWrRspMsg);
		pRootWrRspMsg = NULL;

		// Add last timestamp
		addFieldToMsg(sMsg, "wrRespRcvdInApp", (std::to_string(commonUtilKPI::get_micros(a_msgWrResp.getTimestamp()))).c_str(), true);
	}
	catch(const std::exception& e)
	{
		DO_LOG_ERROR(e.what());
	}

	return sMsg;
}
