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
#include "PeriodicReadFeature.hpp"

// Structure used for on-demand operation 
struct onDemandmsg
{
	string app_seq;
	string command;
	string value;
	string wellhead;
	string version;
	string sourcetopic;
	string timestamp;
	string usec;
	string tsMsgRcvdFromMQTT;
	string tsMsgPublishOnEIS;
	struct timespec m_tsReqRcvd;
};

class onDemandHandler
{
	bool m_bIsWriteInitialized;

	onDemandHandler();
	onDemandHandler(onDemandHandler const&);             /// copy constructor is private
	onDemandHandler& operator=(onDemandHandler const&);  /// assignment operator is private

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
	bool getOperation(string a_sTopic,
			globalConfig::COperation& a_Operation,
			void **vpCallback,
			int& a_iRetry,
			long& a_lPriority,
			bool& a_bIsWriteReq,
			bool& a_bIsRT);

public:
	static onDemandHandler& Instance();

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
	bool processMsg(msg_envelope_t *msg, std::string stTopic,
			bool a_bIsRT, void *vpCallback,
			const int a_iRetry,
			const long a_lPriority,
			const bool a_bIsWriteReq);

	/**
	 * Function to get value from zmq message based on given key
	 * @param msg	:	[in] actual message received from ZMQ
	 * @param a_sKey:	[in] key to find
	 * @return[string]  : on Success return actual value
	 * 					: On failure - return empty string
	 */
	string getMsgElement(msg_envelope_t *msg, string a_sKey);

	/**
	* Handler function to start the processing of on-demand requests.
	* @param a_pstMbusApiPram	:[in] Structure to read data received from ZMQ
	* @param topic				:[in] topic for zmq listening
	* @param vpCallback			:[in] set the stack callback as per the operation
	* @param a_bIsWriteReq		:[in] flag used to distinguish read/write request for further processing
	* @return 	eMbusAppErrorCode : Error code
	*/
	eMbusAppErrorCode onDemandInfoHandler(MbusAPI_t *a_pstMbusApiPram,
			const string a_STopic,
			void *vpCallback,
			bool a_IsWriteReq);

	/**
	 * Function to parse request JSON and fill the structure.
	 * @param stMbusApiPram		:[out] modbus API param structure to fill from received msg
	 * @param funcCode			:[out] function code of the request
	 * @param txID				:[in] request transaction id
	 * @param a_IsWriteReq		:[out] boolean variable to differentiate between read/write request
	 * @return appropriate error code
	 */
	eMbusAppErrorCode jsonParserForOnDemandRequest(MbusAPI_t& stMbusApiPram,
											unsigned char& funcCode,
											unsigned short txID,
											const bool a_IsWriteReq);

	void setCallbackforOnDemand(void*** ptrAppCallback, bool isRTFlag, bool isWriteFlag, MbusAPI_t &stMbusApiPram);

	void createOnDemandListener();

	/**
	* Thread to listen for any on-demand request on ZMQ
	* @param topic			:[in] topic for zmq listening
	* @param operation		:[out] operation info used to set thread parameters
	* @param a_bIsRT		:[out] flag used to distinguish RT/NON-RT request for further processing
	* @param vpCallback	:[out] set the stack callback as per the operation
	* @param a_iRetry		:[out] set the retry value to be used for application retry mechanism
	* @param a_lPriority	:[out] set the priority value to be used for stack priority queues
	* @param a_bIsWriteReq	:[out] flag used to distinguish read/write request for further processing
	* @return Nothing
	*/
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
