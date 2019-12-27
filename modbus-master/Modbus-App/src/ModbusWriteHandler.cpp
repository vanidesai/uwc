/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "ModbusWriteHandler.hpp"
#include "BoostLogger.hpp"
#include "utils/YamlUtil.hpp"
#include <thread>
#include <mutex>
#include "cjson/cJSON.h"
#include "NetworkInfo.hpp"
#include "ZmqHandler.hpp"
#include "ConfigManager.hpp"

#include <sys/msg.h>
#include <fstream>
#include <cstdlib>
#include <stdio.h>
#include "eis/msgbus/msgbus.h"
#include "eis/utils/json_config.h"

/// stop thread flag
extern std::atomic<bool> g_stopThread;
/// reference if to store request data
std::atomic<unsigned short> refId;

modWriteHandler::modWriteHandler() : m_bIsWriteInitialized(false)
{
	try
	{
		initWriteSem();
		m_bIsWriteInitialized = true;
	}
	catch(const std::exception& e)
	{
		BOOST_LOG_SEV(lg, warning) << __func__ << " : Unable to initiate write instance: Exception: " << e.what();
		std::cout << "\nException modWriteHandler ::" << __func__ << ": Unable to initiate instance: " << e.what();
	}
}

bool modWriteHandler::initWriteSem()
{
	//
	int ok = sem_init(&semaphoreWriteReq, 0, 0);
	if (ok == -1) {
	   std::cout << "*******Could not create unnamed write semaphore\n";
	   return false;
	}
	return true;
}

eMbusStackErrorCode modWriteHandler::writeInfoHandler()
{
	BOOST_LOG_SEV(lg, debug) << __func__ << " Start";

	stWriteRequest writeReq;
	eMbusStackErrorCode eFunRetType = MBUS_STACK_NO_ERROR;
	unsigned char  m_u8FunCode;
	MbusAPI_t stMbusApiPram = {};
	cJSON *root = NULL;

	while(false == g_stopThread.load())
	{
		do
		{
			sem_wait(&semaphoreWriteReq);
			try
			{
				if(false == getWriteDataToProcess(writeReq))
				{
					return MBUS_STACK_ERROR_QUEUE_SEND;
				}

				BOOST_LOG_SEV(lg, error) << __func__ << " write Processing initiated :: " << writeReq.m_strMsg;

				root = cJSON_Parse(writeReq.m_strMsg.c_str());
				if(NULL == root)
				{
					return MBUS_JSON_APP_ERROR_EXCEPTION_RISE;
				}

				refId++;
				/// restarting refId once reached max. limit.
				if(refId >= 65535)
					refId = 1;

				cJSON *cmd1 = cJSON_GetObjectItem(root,"app_seq");
				if(NULL != cmd1)
				{
					string stAppSeqNum = cmd1->valuestring;
					zmq_handler::insertAppSeq(refId, stAppSeqNum);
				}
				else
				{
					std::cout << "Invalid ip json..."<< std::endl;
					BOOST_LOG_SEV(lg, error) << __func__ << " Invalid ip json...";
					eFunRetType =  MBUS_STACK_ERROR_INVALID_INPUT_PARAMETER;
				}

				stMbusApiPram.m_pu8Data = NULL;
				stMbusApiPram.m_u16TxId = refId;


				if(MBUS_STACK_NO_ERROR == eFunRetType)
				{
					eFunRetType = jsonParserForWrite(writeReq.m_strTopic, writeReq.m_strMsg, stMbusApiPram, m_u8FunCode);
				}

				if(MBUS_STACK_NO_ERROR == eFunRetType)
				{
					if(MBUS_MIN_FUN_CODE != m_u8FunCode)
					{
						eFunRetType = Modbus_Stack_API_Call(
								m_u8FunCode,
								&stMbusApiPram,
								(void*)ModbusMaster_AppCallback);
					}
				}
			}
			catch(const std::exception &e)
			{
				eFunRetType = MBUS_JSON_APP_ERROR_EXCEPTION_RISE;
				BOOST_LOG_SEV(lg, error) << __func__ << " " << e.what();
			}

			if(NULL != root)
				cJSON_Delete(root);

			if(NULL != stMbusApiPram.m_pu8Data)
			{
				delete[] stMbusApiPram.m_pu8Data;
				stMbusApiPram.m_pu8Data  = NULL;
			}
		}while(0);
	}

	BOOST_LOG_SEV(lg, debug) << __func__ << " End";
	return eFunRetType;
}

int char2int(char input)
{
	BOOST_LOG_SEV(lg, debug) << __func__ << " Start";
	if(input >= '0' && input <= '9')
		return input - '0';
	if(input >= 'A' && input <= 'F')
		return input - 'A' + 10;
	if(input >= 'a' && input <= 'f')
		return input - 'a' + 10;

	BOOST_LOG_SEV(lg, debug) << __func__ << " End";
	throw std::invalid_argument("Invalid input string");
}

int hex2bin(const std::string &src, int iOpLen, uint8_t* target)
{
	BOOST_LOG_SEV(lg, debug) << __func__ << " Start";
	int iOpCharPos = 0;
	int i = 0;
	try
	{
		int iLen = src.length();
		//std::cout << "\ninput: " << src << " with length: " << iLen << ", output: ";
		if(0 != (iLen%2))
		{
			std::cout << "\n input string is not proper \n";
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
			std::cout << "\n Input length: " << iLen << ", Output length: " << iOpLen << " Mismatch";
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
				std::cout << std::hex << (unsigned short)byte2 << " ";
				target[iOpCharPos] = byte2;
				iOpCharPos++;
			}
			std::cout << std::hex << (unsigned short)byte1 << "\n";
			target[iOpCharPos] = byte1;
			iOpCharPos++;
			i = i + 2;
		}
	}
	catch(std::exception &e)
	{
		std::cout << __func__ << ":Exception: " << e.what() << " while parsing hex string " << src << std::endl;
		return -1;
	}
	BOOST_LOG_SEV(lg, debug) << __func__ << " End";
	return iOpCharPos;
}

eMbusStackErrorCode modWriteHandler::jsonParserForWrite(std::string a_sTopic,
											std::string& msg,
											MbusAPI_t &stMbusApiPram,
											unsigned char& funcCode)
{
	BOOST_LOG_SEV(lg, debug) << __func__ << " Start";
	eMbusStackErrorCode eFunRetType = MBUS_STACK_NO_ERROR;
	cJSON *root = cJSON_Parse(msg.c_str());
	try
	{
		string stCommand, stValue;

		cJSON *cmd=cJSON_GetObjectItem(root,"command");
		cJSON *value=cJSON_GetObjectItem(root,"value");
		if(cmd && value)
		{
			stCommand = cmd->valuestring;
			stValue = value->valuestring;
		}
		else
		{
			std::cout << __func__ << " Invalid input json parameter." << std::endl;
			BOOST_LOG_SEV(lg, error) << __func__ << " Invalid input json parameter.";
			eFunRetType = MBUS_JSON_APP_ERROR_INVALID_INPUT_PARAMETER;
		}
		if(MBUS_STACK_NO_ERROR == eFunRetType)
		{
			a_sTopic.erase(a_sTopic.end()-6, a_sTopic.end());	/// 6 is the length of "_Write"
			string stTopic = a_sTopic + SEPARATOR_CHAR + stCommand;

			std::map<std::string, network_info::CUniqueDataPoint> mpp = network_info::getUniquePointList();
			struct network_info::stModbusAddrInfo addrInfo = mpp.at(stTopic).getWellSiteDev().getAddressInfo();

			//if(NULL != stMbusApiPram)
			{
				network_info::CDataPoint obj = mpp.at(stTopic).getDataPoint();
#ifdef MODBUS_STACK_TCPIP_ENABLED
				string stIpAddress = addrInfo.m_stTCP.m_sIPAddress;
				stMbusApiPram.m_u16Port = addrInfo.m_stTCP.m_ui16PortNumber;
				stMbusApiPram.m_u8DevId = addrInfo.m_stTCP.m_uiUnitID;
				CommonUtils::ConvertIPStringToCharArray(stIpAddress,&(stMbusApiPram.m_u8IpAddr[0]));
#else
				stMbusApiPram.m_u8DevId = addrInfo.m_stRTU.m_uiSlaveId;
#endif

				funcCode = (uint8_t)((obj.getAddress().m_eType == network_info::eEndPointType::eCoil)?
						WRITE_SINGLE_COIL:WRITE_SINGLE_REG);
				stMbusApiPram.m_u16StartAddr = (uint16_t)obj.getAddress().m_iAddress;
				stMbusApiPram.m_u16Quantity = (uint16_t)obj.getAddress().m_iWidth;

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
						if( (0 == stValue.compare("0x00")) || (0 == stValue.compare("0X00"))
								|| (0 == stValue.compare("00")) || (0 == stValue.compare("0")))
						{
							stValue = "0x0000";
						}
						else
						{
							stValue = "0xFF00";
						}
					}
					hex2bin(stValue, stMbusApiPram.m_u16ByteCount, stMbusApiPram.m_pu8Data);
				}
				else
				{
					eFunRetType = MBUS_JSON_APP_ERROR_EXCEPTION_RISE;
					BOOST_LOG_SEV(lg, error) << __func__ << " Unable to allocate memory. Request not sent";
				}
			}
		}
	}
	catch(const std::exception &e)
	{
		eFunRetType = MBUS_JSON_APP_ERROR_EXCEPTION_RISE;
		BOOST_LOG_SEV(lg, error) << __func__ << " " << e.what();
	}

	if(NULL != root)
		cJSON_Delete(root);

	BOOST_LOG_SEV(lg, debug) << __func__ << " End";
	return eFunRetType;
}

//TODO function name change
void modWriteHandler::createWriteListener()
{
	BOOST_LOG_SEV(lg, debug) << __func__ << " Start";
	if(getenv("SubTopics") == NULL)
	{
		cout<< __func__<<" Error:: SubTopics are not configured.." << endl;
		return;
	}

	/// parse all the subtopics
	std::vector<std::string> stTopics = CfgManager::Instance().getEnvConfig().get_topics_from_env("sub");

	if(stTopics.empty())
	{
		std::cout << __func__ << " sub topic is not available. " << std::endl;
		BOOST_LOG_SEV(lg, info) << __func__ << " No subscribe topic is available.";
		return;
	}

	for(auto sTopic : stTopics)
	{
		std::size_t pos = sTopic.find('/');
		if (std::string::npos != pos)
		{
			std::string subTopic(sTopic.substr(pos + 1));
			std::thread(&modWriteHandler::subscribeDeviceListener, this, subTopic).detach();
		}
		else
		{
			BOOST_LOG_SEV(lg, error) << __func__ << " Incorrect topic name format:" << sTopic;
		}
	}
	BOOST_LOG_SEV(lg, debug) << __func__ << " End";
}

modWriteHandler& modWriteHandler::Instance()
{
	static modWriteHandler _self;
	return _self;
}

void modWriteHandler::subscribeDeviceListener(const std::string stTopic)
{
	msg_envelope_t *msg = NULL;
	msg_envelope_serialized_part_t* parts = NULL;
	int num_parts = 0;
	msgbus_ret_t ret;
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

				BOOST_LOG_SEV(lg, error) << __func__ << " Failed to receive message (errno: %d)" << ret;
				continue;
			}

			num_parts = msgbus_msg_envelope_serialize(msg, &parts);
			if(num_parts <= 0)
			{
				BOOST_LOG_SEV(lg, error) << __func__ <<
						" Failed to serialize message";
			}

			if(NULL != parts[0].bytes)
			{
				struct stWriteRequest stWriteRequestNode;
				std::string strMsg(parts[0].bytes);

				stWriteRequestNode.m_strTopic = stTopic;
				stWriteRequestNode.m_strMsg = strMsg;

				BOOST_LOG_SEV(lg, error) << __func__ << " write initiated for msg:: " << strMsg;
				/// pushing write request to q to process.
				pushToWriteTCPQueue(stWriteRequestNode);
			}

			msgbus_msg_envelope_serialize_destroy(parts, num_parts);
			msgbus_msg_envelope_destroy(msg);
			msg = NULL;
			parts = NULL;

			if(parts != NULL)
				msgbus_msg_envelope_serialize_destroy(parts, num_parts);
		}
	}
	catch(const std::exception& e)
	{
		BOOST_LOG_SEV(lg, error) << __func__ << " " << e.what();
		std::cout << "\nException subscribeDeviceListener ::" << __func__ << e.what();
	}
}

bool modWriteHandler::pushToWriteTCPQueue(struct stWriteRequest &stWriteRequestNode)
{
	try
	{
		std::lock_guard<std::mutex> lock(__writeReqMutex);
		stackTCPWriteReqQ.push(stWriteRequestNode);
		// Signal thread to process
		sem_post(&semaphoreWriteReq);

		return true;
	}
	catch(const std::exception& e)
	{
		BOOST_LOG_SEV(lg, warning) << __func__ << " Exception in modWriteHandler: " << e.what();
	}

	return false;
}

void modWriteHandler::initWriteHandlerThreads()
{
	static bool bWriteSpawned = false;
	try
	{
		if(false == bWriteSpawned)
		{
			// Spawn 5 thread to process write request
			//for (int i = 0; i < 5; i++)
			{
				std::thread{std::bind(&modWriteHandler::writeInfoHandler, std::ref(*this))}.detach();
			}
			bWriteSpawned = true;
		}
	}
	catch(const std::exception& e)
	{
		BOOST_LOG_SEV(lg, warning) << __func__ << " : Unable to initiate write handler instance: Exception:" << e.what();
		std::cout << "\nException modWriteHandler ::" << __func__ << ": Unable to initiate write handler instance: " << e.what();
	}
}

bool modWriteHandler::getWriteDataToProcess(struct stWriteRequest &stWriteProcessNode)
{
	BOOST_LOG_SEV(lg, error) << __func__ << " Start";
	try
	{
		std::lock_guard<std::mutex> lock(__writeReqMutex);
		stWriteProcessNode = stackTCPWriteReqQ.front();
		stackTCPWriteReqQ.pop();

		return true;
	}
	catch(const std::exception& e)
	{
		BOOST_LOG_SEV(lg, warning) << __func__ << " Exception in modWriteHandler: " << e.what();
	}

	BOOST_LOG_SEV(lg, error) << __func__ << " End";
	return false;
}
