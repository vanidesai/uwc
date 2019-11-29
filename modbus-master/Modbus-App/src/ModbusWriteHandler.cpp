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
#include "YamlUtil.h"
#include <thread>
#include <mutex>
#include "cJSON.h"
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
	modbusInterface 	 *MbusInterface = NULL;
	cJSON *root = cJSON_Parse(msg.c_str());
	try
	{
		pstModbusRxPacket = new RestMbusReqGeneric_t();
		static unsigned short refId = 1;
		/// restarting refId once reached max. limit.
		if(refId >= 65535)
			refId = 1;

		string stAppSeqNum = cJSON_GetObjectItem(root,"app_seq")->valuestring;
		zmq_handler::insertAppSeq(refId, stAppSeqNum);

		pstModbusRxPacket->m_stReqData.m_pu8Data = NULL;
		pstModbusRxPacket->m_u16ReffId = refId;
		refId++;

		eFunRetType = jsonParserForWrite(a_sTopic, msg, pstModbusRxPacket);

		if(MBUS_STACK_NO_ERROR == eFunRetType)
		{
			MbusInterface = new modbusInterface();

			eFunRetType = (eMbusStackErrorCode)MbusInterface->
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

	if(MbusInterface != NULL)
	{
		delete(MbusInterface);
		MbusInterface = NULL;
	}

	if(NULL != root)
		cJSON_Delete(root);

	BOOST_LOG_SEV(lg, debug) << __func__ << " End";
	return eFunRetType;
}

int char2int(char input)
{
	if(input >= '0' && input <= '9')
		return input - '0';
	if(input >= 'A' && input <= 'F')
		return input - 'A' + 10;
	if(input >= 'a' && input <= 'f')
		return input - 'a' + 10;

	throw std::invalid_argument("Invalid input string");
}

int hex2bin(const std::string &src, int iOpLen, uint8_t* target)
{
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
	return iOpCharPos;
}

eMbusStackErrorCode modWriteInfo::jsonParserForWrite(const std::string a_sTopic,
											std::string& msg,
											RestMbusReqGeneric_t *pstModbusRxPacket)
{
	BOOST_LOG_SEV(lg, debug) << __func__ << " Start";
	eMbusStackErrorCode eFunRetType = MBUS_STACK_NO_ERROR;
	cJSON *root = cJSON_Parse(msg.c_str());
	try
	{
		int quantity, startAddr, funcCode;

		string stCommand = cJSON_GetObjectItem(root,"command")->valuestring;
		a_sTopic.erase(a_sTopic.end()-6, a_sTopic.end());	/// 6 is the length of "_Write"
		string stTopic = a_sTopic + SEPARATOR_CHAR + stCommand;

		std::map<std::string, network_info::CUniqueDataPoint> mpp = network_info::getUniquePointList();
		struct network_info::stModbusAddrInfo addrInfo = mpp.at(stTopic).getWellSiteDev().getAddressInfo();

		string stIpAddress = addrInfo.m_stTCP.m_sIPAddress;
		unsigned int port = addrInfo.m_stTCP.m_uiPortNumber;

		const std::vector< network_info::CDataPoint> dataPtList = mpp.at(stTopic).
				getWellSiteDev().getDevInfo().getDataPoints();

		for(auto dataPoint : dataPtList)
		{
			if(dataPoint.getID() == stCommand)
			{
				const struct network_info::stDataPointAddress& tessgg = dataPoint.getAddress();
				quantity = tessgg.m_iWidth;		/// quantity
				startAddr = tessgg.m_iAddress;	/// start address
				/// function code
				funcCode = (tessgg.m_eType ==  network_info::eEndPointType::eCoil)?WRITE_SINGLE_COIL:WRITE_SINGLE_REG;
			}
		}

		if(NULL != pstModbusRxPacket)
		{
			CommonUtils::ConvertIPStringToCharArray(stIpAddress,&(pstModbusRxPacket->m_u8IpAddr[0]));
			pstModbusRxPacket->m_u8FunCode = (uint8_t)funcCode;
			pstModbusRxPacket->m_u8NodeId = (uint8_t)1;
			pstModbusRxPacket->m_u16StartAddr = (uint16_t)startAddr;
			pstModbusRxPacket->m_u16Quantity = (uint16_t)quantity;

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
			// TODO fill data to structure
			string stValue = cJSON_GetObjectItem(root,"value")->valuestring;
			hex2bin(stValue, pstModbusRxPacket->m_stReqData.m_u8DataLen, pstModbusRxPacket->m_stReqData.m_pu8Data);
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

		while(msgbus_ctx.m_pContext != NULL)
		{
			ret = msgbus_recv_wait(msgbus_ctx.m_pContext, stsub_ctx.sub_ctx, &msg);
			if(ret != MSG_SUCCESS)
			{
				// Interrupt is an acceptable error
				if(ret == MSG_ERR_EINTR)
					break;
				BOOST_LOG_SEV(lg, error) << __func__ <<
						" Failed to receive message (errno: %d)" << ret;
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
