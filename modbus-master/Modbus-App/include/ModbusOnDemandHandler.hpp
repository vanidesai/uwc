/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

/*** ModbusOnDemandHandler.hpp file performs function for On-Demand requests  */

#ifndef INCLUDE_MODBUSONDEMANDHANDLER_HPP_
#define INCLUDE_MODBUSONDEMANDHANDLER_HPP_

#include "Common.hpp"
#include <vector>
#include <queue>
#include <semaphore.h>
#include <mutex>
#include "eis/msgbus/msgbus.h"
#include "cjson/cJSON.h"
#include "PeriodicReadFeature.hpp"

/**onDemandHandler class includes the On-demand functionality
 * On demand Handler class to start processing of on-demand requests
*/
class onDemandHandler
{
	bool m_bIsWriteInitialized; /** write instance (true or false)*/

	onDemandHandler(); //Default constructor
	onDemandHandler(onDemandHandler const&);             /// copy constructor is private
	onDemandHandler& operator=(onDemandHandler const&);  /// assignment operator is private

	bool getOperation(string a_sTopic,
			globalConfig::COperation& a_Operation,
			void **vpCallback,
			int& a_iRetry,
			long& a_lPriority,
			bool& a_bIsWriteReq,
			bool& a_bIsRT);

public:
	static onDemandHandler& Instance();

	bool processMsg(msg_envelope_t *msg, std::string stTopic,
			bool a_bIsRT, void *vpCallback,
			const int a_iRetry,
			const long a_lPriority,
			const bool a_bIsWriteReq);

	string getMsgElement(msg_envelope_t *msg, string a_sKey);

	eMbusAppErrorCode onDemandInfoHandler(MbusAPI_t *a_pstMbusApiPram,
			const string a_STopic,
			void *vpCallback,
			bool a_IsWriteReq);

	eMbusAppErrorCode jsonParserForOnDemandRequest(MbusAPI_t& stMbusApiPram,
											unsigned char& funcCode,
											unsigned short txID,
											const bool a_IsWriteReq);

	void setCallbackforOnDemand(void*** ptrAppCallback, bool isRTFlag, bool isWriteFlag, MbusAPI_t &stMbusApiPram);

	void createOnDemandListener();

	void subscribeDeviceListener(const std::string stTopic,
			const globalConfig::COperation a_refOps,
			bool a_bIsRT,
			void *vpCallback,
			const int a_iRetry,
			const long a_lPriority,
			const bool a_bIsWriteReq);

	bool isWriteInitialized() {return m_bIsWriteInitialized;}

	bool validateInputJson(std::string stSourcetopic, std::string stWellhead, std::string stCommand);

	void createErrorResponse(eMbusAppErrorCode errorCode,
			uint8_t  u8FunCode,
			unsigned short txID,
			bool isRT,
			bool isWrite);

	bool compareString(const std::string stBaseString, const std::string strToCompare);
};


#endif /* INCLUDE_MODBUSONDEMANDHANDLER_HPP_ */
