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
* @param a_pstMbusApiPram	:[in] Structure to read data received from ZMQ
* @param topic				:[in] topic for zmq listening
* @param vpCallback			:[in] set the stack callback as per the operation
* @param a_bIsWriteReq		:[in] flag used to distinguish read/write request for further processing
* @return 	eMbusAppErrorCode : Error code
*/
eMbusAppErrorCode onDemandHandler::onDemandInfoHandler(MbusAPI_t *a_pstMbusApiPram,
		const string a_sTopic,
		void *vpCallback,
		bool a_IsWriteReq)
{

	eMbusAppErrorCode eFunRetType = APP_SUCCESS;
	unsigned char  m_u8FunCode;

	// check for NULL
	if(NULL == a_pstMbusApiPram || NULL == vpCallback)
	{
		DO_LOG_ERROR("NULL pointer received..discarding the request");
		return APP_INTERNAL_ERORR;
	}

	try
	{
		/// Transaction ID
		a_pstMbusApiPram->m_u16TxId = PublishJsonHandler::instance().getTxId();

		/// Function called to parse request JSON and fill structure
		eFunRetType = jsonParserForOnDemandRequest(*a_pstMbusApiPram,
				m_u8FunCode,
				a_pstMbusApiPram->m_u16TxId,
				a_IsWriteReq);

		/// inserting structure to map for retry and to create response JSON.
		if(false == common_Handler::insertReqData(a_pstMbusApiPram->m_u16TxId, *a_pstMbusApiPram))
		{
			DO_LOG_ERROR("Failed to add MbusAPI_t data to map.");
		}

		if(APP_SUCCESS == eFunRetType && MBUS_MIN_FUN_CODE != m_u8FunCode)
		{
			eFunRetType = (eMbusAppErrorCode)Modbus_Stack_API_Call(
					m_u8FunCode,
					a_pstMbusApiPram,
					vpCallback);

			if(APP_SUCCESS != eFunRetType)
			{
				DO_LOG_ERROR("Failed to initiate request from stack");
				eFunRetType = APP_ERROR_REQUEST_SEND_FAILED;
			}
			else
			{
				DO_LOG_DEBUG("Request is successfully sent to end device");
			}
		}
		else
		{
			if(APP_ERROR_UNKNOWN_SERVICE_REQUEST != eFunRetType)
			{
				std::string strTopic = "";
				/// error response if request JSON is invalid
				createErrorResponse(eFunRetType,
						m_u8FunCode,
						a_pstMbusApiPram->m_u16TxId,
						a_pstMbusApiPram->m_stOnDemandReqData.m_isRT,
						a_IsWriteReq);
			}
		}
	}
	catch(const std::exception &e)
	{
		eFunRetType = APP_JSON_PARSING_EXCEPTION;
		DO_LOG_FATAL(e.what());
	}

	return eFunRetType;
}

/**
 * Function to convert values from character to integer.
 * @param input :[in] char to convert to integer
 * @return int	:converted integer value
 */
int char2int(char input)
{
	//DO_LOG_DEBUG("Start");
	if(input >= '0' && input <= '9')
		return input - '0';
	if(input >= 'A' && input <= 'F')
		return input - 'A' + 10;
	if(input >= 'a' && input <= 'f')
		return input - 'a' + 10;

	//DO_LOG_DEBUG("End");
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
	//DO_LOG_DEBUG("End");
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

	//DO_LOG_DEBUG("End");
	return retValue;
}

/**
 * Function to parse request JSON and fill the structure.
 * @param stMbusApiPram		:[out] modbus API param structure to fill from received msg
 * @param funcCode			:[out] function code of the request
 * @param txID				:[in] request transaction id
 * @param a_IsWriteReq		:[out] boolean variable to differentiate between read/write request
 * @return appropriate error code
 */
eMbusAppErrorCode onDemandHandler::jsonParserForOnDemandRequest(MbusAPI_t& a_stMbusApiPram,
											unsigned char& funcCode,
											unsigned short txID,
											const bool a_IsWriteReq)
{
	// locals
	eMbusAppErrorCode eFunRetType = APP_SUCCESS;
	network_info::CDataPoint obj;
	string strSourceTopic, strValue;
	bool isValidJson = false;
	try
	{
		/// to check all the values are present in request JSON.
		if(!a_stMbusApiPram.m_stOnDemandReqData.m_strMetric.empty()
				&& !a_stMbusApiPram.m_stOnDemandReqData.m_strWellhead.empty()
				&& !a_stMbusApiPram.m_stOnDemandReqData.m_strVersion.empty()
				&& !a_stMbusApiPram.m_stOnDemandReqData.m_strTopic.empty()
				&& !a_stMbusApiPram.m_stOnDemandReqData.m_strMqttTime.empty()
				&& !a_stMbusApiPram.m_stOnDemandReqData.m_strEisTime.empty())
		{
			isValidJson = true;

			/// Comparing sourcetopic for read/write request.
			strSourceTopic = a_stMbusApiPram.m_stOnDemandReqData.m_strTopic;

			if(true == a_IsWriteReq)
			{
				if(!a_stMbusApiPram.m_stOnDemandReqData.m_sValue.empty())
				{
					//strValue = value->valuestring;
					strValue = a_stMbusApiPram.m_stOnDemandReqData.m_sValue;
				}
				else
				{
					DO_LOG_ERROR(" Invalid input json parameter for write request");
					eFunRetType = APP_ERROR_INVALID_INPUT_JSON;
				}
			}

			isValidJson = validateInputJson(strSourceTopic,
					a_stMbusApiPram.m_stOnDemandReqData.m_strWellhead,
					a_stMbusApiPram.m_stOnDemandReqData.m_strMetric);
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
		a_stMbusApiPram.m_u16Port = addrInfo.m_stTCP.m_ui16PortNumber;
		a_stMbusApiPram.m_u8DevId = addrInfo.m_stTCP.m_uiUnitID;
		CommonUtils::ConvertIPStringToCharArray(stIpAddress,&(a_stMbusApiPram.m_u8IpAddr[0]));
#else
		a_stMbusApiPram.m_u8DevId = addrInfo.m_stRTU.m_uiSlaveId;
#endif
		a_stMbusApiPram.m_stOnDemandReqData.m_isByteSwap = obj.getAddress().m_bIsByteSwap;
		a_stMbusApiPram.m_stOnDemandReqData.m_isWordSwap = obj.getAddress().m_bIsWordSwap;

		a_stMbusApiPram.m_u16StartAddr = (uint16_t)obj.getAddress().m_iAddress;
		a_stMbusApiPram.m_u16Quantity = (uint16_t)obj.getAddress().m_iWidth;

		network_info::eEndPointType eType = obj.getAddress().m_eType;

		/// to find function code of received requestS
		switch(eType)
		{
		case network_info::eEndPointType::eCoil:
			funcCode = a_IsWriteReq ? WRITE_SINGLE_COIL: READ_COIL_STATUS;
			break;
		case network_info::eEndPointType::eHolding_Register:
			funcCode = a_IsWriteReq ? a_stMbusApiPram.m_u16Quantity == 1 ? WRITE_SINGLE_REG: WRITE_MULTIPLE_REG : READ_HOLDING_REG;
			break;
		case network_info::eEndPointType::eInput_Register:
			funcCode = READ_INPUT_REG;
			break;
		case network_info::eEndPointType::eDiscrete_Input:
			funcCode = READ_INPUT_STATUS;
			break;
		default:
			DO_LOG_ERROR(" Invalid type in datapoint:: " + a_stMbusApiPram.m_stOnDemandReqData.m_strMetric);
			break;
		}

		if(a_IsWriteReq && (funcCode == READ_INPUT_REG || funcCode == READ_INPUT_STATUS))
		{
			funcCode = MBUS_MAX_FUN_CODE;
			return APP_ERROR_POINT_IS_NOT_WRITABLE;
		}

		if(WRITE_MULTIPLE_REG == funcCode)
		{
			a_stMbusApiPram.m_u16ByteCount = a_stMbusApiPram.m_u16Quantity*2;
		}
		else if(WRITE_MULTIPLE_COILS == funcCode)
		{
			uint8_t u8ByteCount = (0 != (a_stMbusApiPram.m_u16Quantity%8))
															?((a_stMbusApiPram.m_u16Quantity/8)+1)
																	:(a_stMbusApiPram.m_u16Quantity/8);

			a_stMbusApiPram.m_u16ByteCount = (uint8_t)u8ByteCount;
		}
		else if(WRITE_SINGLE_COIL == funcCode ||
				WRITE_SINGLE_REG == funcCode)
		{
			a_stMbusApiPram.m_u16ByteCount = MODBUS_SINGLE_REGISTER_LENGTH;
		}

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
		if(true == a_IsWriteReq && funcCode != WRITE_MULTIPLE_COILS)
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
			int retVal = hex2bin(strValue, a_stMbusApiPram.m_u16ByteCount, a_stMbusApiPram.m_pu8Data);
			if(-1 == retVal)
			{
				DO_LOG_FATAL("Invalid value in request json.");
				eFunRetType = APP_ERROR_INVALID_INPUT_JSON;
			}
		}
	}
	catch(const std::exception &e)
	{
		eFunRetType = APP_JSON_PARSING_EXCEPTION;
		DO_LOG_FATAL(e.what());
	}

	return eFunRetType;
}

/**
 * Function will get the realtime parameters per operation required for on-demand operation
 * These parameters will be used for individual threads for on-demand operations
 * @param topic			:[in] topic for which to retrieve operation info
 * @param operation		:[out] operation info
 * @param vpCallback	:[out] set the stack callback as per the operation
 * @param a_iRetry		:[out] set the retry value to be used for application retry mechanism
 * @param a_lPriority	:[out] set the priority value to be used for stack priority queues
 * @param a_bIsWriteReq	:[out] flag used to distinguish read/write request for further processing
 * @param a_bIsRT		:[out] flag used to distinguish RT/NON-RT request for further processing
 * @return true/false based on success/error
 */
bool onDemandHandler::getOperation(string topic, globalConfig::COperation& operation,
		void **vpCallback,
		int& a_iRetry,
		long& a_lPriority,
		bool& a_bIsWriteReq,
		bool& a_bIsRT)
{
	bool bRet = true;
	if(std::string::npos != topic.find(READ,
			topic.length() - std::string(READ).length(),
			std::string(READ).length()))
	{
		operation = globalConfig::CGlobalConfig::getInstance().
				getOpOnDemandReadConfig().getNonRTConfig();
		*vpCallback = (void *)OnDemandRead_AppCallback;

		// get retry value per operation to be used for retry mechanism
		a_iRetry = globalConfig::CGlobalConfig::getInstance().
				getOpOnDemandReadConfig().getNonRTConfig().getRetries();

		/// get the priority
		a_lPriority = common_Handler::getReqPriority(
				globalConfig::CGlobalConfig::getInstance().
				getOpOnDemandReadConfig().getNonRTConfig());

		// set flag to distinguish read/write request
		a_bIsWriteReq = false;
		a_bIsRT = false;
	}
	else if(std::string::npos != topic.find(READ_RT,
			topic.length() - std::string(READ_RT).length(),
			std::string(READ_RT).length()))
	{
		operation = globalConfig::CGlobalConfig::getInstance().
				getOpOnDemandReadConfig().getRTConfig();
		*vpCallback = (void *)OnDemandReadRT_AppCallback;

		// get retry value per operation to be used for retry mechanism
		a_iRetry = globalConfig::CGlobalConfig::getInstance().getOpOnDemandReadConfig().
				getRTConfig().getRetries();

		/// get the priority
		a_lPriority = common_Handler::getReqPriority(
				globalConfig::CGlobalConfig::getInstance().
				getOpOnDemandReadConfig().getRTConfig());

		// set flag to distinguish read/write request
		a_bIsWriteReq = false;

		// set realtime to true
		a_bIsRT = true;
	}
	else if(std::string::npos != topic.find(WRITE,
			topic.length() - std::string(WRITE).length(),
			std::string(WRITE).length()))
	{
		operation = globalConfig::CGlobalConfig::getInstance().
				getOpOnDemandWriteConfig().getNonRTConfig();
		*vpCallback = (void *)OnDemandWrite_AppCallback;

		// get retry value per operation to be used for retry mechanism
		a_iRetry = globalConfig::CGlobalConfig::getInstance().getOpOnDemandWriteConfig().
				getNonRTConfig().getRetries();

		/// get the priority
		a_lPriority = common_Handler::getReqPriority(
				globalConfig::CGlobalConfig::getInstance().
				getOpOnDemandWriteConfig().getNonRTConfig());

		// set flag to distinguish read/write request
		a_bIsWriteReq = true;

		a_bIsRT = false;
	}
	else if(std::string::npos != topic.find(WRITE_RT,
			topic.length() - std::string(WRITE_RT).length(),
			std::string(WRITE_RT).length()))
	{
		operation = globalConfig::CGlobalConfig::getInstance().
				getOpOnDemandWriteConfig().getRTConfig();
		*vpCallback = (void *)OnDemandWriteRT_AppCallback;

		// get retry value per operation to be used for retry mechanism
		a_iRetry = globalConfig::CGlobalConfig::getInstance().
				getOpOnDemandWriteConfig().getRTConfig().getRetries();

		/// get the priority
		a_lPriority = common_Handler::getReqPriority(
				globalConfig::CGlobalConfig::getInstance().
				getOpOnDemandWriteConfig().getRTConfig());

		// set flag to distinguish read/write request
		a_bIsWriteReq = true;

		// set realtime to true
		a_bIsRT = true;
	}
	else
	{
		DO_LOG_ERROR("Invalid topic name in SubTopics");
		bRet = false;
		a_iRetry = 0;
		vpCallback = NULL;
		*vpCallback = NULL;
	}

	return bRet;
}

/**
 * Function to start thread to listen on all subTopics for on-demand requests
 */
void onDemandHandler::createOnDemandListener()
{
	DO_LOG_DEBUG("Start");
	bool bIsRT = false;
	void *vpCallback = NULL;
	int iRetry = 0;
	long a_lPriority;
	bool a_bIsWriteReq = false;

	std::vector<std::string> stTopics = PublishJsonHandler::instance().getSubTopicList();
	for(std::vector<std::string>::iterator it = stTopics.begin(); it != stTopics.end(); ++it)
	{
		if(it->empty()) {
			DO_LOG_ERROR("SubTopics are not configured");
			continue;
		}
		globalConfig::COperation ops;
		if(!getOperation(*it, ops, &vpCallback, iRetry, a_lPriority, a_bIsWriteReq, bIsRT))
		{
			DO_LOG_ERROR("Invalid topic name in SubTopics..hence ignoring");

			// continue iterating next topic
			continue;
		}

		// create separate thread per topic mentioned in SubTopics section in docker-compose.yml file
		std::thread(&onDemandHandler::subscribeDeviceListener, this, *it, ops,
				bIsRT, vpCallback, iRetry,
				a_lPriority,
				a_bIsWriteReq).detach();
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
 * Function to get value from zmq message based on given key
 * @param msg	:	[in] actual message received from ZMQ
 * @param a_sKey:	[in] key to find
 * @return[string]  : on Success return actual value
 * 					: On failure - return empty string
 */
string onDemandHandler::getMsgElement(msg_envelope_t *a_Msg,
		string a_sKey)
{
	msg_envelope_elem_body_t* data = NULL;

	// check for NULL
	if(NULL == a_Msg)
	{
		DO_LOG_ERROR("NULL msg received from ZMQ");
		return "";
	}

	// get the value
	msgbus_ret_t msgRet = msgbus_msg_envelope_get(a_Msg, a_sKey.c_str(), &data);

#ifdef INSTRUMENTATION_LOG
	DO_LOG_DEBUG(a_sKey +":" +data->body.string);
#endif
	if(msgRet != MSG_SUCCESS)
	{
		DO_LOG_ERROR(a_sKey + " key not present in message: ");
	}
	else
	{
		// Since all the parameters are in string format, hence string is returned
		// If any other parameter is added with different data types in JSON payload then return value needs to changed
		if(MSG_ENV_DT_STRING == data->type)
		{
			return data->body.string;
		}
	}
	return "";
}


/**
 * generic function to process message received from ZMQ.
 * @param msg			:[in] actual message received from zmq
 * @param topic			:[in] topic for zmq listening
 * @param a_bIsRT		:[in] flag used to distinguish RT/NON-RT request for further processing
 * @param vpCallback	:[in] set the stack callback as per the operation
 * @param a_iRetry		:[in] set the retry value to be used for application retry mechanism
 * @param a_lPriority	:[in] set the priority value to be used for stack priority queues
 * @param a_bIsWriteReq	:[in] flag used to distinguish read/write request for further processing
 * @return[bool] true: on Success
 * 				 false: On failure
 */
bool onDemandHandler::processMsg(msg_envelope_t *msg,
		std::string stTopic,
		bool a_bIsRT,
		void *vpCallback,
		const int a_iRetry,
		const long a_lPriority,
		const bool a_bIsWriteReq)
{

	MbusAPI_t stMbusApiPram = {};
	struct onDemandmsg zmqMsg;
	timespec_get(&stMbusApiPram.m_stOnDemandReqData.m_obtReqRcvdTS, TIME_UTC);
	bool bRet = false;

	if(NULL == msg || NULL == vpCallback)
	{
		DO_LOG_ERROR("NULL pointer received while processing msg. hence discarding the msg");

		if(msg != NULL)
		{
			msgbus_msg_envelope_destroy(msg);
		}
		msg = NULL;
		return false;
	}

#ifdef INSTRUMENTATION_LOG
		DO_LOG_DEBUG("On-demand request received on "+ stTopic + " realtime:: "+ to_string(a_bIsRT) + " with following parameters::");
#endif

	stMbusApiPram.m_stOnDemandReqData.m_strAppSeq = getMsgElement(msg, "app_seq");
	stMbusApiPram.m_stOnDemandReqData.m_strMetric = getMsgElement(msg, "command");
	stMbusApiPram.m_stOnDemandReqData.m_sValue = getMsgElement(msg, "value");
	stMbusApiPram.m_stOnDemandReqData.m_strWellhead = getMsgElement(msg, "wellhead");
	stMbusApiPram.m_stOnDemandReqData.m_strVersion = getMsgElement(msg, "version");
	stMbusApiPram.m_stOnDemandReqData.m_strTopic = getMsgElement(msg, "sourcetopic");
	stMbusApiPram.m_stOnDemandReqData.m_sTimestamp = getMsgElement(msg, "timestamp");
	stMbusApiPram.m_stOnDemandReqData.m_sUsec = getMsgElement(msg, "usec");
	stMbusApiPram.m_stOnDemandReqData.m_strMqttTime = getMsgElement(msg, "tsMsgRcvdFromMQTT");
	stMbusApiPram.m_stOnDemandReqData.m_strEisTime = getMsgElement(msg, "tsMsgPublishOnEIS");
	stMbusApiPram.m_stOnDemandReqData.m_isRT = a_bIsRT;

	// fill retry and priority used for further processing
	stMbusApiPram.m_nRetry = a_iRetry;
	stMbusApiPram.m_lPriority = a_lPriority;

	onDemandInfoHandler(&stMbusApiPram, stTopic, vpCallback, a_bIsWriteReq);

	if(msg != NULL)
	{
		msgbus_msg_envelope_destroy(msg);
	}
	msg = NULL;

	return bRet;
} 

/**
* Thread to listen for on-demand request on ZMQ for all the topics mentioned in SubTopics section
* @param topic			:[in] topic for zmq listening
* @param operation		:[out] operation info used to set thread parameters
* @param a_bIsRT		:[out] flag used to distinguish RT/NON-RT request for further processing
* @param vpCallback	:[out] set the stack callback as per the operation
* @param a_iRetry		:[out] set the retry value to be used for application retry mechanism
* @param a_lPriority	:[out] set the priority value to be used for stack priority queues
* @param a_bIsWriteReq	:[out] flag used to distinguish read/write request for further processing
* @return Nothing
*/
void onDemandHandler::subscribeDeviceListener(const std::string stTopic,
		const globalConfig::COperation a_refOps,
		bool a_bIsRT,
		void *vpCallback,
		const int a_iRetry,
		const long a_lPriority,
		const bool a_bIsWriteReq)
{
	msg_envelope_t *msg = NULL;
	msgbus_ret_t ret = MSG_SUCCESS;

	if(NULL == vpCallback)
	{
		DO_LOG_ERROR("NULL callback value is received..");
		return;
	}

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
			processMsg(msg, stTopic, a_bIsRT, vpCallback, a_iRetry, a_lPriority, a_bIsWriteReq);
		}
	}
	catch(const std::exception& e)
	{
		DO_LOG_FATAL(e.what());
	}
}
