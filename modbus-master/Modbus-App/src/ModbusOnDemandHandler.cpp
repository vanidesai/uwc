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

/// patterns to be used to find on-demand topic strings
/// topic syntax -
/// for non-RT topic for read - <topic_name>__RdReq
/// for RT topic read RT - <topic_name>__RdReq_RT
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
		DO_LOG_FATAL("Unable to initiate write instance");
		DO_LOG_FATAL(e.what());
	}
}

/**
 * Function to compare two strings.
 * @param stBaseString	:[in] original string from which comparison can be made
 * @param strToCompare	:[in] string value to compare with base string
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
 * Function to generate error response if request json is invalid.
 * @param errorCode	:[in] Error code for invalid json
 * @param u8FunCode	:[in] Request function code
 * @param txID		:[in] Transaction id of request.
 * @param isRT		:[in] Flag to differentiate between RT/Non-RT requests.
 * @param isWrite	:[in] Flag to differentiate between read and write for on-demand request
 */
void onDemandHandler::createErrorResponse(eMbusAppErrorCode errorCode,
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
	objCallback.m_u8ExceptionExcStatus = 0;
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

	/// handle response function to send response on EIS.
	CPeriodicReponseProcessor::Instance().handleResponse(&objCallback,
															operationType,
															strResponseTopic,
															isRT);
}

/**
 * Handler function to start the processing of on-demand requests.
 * @param stRequestData			:[in] Request structure containing topic and request message.
 * @return 	eMbusAppErrorCode : Error code
 */
eMbusAppErrorCode onDemandHandler::onDemandInfoHandler(stRequest& stRequestData)
{
	DO_LOG_DEBUG("Start");

	eMbusAppErrorCode eFunRetType = APP_SUCCESS;
	unsigned char  m_u8FunCode;
	MbusAPI_t stMbusApiPram = {};
	cJSON *root = NULL;

	try
	{
		/// Transaction ID
		stMbusApiPram.m_u16TxId = PublishJsonHandler::instance().getTxId();
		// Assign timestamp for future use
		stMbusApiPram.m_stOnDemandReqData.m_obtReqRcvdTS = stRequestData.m_tsReqRcvd;
#ifdef INSTRUMENTATION_LOG
		DO_LOG_DEBUG("On-demand request::" + stRequestData.m_strMsg);
#endif

		root = cJSON_Parse(stRequestData.m_strMsg.c_str());
		void* ptrAppCallback = NULL;

		/// Comparing request topic for RT/Non-RT requests
		string strSearchString = "_", tempTopic;
		std::size_t found = stRequestData.m_strTopic.find_last_of(strSearchString);
		if (found!=std::string::npos)
		{
			tempTopic = stRequestData.m_strTopic.substr(found+1, stRequestData.m_strTopic.length());
		}
		string strToCompare = "RT";
		stMbusApiPram.m_stOnDemandReqData.m_isRT = compareString(tempTopic, strToCompare);
		bool isWrite = false;

		/// Function called to parse request JSON and fill structure
		eFunRetType = jsonParserForOnDemandRequest(root,
				stMbusApiPram,
				m_u8FunCode,
				stMbusApiPram.m_u16TxId,
				isWrite,
				&ptrAppCallback);

		/// inserting structure to map for retry and to create response JSON.
		if(false == common_Handler::insertReqData(stMbusApiPram.m_u16TxId, stMbusApiPram))
		{
			DO_LOG_WARN("Failed to add MbusAPI_t data to map.");
		}

		if(APP_SUCCESS == eFunRetType && MBUS_MIN_FUN_CODE != m_u8FunCode)
		{
			DO_LOG_DEBUG("On-Demand Process initiated...");
			eFunRetType = (eMbusAppErrorCode) Modbus_Stack_API_Call(
					m_u8FunCode,
					&stMbusApiPram,
					ptrAppCallback);
		}
		else
		{
			if(APP_ERROR_UNKNOWN_SERVICE_REQUEST != eFunRetType)
			{
				std::string strTopic = "";
				/// error response if request JSON is invalid
				createErrorResponse(eFunRetType,
						m_u8FunCode,
						stMbusApiPram.m_u16TxId,
						stMbusApiPram.m_stOnDemandReqData.m_isRT,
						isWrite);
			}
		}
	}
	catch(const std::exception &e)
	{
		eFunRetType = APP_JSON_PARSING_EXCEPTION;
		DO_LOG_FATAL(e.what());
	}

	if(NULL != root)
		cJSON_Delete(root);

	DO_LOG_DEBUG("End");

	return eFunRetType;
}

/**
 * Function to convert values from character to integer.
 * @param input :[in] char to convert to integer
 * @return int	:converted integer value
 */
int char2int(char input)
{
	DO_LOG_DEBUG("Start");
	if(input >= '0' && input <= '9')
		return input - '0';
	if(input >= 'A' && input <= 'F')
		return input - 'A' + 10;
	if(input >= 'a' && input <= 'f')
		return input - 'a' + 10;

	DO_LOG_DEBUG("End");
	throw std::invalid_argument("Invalid input string");
}

/**
 * Function to convert hex value from string to uint8_t array
 * @param src		:[in] hex string to convert to uint8_t
 * @param iOpLen	:[in] length of the value
 * @param target	:[out] to store converted value
 * @return int		: -1 if error else position of output character
 */
int hex2bin(const std::string &src, int iOpLen, uint8_t* target)
{
	DO_LOG_DEBUG("Start");
	int iOpCharPos = 0;
	int i = 0;
	try
	{
		int iLen = src.length();
		if(0 != (iLen%2))
		{
			DO_LOG_ERROR("input string is not proper");
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
			DO_LOG_ERROR("width mismatch " + to_string(iLen) + "!=" + to_string(iOpLen));
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
		DO_LOG_FATAL(e.what());
		return -1;
	}
	DO_LOG_DEBUG("End");
	return iOpCharPos;
}

/**
 * Function to validate request JSON for wellhead & command
 * @param stSourcetopic	:[in] source topic
 * @param stWellhead	:[in] wellhead name
 * @param stCommand		:[in] command name
 * @return 	true : on success,
 * 			false : on error
 */
bool onDemandHandler::validateInputJson(std::string stSourcetopic, std::string stWellhead, std::string stCommand)
{
	DO_LOG_DEBUG("End");
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
		DO_LOG_FATAL(e.what());
		DO_LOG_DEBUG("i/p json wellhead or command mismatch with sourcetopic");
	}

	DO_LOG_DEBUG("End");
	return retValue;
}

/**
 * function to set callback for Read/Write for RT/Non-RT requests.
 * @param ptrAppCallback:[out] function pointer of callback function
 * @param isRTFlag		:[in] flag to decide RT/Non-RT request
 * @param isWriteFlag	:[in] flag to decide read/write request
 * @param stMbusApiPram	:[out] structure to fill retry value if timeout occurs.
 */
void onDemandHandler::setCallbackforOnDemand(void*** ptrAppCallback, bool isRTFlag, bool isWriteFlag, MbusAPI_t &stMbusApiPram)
{
	if(true == isRTFlag && true == isWriteFlag)	/// for RT Write request
	{
		**ptrAppCallback = (void*)OnDemandWriteRT_AppCallback;
		stMbusApiPram.m_nRetry = globalConfig::CGlobalConfig::getInstance().getOpOnDemandWriteConfig().getRTConfig().getRetries();
	}
	else if(false == isRTFlag && true == isWriteFlag)	/// for non-RT Write request
	{
		**ptrAppCallback = (void*)OnDemandWrite_AppCallback;
		stMbusApiPram.m_nRetry = globalConfig::CGlobalConfig::getInstance().getOpOnDemandWriteConfig().getNonRTConfig().getRetries();
	}
	else if(true == isRTFlag && false == isWriteFlag)	//// for RT read request
	{
		**ptrAppCallback = (void*)OnDemandReadRT_AppCallback;
		stMbusApiPram.m_nRetry = globalConfig::CGlobalConfig::getInstance().getOpOnDemandReadConfig().getRTConfig().getRetries();
	}
	else	/// For non-RT read request
	{
		**ptrAppCallback = (void*)OnDemandRead_AppCallback;
		stMbusApiPram.m_nRetry = globalConfig::CGlobalConfig::getInstance().getOpOnDemandReadConfig().getNonRTConfig().getRetries();
	}
}

/**
 * Function to parse request JSON and fill the structure.
 * @param root			:[in] request message in JSON format
 * @param stMbusApiPram	:[out] modbus API param structure to fill from request JSON
 * @param funcCode		:[out] function code of the request
 * @param txID			:[in] request transaction id
 * @param isWrite		:[out] boolean variable to differentiate between read/write request
 * @param ptrAppCallback:[out] function pointer to get callback function
 * @return appropriate error code
 */
eMbusAppErrorCode onDemandHandler::jsonParserForOnDemandRequest(cJSON *root,
											MbusAPI_t &stMbusApiPram,
											unsigned char& funcCode,
											unsigned short txID,
											bool& isWrite,
											void** ptrAppCallback)
{
	if(NULL == root)
	{
		DO_LOG_ERROR(" Invalid input. cJSON root is null");
		return APP_ERROR_INVALID_INPUT_JSON;
	}
	DO_LOG_DEBUG("Start");

	eMbusAppErrorCode eFunRetType = APP_SUCCESS;
	string strCommand, strValue, strWellhead, strVersion, strSourceTopic, strAppSeq;
	network_info::CDataPoint obj;
	bool isValidJson = false;
	try
	{
		if(APP_SUCCESS == eFunRetType)
		{
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

			/// to check all the values are present in request JSON.
			if(cmd && appseq && wellhead && version && timestamp && usec && sourcetopic && mqttTime && eisTime)
			{
				isValidJson = true;
				strAppSeq = appseq->valuestring;
				strCommand = cmd->valuestring;
				strWellhead = wellhead->valuestring;
				strVersion = version->valuestring;
				strSourceTopic = sourcetopic->valuestring;

				stMbusApiPram.m_stOnDemandReqData.m_strAppSeq = strAppSeq;
				stMbusApiPram.m_stOnDemandReqData.m_strMetric = strCommand;
				stMbusApiPram.m_stOnDemandReqData.m_strVersion = strVersion;
				stMbusApiPram.m_stOnDemandReqData.m_strWellhead = strWellhead;
				stMbusApiPram.m_stOnDemandReqData.m_strTopic = strSourceTopic;
				stMbusApiPram.m_stOnDemandReqData.m_strMqttTime = mqttTime->valuestring;
				stMbusApiPram.m_stOnDemandReqData.m_strEisTime = eisTime->valuestring;

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
						DO_LOG_ERROR(" Invalid input json parameter for write request");
						eFunRetType = APP_ERROR_INVALID_INPUT_JSON;
					}
				}

				isValidJson = validateInputJson(strSourceTopic, strWellhead, strCommand);
			}
			if(!isValidJson)
			{
				DO_LOG_ERROR(" Invalid input json parameter or topic.");
				eFunRetType = APP_ERROR_INVALID_INPUT_JSON;
			}

			string strSearchString = "/", stTopic = "";
			std::size_t found = strSourceTopic.find_last_of(strSearchString);
			if (found!=std::string::npos)
			{
				stTopic = strSourceTopic.substr(0, found);
			}
			if(stTopic.empty())
			{
				DO_LOG_ERROR("Topic is not found in request json.");
				eFunRetType = APP_ERROR_INVALID_INPUT_JSON;
			}

			const std::map<std::string, network_info::CUniqueDataPoint>& mpp = network_info::getUniquePointList();
			struct network_info::stModbusAddrInfo addrInfo;
			try
			{
				addrInfo = mpp.at(stTopic).getWellSiteDev().getAddressInfo();
			}
			catch(const std::out_of_range& oor)
			{
				DO_LOG_INFO(" Request is not for this application." + std::string(oor.what()));

				return APP_ERROR_UNKNOWN_SERVICE_REQUEST;
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
			stMbusApiPram.m_stOnDemandReqData.m_isByteSwap = obj.getAddress().m_bIsByteSwap;
			stMbusApiPram.m_stOnDemandReqData.m_isWordSwap = obj.getAddress().m_bIsWordSwap;

			setCallbackforOnDemand(&ptrAppCallback, stMbusApiPram.m_stOnDemandReqData.m_isRT, isWrite, stMbusApiPram);

			stMbusApiPram.m_u16StartAddr = (uint16_t)obj.getAddress().m_iAddress;
			stMbusApiPram.m_u16Quantity = (uint16_t)obj.getAddress().m_iWidth;
			if(true == isWrite)
			{
				if(stMbusApiPram.m_stOnDemandReqData.m_isRT)
				{
					stMbusApiPram.m_lPriority = common_Handler::getReqPriority(
							globalConfig::CGlobalConfig::getInstance().
							getOpOnDemandWriteConfig().getRTConfig());
				}
				else
				{
					stMbusApiPram.m_lPriority = common_Handler::getReqPriority(
							globalConfig::CGlobalConfig::getInstance().
							getOpOnDemandWriteConfig().getNonRTConfig());
				}
			}
			else
			{
				if(stMbusApiPram.m_stOnDemandReqData.m_isRT)
				{
					stMbusApiPram.m_lPriority = common_Handler::getReqPriority(
							globalConfig::CGlobalConfig::getInstance().
							getOpOnDemandReadConfig().getRTConfig());
				}
				else
				{
					stMbusApiPram.m_lPriority = common_Handler::getReqPriority(
							globalConfig::CGlobalConfig::getInstance().
							getOpOnDemandReadConfig().getNonRTConfig());
				}
			}
			network_info::eEndPointType eType = obj.getAddress().m_eType;

			/// to find function code of received requestS
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
				DO_LOG_ERROR(" Invalid type in datapoint:: " + strCommand);
				break;
			}

			if(isWrite && (funcCode == READ_INPUT_REG || funcCode == READ_INPUT_STATUS))
			{
				funcCode = MBUS_MAX_FUN_CODE;
				return APP_ERROR_POINT_IS_NOT_WRITABLE;
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

			//if(NULL != stMbusApiPram.m_pu8Data)
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
						eFunRetType = APP_ERROR_INVALID_INPUT_JSON;
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
						DO_LOG_FATAL("Invalid value in request json.");
						eFunRetType = APP_ERROR_INVALID_INPUT_JSON;
					}
				}
			}
			/*else
			{
				eFunRetType = APP_ERROR_MEMORY_ALLOC_FAILED;
				DO_LOG_ERROR(" Unable to allocate memory. Request not sent");
			}*/
		}
	}
	catch(const std::exception &e)
	{
		eFunRetType = APP_JSON_PARSING_EXCEPTION;
		DO_LOG_FATAL(e.what());
	}

	DO_LOG_DEBUG("End");

	return eFunRetType;
}

/**
 * get operation info from global config depending on the topic name
 * @param topic			:[in] topic for which to retrieve operation info
 * @param operation		:[out] operation info
 * @return 	true : on success,
 * 			false : on error
 */
bool onDemandHandler::getOperation(string topic, globalConfig::COperation& operation)
{
	bool bRet = true;
	if(std::string::npos != topic.find(READ,
			topic.length() - std::string(READ).length(),
			std::string(READ).length()))
	{
		operation = globalConfig::CGlobalConfig::getInstance().
				getOpOnDemandReadConfig().getNonRTConfig();
	}
	else if(std::string::npos != topic.find(READ_RT,
			topic.length() - std::string(READ_RT).length(),
			std::string(READ_RT).length()))
	{
		operation = globalConfig::CGlobalConfig::getInstance().
				getOpOnDemandReadConfig().getRTConfig();
	}
	else if(std::string::npos != topic.find(WRITE,
			topic.length() - std::string(WRITE).length(),
			std::string(WRITE).length()))
	{
		operation = globalConfig::CGlobalConfig::getInstance().
				getOpOnDemandWriteConfig().getNonRTConfig();
	}
	else if(std::string::npos != topic.find(WRITE_RT,
			topic.length() - std::string(WRITE_RT).length(),
			std::string(WRITE_RT).length()))
	{
		operation = globalConfig::CGlobalConfig::getInstance().
				getOpOnDemandWriteConfig().getRTConfig();
	}
	else
	{
		DO_LOG_ERROR("Invalid topic name in SubTopics");
		bRet = false;
	}

	return bRet;
}

/**
 * Function to start thread to listen on all subTopics for on-demand requests
 */
void onDemandHandler::createOnDemandListener()
{
	DO_LOG_DEBUG("Start");

	std::vector<std::string> stTopics = PublishJsonHandler::instance().getSubTopicList();
	for(std::vector<std::string>::iterator it = stTopics.begin(); it != stTopics.end(); ++it)
	{
		if(it->empty()) {
			DO_LOG_ERROR("SubTopics are not configured");
			continue;
		}
		globalConfig::COperation ops;
		if(!getOperation(*it, ops))
		{
			DO_LOG_ERROR("Invalid topic name in SubTopics..hence ignoring");

			// continue iterating next topic
			continue;
		}

		std::thread(&onDemandHandler::subscribeDeviceListener, this, *it, ops).detach();
	}
	DO_LOG_DEBUG("End");
}

/**
 * Function to get OnDemand Handler reference.
 * @return Return reference of OnDemand Handler class
 */
onDemandHandler& onDemandHandler::Instance()
{
	static onDemandHandler _self;
	return _self;
}

/**
 * Function to serialize ZMQ message and send for processing.
 * @param msg	:	[in] actual message
 * @param stTopic:	[in] received topic
 * @return[bool] true: on Success
 * 				 false: On failure
 */
bool onDemandHandler::processMsg(msg_envelope_t *msg, std::string stTopic)
{
	struct stRequest stRequestNode;
	timespec_get(&stRequestNode.m_tsReqRcvd, TIME_UTC);
	msg_envelope_serialized_part_t* parts = NULL;
	int num_parts = 0;
	bool bRet = false;

	if(NULL == msg)
	{
		DO_LOG_ERROR(
				"NULL pointer received while processing msg.");
		return false;
	}
	num_parts = msgbus_msg_envelope_serialize(msg, &parts);
	if(num_parts <= 0)
	{
		DO_LOG_ERROR(
				" Failed to serialize message");
	}

	if(NULL != parts && NULL != parts[0].bytes)
	{
		std::string strMsg(parts[0].bytes);

		stRequestNode.m_strTopic = stTopic;
		stRequestNode.m_strMsg = strMsg;

		DO_LOG_INFO(" on-demand request initiated for msg:: "+ strMsg);
		onDemandInfoHandler(stRequestNode);

		bRet = true;
	}
	else
	{
		DO_LOG_ERROR(
				"NULL pointer received while processing msg.");
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
 * Thread to listen for any on-demand request on ZMQ
 * @param stTopic	:[in] topic to subscribe
 * @param a_refOps	:[in] global configuration value for QOS,retry, RT and operation priority.
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
				DO_LOG_ERROR("Failed to receive message errno ::" + std::to_string(ret));
				continue;
			}
			/// process messages
			processMsg(msg, stTopic);
		}
	}
	catch(const std::exception& e)
	{
		DO_LOG_FATAL(e.what());
	}
}
