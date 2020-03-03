/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#ifndef INCLUDE_INC_PERIODICREAD_HPP_
#define INCLUDE_INC_PERIODICREAD_HPP_

#include "ModbusContainerConfig.hpp"
#include <semaphore.h>

#include <list>
#include <thread>
#include <queue>
#include <algorithm>
#include <mutex>
#include <chrono>
#include <vector>
#include <numeric>
#include "Common.hpp"
//#include "session.hpp"
//#include "cpprest/http_listener.h"
//#include "Httprest.hpp"
#include "cjson/cJSON.h"
#include "PublishJson.hpp"
#include "API.h"

/// node for response Q
struct stStackResponse
{
	stException_t m_stException;
	std::vector<uint8_t> m_Value;
	bool bIsValPresent;
	uint8_t u8Reason;
	uint16_t u16TransacID;
	stTimeStamps m_objStackTimestamps;
	long m_lPriority;
	uint8_t  m_u8FunCode;
	eMbusResponseType m_operationType;
};

class CRefDataForPolling; 

// class for Read periodic
class CPeriodicReponseProcessor{
private:
	std::queue <stStackResponse> stackResQ;
	std::mutex __resQMutex;
	sem_t semaphoreRespProcess;
	bool m_bIsInitialized;

	BOOLEAN pushToQueue(struct stStackResponse &stStackResNode);
	BOOLEAN getDataToProcess(struct stStackResponse &a_stStackResNode);

	BOOLEAN prepareResponseJson(msg_envelope_t** a_pmsg, const CRefDataForPolling* a_objReqData, stStackResponse a_stResp);
	BOOLEAN postResponseJSON(stStackResponse& a_stResp, const CRefDataForPolling* a_objReqData);
	BOOLEAN postResponseJSON(stStackResponse& a_stResp);

	BOOLEAN initSem();
	eMbusStackErrorCode respProcessThreads();

	CPeriodicReponseProcessor();
	CPeriodicReponseProcessor(CPeriodicReponseProcessor const&);             /// copy constructor is private
	CPeriodicReponseProcessor& operator=(CPeriodicReponseProcessor const&);  /// assignment operator is private

public:
	static CPeriodicReponseProcessor& Instance();
	void handleResponse(stMbusAppCallbackParams_t *pstMbusAppCallbackParams, eMbusResponseType respType);
	bool isInitialized() {return m_bIsInitialized;}
	void initRespHandlerThreads();
	BOOLEAN postDummyBADResponse(const CRefDataForPolling& a_objReqData);
};

eMbusStackErrorCode SubscibeOrUnSubPeriodicRead(RestRdPeriodicTagPart_t &lRdPeridList);


#endif /* INCLUDE_INC_PERIODICREAD_HPP_ */
