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

std::mutex writeMutex;

eMbusStackErrorCode modWriteInfo::writeInfoHandler(const std::string a_sTopic, std::string& msg)
{
	BOOST_LOG_SEV(lg, debug) << __func__ << " Start";

	std::lock_guard<std::mutex> lock(writeMutex);

	eMbusStackErrorCode eFunRetType = MBUS_STACK_NO_ERROR;
	RestMbusReqGeneric_t *pstModbusRxPacket = NULL;
	///modbusInterface 	 *MbusInterface = NULL;
	cJSON *root = cJSON_Parse(msg.c_str());
	if(NULL == root)
	{
		return MBUS_JSON_APP_ERROR_EXCEPTION_RISE;
	}
	try
	{
		pstModbusRxPacket = new RestMbusReqGeneric_t();
		static unsigned short refId = 1;
		if(NULL == pstModbusRxPacket)
		{
			if(NULL != root)
					cJSON_Delete(root);
			return MBUS_JSON_APP_ERROR_EXCEPTION_RISE;
		}
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

		pstModbusRxPacket->m_stReqData.m_pu8Data = NULL;
		pstModbusRxPacket->m_u16ReffId = refId;
		refId++;

		if(MBUS_STACK_NO_ERROR == eFunRetType)
			eFunRetType = jsonParserForWrite(a_sTopic, msg, pstModbusRxPacket);

		if(MBUS_STACK_NO_ERROR == eFunRetType)
		{
			modbusInterface MbusInterface;// = new modbusInterface();

			eFunRetType = (eMbusStackErrorCode)MbusInterface.
					MbusApp_Process_Request(pstModbusRxPacket);
		}

	}
	catch(const std::exception &e)
	{
		eFunRetType = MBUS_JSON_APP_ERROR_EXCEPTION_RISE;
		BOOST_LOG_SEV(lg, error) << __func__ << " " << e.what();
	}

	if(NULL != pstModbusRxPacket)
	{
		if(NULL != pstModbusRxPacket->m_stReqData.m_pu8Data)
		{
			delete[](pstModbusRxPacket->m_stReqData.m_pu8Data);
			pstModbusRxPacket->m_stReqData.m_pu8Data = NULL;
		}
		delete(pstModbusRxPacket);
		pstModbusRxPacket = NULL;
	}

	/*if(MbusInterface != NULL)
	{
		delete(MbusInterface);
		MbusInterface = NULL;
	}*/

	if(NULL != root)
		cJSON_Delete(root);

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

eMbusStackErrorCode modWriteInfo::jsonParserForWrite(std::string a_sTopic,
											std::string& msg,
											RestMbusReqGeneric_t *pstModbusRxPacket)
{
	BOOST_LOG_SEV(lg, debug) << __func__ << " Start";
	eMbusStackErrorCode eFunRetType = MBUS_STACK_NO_ERROR;
	cJSON *root = cJSON_Parse(msg.c_str());
	try
	{
		//int quantity, startAddr, funcCode;
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

#ifdef MODBUS_STACK_TCPIP_ENABLED
		string stIpAddress = addrInfo.m_stTCP.m_sIPAddress;
		pstModbusRxPacket->m_u16Port = addrInfo.m_stTCP.m_ui16PortNumber;
#endif

			if(NULL != pstModbusRxPacket)
			{
				network_info::CDataPoint obj = mpp.at(stTopic).getDataPoint();
#ifdef MODBUS_STACK_TCPIP_ENABLED
			pstModbusRxPacket->m_u8DevId = addrInfo.m_stTCP.m_uiUnitID;
			CommonUtils::ConvertIPStringToCharArray(stIpAddress,&(pstModbusRxPacket->m_u8IpAddr[0]));
#else
			pstModbusRxPacket->m_u8DevId = addrInfo.m_stRTU.m_uiSlaveId;
#endif
				pstModbusRxPacket->m_u8FunCode =
						(uint8_t)((obj.getAddress().m_eType == network_info::eEndPointType::eCoil)?
																WRITE_SINGLE_COIL:WRITE_SINGLE_REG);
				pstModbusRxPacket->m_u8NodeId = (uint8_t)1;
				pstModbusRxPacket->m_u16StartAddr = (uint16_t)obj.getAddress().m_iAddress;
				pstModbusRxPacket->m_u16Quantity = (uint16_t)obj.getAddress().m_iWidth;

				if(WRITE_MULTIPLE_REG == pstModbusRxPacket->m_u8FunCode)
				{
					pstModbusRxPacket->m_stReqData.m_u8DataLen = pstModbusRxPacket->m_u16Quantity*2;
				}
				else if(WRITE_MULTIPLE_COILS == pstModbusRxPacket->m_u8FunCode)
				{
					uint8_t u8ByteCount = (0 != (pstModbusRxPacket->m_u16Quantity%8))
													?((pstModbusRxPacket->m_u16Quantity/8)+1)
															:(pstModbusRxPacket->m_u16Quantity/8);

					pstModbusRxPacket->m_stReqData.m_u8DataLen = (uint8_t)u8ByteCount;
				}
				else if(WRITE_SINGLE_COIL == pstModbusRxPacket->m_u8FunCode ||
						WRITE_SINGLE_REG == pstModbusRxPacket->m_u8FunCode)
				{
					pstModbusRxPacket->m_stReqData.m_u8DataLen = MODBUS_SINGLE_REGISTER_LENGTH;
				}
				pstModbusRxPacket->m_stReqData.m_pu8Data = new uint8_t[pstModbusRxPacket->m_stReqData.m_u8DataLen]();
				if(NULL != pstModbusRxPacket->m_stReqData.m_pu8Data)
				{
					if(WRITE_SINGLE_COIL == pstModbusRxPacket->m_u8FunCode)
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
					hex2bin(stValue, pstModbusRxPacket->m_stReqData.m_u8DataLen, pstModbusRxPacket->m_stReqData.m_pu8Data);
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

modWriteInfo& modWriteInfo::Instance()
{
	static modWriteInfo _self;
	return _self;
}

void modWriteHandler::writeToDevicwe(const std::string a_sTopic, std::string msg)
{
	modWriteInfo::Instance().writeInfoHandler(a_sTopic, msg);
}

std::string modWriteHandler::ltrim(const std::string& value)
{
    size_t start = value.find_first_not_of(" ");
    return (start == std::string::npos) ? "" : value.substr(start);
}


std::string modWriteHandler::rtrim(const std::string& value)
{
    size_t end = value.find_last_not_of(" ");
    return (end == std::string::npos) ? "" : value.substr(0, end + 1);
}


std::string modWriteHandler::trim(const std::string& value)
{
    return rtrim(ltrim(value));
}

void modWriteHandler::tokenize(const std::string& tokenizable_data,
                          std::vector<std::string>& tokenized_data,
                          const char delimeter)
{
    std::stringstream topic_stream(tokenizable_data);
    std::string data;

    // Tokenizing based on delimeter
    while(getline(topic_stream, data, delimeter)) {
        trim(data);
        tokenized_data.push_back(data);
    }
}

void modWriteHandler::zmqReadDeviceMessage()
{
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

	for(auto subTopic : stTopics)
	{
		std::vector<std::string> tempTopic;
		tokenize(subTopic, tempTopic, '/');
		std::thread(&modWriteHandler::subscribeDeviceListener, this, tempTopic[1]).detach();
	}
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
	//recv_ctx_t* sub_ctx = NULL;
	int num_parts = 0;
	msgbus_ret_t ret;
	try
	{
		zmq_handler::stZmqContext msgbus_ctx = zmq_handler::getCTX(stTopic);
		//ret = msgbus_subscriber_new(msgbus_ctx.m_pContext, stTopic.c_str(), NULL, &sub_ctx);
		zmq_handler::stZmqSubContext stsub_ctx = zmq_handler::getSubCTX(stTopic);

		while((msgbus_ctx.m_pContext != NULL) && (NULL != stsub_ctx.sub_ctx))
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
				std::string s(parts[0].bytes);
				std::thread(&modWriteHandler::writeToDevicwe, this, stTopic, s).detach();
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

void modWriteHandler::initZmqReadThread()
{
	try
	{
		std::thread{std::bind(&modWriteHandler::zmqReadDeviceMessage, std::ref(*this))}.detach();

	}
	catch(const std::exception& e)
	{
		BOOST_LOG_SEV(lg, error) << __func__ << " " << e.what();
		std::cout << "\nException modWriteHandler ::" << __func__ << ": Unable to initiate instance: " << e.what();
	}
}
