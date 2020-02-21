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
	std::atomic<uint32_t> m_u32Interval;
	std::atomic<uint32_t> m_u32RemainingInterval;
	
	std::vector<CRefDataForPolling> m_vPolledPoints;
	std::mutex m_vectorMutex;

	public:
	CTimeRecord(uint32_t a_u32Interval, CRefDataForPolling &a_oPoint)
	: m_u32Interval(a_u32Interval), m_u32RemainingInterval(a_u32Interval)
	{
		m_vPolledPoints.push_back(a_oPoint);
	}

	CTimeRecord(CTimeRecord &a_oTimeRecord)
	: m_vPolledPoints(a_oTimeRecord.m_vPolledPoints)
	{
		m_u32Interval.store(a_oTimeRecord.m_u32Interval);
		m_u32RemainingInterval.store(a_oTimeRecord.m_u32Interval);
	}
	
	~CTimeRecord();
	uint32_t decrementTime(uint32_t a_uiSec)
	{
		m_u32RemainingInterval -= a_uiSec;
		if (0 == m_u32RemainingInterval)
		{
			m_u32RemainingInterval.store(m_u32Interval);
			return 0;
		}
		//cout << "Remaining time to send request is :: "<<m_u32RemainingInterval<<endl;
		return m_u32RemainingInterval;
	}
	std::vector<CRefDataForPolling>& getPolledPointList()
	{
		//
		//std::lock_guard<std::mutex> lock(m_vectorMutex);
		return m_vPolledPoints;
	}
	bool add(CRefDataForPolling &a_oPoint);
	uint32_t size() 
	{
		std::lock_guard<std::mutex> lock(m_vectorMutex);
		return (uint32_t)m_vPolledPoints.size(); 
	}
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
	
	//void ioPeriodicReadTimer(int v);

	public:
	// Function to get single instance of this class
	static CTimeMapper& instance()
	{
		static CTimeMapper timeMapper;
		return timeMapper;
	}
	void ioPeriodicReadTimer(int v);
	//static void timerThreadFunc(const boost::system::error_code& e, boost::asio::steady_timer* t);
	void checkTimer();
	void initTimerFunction();

	~CTimeMapper();
	std::vector<CRefDataForPolling>& getPolledPointList(uint32_t uiRef)
	{
		return m_mapTimeRecord.at(uiRef).getPolledPointList();
	}
	bool insert(uint32_t a_uTime, CRefDataForPolling &a_oPoint);
};

class CRequestInitiator
{
private:
	CRequestInitiator(const CRequestInitiator&) = delete;	 			// Copy construct
	CRequestInitiator& operator=(const CRequestInitiator&) = delete;	// Copy assign

	// Default constructor
	CRequestInitiator();

	void threadReqInit();

	std::atomic<unsigned int> m_uiIsNextRequest;
	//std::vector<std::string> m_vsPrevPendingRequests;
	//std::mutex m_mutexPrevReqVector;
	sem_t semaphoreReqProcess;

	std::map<unsigned short, CRefDataForPolling> m_mapTxIDReqData;
	/// mutex for operation on m_mapTxIDReqData map
	std::mutex m_mutextTxIDMap;

	bool init();
	bool sendRequest(CRefDataForPolling a_stRdPrdObj);

	std::queue <uint32_t> m_qReqFreq;
	std::mutex m_mutexReqFreqQ;
	bool getFreqRefForPollCycle(uint32_t &a_uiRef);
	bool pushPollFreqToQueue(uint32_t &a_uiRef);

public:
	~CRequestInitiator();

	// Function to get single instance of this class
	static CRequestInitiator& instance()
	{
		static CRequestInitiator self;
		return self;
	}

	void initiateRequests(uint32_t a_uiRef);

	CRefDataForPolling getTxIDReqData(unsigned short);

	// function to insert new entry in map
	void insertTxIDReqData(unsigned short, CRefDataForPolling);

	// function to remove entry from the map once reply is sent
	void removeTxIDReqData(unsigned short);
};

class CRefDataForPolling
{
	const network_info::CUniqueDataPoint& m_objDataPoint;
	const struct zmq_handler::stZmqContext& m_objBusContext;
	const struct zmq_handler::stZmqPubContext& m_objPubContext;

	uint8_t m_uiFuncCode;
	//CRefDataForPolling& operator=(const CRefDataForPolling&) = delete;	// Copy assign

	public:
	CRefDataForPolling(const CUniqueDataPoint &a_objDataPoint, struct stZmqContext& a_objBusContext, struct stZmqPubContext& a_objPubContext, uint8_t a_uiFuncCode) :
				m_objDataPoint{a_objDataPoint}, m_objBusContext{a_objBusContext}, m_objPubContext{a_objPubContext}, m_uiFuncCode{a_uiFuncCode}
	{
		//std::cout << "\nin CRefDataForPolling ctor";
	}

	uint8_t getFunctionCode() {return m_uiFuncCode;}
	//const CUniqueDataPoint & getDataPoint() {return m_objDataPoint;}
	//const struct stZmqContext & getBusContext() {return m_objBusContext;}

	const CUniqueDataPoint & getDataPoint() const {return m_objDataPoint;}
	const struct stZmqContext & getBusContext() const {return m_objBusContext;}
	const struct stZmqPubContext & getPubContext() const {return m_objPubContext;}
};

/**
 * namespace for Linux based timer API's
 */
namespace LinuxTimer
{
	/**
	 * Function to start Linux timer
	 * @param
	 * lNextTimerTick : use to set next tick count
	 * @return : true on success and false on failure
	 */
	bool start_timer(long lNextTimerTick);

	/**
	 * Function to stop Linux timer
	 * @param : Nothing
	 * @return : true on success and false on failure
	 */
	bool stop_timer(void);

	/**
	 * Function to get timer callback based on signal
	 * @param :
	 * iSignal : signal to get timer callback
	 * @return : Nothing
	 */
	void timer_callback(int iSignal);
}  // namespace LinuxTimer


#endif /* INCLUDE_INC_PERIODICREADFEATURE_HPP_ */
