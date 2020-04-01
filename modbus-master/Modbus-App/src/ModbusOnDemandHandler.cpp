/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "Logger.hpp"

#include "utils/YamlUtil.hpp"
#include <thread>
#include <mutex>
#include <functional>

#include "NetworkInfo.hpp"
#include "ConfigManager.hpp"

#include <sys/msg.h>
#include <fstream>
#include <cstdlib>
#include <stdio.h>
#include "eis/utils/json_config.h"
#include "ModbusOnDemandHandler.hpp"

/// stop thread flag
extern std::atomic<bool> g_stopThread;
/// reference if to store request data

// patterns to be used to find on-demand topic strings
// topic syntax -
// for non-RT topic for read - <topic_name>__RdReq
// for RT topic read RT - <topic_name>__RdReq_RT
#define READ	 	"_RdReq"
#define READ_RT 	"_RdReq_RT"
#define WRITE 		"_WrReq"
#define WRITE_RT 	"_WrReq_RT"

/**
 * Constructor
 */
onDemandHandler::onDemandHandler() : m_bIsWriteInitialized(false)
{
	try
	{
		m_bIsWriteInitialized = true;
	}
	catch(const std::exception& e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS("Unable to initiate write instance"));
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}
}

/**
 * Validate input json
 * @param stBaseString	:[in] original string from which comparison can be made
 * @param strToCompare	:[in] string value to compare
 * @return 	true : on success,
 * 			false : on error
 */
bool onDemandHandler::compareString(const std::string stBaseString, const std::string strToCompare)
{
	return std::equal(stBaseString.begin(), stBaseString.end(),
			strToCompare.begin(),
            [](char a, char b) {
                return tolower(a) == tolower(b);
            });
}

/**
 * Create error response
 * @param ptMsg 	:[out] pointer to message envelope structure to fill up
 * @param errorCode	:[in] error code
 * @param u8FunCode	:[in] function code
 * @param strTopic	:[in] topic for which to create error response
 * @param txID		:[in] transaction id
 */
void onDemandHandler::createErrorResponse(eMbusStackErrorCode errorCode,
		uint8_t  u8FunCode,
		unsigned short txID,
		bool isRT,
		bool isWrite)
{
	stMbusAppCallbackParams_t objCallback;
	eMbusCallbackType operationType = MBUS_CALLBACK_ONDEMAND_READ;
	std::string strResponseTopic = "";
	timespec ts{0};
	objCallback.m_u8FunctionCode = u8FunCode;
	objCallback.m_u8ExceptionExcCode = errorCode;
	objCallback.m_u8ExceptionExcStatus = 2;
	objCallback.m_u16TransactionID = txID;
	objCallback.m_objTimeStamps.tsReqRcvd = ts;
	objCallback.m_objTimeStamps.tsReqSent = ts;
	objCallback.m_objTimeStamps.tsRespRcvd = ts;
	objCallback.m_objTimeStamps.tsRespSent = ts;

	if(true == isRT && true == isWrite)
	{
		operationType = MBUS_CALLBACK_ONDEMAND_WRITE_RT;
		strResponseTopic = PublishJsonHandler::instance().getSWriteResponseTopicRT();
	}
	else if(false == isRT && true == isWrite)
	{
		operationType = MBUS_CALLBACK_ONDEMAND_WRITE;
		strResponseTopic = PublishJsonHandler::instance().getSWriteResponseTopic();
	}
	else if(true == isRT && false == isWrite)
	{
		operationType = MBUS_CALLBACK_ONDEMAND_READ_RT;
		strResponseTopic = PublishJsonHandler::instance().getSReadResponseTopicRT();
	}
	else
	{
		operationType = MBUS_CALLBACK_ONDEMAND_READ;
		strResponseTopic = PublishJsonHandler::instance().getSReadResponseTopic();
	}

	// handle response
	CPeriodicReponseProcessor::Instance().handleResponse(&objCallback,
															operationType,
															strResponseTopic);
}

/**
 * Thread function for on-demand info handler
 * @return appropriate error code
 */
eMbusStackErrorCode onDemandHandler::onDemandInfoHandler(stRequest& stRequestData)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start"));

	eMbusStackErrorCode eFunRetType = MBUS_STACK_NO_ERROR;
	unsigned char  m_u8FunCode;
	MbusAPI_t stMbusApiPram = {};
	cJSON *root = NULL;

#ifdef REALTIME_THREAD_PRIORITY
	PublishJsonHandler::instance().set_thread_priority();
#endif
	try
	{
		/// Enter TX Id
		stMbusApiPram.m_u16TxId = PublishJsonHandler::instance().getTxId();
#ifdef INSTRUMENTATION_LOG
		CLogger::getInstance().log(DEBUG, LOGDETAILS("On-demand request::" + stRequestData.m_strMsg));
#endif

		root = cJSON_Parse(stRequestData.m_strMsg.c_str());
		stOnDemandRequest reqData;
		void* ptrAppCallback = NULL;

		/// Comparing request topic for RT/Non-RT requests
		string strSearchString = "_", tempTopic;
		std::size_t found = stRequestData.m_strTopic.find_last_of(strSearchString);
		if (found!=std::string::npos)
		{
			tempTopic = stRequestData.m_strTopic.substr(found+1, stRequestData.m_strTopic.length());
		}
		string strToCompare = "RT";
		reqData.m_isRT = compareString(tempTopic, strToCompare);
		bool isWrite = false;

		eFunRetType = jsonParserForOnDemandRequest(root,
				stMbusApiPram,
				m_u8FunCode,
				stMbusApiPram.m_u16TxId,
				isWrite,
				reqData,
				&ptrAppCallback);

		if(false == common_Handler::insertOnDemandReqData(stMbusApiPram.m_u16TxId, reqData))
		{
			CLogger::getInstance().log(WARN, LOGDETAILS("Failed to add OnDemand data to map."));
		}
		if(false == common_Handler::insertReqData(stMbusApiPram.m_u16TxId, stMbusApiPram))
		{
			CLogger::getInstance().log(WARN, LOGDETAILS("Failed to add MbusAPI_t data to map."));
		}

		if(MBUS_STACK_NO_ERROR == eFunRetType && MBUS_MIN_FUN_CODE != m_u8FunCode)
		{
			CLogger::getInstance().log(DEBUG, LOGDETAILS("On-Demand Process initiated..."));
			eFunRetType = (eMbusStackErrorCode) Modbus_Stack_API_Call(
					m_u8FunCode,
					&stMbusApiPram,
					ptrAppCallback);
		}
		else
		{
			if(MBUS_APP_ERROR_UNKNOWN_SERVICE_REQUEST != eFunRetType)
			{
				std::string strTopic = "";
				createErrorResponse(eFunRetType, m_u8FunCode, stMbusApiPram.m_u16TxId, reqData.m_isRT, isWrite);
			}
		}
	}
	catch(const std::exception &e)
	{
		eFunRetType = MBUS_JSON_APP_ERROR_EXCEPTION_RISE;
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}

	if(NULL != root)
		cJSON_Delete(root);

	if(NULL != stMbusApiPram.m_pu8Data)
	{
		delete[] stMbusApiPram.m_pu8Data;
		stMbusApiPram.m_pu8Data  = NULL;
	}

	CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));

	return eFunRetType;
}

/**
 * Convert char to int
 * @param input :[in] char to convert to int
 * @return converted integer
 */
int char2int(char input)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start"));
	if(input >= '0' && input <= '9')
		return input - '0';
	if(input >= 'A' && input <= 'F')
		return input - 'A' + 10;
	if(input >= 'a' && input <= 'f')
		return input - 'a' + 10;

	CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));
	throw std::invalid_argument("Invalid input string");
}

/**
 * convert hex to binary
 * @param src	:[in] hex string to convert to binary
 * @param iOpLen:[in]iOpLen
 * @param target:[out] uint8_t
 * @return
 */
int hex2bin(const std::string &src, int iOpLen, uint8_t* target)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start"));
	int iOpCharPos = 0;
	int i = 0;
	try
	{
		int iLen = src.length();
		if(0 != (iLen%2))
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("input string is not proper"));
			return -1;
		}
		// Check if hex string starts with 0x. If yes ignore first 2 letters
		if( ('0' == src[0]) && (('X' == src[1]) || ('x' == src[1])) )
		{
			i = 2;
		}
		iLen = iLen - i;
		if((iLen / 2) != iOpLen)
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("width mismatch " + to_string(iLen) + "!=" + to_string(iOpLen)));
			return -1;
		}
		iLen = iLen + i;
		while (i+1 < iLen)
		{
			unsigned char byte1 = char2int(src[i])*16 + char2int(src[i+1]);
			i = i + 2;
			if ((i+1) < iLen)
			{
				unsigned char byte2 = char2int(src[i])*16 + char2int(src[i+1]);
				target[iOpCharPos] = byte2;
				iOpCharPos++;
			}
			target[iOpCharPos] = byte1;
			iOpCharPos++;
			i = i + 2;
		}
	}
	catch(std::exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		return -1;
	}
	CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));
	return iOpCharPos;
}

/**
 * Validate input json
 * @param stSourcetopic	:[in] source topic
 * @param stWellhead	:[in] well head
 * @param stCommand		:[in] command
 * @return 	true : on success,
 * 			false : on error
 */
bool onDemandHandler::validateInputJson(std::string stSourcetopic, std::string stWellhead, std::string stCommand)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));
	bool retValue = false;
	try
	{
		std::size_t found=std::string::npos;
		std::size_t found1=std::string::npos;
		std::size_t found2=std::string::npos;
		std::size_t found3=std::string::npos;
		std::string strSearchStrin = "/", tempWellhead, tempCommand;
		found = stSourcetopic.find(strSearchStrin);
		if (found!=std::string::npos)
		{
			found1 = stSourcetopic.find(strSearchStrin.c_str(), found+1);
		}
		if (found1!=std::string::npos)
		{
			found2 = stSourcetopic.find(strSearchStrin.c_str(), found1+1);
		}
		if (found2!=std::string::npos)
		{
			tempWellhead = stSourcetopic.substr(found1+1, found2-found1-1);
			found3 = stSourcetopic.find(strSearchStrin.c_str(), found2+1);
		}
		if (found3!=std::string::npos)
		{
			tempCommand = stSourcetopic.substr(found2+1, found3-found2-1);
		}

		if(tempWellhead == stWellhead && tempCommand == stCommand)
		{
			retValue = true;
		}
	}
	catch(const std::exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		CLogger::getInstance().log(DEBUG, LOGDETAILS("i/p json wellhead or command mismatch with sourcetopic"));
	}

	CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));
	return retValue;
}

/**
 * function to set callback for Read/Write for RT/Non-RT
 * @param ptrAppCallback:[out] function pointer of callback function
 * @param isRTFlag	:[in] flag to decide RT/Non-RT request
 * @param isWriteFlag	:[in] flag to decide read/write request
 */
void onDemandHandler::setCallbackforOnDemand(void*** ptrAppCallback, bool isRTFlag, bool isWriteFlag, MbusAPI_t &stMbusApiPram)
{
	if(true == isRTFlag && true == isWriteFlag)
	{
		**ptrAppCallback = (void*)OnDemandWriteRT_AppCallback;
		stMbusApiPram.m_nRetry = globalConfig::CGlobalConfig::getInstance().getOpOnDemandWriteConfig().getRTConfig().getRetries();
	}
	else if(false == isRTFlag && true == isWriteFlag)
	{
		**ptrAppCallback = (void*)OnDemandWrite_AppCallback;
		stMbusApiPram.m_nRetry = globalConfig::CGlobalConfig::getInstance().getOpOnDemandWriteConfig().getNonRTConfig().getRetries();
	}
	else if(true == isRTFlag && false == isWriteFlag)
	{
		**ptrAppCallback = (void*)OnDemandReadRT_AppCallback;
		stMbusApiPram.m_nRetry = globalConfig::CGlobalConfig::getInstance().getOpOnDemandReadConfig().getRTConfig().getRetries();
	}
	else
	{
		**ptrAppCallback = (void*)OnDemandRead_AppCallback;
		stMbusApiPram.m_nRetry = globalConfig::CGlobalConfig::getInstance().getOpOnDemandReadConfig().getNonRTConfig().getRetries();
	}
}

/**
 * json parser for on demand requests
 * @param root			:[in] json string
 * @param stMbusApiPram	:[in] modbus API param
 * @param funcCode		:[in] function code
 * @param txID			:[in] transaction id
 * @param reqData		:[in] on demand request
 * @param ptrAppCallback:[out] function pointer of callback function
 * @return appropriate error code
 */
eMbusStackErrorCode onDemandHandler::jsonParserForOnDemandRequest(cJSON *root,
											MbusAPI_t &stMbusApiPram,
											unsigned char& funcCode,
											unsigned short txID,
											bool& isWrite,
											stOnDemandRequest& reqData,
											void** ptrAppCallback)
{
	if(NULL == root)
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS(" Invalid input. cJSON root is null"));
		return MBUS_JSON_APP_ERROR_INVALID_INPUT_PARAMETER;
	}
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start"));

	eMbusStackErrorCode eFunRetType = MBUS_STACK_NO_ERROR;
	string strCommand, strValue, strWellhead, strVersion, strSourceTopic, strAppSeq;
	network_info::CDataPoint obj;
	bool isValidJson = false;
	try
	{
		if(MBUS_STACK_NO_ERROR == eFunRetType)
		{
			stMbusApiPram.m_pu8Data = NULL;

			cJSON *appseq = cJSON_GetObjectItem(root,"app_seq");
			cJSON *cmd=cJSON_GetObjectItem(root,"command");
			cJSON *value=cJSON_GetObjectItem(root,"value");
			cJSON *wellhead=cJSON_GetObjectItem(root,"wellhead");
			cJSON *version=cJSON_GetObjectItem(root,"version");
			cJSON *sourcetopic=cJSON_GetObjectItem(root,"sourcetopic");
			cJSON *timestamp=cJSON_GetObjectItem(root,"timestamp");
			cJSON *usec=cJSON_GetObjectItem(root,"usec");
			cJSON *mqttTime=cJSON_GetObjectItem(root,"tsMsgRcvdFromMQTT");
			cJSON *eisTime=cJSON_GetObjectItem(root,"tsMsgPublishOnEIS");

			if(cmd && appseq && wellhead && version && timestamp && usec && sourcetopic && mqttTime && eisTime)
			{
				isValidJson = true;
				strAppSeq = appseq->valuestring;
				strCommand = cmd->valuestring;
				strWellhead = wellhead->valuestring;
				strVersion = version->valuestring;
				strSourceTopic = sourcetopic->valuestring;

				reqData.m_strAppSeq = strAppSeq;
				reqData.m_strMetric = strCommand;
				reqData.m_strVersion = strVersion;
				reqData.m_strWellhead = strWellhead;
				reqData.m_strTopic = strSourceTopic;
				reqData.m_strMqttTime = mqttTime->valuestring;
				reqData.m_strEisTime = eisTime->valuestring;
				timespec_get(&reqData.m_obtReqRcvdTS, TIME_UTC);

				/// Comparing sourcetopic for read/write request.
				string strSearchString = "/", tempTopic;
				std::size_t found = strSourceTopic.find_last_of(strSearchString);
				if (found!=std::string::npos)
				{
					tempTopic = strSourceTopic.substr(found+1, strSourceTopic.length());
				}
				string strToCompare = "write";
				isWrite = compareString(tempTopic, strToCompare);

				if(true == isWrite)
				{
					if(value)
					{
						strValue = value->valuestring;
					}
					else
					{
						CLogger::getInstance().log(ERROR, LOGDETAILS(" Invalid input json parameter for write request"));
						eFunRetType = MBUS_JSON_APP_ERROR_INVALID_INPUT_PARAMETER;
					}
				}

				isValidJson = validateInputJson(strSourceTopic, strWellhead, strCommand);
			}
			if(!isValidJson)
			{
				CLogger::getInstance().log(ERROR, LOGDETAILS(" Invalid input json parameter or topic."));
				eFunRetType = MBUS_JSON_APP_ERROR_INVALID_INPUT_PARAMETER;
			}

			string strSearchString = "/", stTopic = "";
			std::size_t found = strSourceTopic.find_last_of(strSearchString);
			if (found!=std::string::npos)
			{
				stTopic = strSourceTopic.substr(0, found);
			}
			if(stTopic.empty())
			{
				CLogger::getInstance().log(ERROR, LOGDETAILS("Topic is not found in request json."));
				eFunRetType = MBUS_JSON_APP_ERROR_INVALID_INPUT_PARAMETER;
			}

			const std::map<std::string, network_info::CUniqueDataPoint>& mpp = network_info::getUniquePointList();
			struct network_info::stModbusAddrInfo addrInfo;
			try
			{
				addrInfo = mpp.at(stTopic).getWellSiteDev().getAddressInfo();
			}
			catch(const std::out_of_range& oor)
			{
				CLogger::getInstance().log(INFO, LOGDETAILS(" Request is not for this application." + std::string(oor.what())));

				return MBUS_APP_ERROR_UNKNOWN_SERVICE_REQUEST;
			}

			obj = mpp.at(stTopic).getDataPoint();
#ifdef MODBUS_STACK_TCPIP_ENABLED
			string stIpAddress = addrInfo.m_stTCP.m_sIPAddress;
			stMbusApiPram.m_u16Port = addrInfo.m_stTCP.m_ui16PortNumber;
			stMbusApiPram.m_u8DevId = addrInfo.m_stTCP.m_uiUnitID;
			CommonUtils::ConvertIPStringToCharArray(stIpAddress,&(stMbusApiPram.m_u8IpAddr[0]));
#else
			stMbusApiPram.m_u8DevId = addrInfo.m_stRTU.m_uiSlaveId;
#endif
			reqData.m_isByteSwap = obj.getAddress().m_bIsByteSwap;
			reqData.m_isWordSwap = obj.getAddress().m_bIsWordSwap;

			setCallbackforOnDemand(&ptrAppCallback, reqData.m_isRT, isWrite, stMbusApiPram);

			stMbusApiPram.m_u16StartAddr = (uint16_t)obj.getAddress().m_iAddress;
			stMbusApiPram.m_u16Quantity = (uint16_t)obj.getAddress().m_iWidth;
			if(true == isWrite)
			{
				stMbusApiPram.m_lPriority = ON_DEMAND_WRITE_PRIORITY;
			}
			else
			{
				stMbusApiPram.m_lPriority = ON_DEMAND_READ_PRIORITY;
			}
			network_info::eEndPointType eType = obj.getAddress().m_eType;

			switch(eType)
			{
			case network_info::eEndPointType::eCoil:
				funcCode = isWrite ? WRITE_SINGLE_COIL: READ_COIL_STATUS;
				break;
			case network_info::eEndPointType::eHolding_Register:
				funcCode = isWrite ? stMbusApiPram.m_u16Quantity == 1 ? WRITE_SINGLE_REG: WRITE_MULTIPLE_REG : READ_HOLDING_REG;
				break;
			case network_info::eEndPointType::eInput_Register:
				funcCode = READ_INPUT_REG;
				break;
			case network_info::eEndPointType::eDiscrete_Input:
				funcCode = READ_INPUT_STATUS;
				break;
			default:
				CLogger::getInstance().log(ERROR, LOGDETAILS(" Invalid type in datapoint:: " + strCommand));
				break;
			}

			if(isWrite && (funcCode == READ_INPUT_REG || funcCode == READ_INPUT_STATUS))
			{
				funcCode = MBUS_MAX_FUN_CODE;
				return MBUS_APP_ERROR_IMPROPER_METHOD;
			}

			if(WRITE_MULTIPLE_REG == funcCode)
			{
				stMbusApiPram.m_u16ByteCount = stMbusApiPram.m_u16Quantity*2;
			}
			else if(WRITE_MULTIPLE_COILS == funcCode)
			{
				uint8_t u8ByteCount = (0 != (stMbusApiPram.m_u16Quantity%8))
													?((stMbusApiPram.m_u16Quantity/8)+1)
															:(stMbusApiPram.m_u16Quantity/8);

				stMbusApiPram.m_u16ByteCount = (uint8_t)u8ByteCount;
			}
			else if(WRITE_SINGLE_COIL == funcCode ||
					WRITE_SINGLE_REG == funcCode)
			{
				stMbusApiPram.m_u16ByteCount = MODBUS_SINGLE_REGISTER_LENGTH;
			}
			stMbusApiPram.m_pu8Data = new uint8_t[stMbusApiPram.m_u16ByteCount]();
			if(NULL != stMbusApiPram.m_pu8Data)
			{
				if(WRITE_SINGLE_COIL == funcCode)
				{
					// If value is 0x01, then write 0xFF00
					if( (0 == strValue.compare("0x00")) ||
							(0 == strValue.compare("0X00"))  ||
							(0 == strValue.compare("00")))
					{
						strValue = "0x0000";
					}
					else if( (0 == strValue.compare("0x01")) ||
							(0 == strValue.compare("0X01")) ||
							(0 == strValue.compare("01")))
					{
						strValue = "0xFF00";
					}
					else
					{
						eFunRetType = MBUS_JSON_APP_ERROR_INVALID_INPUT_PARAMETER;
					}
				}
				if(true == isWrite && funcCode != WRITE_MULTIPLE_COILS)
				{
					if((true == obj.getAddress().m_bIsByteSwap || true == obj.getAddress().m_bIsWordSwap))
					{
						std::vector<uint8_t> tempVt;
						int i = 0;
						if( ('0' == strValue[0]) && (('X' == strValue[1]) || ('x' == strValue[1])) )
						{
							i = 2;
						}
						int iLen = strValue.length();
						while(i < iLen)
						{
							unsigned char byte1 = char2int(strValue[i])*16 + char2int(strValue[i+1]);
							tempVt.push_back(byte1);
							i = i+2;
						}
						strValue = common_Handler::swapConversion(tempVt,
								!obj.getAddress().m_bIsByteSwap,
								obj.getAddress().m_bIsWordSwap);
					}
					int retVal = hex2bin(strValue, stMbusApiPram.m_u16ByteCount, stMbusApiPram.m_pu8Data);
					if(-1 == retVal)
					{
						CLogger::getInstance().log(FATAL, LOGDETAILS("Invalid value in request json."));
						eFunRetType = MBUS_JSON_APP_ERROR_INVALID_INPUT_PARAMETER;
					}
				}
			}
			else
			{
				eFunRetType = MBUS_JSON_APP_ERROR_MALLOC_FAILED;
				CLogger::getInstance().log(ERROR, LOGDETAILS(" Unable to allocate memory. Request not sent"));
			}
		}
	}
	catch(const std::exception &e)
	{
		eFunRetType = MBUS_JSON_APP_ERROR_EXCEPTION_RISE;
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}

	CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));

	return eFunRetType;
}

/**
 * get operation info from global config depending on the topic name
 * @param topic			:[in] topic for which to retrieve operation info
 * @param operation		:[out] operation info
 * @return none
 */
bool onDemandHandler::getOperation(string topic, globalConfig::COperation& operation)
{
	bool bRet = true;
	if(std::string::npos != topic.find(READ, topic.length() - std::string(READ).length(), std::string(READ).length()))
	{
		operation = globalConfig::CGlobalConfig::getInstance().getOpOnDemandReadConfig().getNonRTConfig();
	}
	else if(std::string::npos != topic.find(READ_RT, topic.length() - std::string(READ_RT).length(), std::string(READ_RT).length()))
	{
		operation = globalConfig::CGlobalConfig::getInstance().getOpOnDemandReadConfig().getRTConfig();
	}
	else if(std::string::npos != topic.find(WRITE, topic.length() - std::string(WRITE).length(), std::string(WRITE).length()))
	{
		operation = globalConfig::CGlobalConfig::getInstance().getOpOnDemandWriteConfig().getNonRTConfig();
	}
	else if(std::string::npos != topic.find(WRITE_RT, topic.length() - std::string(WRITE_RT).length(), std::string(WRITE_RT).length()))
	{
		operation = globalConfig::CGlobalConfig::getInstance().getOpOnDemandWriteConfig().getRTConfig();
	}
	else
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS("Invalid topic name in SubTopics"));
		bRet = false;
	}

	return bRet;
}

/**
 * Create write listener
 */
void onDemandHandler::createOnDemandListener()
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start"));

	std::vector<std::string> stTopics = PublishJsonHandler::instance().getSubTopicList();
	for(std::vector<std::string>::iterator it = stTopics.begin(); it != stTopics.end(); ++it)
	{
		if(it->empty()) {
			CLogger::getInstance().log(ERROR, LOGDETAILS("SubTopics are not configured"));
			continue;
		}
		globalConfig::COperation ops;
		if(!getOperation(*it, ops))
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("Invalid topic name in SubTopics..hence ignoring"));

			// continue iterating next topic
			continue;
		}

		std::thread(&onDemandHandler::subscribeDeviceListener, this, *it, ops).detach();
	}
	CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));
}

/**
 * Return single instance of this class
 * @return
 */
onDemandHandler& onDemandHandler::Instance()
{
	static onDemandHandler _self;
	return _self;
}

/**
 * Process ZMQ message
 * @param msg	:	[in] actual message
 * @param stTopic:	[in] received topic
 * @param bIsWrite:	[in] flag to distinguish between read and write request
 */
bool onDemandHandler::processMsg(msg_envelope_t *msg, std::string stTopic)
{
	msg_envelope_serialized_part_t* parts = NULL;
	int num_parts = 0;
	bool bRet = false;

	if(NULL == msg)
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS(
				"NULL pointer received while processing msg."));
		return false;
	}
	num_parts = msgbus_msg_envelope_serialize(msg, &parts);
	if(num_parts <= 0)
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS(
				" Failed to serialize message"));
	}

	if(NULL != parts && NULL != parts[0].bytes)
	{
		struct stRequest stRequestNode;
		std::string strMsg(parts[0].bytes);

		stRequestNode.m_strTopic = stTopic;
		stRequestNode.m_strMsg = strMsg;

		CLogger::getInstance().log(INFO, LOGDETAILS(" on-demand request initiated for msg:: "+ strMsg));
		onDemandInfoHandler(stRequestNode);

		bRet = true;
	}
	else
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS(
				"NULL pointer received while processing msg."));
		bRet = false;
	}

	if(parts != NULL)
	{
		msgbus_msg_envelope_serialize_destroy(parts, num_parts);
	}
	if(msg != NULL)
	{
		msgbus_msg_envelope_destroy(msg);
	}
	msg = NULL;
	parts = NULL;

	return bRet;
} 

/**
 * Subscribe to device listener
 * @param stTopic	:[in] topic to subscribe
 */
void onDemandHandler::subscribeDeviceListener(const std::string stTopic,
		const globalConfig::COperation a_refOps)
{
	msg_envelope_t *msg = NULL;
	msgbus_ret_t ret;

	//set the thread priority
	globalConfig::set_thread_sched_param(a_refOps);

	globalConfig::display_thread_sched_attr(stTopic + " subscribeDeviceListener param :: ");
	try
	{
		zmq_handler::stZmqContext msgbus_ctx = zmq_handler::getCTX(stTopic);
		zmq_handler::stZmqSubContext stsub_ctx = zmq_handler::getSubCTX(stTopic);

		while((msgbus_ctx.m_pContext != NULL) && (NULL != stsub_ctx.sub_ctx)
				&& (false == g_stopThread.load()))
		{
			ret = msgbus_recv_wait(msgbus_ctx.m_pContext, stsub_ctx.sub_ctx, &msg);
			if(ret != MSG_SUCCESS)
			{
				CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to receive message errno ::" + std::to_string(ret)));
				continue;
			}
			/// process messages
			processMsg(msg, stTopic);
		}
	}
	catch(const std::exception& e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}
}
