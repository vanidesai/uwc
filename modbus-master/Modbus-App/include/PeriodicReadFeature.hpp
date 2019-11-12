/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#ifndef INCLUDE_INC_PERIODICREADFEATURE_HPP_
#define INCLUDE_INC_PERIODICREADFEATURE_HPP_

//#include "bacDELDef.h"
//#include "BACnetStack.hpp"
#include <vector>
#include <map>
#include <mutex>
//#include "HttpService.hpp"
#include <boost/asio.hpp>
#include <semaphore.h>

//#define QOS         1
#define TIMEOUT     10000L
#define MAX_RDPRD_REQ_RETRIES 10

class CMapHIDRdPeriod {
	private:
	CMapHIDRdPeriod(const CMapHIDRdPeriod&) = delete;	 			// Copy construct
	CMapHIDRdPeriod& operator=(const CMapHIDRdPeriod&) = delete;	// Copy assign

	std::map<std::string, RestRdPeriodicTagPart_t> m_mapHID;
	std::mutex m_mapMutex;
	bool processElement(std::string &a_sHID, RestRdPeriodicTagPart_t &a_stRdPeriod);
	
	// Default constructor
	CMapHIDRdPeriod() {}
	
	public:
	
	// Function to get single instance of this class
	static CMapHIDRdPeriod& instance()
	{
		static CMapHIDRdPeriod mapHIDRdPeriod;
		return mapHIDRdPeriod;
	}
	
	~CMapHIDRdPeriod();
	
	bool getElement(const std::string &a_sHID, RestRdPeriodicTagPart_t &a_oRdPrd);
	bool processElement(RestRdPeriodicTagPart_t &a_stRdPeriod);
	bool removeElement(const std::string &a_sHID);
	void print();
};

class CTimeRecord
{
	private:
	CTimeRecord(const CTimeRecord&) = delete;	 			// Copy construct
	CTimeRecord& operator=(const CTimeRecord&) = delete;	// Copy assign

	// Interval
	std::atomic<uint32_t> m_u32Interval;
	std::atomic<uint32_t> m_u32RemainingInterval;
	
	std::vector<std::string> m_vsRefID;
	std::mutex m_vectorMutex;

	public:
	//CTimeRecord() {}

	CTimeRecord(uint32_t a_u32Interval, const std::string &a_sRefID)
	: m_u32Interval(a_u32Interval), m_u32RemainingInterval(a_u32Interval)
	{
		m_vsRefID.push_back(a_sRefID);
	}

	CTimeRecord(CTimeRecord &a_oTimeRecord)
	: m_vsRefID(a_oTimeRecord.m_vsRefID)
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
	std::vector<std::string> getRefIDList()
	{
		//
		std::lock_guard<std::mutex> lock(m_vectorMutex);
		return m_vsRefID;
	}
	bool add(const std::string &a_sHID);
	bool remove(const std::string &a_sHID);
	uint32_t size() 
	{
		std::lock_guard<std::mutex> lock(m_vectorMutex);
		return (uint32_t)m_vsRefID.size(); 
	}
	void print();
};

class CTimeMapper
{
	private:
	CTimeMapper(const CTimeMapper&) = delete;	 			// Copy construct
	CTimeMapper& operator=(const CTimeMapper&) = delete;	// Copy assign

	//std::map<uint32_t, CTimeRecord&> m_mapTimeRecord;
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
	static void timerThreadFunc(const boost::system::error_code& e, boost::asio::steady_timer* t);
	void checkTimer();
	void initTimerFunction();

	~CTimeMapper();
	
	bool insert(uint32_t a_uTime, const std::string &a_sHID);
	bool remove(uint32_t a_uTime, const std::string &a_sHID);
	void print();
};

class CRequestInitiator
{
private:
	CRequestInitiator(const CRequestInitiator&) = delete;	 			// Copy construct
	CRequestInitiator& operator=(const CRequestInitiator&) = delete;	// Copy assign

	// Default constructor
	CRequestInitiator();

	void threadRequestInit(std::vector<std::string> a_vsRefID);

	std::atomic<unsigned int> m_uiIsNextRequest;
	std::vector<std::string> m_vsPrevPendingRequests;
	//std::mutex m_mutexPrevReqVector;
	sem_t semReqInit;

	bool initSem();
	void addPendingReq(std::vector<std::string> a_vsRefID, unsigned int a_index);
	bool sendRequest(RestRdPeriodicTagPart_t &a_stRdPrdObj);

public:
	~CRequestInitiator();

	// Function to get single instance of this class
	static CRequestInitiator& instance()
	{
		static CRequestInitiator self;
		return self;
	}

	void initiateRequests(std::vector<std::string> a_vsRefID);
};

#endif /* INCLUDE_INC_PERIODICREADFEATURE_HPP_ */
