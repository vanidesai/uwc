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
#include <queue>
#include <semaphore.h>
#include <mutex>

/// node for writerequest Q
struct stWriteRequest
{
	std::string m_strTopic;
	std::string m_strMsg;
};

class modWriteHandler
{
	std::queue <stWriteRequest> stackTCPWriteReqQ;
	std::mutex __writeReqMutex;
	sem_t semaphoreWriteReq;
	bool m_bIsWriteInitialized;

	modWriteHandler();
	modWriteHandler(modWriteHandler const&);             /// copy constructor is private
	modWriteHandler& operator=(modWriteHandler const&);  /// assignment operator is private

	/// method to initiate the write semaphore
	BOOLEAN initWriteSem();

	/// method to push write request to queue
	BOOLEAN pushToWriteTCPQueue(struct stWriteRequest &stWriteRequestNode);

	/// method to get data from q to process write.
	BOOLEAN getWriteDataToProcess(struct stWriteRequest &stWriteProcessNode);

public:
	static modWriteHandler& Instance();

	eMbusStackErrorCode writeInfoHandler();

	eMbusStackErrorCode jsonParserForWrite(const std::string a_sTopic,
											std::string& writeReqJson,
											MbusAPI_t &stMbusApiPram,
											unsigned char& funcCode);

	void createWriteListener();

	void subscribeDeviceListener(const std::string stTopic);

	bool isWriteInitialized() {return m_bIsWriteInitialized;}

	void initWriteHandlerThreads();
};


#endif /* INCLUDE_MODBUSWRITEHANDLER_HPP_ */
