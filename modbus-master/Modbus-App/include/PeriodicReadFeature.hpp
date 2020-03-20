/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

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

//#define QOS         1
//#define TIMEOUT     10000L
//#define MAX_RDPRD_REQ_RETRIES 10

using network_info::CUniqueDataPoint;
using zmq_handler::stZmqContext;
using zmq_handler::stZmqPubContext;

class CRefDataForPolling;

class CTimeRecord
{
	private:
	CTimeRecord(const CTimeRecord&) = delete;	 			// Copy construct
	CTimeRecord& operator=(const CTimeRecord&) = delete;	// Copy assign

	// Interval
	std::atomic<uint32_t> m_u32Interval; // in milliseconds
	std::atomic<uint32_t> m_u32RemainingInterval; // in milliseconds
	
	// Cut-off interval
	std::atomic<uint32_t> m_u32CutoffInterval; // in milliseconds
	std::atomic<uint32_t> m_u32RemainingCutoffInterval; // in milliseconds
	std::atomic<bool> m_bIsPollingNow; // Flag is true when polling counter is zero
	std::atomic<bool> m_bIsCutoffNow; // Flag is true when cutoff counter is zero

	std::vector<CRefDataForPolling> m_vPolledPoints;
	std::vector<CRefDataForPolling> m_vPolledPointsRT;
	std::mutex m_vectorMutex;
	bool m_bIsRTAvailable;
	bool m_bIsNonRTAvailable;

	public:
	CTimeRecord(uint32_t a_u32Interval, CRefDataForPolling &a_oPoint);

	CTimeRecord(CTimeRecord &a_oTimeRecord)
	: m_vPolledPoints(a_oTimeRecord.m_vPolledPoints), m_vPolledPointsRT(a_oTimeRecord.m_vPolledPointsRT),
	  m_bIsRTAvailable(a_oTimeRecord.m_bIsRTAvailable), m_bIsNonRTAvailable(a_oTimeRecord.m_bIsNonRTAvailable)
	{
		m_u32Interval.store(a_oTimeRecord.m_u32Interval);
		m_u32RemainingInterval.store(a_oTimeRecord.m_u32Interval);

		m_u32CutoffInterval.store(a_oTimeRecord.m_u32CutoffInterval);
		m_u32RemainingCutoffInterval.store(a_oTimeRecord.m_u32RemainingCutoffInterval);
	}
	
	~CTimeRecord();
	void decrementTimerCounters(uint32_t a_uiMSec)
	{
		// Assign default values to flags
		m_bIsPollingNow.store(false);
		m_bIsCutoffNow.store(false);

		// Decrement counters
		m_u32RemainingInterval -= a_uiMSec;
		m_u32RemainingCutoffInterval -= a_uiMSec;

		// Check if cutoff timer is arrived
		if (0 >= m_u32RemainingCutoffInterval)
		{
			// This is cutoff moment. Set the flag
			m_bIsCutoffNow.store(true);
			// For now, reload the cutoff timer counter with higher than polling frequency
			// This will be reset with proper value once polling frequency is hit
			m_u32RemainingCutoffInterval.store(m_u32Interval*2);
		}
		// Check if polling timer is arrived
		if (0 >= m_u32RemainingInterval)
		{
			// This is polling moment. Set the flag
			m_bIsPollingNow.store(true);
			// Reload the timer counter
			m_u32RemainingInterval.store(m_u32Interval);
			// Load the cutoff counter bcoz polling frequency is hit !
			m_u32RemainingCutoffInterval.store(m_u32CutoffInterval);
		}
	}
	bool isCutoffNow()
	{
		return m_bIsCutoffNow.load();
	}
	bool isPollingNow()
	{
		return m_bIsPollingNow.load();
	}
	uint32_t getIntervalTimerCounter()
	{
		return m_u32RemainingInterval.load();
	}
	uint32_t getCutoffIntervalTimerCounter()
	{
		return m_u32RemainingCutoffInterval.load();
	}

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
		//std::lock_guard<std::mutex> lock(m_vectorMutex);
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

class CTimeMapper
{
	private:
	CTimeMapper(const CTimeMapper&) = delete;	 			// Copy construct
	CTimeMapper& operator=(const CTimeMapper&) = delete;	// Copy assign

	std::map<uint32_t, CTimeRecord> m_mapTimeRecord;
	std::mutex m_mapMutex;
	
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
	void checkTimer(uint32_t a_uiInterval);
	void initTimerFunction();

	~CTimeMapper();
	std::vector<CRefDataForPolling>& getPolledPointList(uint32_t uiRef, bool a_bIsRT)
	{
		if(true == a_bIsRT)
		{
			return m_mapTimeRecord.at(uiRef).getPolledPointListRT();
		}
		return m_mapTimeRecord.at(uiRef).getPolledPointList();
	}
	/*std::vector<CRefDataForPolling>& getPolledPointListRT(uint32_t uiRef)
	{
		return m_mapTimeRecord.at(uiRef).getPolledPointListRT();
	}*/
	bool insert(uint32_t a_uTime, CRefDataForPolling &a_oPoint);

	uint32_t getMinTimerFrequency();
};

class CRequestInitiator
{
private:
	CRequestInitiator(const CRequestInitiator&) = delete;	 			// Copy construct
	CRequestInitiator& operator=(const CRequestInitiator&) = delete;	// Copy assign

	// Default constructor
	CRequestInitiator();

	void threadReqInit(bool isRTPoint);
	void threadCheckCutoffRespInit(bool isRTPoint);

	void initiateRequest(std::vector<CRefDataForPolling>&, bool isRTRequest);

	std::atomic<unsigned int> m_uiIsNextRequest;
	sem_t semaphoreReqProcess, semaphoreRespProcess;
	sem_t semaphoreRTReqProcess, semaphoreRTRespProcess;

	std::map<unsigned short, std::reference_wrapper<CRefDataForPolling>> m_mapTxIDReqData;
	/// mutex for operation on m_mapTxIDReqData map
	std::mutex m_mutextTxIDMap;

	bool init();
	bool sendRequest(CRefDataForPolling &a_stRdPrdObj, uint16_t &m_u16TxId, bool isRTRequest);

	std::queue <uint32_t> m_qReqFreq, m_qRespFreq;
	std::queue <uint32_t> m_qReqFreqRT, m_qRespFreqRT;
	std::mutex m_mutexReqFreqQ, m_mutexRespFreqQ;
	bool getFreqRefForPollCycle(uint32_t &a_uiRef, bool a_bIsRT, bool a_bIsReq);
	bool pushPollFreqToQueue(uint32_t &a_uiRef, CTimeRecord &a_objTimeRecord, bool a_bIsReq);

public:
	~CRequestInitiator();

	// Function to get single instance of this class
	static CRequestInitiator& instance()
	{
		static CRequestInitiator self;
		return self;
	}

	void initiateMessages(uint32_t a_uiRef, CTimeRecord &a_objTimeRecord, bool a_bIsReq);

	CRefDataForPolling& getTxIDReqData(unsigned short);

	// function to insert new entry in map
	void insertTxIDReqData(unsigned short, CRefDataForPolling&);

	// function to remove entry from the map once reply is sent
	void removeTxIDReqData(unsigned short);

	const sem_t& getSemaphoreReqProcess() const {
		return semaphoreReqProcess;
	}

	const sem_t& getSemaphoreRTReqProcess() const {
		return semaphoreRTReqProcess;
	}
};

struct stLastGoodResponse
{
	std::string m_sValue;
	stTimeStamps m_objStackTimestamps;
};

class CRefDataForPolling
{
	const network_info::CUniqueDataPoint& m_objDataPoint;
	const struct zmq_handler::stZmqContext& m_objBusContext;
	const struct zmq_handler::stZmqPubContext& m_objPubContext;

	uint8_t m_uiFuncCode;

	std::atomic<bool> m_bIsRespPosted;

	std::atomic<bool> m_bIsLastRespAvailable;
	stLastGoodResponse m_oLastGoodResponse;
	std::mutex m_mutexLastResp;

	std::atomic<uint16_t> m_uReqTxID;

	public:
	CRefDataForPolling(const CUniqueDataPoint &a_objDataPoint, struct stZmqContext& a_objBusContext, struct stZmqPubContext& a_objPubContext, uint8_t a_uiFuncCode);

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
	const struct stZmqContext & getBusContext() const {return m_objBusContext;}
	const struct stZmqPubContext & getPubContext() const {return m_objPubContext;}

	bool saveGoodResponse(const std::string a_sValue, stTimeStamps a_objStackTimestamps);
	stLastGoodResponse getLastGoodResponse();

	uint16_t getReqTxID() { return m_uReqTxID.load(); };
	void setReqTxID(uint16_t a_uTxID) { m_uReqTxID.store(a_uTxID); };

	bool isLastRespAvailable() const {return m_bIsLastRespAvailable.load();};
};

/**
 * namespace for Periodic timer API's
 */
namespace PeriodicTimer
{
	/**
	 * Function to start Linux timer
	 * @param
	 * interval : interval in milliseconds
	 * @return : nothing
	 */
	void timer_start(uint32_t interval);

	/**
	 * Function to stop Linux timer
	 * @param : Nothing
	 * @return : Nothing
	 */
	void timer_stop(void);

	/**
	 *Function to get timer callback
	 *@param : [in] interval in milliseconds
	 *@return : Nothing
	 */
	void timerThread(uint32_t interval);

}  // namespace LinuxTimer


#endif /* INCLUDE_INC_PERIODICREADFEATURE_HPP_ */
