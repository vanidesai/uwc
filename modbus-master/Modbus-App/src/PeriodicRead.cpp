/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/
#include "Logger.hpp"

#include "PeriodicRead.hpp"
#include "PeriodicReadFeature.hpp"
#include "utils/YamlUtil.hpp"
#include <sstream>
#include <ctime>
#include <chrono>

/// flag to check thread stop condition
std::atomic<bool> g_stopThread;

extern "C" {
	#include <safe_lib.h>
}

#ifdef PERFTESTING // will be removed afterwards

	/// added for performance test
	#include <fstream>
	#include <ctime>

	/// added for performance test
	//static uint32_t abort_count = 0;
	static uint32_t success_count = 0;
	static uint32_t error_count = 0;
	//static uint32_t nodata_count = 0;
	//static uint32_t send_request_stat = 0;
	static uint32_t other_status = 0;

	// Declaring argument for time()
	time_t tt;

	// Declaring variable to store return value of
	// localtime()
	struct tm * ti;

	/// added for performance test
	static int iNumOfAck = 0;

	std::atomic<bool> stopPolling;

	bool StopReadPeriodic = true;
	atomic<int>total_read_periodic;

	static int reqCount = 0;

#endif

using namespace std;

std::atomic<unsigned short> g_u16TxId;

/// variable to store timer instance
timer_t gTimerid;

void getTimeBasedParams(const CRefDataForPolling& a_objReqData, std::string &a_sTimeStamp, std::string &a_sUsec, std::string &a_sTxID)
{
	a_sTimeStamp.clear();
	a_sUsec.clear();
	a_sTxID.clear();

	const auto p1 = std::chrono::system_clock::now();

	std::time_t rawtime = std::chrono::system_clock::to_time_t(p1);
	std::tm* timeinfo = std::gmtime(&rawtime);
	if(NULL == timeinfo)
	{
		return;
	}
	char buffer [80];

	std::strftime(buffer,80,"%Y-%m-%d %H:%M:%S",timeinfo);
	a_sTimeStamp.insert(0, buffer);

	{
		std::stringstream ss;
		ss << std::chrono::duration_cast<std::chrono::microseconds>(p1.time_since_epoch()).count();
		a_sUsec.insert(0, ss.str());
	}

	{
		unsigned long long int u64{
			(unsigned long long int)(std::chrono::duration_cast<std::chrono::milliseconds>(p1.time_since_epoch()).count())
		};
		std::stringstream ss;
		ss << (unsigned long long)(((unsigned long long)(a_objReqData.getDataPoint().getMyRollID()) << 48) | u64);
		a_sTxID.insert(0, ss.str());
	}
}

BOOLEAN CPeriodicReponseProcessor::prepareResponseJson(msg_envelope_t** a_pMsg, const CRefDataForPolling& a_objReqData, stStackResponse a_stResp)
{
	bool bRetValue = true;
	msg_envelope_t *msg = NULL;
	try
	{
		std::string sTimestamp, sUsec, sTxID;
		getTimeBasedParams(a_objReqData, sTimestamp, sUsec, sTxID);
		
		msg_envelope_elem_body_t* ptVersion = msgbus_msg_envelope_new_string("2.0");
		msg_envelope_elem_body_t* ptDriverSeq = msgbus_msg_envelope_new_string(sTxID.c_str());
		msg_envelope_elem_body_t* ptTimeStamp = msgbus_msg_envelope_new_string(sTimestamp.c_str());
		msg_envelope_elem_body_t* ptUsec = msgbus_msg_envelope_new_string(sUsec.c_str());
		string sTopic = a_objReqData.getDataPoint().getID() + SEPARATOR_CHAR + PERIODIC_GENERIC_TOPIC;
		msg_envelope_elem_body_t* ptTopic = msgbus_msg_envelope_new_string(a_objReqData.getDataPoint().getID().c_str());
		msg_envelope_elem_body_t* ptWellhead = msgbus_msg_envelope_new_string(a_objReqData.getDataPoint().getWellSite().getID().c_str());
		msg_envelope_elem_body_t* ptMetric = msgbus_msg_envelope_new_string(a_objReqData.getDataPoint().getDataPoint().getID().c_str());
		msg_envelope_elem_body_t* ptQos =  msgbus_msg_envelope_new_string(a_objReqData.getDataPoint().getDataPoint().getPollingConfig().m_usQOS.c_str());

		msg = msgbus_msg_envelope_new(CT_JSON);
		msgbus_msg_envelope_put(msg, "version", ptVersion);
		msgbus_msg_envelope_put(msg, "driver_seq", ptDriverSeq);
		msgbus_msg_envelope_put(msg, "timestamp", ptTimeStamp);
		msgbus_msg_envelope_put(msg, "usec", ptUsec);
		msgbus_msg_envelope_put(msg, "topic", ptTopic);
		msgbus_msg_envelope_put(msg, "wellhead", ptWellhead);
		msgbus_msg_envelope_put(msg, "metric", ptMetric);
		msgbus_msg_envelope_put(msg, "qos", ptQos);

		//// fill value
		*a_pMsg = msg;

		if(TRUE == a_stResp.bIsValPresent)
		{
			std::vector<uint8_t> vt = a_stResp.m_Value;
			if(0 != vt.size())
			{
				std::string sVal = zmq_handler::swapConversion(vt,
						a_objReqData.getDataPoint().getDataPoint().getAddress().m_bIsByteSwap,
						a_objReqData.getDataPoint().getDataPoint().getAddress().m_bIsWordSwap);

				msg_envelope_elem_body_t* ptValue = msgbus_msg_envelope_new_string(sVal.c_str());
				msg_envelope_elem_body_t* ptStatus = msgbus_msg_envelope_new_string("Good");
				msgbus_msg_envelope_put(msg, "value", ptValue);
				msgbus_msg_envelope_put(msg, "status", ptStatus);
			}
			else
			{
				msg_envelope_elem_body_t* ptValue = msgbus_msg_envelope_new_string("");
				msg_envelope_elem_body_t* ptStatus = msgbus_msg_envelope_new_string("Bad");
				msgbus_msg_envelope_put(msg, "value", ptValue);
				msgbus_msg_envelope_put(msg, "status", ptStatus);
			}
		}
		else
		{
			msg_envelope_elem_body_t* ptValue = msgbus_msg_envelope_new_string("");
			msg_envelope_elem_body_t* ptStatus = msgbus_msg_envelope_new_string("Bad");
			msgbus_msg_envelope_put(msg, "value", ptValue);
			msgbus_msg_envelope_put(msg, "status", ptStatus);
		}
	}
    catch(const std::exception& e)
	{
    	CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		bRetValue = false;
	}

	return bRetValue;
}

BOOLEAN CPeriodicReponseProcessor::postResponseJSON(stStackResponse& a_stResp)
{
	msg_envelope_t* g_msg = NULL;

	try
	{
		std::string sTopic("");
		const CRefDataForPolling& objReqData = CRequestInitiator::instance().getTxIDReqData(a_stResp.u16TransacID);
		if(FALSE == prepareResponseJson(&g_msg, objReqData, a_stResp))
		{
			CLogger::getInstance().log(INFO, LOGDETAILS( " Error in preparing response"));
			return FALSE;
		}
		else
		{
			if(true == PublishJsonHandler::instance().publishJson(g_msg, objReqData.getBusContext().m_pContext,
					objReqData.getPubContext().m_pContext,
					PublishJsonHandler::instance().getPolledDataTopic()))
			{
				msg_envelope_serialized_part_t* parts = NULL;
				int num_parts = msgbus_msg_envelope_serialize(g_msg, &parts);
				if(num_parts > 0)
				{
					if(NULL != parts[0].bytes)
					{
						std::string s(parts[0].bytes);

						string temp = "Response ZMQ Publish Time: ";
						temp.append(std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()));
						temp.append(",TxID: ");
						temp.append(std::to_string(a_stResp.u16TransacID));
						temp.append(",Msg: ");
						temp.append(s);

						CLogger::getInstance().log(INFO, LOGDETAILS(temp));

					}

					msgbus_msg_envelope_serialize_destroy(parts, num_parts);
				}
			}
		}
	}
	catch(const std::exception& e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		cout << __DATE__ << " " << __TIME__ << " " << __func__ << ": " << e.what()  << "Tx ID::" << a_stResp.u16TransacID<< std::endl;
	}

	if(NULL != g_msg)
	{
		msgbus_msg_envelope_destroy(g_msg);
	}

	// return true on success
	return TRUE;
}

bool CPeriodicReponseProcessor::initSem()
{
	//
	int ok = sem_init(&semaphoreRespProcess, 0, 0 /* Initial value of zero*/);
	if (ok == -1) {
	   std::cout << "*******Could not create unnamed semaphore\n";
	   return false;
	}
	return true;
}

eMbusStackErrorCode CPeriodicReponseProcessor::CPeriodicReponseProcessor::respProcessThreads()
{
	eMbusStackErrorCode eRetType = MBUS_STACK_NO_ERROR;

	while(false == g_stopThread.load())
	{

		/// iterate Queue one by one to send message on Pl-bus
		do
		{
			// Wait for response
			sem_wait(&semaphoreRespProcess);
			stStackResponse res;
			try
			{
				if(false == getDataToProcess(res))
				{
					break;
				}

				// Cases:
				// 1. Success / Error response received from end device
				// 2. Error response received from stack (e.g. request time-out)

				if(1 == res.u8Reason)	// 1 means success i.e. response received.
				{
					// Case 1. Success / Error response received from end device
					//if(PDU_TYPE_COMPLEX_ACK == res.stIpArgs.m_stNPDUData.m_stAPDUData.m_ePduType)
					if(true == res.bIsValPresent)
					{
#ifdef PERFTESTING
						++iNumOfAck;
						//cout << asctime(ti) << "Number of Complex-Ack received :: "<< ++iNumOfAck << endl;

						if(iNumOfAck == total_read_periodic.load())
						{
							iNumOfAck = 0;
						}

						success_count++;
#endif

						// fill the response in JSON
						postResponseJSON(res);

						// set the RETURN type
						eRetType = MBUS_STACK_NO_ERROR;
					}
					else
					{
#ifdef PERFTESTING
						error_count++;
#endif

						// fill the response in JSON
						postResponseJSON(res);

						// set the RETURN type
						eRetType = MBUS_STACK_ERROR_RECV_FAILED;
					}
				}
				else
				{
#ifdef PERFTESTING
					other_status++;
#endif
					// Case 2. Error response received from stack (e.g. request time-out)
					// fill the response in JSON
					postResponseJSON(res);
				}

				/// remove node from TxID map
				CRequestInitiator::instance().removeTxIDReqData(res.u16TransacID);
				res.m_Value.clear();
			}
			catch(const std::exception& e)
			{
#ifdef PERFTESTING
				other_status++;
#endif
				CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
				cout << __DATE__ << " " << __TIME__ << " " << __func__ << ": " << e.what() << std::endl;
				//return FALSE;
			}

		}while(0);
	}

	return eRetType;
}

bool CPeriodicReponseProcessor::pushToQueue(struct stStackResponse &stStackResNode)
{
	try
	{
		std::lock_guard<std::mutex> lock(__resQMutex);
		stackResQ.push(stStackResNode);
		// Signal response process thread
		sem_post(&semaphoreRespProcess);

		return true;
	}
	catch(const std::exception& e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}

	return false;
}

bool CPeriodicReponseProcessor::getDataToProcess(struct stStackResponse &a_stStackResNode)
{
	try
	{
		std::lock_guard<std::mutex> lock(__resQMutex);
		a_stStackResNode = stackResQ.front();
		stackResQ.pop();

		return true;
	}
	catch(const std::exception& e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));

	}

	return false;
}

void CPeriodicReponseProcessor::handleResponse(uint8_t  u8UnitID,
					 uint16_t u16TransacID,
					 uint8_t* pu8IpAddr,
					 uint8_t  u8FunCode,
					 stException_t  *pstException,
					 uint8_t  u8numBytes,
					 uint8_t* pu8data,
					 uint16_t  u16StartAddress,
					 uint16_t  u16Quantity)
{
	struct stStackResponse stStackResNode;
	stStackResNode.bIsValPresent = false;
	stStackResNode.u8Reason = 0;

	try
	{
		if(pu8data == NULL)
		{
			return;
		}

		if( pstException != NULL )
		{
			stStackResNode.m_stException.m_u8ExcCode = pstException->m_u8ExcCode;
			stStackResNode.m_stException.m_u8ExcStatus = pstException->m_u8ExcStatus;

			stStackResNode.u16TransacID = u16TransacID;

			if((0 == pstException->m_u8ExcStatus) &&
					(0 == pstException->m_u8ExcCode))
			{
				stStackResNode.u8Reason = 1;
				stStackResNode.bIsValPresent = true;
				for (uint8_t index=0; index < u8numBytes; index++)
				{
					stStackResNode.m_Value.push_back(pu8data[index]);
				}
			}
			else
			{
				stStackResNode.u8Reason = 0;
				stStackResNode.bIsValPresent = false;
			}

			pushToQueue(stStackResNode);
		}
	}
	catch(const std::exception& e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}

}

// function to receive RP-A call back
#ifdef MODBUS_STACK_TCPIP_ENABLED
eMbusStackErrorCode readPeriodicCallBack(uint8_t  u8UnitID,
										 uint16_t u16TransacID,
										 uint8_t* pu8IpAddr,
										 uint16_t u16Port,
										 uint8_t  u8FunCode,
										 stException_t  *pstException,
										 uint8_t  u8numBytes,
										 uint8_t* pu8data,
										 uint16_t  u16StartAddress,
										 uint16_t  u16Quantity)
#else
eMbusStackErrorCode readPeriodicCallBack(uint8_t  u8UnitID,
										 uint16_t u16TransacID,
										 uint8_t* pu8IpAddr,
										 uint8_t  u8FunCode,
										 stException_t  *pstException,
										 uint8_t  u8numBytes,
										 uint8_t* pu8data,
										 uint16_t  u16StartAddress,
										 uint16_t  u16Quantity)
#endif
{
	/// validate pointer
	if(NULL == pu8data)
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS(" No data received from stack"));
		return MBUS_STACK_ERROR_RECV_FAILED;
	}

	// handle response
	CPeriodicReponseProcessor::Instance().handleResponse(u8UnitID, u16TransacID, pu8IpAddr, u8FunCode, pstException,
			u8numBytes, pu8data, u16StartAddress, u16Quantity);

	return MBUS_STACK_NO_ERROR;
}

CPeriodicReponseProcessor::CPeriodicReponseProcessor() : m_bIsInitialized(false)
{
	try
	{
		initSem();
		m_bIsInitialized = true;

		//bIsSubscriptionStarted = 0;
	}
	catch(const std::exception& e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS("Unable to initiate instance"));
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		std::cout << "\nException CPeriodicReponseProcessor ::" << __func__ << ": Unable to initiate instance: " << e.what();
	}
}

CPeriodicReponseProcessor& CPeriodicReponseProcessor::Instance()
{
	static CPeriodicReponseProcessor _self;
	return _self;
}

void CPeriodicReponseProcessor::initRespHandlerThreads()
{
	static bool bSpawned = false;
	try
	{
		if(false == bSpawned)
		{
			// Spawn 5 thread to process responses
			for (int i = 0; i < 5; i++)
			{
				std::thread{std::bind(&CPeriodicReponseProcessor::respProcessThreads, std::ref(*this))}.detach();
			}
			bSpawned = true;
		}
	}
	catch(const std::exception& e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS("Unable to initiate instance"));
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		std::cout << "\nException CPeriodicReponseProcessor ::" << __func__ << ": Unable to initiate instance: " << e.what();
	}
}

bool CRequestInitiator::init()
{
	std::thread{std::bind(&CRequestInitiator::threadReqInit, std::ref(*this))}.detach();
	
	int ok = sem_init(&semaphoreReqProcess, 0, 0 /* Initial value of zero*/);
	if (ok == -1) {
	   std::cout << "*******Could not create unnamed semaphore\n";
	   return false;
	}
	return true;
}

bool CRequestInitiator::pushPollFreqToQueue(uint32_t &a_uiRef)
{
	try
	{
		std::lock_guard<std::mutex> lock(m_mutexReqFreqQ);
		m_qReqFreq.push(a_uiRef);
		// Signal response process thread
		sem_post(&semaphoreReqProcess);

		return true;
	}
	catch(const std::exception& e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}

	return false;
}

bool CRequestInitiator::getFreqRefForPollCycle(uint32_t &a_uiRef)
{
	try
	{
		std::lock_guard<std::mutex> lock(m_mutexReqFreqQ);
		a_uiRef = m_qReqFreq.front();
		m_qReqFreq.pop();

		return true;
	}
	catch(const std::exception& e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}

	return false;
}

void CRequestInitiator::threadReqInit()
{
	try
	{
		while(false == g_stopThread.load())
		{
			do
			{
				// Wait for response
				sem_wait(&semaphoreReqProcess);
				if(true == g_stopThread.load())
				{
					break;
				}
				uint32_t uiRef;
				if(false == getFreqRefForPollCycle(uiRef))
				{
					break;
				}
				//cout << "CRequestInitiator::threadReqInit():Semaphore signalled: Freq " << uiRef << std::endl;
				std::vector<CRefDataForPolling>& a_vReqData = CTimeMapper::instance().getPolledPointList(uiRef);
				for(auto a_oReqData: a_vReqData)
				{
					CRefDataForPolling objReqData = a_oReqData;

					//if(true == bIsFound)
					{
						// The entry is found in map
						// Send a request
						if (true == sendRequest(objReqData))
						{
							// Request is sent successfully
							// No action
		#ifdef PERFTESTING // will be removed afterwards
							++reqCount;
							if(reqCount == total_read_periodic.load())
							{
								reqCount = 0;
								cout << "Polling is stopped ...." << endl;
								stopPolling.store(true);
								break;
							}
		#endif
						}
						else
						{
						}
					}
				}

			} while(0);
		}
	}
	catch (exception &e)
	{
		std::cout << "Exception in CMapHIDRdPeriod::threadRequestInit: " << e.what() << endl;
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}
}

void CRequestInitiator::initiateRequests(uint32_t a_uiRef)
{
	try
	{
#ifdef PERFTESTING // will be removed afterwards
		if(!stopPolling.load())
#endif
		{
			pushPollFreqToQueue(a_uiRef);
		}
	}
	catch (exception &e)
	{
		std::cout << "Exception in CMapHIDRdPeriod::initiateRequests: " << e.what() << endl;
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}
}

CRequestInitiator::~CRequestInitiator()
{
}

CRequestInitiator::CRequestInitiator() : m_uiIsNextRequest(0)
{
	try
	{
		init();
	}
	catch(const std::exception& e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS("Unable to initiate instance: Exception:"));
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		std::cout << "\nException CRequestInitiator ::" << __func__ << ": Unable to initiate instance: " << e.what();
	}
}

CRefDataForPolling CRequestInitiator::getTxIDReqData(unsigned short tokenId)
{
	/// Ensure that only on thread can execute at a time
	std::lock_guard<std::mutex> lock(m_mutextTxIDMap);

	// return the request ID
	return m_mapTxIDReqData.at(tokenId);
}

// function to insert new entry in map
void CRequestInitiator::insertTxIDReqData(unsigned short token, CRefDataForPolling objRefData)
{
	/// Ensure that only on thread can execute at a time
	std::lock_guard<std::mutex> lock(m_mutextTxIDMap);

	// insert the data in request map
	m_mapTxIDReqData.insert(pair <unsigned short, CRefDataForPolling> (token, objRefData));
}

// function to remove entry from the map once reply is sent
void CRequestInitiator::removeTxIDReqData(unsigned short tokenId)
{
	/// Ensure that only on thread can execute at a time
	std::lock_guard<std::mutex> lock(m_mutextTxIDMap);
	m_mapTxIDReqData.erase(tokenId);
}

CTimeMapper::CTimeMapper()
{
}

void CTimeMapper::initTimerFunction()
{
	//m_thread =
	//std::thread(&CTimeMapper::ioPeriodicReadTimer, this, 5).detach();
	// Init CRequestInitiator
	CRequestInitiator::instance();
	//std::cout << "\nCreated periodic read timer thread\n";
}

void CTimeMapper::checkTimer()
{
    try
	{
    	std::lock_guard<std::mutex> lock(m_mapMutex);
		for (auto &element : m_mapTimeRecord)
		{
			CTimeRecord &a = element.second;
			if(0 == a.decrementTime(1))
			{
				CRequestInitiator::instance().initiateRequests(element.first);
			}
		}
	}
	catch (exception &e)
	{
		std::cout << "Exception in TimeMapper timer function: " << e.what() << endl;
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}
}

bool CRequestInitiator::sendRequest(CRefDataForPolling a_stRdPrdObj)
{
	MbusAPI_t stMbusApiPram = {};
	uint8_t u8ReturnType = MBUS_STACK_NO_ERROR;
	bool bRet = false;

	try
	{
		stMbusApiPram.m_u16StartAddr = a_stRdPrdObj.getDataPoint().getDataPoint().getAddress().m_iAddress;
		stMbusApiPram.m_u16Quantity = a_stRdPrdObj.getDataPoint().getDataPoint().getAddress().m_iWidth;
		stMbusApiPram.m_u16ByteCount = a_stRdPrdObj.getDataPoint().getDataPoint().getAddress().m_iWidth;

		// Coil and discrete input are single bytes. All others are 2 byte registers
		if( 
		(network_info::eEndPointType::eCoil != a_stRdPrdObj.getDataPoint().getDataPoint().getAddress().m_eType) &&
		(network_info::eEndPointType::eDiscrete_Input != a_stRdPrdObj.getDataPoint().getDataPoint().getAddress().m_eType)
		)
		{
			// as default value for register is 2 bytes
			stMbusApiPram.m_u16ByteCount = stMbusApiPram.m_u16Quantity *2;
		}

		/*Enter TX Id*/
		stMbusApiPram.m_u16TxId = (unsigned short)g_u16TxId.load();
#ifdef UNIT_TEST
		stMbusApiPram.m_u16TxId = 5;
#endif
		g_u16TxId++;
		//a_stRdPrdObj.m_u16TxId = stMbusApiPram.m_u16TxId;

		CRequestInitiator::instance().insertTxIDReqData(stMbusApiPram.m_u16TxId, a_stRdPrdObj);
#ifdef MODBUS_STACK_TCPIP_ENABLED
		// fill the unit ID
		stMbusApiPram.m_u8DevId = a_stRdPrdObj.getDataPoint().getWellSiteDev().getAddressInfo().m_stTCP.m_uiUnitID;
		std::string sIPAddr{a_stRdPrdObj.getDataPoint().getWellSiteDev().getAddressInfo().m_stTCP.m_sIPAddress};

		CommonUtils::ConvertIPStringToCharArray(sIPAddr,stMbusApiPram.m_u8IpAddr);

		/// fill tcp port
		stMbusApiPram.m_u16Port = a_stRdPrdObj.getDataPoint().getWellSiteDev().getAddressInfo().m_stTCP.m_ui16PortNumber;

		u8ReturnType = Modbus_Stack_API_Call(a_stRdPrdObj.getFunctionCode(), &stMbusApiPram, (void *)readPeriodicCallBack);

#else
		stMbusApiPram.m_u8DevId = a_stRdPrdObj.getDataPoint().getWellSiteDev().getAddressInfo().m_stRTU.m_uiSlaveId;
		//stMbusApiPram.m_u8DevId = 10; //Slave id
		//cout<< "#########----------------------- stMbusApiPram.m_u8DevId == "<< (unsigned)stMbusApiPram.m_u8DevId <<endl;

		u8ReturnType = Modbus_Stack_API_Call(a_stRdPrdObj.getFunctionCode(), &stMbusApiPram, (void *)readPeriodicCallBack);
#endif

		if(MBUS_STACK_NO_ERROR == u8ReturnType)
		{
			bRet = true;
		}
		else
		{
			// In case of error, immediately retry in next iteration
			//a_stRdPrdObj.bIsRespAwaited = false;
			{
				string l_stErrorString = "Request initiation error:: "+ to_string(u8ReturnType)+" " + "DeviceID ::" + to_string(stMbusApiPram.m_u16TxId);
				bRet = true;
			}
		}
	}
	catch(exception &e)
	{
		std::cout << "Exception in request init: " << e.what() << endl;
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}

	return bRet;
}

bool CTimeMapper::insert(uint32_t a_uTime, CRefDataForPolling &a_oPointD)
{
	bool bRet = true;
	// 1. Check if record for this time exists or not
	// 2. If not, create a new one and insert
	// 3. If exists, append the given reference to existing list
	try
	{
		std::lock_guard<std::mutex> lock(m_mapMutex);
		std::map<uint32_t, CTimeRecord>::iterator it = m_mapTimeRecord.find(a_uTime);

		if(m_mapTimeRecord.end() != it)
		{
			// It means record exists
			it->second.add(a_oPointD);
		}
		else
		{
			// Record does not exist
			CTimeRecord oTimeRecord(a_uTime, a_oPointD);
			m_mapTimeRecord.emplace(a_uTime, oTimeRecord);
		}
	}
	catch (exception &e)
	{
		//std::cout << "Time map insert failed: " << a_uTime << ", " << a_sHID << " : " << e.what() << endl;
		string tempw0 = "Exception in CTimeMapper: ";
		tempw0.append(e.what());
		tempw0.append(", Time: ");
		tempw0.append(std::to_string(a_uTime));
		tempw0.append(", Point: ");
		tempw0.append(a_oPointD.getDataPoint().getID());
		CLogger::getInstance().log(FATAL, LOGDETAILS(tempw0));
		bRet = false;
	}
	return bRet;
}

CTimeMapper::~CTimeMapper()
{
	try
	{
		// Clear map
		std::lock_guard<std::mutex> lock(m_mapMutex);
		m_mapTimeRecord.clear();
	}
	catch (exception &e)
	{
		std::cout << "Exception in TimeMapper Deletion: " << e.what() << endl;
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}
}

void CTimeMapper::print()
{
	/*try
	{
		// Clear map
		std::lock_guard<std::mutex> lock(m_mapMutex);
		for (auto &element : m_mapTimeRecord)
		{
			CTimeRecord &a = element.second;
			std::cout << element.first << "=(" ;
			a.print();
			std::cout  << ")" << endl ;
		}
		std::cout << endl;
	}
	catch (exception &e)
	{
		std::cout << "Exception in TimeMapper Deletion: " << e.what() << endl;
	}*/
}

bool CTimeRecord::add(CRefDataForPolling &a_oPoint)
{
	bool bRet = true;
	// 1. Check if record for this time exists or not
	// 2. If exists, no action
	// 3. If does not exist, then add the reference
	try
	{
		std::lock_guard<std::mutex> lock(m_vectorMutex);
		{
			// Add the reference
			m_vPolledPoints.push_back(a_oPoint);
		}
	}
	catch (exception &e)
	{
		//std::cout << "TimeRecord insert failed: " << a_sHID << " : " << e.what() << endl;
		string tempw = " Exception in CTimeRecord:";
		tempw.append(e.what());
		tempw.append(", Point: ");
		tempw.append(a_oPoint.getDataPoint().getID());
		CLogger::getInstance().log(FATAL, LOGDETAILS(tempw));
		bRet = false;
	}
	return bRet;
}


CTimeRecord::~CTimeRecord()
{
	try
	{
		// Clear vector of reference IDs
		std::lock_guard<std::mutex> lock(m_vectorMutex);
		m_vPolledPoints.clear();
	}
	catch (exception &e)
	{
		std::cout << "Exception in TimeRecord Deletion: " << e.what() << endl;
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}
}

void CTimeRecord::print()
{
	/*try
	{
		// Clear vector of reference IDs
		std::lock_guard<std::mutex> lock(m_vectorMutex);
		std::cout << "Interval:" << m_u32Interval << " ";
		for (auto &element : m_vsRefID)
		{
			std::cout << element << " ";
		}
		//std::cout << endl;
	}
	catch (exception &e)
	{
		std::cout << "Exception in TimeRecord Deletion: " << e.what() << endl;
	}*/
}

/**
 * Function to start Linux timer
 * @param
 * lNextTimerTick : use to set next tick count
 * @return : true on success and false on failure
 */
bool LinuxTimer::start_timer(long nextTimerTick)
{
	/// local variables
	struct itimerspec value;
	bool retVal = false;

	value.it_value.tv_sec = 0;

	// waits for nextTimerTick milliseconds before sending timer signal
	value.it_value.tv_nsec = nextTimerTick*1000000;

	// sends timer signal every 1 milliseconds
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_nsec = 1*1000000;

	// create CLOCK_BOOTTIME timer
	int ret = timer_create(CLOCK_BOOTTIME, NULL, &gTimerid);
	if(!ret)
	{
		retVal = true;
	}
	else
	{
		string temp10 = " Failed to set timer.";
		temp10.append("Error code ::");
		temp10.append(std::to_string(ret));
		CLogger::getInstance().log(ERROR, LOGDETAILS(temp10));
	   std::cout << "Error: Failed to create timer" << "Error code ::" << ret<< std::endl;
	   return retVal;
	}

	ret = timer_settime(gTimerid, 0, &value, NULL);
	if(!ret)
	{
		retVal = true;
	}
	else
	{
	   string temp11 = " Failed to set timer.";
	   temp11.append("Error code ::");
	   temp11.append(std::to_string(ret));
	   CLogger::getInstance().log(ERROR, LOGDETAILS(temp11));
	   std::cout << "Error: Failed to set timer" << "Error code ::" << ret<< std::endl;
	   return retVal;
	}

	return retVal;
}

/**
 * Function to stop Linux timer
 * @param : Nothing
 * @return : true on success and false on failure
 */
bool LinuxTimer::stop_timer()
{
	/// local variables
	bool retVal = false;
	struct itimerspec value;

	value.it_value.tv_sec = 0;
	value.it_value.tv_nsec = 0;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_nsec = 0;

	int ret = timer_settime (gTimerid, 0, &value, NULL);
	if(!ret)
	{
		retVal = true;
	}
	else
	{
		string tempv = " Failed to set timer.";
		tempv.append(std::to_string(ret));
		CLogger::getInstance().log(ERROR, LOGDETAILS(tempv));
		std::cout << "Error: Failed to set timer" << "Error code ::" << ret<< std::endl;
		return retVal;
	}

	return retVal;
}

/**
 * Function to get timer callback based on signal
 * @param :
 * iSignal : signal to get timer callback
 * @return : Nothing
 */
void LinuxTimer::timer_callback(int iSignal)
{
	//cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << endl;

	CTimeMapper::instance().checkTimer();
}
