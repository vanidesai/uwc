/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef INCLUDE_MODBUSONDEMANDHANDLER_HPP_
#define INCLUDE_MODBUSONDEMANDHANDLER_HPP_

#include "Common.hpp"
#include <vector>
#include <queue>
#include <semaphore.h>
#include <mutex>
#include "eis/msgbus/msgbus.h"
#include "cjson/cJSON.h"
#include "ZmqHandler.hpp"
#include "PeriodicReadFeature.hpp"

#define ON_DEMAND_WRITE_PRIORITY 1 	//Write-On Demand Priority set as highest(1)
#define ON_DEMAND_READ_PRIORITY 2 	//Read-On Demand Priority set as 2

/// node for writerequest Q
struct stRequest
{
	std::string m_strTopic;
	std::string m_strMsg;
};

class onDemandHandler
{
	bool m_bIsWriteInitialized;

	onDemandHandler();
	onDemandHandler(onDemandHandler const&);             /// copy constructor is private
	onDemandHandler& operator=(onDemandHandler const&);  /// assignment operator is private

	/**
	 * get operation info from global config depending on the topic name
	 * @param topic			:[in] topic for which to retrieve operation info
	 * @param operation		:[out] operation info
	 * @return none
	 */
	bool getOperation(string topic, globalConfig::COperation& operation);

public:
	static onDemandHandler& Instance();

	/**
	 * Process ZMQ message
	 * @param msg	:	[in] actual message
	 * @param stTopic:	[in] received topic
	 */
	bool processMsg(msg_envelope_t *msg, std::string stTopic);

	eMbusStackErrorCode onDemandInfoHandler(stRequest& stRequest);

	eMbusStackErrorCode jsonParserForOnDemandRequest(cJSON *root,
											MbusAPI_t &stMbusApiPram,
											unsigned char& funcCode,
											unsigned short txID,
											bool& isWrite,
											stOnDemandRequest& reqData,
											void** ptrAppCallback);

	void setCallbackforOnDemand(void*** ptrAppCallback, bool isRTFlag, bool isWriteFlag, MbusAPI_t &stMbusApiPram);

	void createOnDemandListener();

	void subscribeDeviceListener(const std::string stTopic,
			const globalConfig::COperation a_refOps);

	bool isWriteInitialized() {return m_bIsWriteInitialized;}

	bool validateInputJson(std::string stSourcetopic, std::string stWellhead, std::string stCommand);

	void createErrorResponse(eMbusStackErrorCode errorCode,
			uint8_t  u8FunCode,
			unsigned short txID,
			bool isRT,
			bool isWrite);

	bool compareString(const std::string stBaseString, const std::string strToCompare);
};


#endif /* INCLUDE_MODBUSONDEMANDHANDLER_HPP_ */
