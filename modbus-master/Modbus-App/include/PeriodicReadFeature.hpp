/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

/*** PeriodicReadFeature.hpp to record the time, map the time and initiate the request*/

#ifndef INCLUDE_INC_PERIODICREADFEATURE_HPP_
#define INCLUDE_INC_PERIODICREADFEATURE_HPP_

#include <vector>
#include <map>
#include <mutex>
#include <semaphore.h>
#include "NetworkInfo.hpp"
#include "ZmqHandler.hpp"
#include <functional>
#include "PeriodicRead.hpp"
#include "ConfigManager.hpp"

using network_info::CUniqueDataPoint;
using zmq_handler::stZmqContext;
using zmq_handler::stZmqPubContext;

class CRefDataForPolling;

/*class for time record*/
class CTimeRecord
{
	private:
	CTimeRecord(const CTimeRecord&) = delete;	 			// Copy construct
	CTimeRecord& operator=(const CTimeRecord&) = delete;	// Copy assign

	// Interval
	std::atomic<uint32_t> m_u32Interval; // in milliseconds
	
	// Cut-off interval
	std::atomic<uint32_t> m_u32CutoffInterval; // in milliseconds

	std::vector<CRefDataForPolling> m_vPolledPoints; /** vector of polled points*/
	std::vector<CRefDataForPolling> m_vPolledPointsRT; /** vector of RT polled points*/
	std::mutex m_vectorMutex; /** vector mutex*/
	bool m_bIsRTAvailable; /** Real Time available(true or false)*/
	bool m_bIsNonRTAvailable; /** Non RT available (true or false)*/

	public:
	//constructor
	CTimeRecord(uint32_t a_u32Interval, CRefDataForPolling &a_oPoint);

	CTimeRecord(CTimeRecord &a_oTimeRecord)
	: m_vPolledPoints(a_oTimeRecord.m_vPolledPoints), m_vPolledPointsRT(a_oTimeRecord.m_vPolledPointsRT),
	  m_bIsRTAvailable(a_oTimeRecord.m_bIsRTAvailable), m_bIsNonRTAvailable(a_oTimeRecord.m_bIsNonRTAvailable)
	{
		m_u32Interval.store(a_oTimeRecord.m_u32Interval);

		m_u32CutoffInterval.store(a_oTimeRecord.m_u32CutoffInterval);
	}
	
	~CTimeRecord();

	uint32_t getInterval()
	{
		return m_u32Interval;
	}
	uint32_t getCutoffInterval()
	{
		return m_u32CutoffInterval;
	}
	std::vector<CRefDataForPolling>& getPolledPointList()
	{
		return m_vPolledPoints;
	}
	std::vector<CRefDataForPolling>& getPolledPointListRT()
	{
		return m_vPolledPointsRT;
	}
	bool add(CRefDataForPolling &a_oPoint);
	uint32_t size() 
	{
		std::lock_guard<std::mutex> lock(m_vectorMutex);
		return (uint32_t)(m_vPolledPoints.size() + m_vPolledPointsRT.size());
	}
	bool isRTListAvailable() { return m_bIsRTAvailable; };
	bool isNonRTListAvailable() { return m_bIsNonRTAvailable; };
};

/*Structure of polling tracker*/
struct StPollingTracker
{
	uint32_t m_uiPollInterval; /**  polling interval*/
	std::reference_wrapper<CTimeRecord> m_objTimeRecord; /** wrapper for time record*/
	bool m_bIsPolling;/** polling or not(true or false)*/
    //constructor
	StPollingTracker(uint32_t a_uiPollInterval, std::reference_wrapper<CTimeRecord> a_objTimeRecord, bool a_bIsPolling)
		: m_uiPollInterval{a_uiPollInterval}, m_objTimeRecord{a_objTimeRecord}, m_bIsPolling{a_bIsPolling}
	{
	}
};

/*class for time mapper*/
class CTimeMapper
{
	private:
	CTimeMapper(const CTimeMapper&) = delete;	 			// Copy construct
	CTimeMapper& operator=(const CTimeMapper&) = delete;	// Copy assign

	std::map<uint32_t, CTimeRecord> m_mapTimeRecord; /** map for time record*/
	std::mutex m_mapMutex; /** map mutex */
	
	std::map<uint32_t, std::vector<struct StPollingTracker>> m_mapPollingTracker; /** polling tracker map*/

	// Default constructor
	CTimeMapper();
	
	uint32_t gcd(uint32_t num1, uint32_t num2);

	public:
	// Function to get single instance of this class
	static CTimeMapper& instance()
	{
		static CTimeMapper timeMapper;
		return timeMapper;
	}
	void ioPeriodicReadTimer(int v);
	//void checkTimer(uint32_t a_uiInterval, struct timespec& a_tsPollTime);
	void checkTimer(const uint32_t &a_uiMaxCounter, uint32_t a_uiInterval, struct timespec& a_tsPollTime);
	void initTimerFunction();

	/**
	 * get freq index as per frequency
	 * @param a_uFreq: [in]: frequency to find index
	 * @return 	uint32_t : [out] returns actual index at given frequency
	 */
	uint32_t getFreqIndex(const uint32_t a_uFreq);

	~CTimeMapper();
	std::vector<CRefDataForPolling>& getPolledPointList(uint32_t uiRef, bool a_bIsRT)
	{
		if(true == a_bIsRT)
		{
			return m_mapTimeRecord.at(uiRef).getPolledPointListRT();
		}
		return m_mapTimeRecord.at(uiRef).getPolledPointList();
	}

	bool insert(uint32_t a_uTime, CRefDataForPolling &a_oPoint);

	uint32_t getMinTimerFrequency();

	uint32_t preparePollingTracker();
	bool getPollingTrackerList(uint32_t a_uiCounter, std::vector<StPollingTracker> &a_listPollInterval);
	void addToPollingTracker(uint32_t a_uiCounter, CTimeRecord &a_objTimeRecord, bool a_bIsPolling);
};

/*Structure for polling instance
*/
struct StPollingInstance
{
	uint32_t m_uiPollInterval; /** POlling interval*/
	struct timespec m_tsPollTime; /** object of struct timespec*/
};

/*class For request initiation*/
class CRequestInitiator
{
private:
	CRequestInitiator(const CRequestInitiator&) = delete;	 			// Copy construct
	CRequestInitiator& operator=(const CRequestInitiator&) = delete;	// Copy assign

	// Default constructor
	CRequestInitiator();

	void threadReqInit(bool isRTPoint, const globalConfig::COperation& a_refOps);
	void threadCheckCutoffRespInit(bool isRTPoint, const globalConfig::COperation& a_refOps);

	void initiateRequest(struct timespec &a_stPollTimestamp,
			std::vector<CRefDataForPolling>&,
			bool isRTRequest,
			const long a_lPriority,
			int a_nRetry,
			void* a_ptrCallbackFunc);

	std::atomic<unsigned int> m_uiIsNextRequest; /** next request number*/
	sem_t semaphoreReqProcess, semaphoreRespProcess; /** semaphore for request process and response process*/
	sem_t semaphoreRTReqProcess, semaphoreRTRespProcess; /** semaphore fro RT request process and RT response process*/

	std::map<unsigned short, std::reference_wrapper<CRefDataForPolling>> m_mapTxIDReqData; /**  map for transaction request data*/
	std::map<unsigned short, std::reference_wrapper<CRefDataForPolling>> m_mapTxIDReqDataRT; /** map for RT transaction request data*/
	/// mutex for operation on m_mapTxIDReqData map
	std::mutex m_mutextTxIDMap, m_mutextTxIDMapRT; /** mutex for transaction ID map and transcation ID RT map */

	bool init();
	bool sendRequest(CRefDataForPolling &a_stRdPrdObj, uint16_t &m_u16TxId,
			bool isRTRequest, const long a_lPriority, int a_nRetry,
			void* a_ptrCallbackFunc);

	std::queue <struct StPollingInstance> m_qReqFreq, m_qRespFreq; /** queue for request frequest and resposnse queue*/
	std::queue <struct StPollingInstance> m_qReqFreqRT, m_qRespFreqRT;/** queue for RT request frequency and RT response frequency*/
	std::mutex m_mutexReqFreqQ, m_mutexRespFreqQ; /** queue mutex*/
	bool getFreqRefForPollCycle(struct StPollingInstance &a_stPollRef, bool a_bIsRT, bool a_bIsReq);
	bool pushPollFreqToQueue(struct StPollingInstance &a_stPollRef, CTimeRecord &a_objTimeRecord, bool a_bIsReq);

public:
	~CRequestInitiator();

	// Function to get single instance of this class
	static CRequestInitiator& instance()
	{
		static CRequestInitiator self;
		return self;
	}

	void initiateMessages(struct StPollingInstance &a_stPollRef, CTimeRecord &a_objTimeRecord, bool a_bIsReq);

	CRefDataForPolling& getTxIDReqData(unsigned short, bool a_bIsRT);

	// function to insert new entry in map
	void insertTxIDReqData(unsigned short, CRefDataForPolling&, bool a_bIsRT);

	// function to check if a txid is present in a map
	bool isTxIDPresent(unsigned short tokenId, bool a_bIsRT);

	// function to remove entry from the map once reply is sent
	void removeTxIDReqData(unsigned short, bool a_bIsRT);

	const sem_t& getSemaphoreReqProcess() const {
		return semaphoreReqProcess;
	}

	const sem_t& getSemaphoreRTReqProcess() const {
		return semaphoreRTReqProcess;
	}
};

/*structure for last good response
*/
struct stLastGoodResponse
{
	std::string m_sValue; /** data value*/
	std::string m_sLastUsec; /** value of last seconds*/
};

/*class of reference data for polling*/
class CRefDataForPolling
{
	const network_info::CUniqueDataPoint& m_objDataPoint; /**reference of class CUniqueDataPoint*/

	uint8_t m_uiFuncCode; /** code of function*/

	std::atomic<bool> m_bIsRespPosted; /** response posted(true or false)*/

	std::atomic<bool> m_bIsLastRespAvailable; /** last response available(true or false) */
	stLastGoodResponse m_oLastGoodResponse; /** reference of struct m_oLastGoodResponse*/
	struct timespec m_stPollTsForReq; /** reference of struct timespec*/
	std::mutex m_mutexLastResp; /** last response mutex */

	std::atomic<uint16_t> m_uReqTxID; /** Request transaction ID*/

	MbusAPI_t m_stMBusReq; /** reference of struct MbusAPI_t*/
	struct timespec m_stRetryTs; /** reference of struct timespec*/
	int m_iReqRetriedCnt; /** retried request count*/

	CRefDataForPolling& operator=(const CRefDataForPolling&) = delete;	// Copy assign

	public:
	CRefDataForPolling(const CUniqueDataPoint &a_objDataPoint, uint8_t a_uiFuncCode);

	CRefDataForPolling(const CRefDataForPolling &);

	bool isResponsePosted()
	{
		return m_bIsRespPosted.load();
	}

	void setResponsePosted(bool a_bIsPosted)
	{
		m_bIsRespPosted.store(a_bIsPosted);
	}

	uint8_t getFunctionCode() {return m_uiFuncCode;}

	const CUniqueDataPoint & getDataPoint() const {return m_objDataPoint;}

	bool saveGoodResponse(const std::string& a_sValue, const std::string& a_sUsec);
	stLastGoodResponse getLastGoodResponse();

	uint16_t getReqTxID() { return m_uReqTxID.load(); };
	void setReqTxID(uint16_t a_uTxID) { m_uReqTxID.store(a_uTxID); };
	void setDataForNewReq(uint16_t a_uTxID, struct timespec& a_tsPoll);

	bool isLastRespAvailable() const {return m_bIsLastRespAvailable.load();};

	struct timespec getTimestampOfPollReq() const { return m_stPollTsForReq;};
	void setTimestampOfPollReq(struct timespec& a_tsPoll) { m_stPollTsForReq = a_tsPoll;};

	int getRetriedCount() const {return m_iReqRetriedCnt;};
	struct timespec getTsForRetry() const {return m_stRetryTs;};
	void flagRetry(struct timespec a_tsRetryDecided)
	{
		m_stRetryTs = a_tsRetryDecided;
		++m_iReqRetriedCnt;
	}

	MbusAPI_t& getMBusReq() {return m_stMBusReq;};
};

/**
 * namespace for Periodic timer API's
 */
namespace PeriodicTimer
{

	void timer_start(uint32_t interval);

	void timer_stop(void);

	void timerThread(uint32_t interval);

}  // namespace PeriodicTimer


#endif /* INCLUDE_INC_PERIODICREADFEATURE_HPP_ */
