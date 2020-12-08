/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

/*** ControlLoopHandler.hpp maintains control loop data and operations */

#ifndef INCLUDE_CONTROLLOOPHANDLER_HPP_
#define INCLUDE_CONTROLLOOPHANDLER_HPP_

#include <string>
#include <map>
#include <vector>
#include <thread>
#include "QueueHandler.hpp"
#include "Logger.hpp"

/** class for control loop operations*/
class CControlLoopOp
{
private:
	std::string m_sId; /** site ID value*/
	std::string m_sPolledTopic; /**Polled topic name*/
	std::string m_sWritePointFullPath; /** value write point*/
	std::string m_sWriteDevName; /** name of write device*/
	std::string m_sWriteWellheadName; /** name of well head*/
	std::string m_sWritePointName; /** point name*/
	uint32_t m_uiDelayMs; /** value of delay in milliseconds*/
	std::string m_sVal; /** site value*/
	CQueueHandler m_q; /** object of class CQueueHandler*/
	std::thread m_thread; /** thread id for this control loop handling */

	void threadPollMonitoring();

public:
	CControlLoopOp(uint32_t a_uiId, const std::string &a_sPolledTopic, const std::string &a_sWritePoint, 
		const std::string &a_sWriteDevName, const std::string &a_sWriteWellheadName, const std::string &a_sWritePointName,
		uint32_t a_uiDelayMs, const std::string &a_sVal)
	: m_sId{std::to_string(a_uiId)}, m_sPolledTopic{a_sPolledTopic}, m_sWritePointFullPath{a_sWritePoint}, 
	m_sWriteDevName{a_sWriteDevName}, m_sWriteWellheadName{a_sWriteWellheadName}, m_sWritePointName{a_sWritePointName},
	m_uiDelayMs{a_uiDelayMs}, m_sVal{a_sVal}, m_q{}
	{}

	CControlLoopOp& operator=(const CControlLoopOp& a_obj)
	{
		m_sId = a_obj.m_sId;
		m_sPolledTopic = a_obj.m_sPolledTopic;
		m_sWritePointFullPath = a_obj.m_sWritePointFullPath;
		m_sWriteDevName = a_obj.m_sWriteDevName;
		m_sWriteWellheadName = a_obj.m_sWriteWellheadName;
		m_sWritePointName = a_obj.m_sWritePointName;
		m_uiDelayMs = a_obj.m_uiDelayMs;
		m_sVal = a_obj.m_sVal;

		return *this;
	}

	CControlLoopOp(const CControlLoopOp& a_obj) 
	: m_sId{a_obj.m_sId}, m_sPolledTopic{a_obj.m_sPolledTopic}, m_sWritePointFullPath{a_obj.m_sWritePointFullPath}, 
		m_sWriteDevName{a_obj.m_sWriteDevName}, m_sWriteWellheadName{a_obj.m_sWriteWellheadName}, 
		m_sWritePointName{a_obj.m_sWritePointName},
		m_uiDelayMs{a_obj.m_uiDelayMs}, m_sVal{a_obj.m_sVal}, m_q{}
	{} 
	
	bool startThread();
	bool stopThread();
	std::string getValue() const {return m_sVal;} 
	std::string getWritePoint() const {return m_sWritePointFullPath;}
	std::string getPolledTopic() const {return m_sPolledTopic;}
	std::string getDevNameForWrReq() const {return m_sWriteDevName;}
	std::string getWellHeadNameForWrReq() const {return m_sWriteWellheadName;}
	std::string getPointNameForWrReq() const {return m_sWritePointName;}
	uint32_t getDelay() const {return m_uiDelayMs;}
	
	CQueueHandler& getQueue() {return m_q;}

	void postDummyAnalysisMsg(const std::string &a_sAppSeq, const std::string &a_sError) const;
};


class CKPIAppConfig;

/** class for control loop mapping operations*/
class CControlLoopMapper
{
	friend CKPIAppConfig; //friend class
	std::map<std::string, std::vector<CControlLoopOp>> m_oControlLoopMap; /** control loop map*/
	std::vector<std::string> m_vsPollTopics; /** vector of poll topics*/
	std::vector<std::string> m_vsWrRspTopics; /** vector of response topics*/
	uint32_t m_uiCtrlLoopCnt; /** count of control loops*/

	/** Default constructor*/
	CControlLoopMapper(): m_oControlLoopMap{}, m_uiCtrlLoopCnt{0}
	{}

	/** delete copy and move constructors and assign operators*/
	CControlLoopMapper(const CControlLoopMapper&) = delete;	 			/** Copy construct*/
	CControlLoopMapper& operator=(const CControlLoopMapper&) = delete;	/** Copy assign*/

	bool insertControlLoopData(const std::string &a_sPolledTopic, const std::string &a_sWriteTopic, uint32_t a_uiDelayMs, const std::string &a_sVal);

	bool verifyPointFullPath(const std::string &a_sFullPath, std::string &a_sDevName, 
						std::string &a_sWellHeadName, std::string &a_sPointName);

public:
	bool triggerControlLoops(std::string& a_sPolledPoint, CMessageObject a_oMsg);
	bool configControlLoopOps(bool a_bIsRTWrite);
	bool stopControlLoopOps();
	bool destroySubCtx();
	bool isControlLoopPollPoint(const std::string &a_sPollTopic);
	bool isControlLoopWrRspPoint(const std::string &a_sWrRspTopic);

	const std::vector<std::string>& getPollingTopics(){ return m_vsPollTopics; }
	const std::vector<std::string>& getWrRspTopics(){ return m_vsWrRspTopics; }

	bool publishWriteReq(const CControlLoopOp& a_rCtrlLoop, const std::string &a_sWrSeq, CMessageObject &a_oPollMsg);
};

/** structure for poll write data*/
struct stPollWrData
{
	CMessageObject m_oPollData; /** object of class CMessageObject */
	struct timespec m_tsStartWrReqCreate;/** reference of structure timespec */

	/** constructor*/
	stPollWrData() 
	: m_oPollData{}, m_tsStartWrReqCreate{}
	{
	}
	stPollWrData(CMessageObject &a_oPollData, struct timespec &a_tsStartWrReqCreate) 
	: m_oPollData{a_oPollData}, m_tsStartWrReqCreate{a_tsStartWrReqCreate}
	{
	}
};

/** class maintaining poll and write request mapping*/
class CPollNWriteReqMapper
{
private:
	std::map<std::string, struct stPollWrData> m_mapPollWrite; /** map for poll and write data*/
	std::mutex m_mapMutex; /** mutex for  map*/

	CPollNWriteReqMapper() {};/** default constructor*/
public:
	static CPollNWriteReqMapper& getInstace()
	{
		static CPollNWriteReqMapper _self;
		return _self;
	}
	bool insertForTracking(std::string a_sKey, struct stPollWrData &a_oData)
	{
		std::unique_lock<std::mutex> lck(m_mapMutex);
		m_mapPollWrite.insert(std::pair <std::string, struct stPollWrData> (a_sKey, a_oData));
		return true;
	}
	bool getForProcessing(std::string a_sKey, struct stPollWrData &a_oData)
	{
		try
		{
			std::unique_lock<std::mutex> lck(m_mapMutex);
			a_oData = m_mapPollWrite.at(a_sKey);
			m_mapPollWrite.erase(a_sKey);
		}
		catch(std::exception &ex)
		{
			DO_LOG_ERROR(ex.what());
			return false;
		}
		return true;
	}
	bool isPresent(std::string a_sKey)
	{
		try
		{
			std::unique_lock<std::mutex> lck(m_mapMutex);
			auto itr = m_mapPollWrite.find(a_sKey);
			if(itr == m_mapPollWrite.end())
			{
				return false;
			}
			return true;
		}
		catch(std::exception &ex)
		{
			DO_LOG_ERROR(ex.what());
			return false;
		}
		return true;
	}
};

#endif /* INCLUDE_CONTROLLOOPHANDLER_HPP_ */
