/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef INCLUDE_MODBUSWRITEHANDLER_HPP_
#define INCLUDE_MODBUSWRITEHANDLER_HPP_

#include "Common.hpp"
#include <vector>

class modWriteInfo
{
	modWriteInfo(){};
	modWriteInfo(modWriteInfo const&);             /// copy constructor is private
	modWriteInfo& operator=(modWriteInfo const&);  /// assignment operator is private
public:
	static modWriteInfo& Instance();
	eMbusStackErrorCode writeInfoHandler(std::string a_sTopic, std::string& msg);
	eMbusStackErrorCode jsonParserForWrite(const std::string a_sTopic, std::string& writeReqJson, RestMbusReqGeneric_t *pstModbusRxPacket);
};

class modWriteHandler
{
	modWriteHandler(){};
	modWriteHandler(modWriteHandler const&);             /// copy constructor is private
	modWriteHandler& operator=(modWriteHandler const&);  /// assignment operator is private

public:
	static modWriteHandler& Instance();
	void initZmqReadThread();

	void zmqReadDeviceMessage();
	void writeToDevicwe(const std::string a_sTopic, std::string msg);

	void subscribeDeviceListener(const std::string stTopic);

	std::string ltrim(const std::string& value);
	std::string rtrim(const std::string& value);
	std::string trim(const std::string& value);
	void tokenize(const std::string& tokenizable_data,
	                          std::vector<std::string>& tokenized_data,
	                          const char delimeter);
};


#endif /* INCLUDE_MODBUSWRITEHANDLER_HPP_ */
