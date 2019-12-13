/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#include "PeriodicRead.hpp"
#include "PeriodicReadFeature.hpp"
#include <boost/bind/bind.hpp>
//#include "PublishJson.hpp"
#include "utils/YamlUtil.hpp"
#include <sstream>
#include <ctime>
#include <chrono>

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
extern src::severity_logger< severity_level > lg;

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
		ss << p1.time_since_epoch().count();
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
		msg_envelope_elem_body_t* ptTopic = msgbus_msg_envelope_new_string(a_objReqData.getDataPoint().getID().c_str());
		msg_envelope_elem_body_t* ptWellhead = msgbus_msg_envelope_new_string(a_objReqData.getDataPoint().getWellSite().getID().c_str());
		msg_envelope_elem_body_t* ptMetric = msgbus_msg_envelope_new_string(a_objReqData.getDataPoint().getDataPoint().getID().c_str());

		msg = msgbus_msg_envelope_new(CT_JSON);
		msgbus_msg_envelope_put(msg, "version", ptVersion);
		msgbus_msg_envelope_put(msg, "driver_seq", ptDriverSeq);
		msgbus_msg_envelope_put(msg, "timestamp", ptTimeStamp);
		msgbus_msg_envelope_put(msg, "usec", ptUsec);
		msgbus_msg_envelope_put(msg, "topic", ptTopic);
		msgbus_msg_envelope_put(msg, "wellhead", ptWellhead);
		msgbus_msg_envelope_put(msg, "metric", ptMetric);

		//// fill value
		*a_pMsg = msg;

		if(TRUE == a_stResp.bIsValPresent)
		{
			std::vector<uint8_t> vt = a_stResp.m_Value;
			if(0 != vt.size())
			{
				static const char* digits = "0123456789ABCDEF";
				std::string sVal(vt.size()*2+2,'0');
				int i = 0;
				sVal[i++] = '0'; sVal[i++] = 'x';

				std::vector<unsigned char>::iterator it = vt.begin();
				while (vt.end() != it)
				{
					// In 2-byte coombination, 1st byte is LSB and 2nd byte is MSB
					// While forming a string, MSB should be taken first
					auto cLSB = *it;
					++it;
					if(vt.end() != it)
					{
						// First take MSB
						auto cMSB = *it;
						++it;
						sVal[i] = digits[(cMSB >> 4) & 0x0F];
						sVal[i+1] = digits[cMSB & 0x0F];
						i += 2;
					}
					// First take MSB
					sVal[i] = digits[(cLSB >> 4) & 0x0F];
					sVal[i+1] = digits[cLSB & 0x0F];
					i += 2;
				}

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
		logMessage(debug,"::Exception is raised. "<<e.what());
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
			logMessage(info,"Error in preparing response");
			return FALSE;
		}
		else
		{
			if(true == PublishJsonHandler::instance().publishJson(g_msg, objReqData.getBusContext().m_pContext,
					objReqData.getDataPoint().getID()))
			{
				msg_envelope_serialized_part_t* parts = NULL;
				int num_parts = msgbus_msg_envelope_serialize(g_msg, &parts);
				if(num_parts > 0)
				{
					if(NULL != parts[0].bytes)
					{
						std::string s(parts[0].bytes);
						BOOST_LOG_SEV(lg, info) << "Response ZMQ Publish Time: "
							<< std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()
							<< ", TxID: " << a_stResp.u16TransacID
							<< ", Msg: " << s;
					}

					msgbus_msg_envelope_serialize_destroy(parts, num_parts);
				}
			}
		}
	}
	catch(const std::exception& e)
	{
		logMessage(debug,"Exception::" << __func__ << ": " << e.what());
		cout << __DATE__ << " " << __TIME__ << " " << __func__ << ": " << e.what() << std::endl;
	}

	if(NULL == g_msg)
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

	while(1)
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
				logMessage(info, "Exception in sendReadPeriodicResponseThread ::" << __func__ << ": " << e.what());
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
		logMessage(warning, "Exception CPeriodicReponseProcessor ::" << __func__ << ": " << e.what();)
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
		logMessage(warning, "Exception CPeriodicReponseProcessor ::" << __func__ << ": " << e.what())
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

			//cout << "ExceptionCode::"<< (unsigned)pstException->m_u8ExcCode<<
			//		" Exception Status::"<< (unsigned)pstException->m_u8ExcStatus <<
			//		" TransactionID::" << (unsigned)u16TransacID << endl;

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
		logMessage(warning,"Exception is raised. "<<e.what());
	}

}

// function to receive RP-A call back
eMbusStackErrorCode readPeriodicCallBack(uint8_t  u8UnitID,
										 uint16_t u16TransacID,
										 uint8_t* pu8IpAddr,
										 uint8_t  u8FunCode,
										 stException_t  *pstException,
										 uint8_t  u8numBytes,
										 uint8_t* pu8data,
										 uint16_t  u16StartAddress,
										 uint16_t  u16Quantity)
{
	/// validate pointer
	if(NULL == pu8data)
	{
		cout <<"No data received from stack " <<endl;
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
		logMessage(warning,"Exception CPeriodicReponseProcessor ::" << __func__ << ": Unable to initiate instance: " << e.what());
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
		logMessage(warning,"Exception CPeriodicReponseProcessor ::" << __func__ << ": Unable to initiate instance: " << e.what());
		std::cout << "\nException CPeriodicReponseProcessor ::" << __func__ << ": Unable to initiate instance: " << e.what();
	}
}

bool CRequestInitiator::initSem()
{
	return true;
}

void CRequestInitiator::threadRequestInit(std::vector<std::vector<CRefDataForPolling>> a_vReqDataList)
{
	try
	{
		for(auto a_vReqData: a_vReqDataList)
		{
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
		}
	}
	catch (exception &e)
	{
		std::cout << "Exception in CMapHIDRdPeriod::threadRequestInit: " << e.what() << endl;
	}
}

void CRequestInitiator::initiateRequests(std::vector<std::vector<CRefDataForPolling>> a_vReqData)
{
	try
	{
#ifdef PERFTESTING // will be removed afterwards
		if(!stopPolling.load())
#endif
		{
			std::thread(&CRequestInitiator::threadRequestInit, this, a_vReqData).detach();
		}
	}
	catch (exception &e)
	{
		std::cout << "Exception in CMapHIDRdPeriod::initiateRequests: " << e.what() << endl;
	}
}

CRequestInitiator::~CRequestInitiator()
{
}

CRequestInitiator::CRequestInitiator() : m_uiIsNextRequest(0)
{
	try
	{
		initSem();
	}
	catch(const std::exception& e)
	{
		logMessage(warning,"Exception CRequestInitiator ::" << __func__ << ": Unable to initiate instance: " << e.what());
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
	std::thread(&CTimeMapper::ioPeriodicReadTimer, this, 5).detach();
	// Init CRequestInitiator
	CRequestInitiator::instance();
	std::cout << "\nCreated periodic read timer thread\n";
}

void CTimeMapper::checkTimer()
{
    std::vector<CRefDataForPolling> vPolledPoints;
    std::vector <std::vector<CRefDataForPolling> > vPolledPointsList;

    try
	{
    	std::lock_guard<std::mutex> lock(m_mapMutex);
		for (auto &element : m_mapTimeRecord)
		{
			CTimeRecord &a = element.second;
			if(0 == a.decrementTime(1))
			{
				std::vector<CRefDataForPolling>& vTemp = a.getPolledPointList();
				vPolledPointsList.push_back(vTemp);
				//vPolledPoints.insert(vPolledPoints.end(), vTemp.begin(), vTemp.end());
			}
		}

		if(vPolledPointsList.size())
		{
			CRequestInitiator::instance().initiateRequests(vPolledPointsList);
		}
	}
	catch (exception &e)
	{
		std::cout << "Exception in TimeMapper timer function: " << e.what() << endl;
	}
}

void CTimeMapper::timerThreadFunc(const boost::system::error_code& e, boost::asio::steady_timer* t)
{
    auto start = chrono::steady_clock::now();
	CTimeMapper::instance().checkTimer();
	auto end = chrono::steady_clock::now();

	auto sleepFor = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

	//cout << "Sleep For::"<<sleepFor << endl;
	/// Reschedule the timer for TIMER_DURATION (i.e. 1 min) in the future
	//t->expires_after(boost::asio::chrono::seconds(60 - sleepFor));
	t->expires_after(boost::asio::chrono::microseconds(1000-sleepFor));

	// Posts the timer event
	t->async_wait(boost::bind(CTimeMapper::timerThreadFunc,
			boost::asio::placeholders::error, t));
}

void CTimeMapper::ioPeriodicReadTimer(int v)
{
	boost::asio::io_context io;

	// Start timer immediately
	//boost::asio::steady_timer t(io, boost::asio::chrono::seconds(1));
	boost::asio::steady_timer t(io);
	t.expires_after(boost::asio::chrono::milliseconds(100));
	t.async_wait(boost::bind(CTimeMapper::timerThreadFunc,
	        boost::asio::placeholders::error, &t));

	io.run();
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
