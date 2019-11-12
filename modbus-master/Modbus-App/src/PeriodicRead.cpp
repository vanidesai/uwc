/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/
/*
#include <list>
#include <thread>
#include <queue>
#include <algorithm>
#include <mutex>
#include <chrono>
#include <vector>
#include <numeric>
#include "Common.hpp"
#include "session.hpp"
#include "cpprest/http_listener.h"
#include "Httprest.hpp"
#include "BoostLogger.hpp"
*/
#include "PeriodicRead.hpp"
//#include "PlatformBusInterface.hpp"
#include "PeriodicReadFeature.hpp"
//#include "RequestMapper.hpp"

//#include <fstream>

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
//using namespace web;
//using namespace boostbeast;

std::atomic<unsigned short> g_u16TxId;
extern src::severity_logger< severity_level > lg;

template <class A, class B>
A ConvertFunc(std::vector<uint8_t> vt)
{
    B val = 0;
    for (int i = 0; i < vt.size(); i++)
    {
        val = (val << 8) | vt[i];
    }
    return *((A*)&val);
}

BOOLEAN CPeriodicReponseProcessor::prepareResponseJson(string &a_sTopic, stStackResponse stPeriodicResp, bool bIsValPresent)
{
	bool bRetValue = true;
	/*
	string IPaddress;
	string RespData;

	HaystackInfo_t stHaystackInfo = {};
	try
	{
		//stHaystackInfo = RequestMapper::instance().getReadPeriodicHaystackStruct(stPeriodicResp.u16TransacID);
		JSONReply["haystack_id"] = json::value::string(stHaystackInfo.m_stHaystackId);
		JSONReply["error"] = json::value::string("");

		a_sTopic = stHaystackInfo.m_sMqttTopic;

		if(TRUE == bIsValPresent)
		{
			JSONReply["status"] = json::value::string("ok");
			std::vector<uint8_t> vt = stPeriodicResp.m_Value;
			switch(stHaystackInfo.m_stDataType)
			{
				case MBUS_DT_BINARY:
				{
					if(0 == vt[0])
					{
						JSONReply["value"] = json::value::boolean(FALSE);
					}
					else
					{
						JSONReply["value"] = json::value::boolean(TRUE);
					}
				}
				break;
				case MBUS_DT_UNSIGNED_INT:
				case MBUS_DT_ENUM:
				{
					JSONReply["value"] = json::value::number(ConvertFunc<unsigned int, unsigned int>(vt));
				}
				break;
				case MBUS_DT_FLOAT:
				{
					JSONReply["value"] = json::value::number(ConvertFunc<float, unsigned int>(vt));
				}
				break;
				case MBUS_DT_SIGNED_SHORT:
				{
					JSONReply["value"] = json::value::number(ConvertFunc<short, unsigned int>(vt));
				}
				break;
				case MBUS_DT_UNSIGNED_SHORT:
				{
					JSONReply["value"] = json::value::number(ConvertFunc<unsigned short, unsigned int>(vt));
				}
				break;
				case MBUS_DT_SIGNED_INT:
				{
					JSONReply["value"] = json::value::number(ConvertFunc<int, unsigned int>(vt));
				}
				break;
				case MBUS_DT_DOUBLE:
				{
					JSONReply["value"] = json::value::number(ConvertFunc<double, unsigned long>(vt));
				}
				break;
				case MBUS_DT_STRING:
				{
					char arr[vt.size()] = {};
					int i = 0;
					for(auto const& value: vt)
					{
						arr[i] = (char)value;
						i++;
					}
					string str(arr);
					JSONReply["value"] = json::value::string(str);
				}
				break;
				case MBUS_DT_SINGED_LONG:
				{
					JSONReply["value"] = json::value::number(ConvertFunc<long, unsigned long>(vt));
				}
				break;
				case MBUS_DT_UNSIGNED_LONG:
				{
					JSONReply["value"] = json::value::number(ConvertFunc<unsigned long, unsigned long>(vt));
				}
				break;
				default:
					bRetValue = false;
				break;

			}
		}
		else
		{
			JSONReply["status"] = json::value::string("Fault");
			JSONReply["error"] = json::value::string("exception_code::"+
										to_string(stPeriodicResp.m_stException.m_u8ExcCode)+
										"  exception_status::"+
										to_string(stPeriodicResp.m_stException.m_u8ExcStatus));
		}
	}
    catch(const std::exception& e)
	{
    	BOOST_LOG_SEV(lg, info) <<"fatal::" << __func__ << "::Exception is raised. "<<e.what();
    	bRetValue = false;
	}
	*/

	return bRetValue;

}

BOOLEAN CPeriodicReponseProcessor::postResponseJSON(stStackResponse& a_stResp)
{
	//json::value ipJson;
	try
	{
		std::string sTopic("");
		// Get Haystack data
		if(FALSE == prepareResponseJson(sTopic, a_stResp, a_stResp.bIsValPresent))
		{
			BOOST_LOG_SEV(lg, info) << "Error::" << __func__ << ": " << " could not extract Haystack info";
			cout << __DATE__ << " " << __TIME__ << " " << __func__ << ":  could not extract Haystack info\n";
			return FALSE;
		}

		//PlBusHandler::instance().publishPlBus(ipJson, sTopic.c_str());
	}
	catch(const std::exception& e)
	{
		BOOST_LOG_SEV(lg, info) << "Exception::" << __func__ << ": " << e.what();
		cout << __DATE__ << " " << __TIME__ << " " << __func__ << ": " << e.what() << std::endl;
		return FALSE;
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

				/// remove node from haystack info map
				//RequestMapper::instance().removeReadPeriodicHaystackStruct(res.u16TransacID);
				res.m_Value.clear();

			}
			catch(const std::exception& e)
			{
#ifdef PERFTESTING
				other_status++;
#endif
				BOOST_LOG_SEV(lg, info) << "Exception in sendReadPeriodicResponseThread ::" << __func__ << ": " << e.what();
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
		BOOST_LOG_SEV(lg, info) << "Exception CPeriodicReponseProcessor ::" << __func__ << ": " << e.what();
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
		BOOST_LOG_SEV(lg, info) << "Exception CPeriodicReponseProcessor ::" << __func__ << ": " << e.what();
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

			cout << "ExceptionCode::"<< (unsigned)pstException->m_u8ExcCode<<
					" Exception Status::"<< (unsigned)pstException->m_u8ExcStatus <<
					" TransactionID::" << (unsigned)u16TransacID << endl;

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
		BOOST_LOG_SEV(lg, info) <<"fatal::" << __func__ << "::Exception is raised. "<<e.what();
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
		BOOST_LOG_SEV(lg, info) << "Exception CPeriodicReponseProcessor ::" << __func__ << ": Unable to initiate instance: " << e.what();
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
		BOOST_LOG_SEV(lg, info) << "Exception CPeriodicReponseProcessor ::" << __func__ << ": Unable to initiate instance: " << e.what();
		std::cout << "\nException CPeriodicReponseProcessor ::" << __func__ << ": Unable to initiate instance: " << e.what();
	}
}

/**
 *
 * DESCRIPTION
 * function to subscribe or un-subscribe periodic read request
 *
 * @param lRdPeridList 			   [in] Read periodic request
 *
 * @return eMbusStackErrorCode 	   [out] return specific error code.
 *
 */
eMbusStackErrorCode SubscibeOrUnSubPeriodicRead(RestRdPeriodicTagPart_t &lRdPeridList)
{
	eMbusStackErrorCode u8ReturnType = MBUS_STACK_NO_ERROR;
	//CPeriodicReponseProcessor *pobjPeriodicRead = NULL;

	RestRdPeriodicTagPart_t stTemp;

	try
	{
		// Check if wrong element is sent for unsubscription
		//if (!(CPeriodicReponseProcessor::CheckInstance()) && false == lRdPeridList.IsSubscription)
		if((false ==
				//CMapHIDRdPeriod::instance().getElement((const char*)(lRdPeridList.m_stHaystackInfo.m_stHaystackId), stTemp))
				CMapHIDRdPeriod::instance().getElement(lRdPeridList.m_stHaystackInfo.m_stHaystackId, stTemp))
				&& (false == lRdPeridList.IsSubscription))
		{
			u8ReturnType = MBUS_APP_SUBSCRIPTION_NOT_FOUND;
		}
		else if(true == CMapHIDRdPeriod::instance().processElement(lRdPeridList))
		{
			//if(MBUS_STACK_NO_ERROR == u8ReturnType)
			if(false == lRdPeridList.IsSubscription)
			{
				//pobjPeriodicRead = CPeriodicReponseProcessor::Instance();
				//u8ReturnType = pobjPeriodicRead->SubOrUnsubPeriodicRead(lRdPeridList);
				u8ReturnType = MBUS_STACK_NO_ERROR;
			}
		}
	}
	catch(const std::exception& e)
	{
		BOOST_LOG_SEV(lg, info) <<"fatal::" << __func__ << "::Exception is raised. "<<e.what();
	}

	return u8ReturnType;
}

bool CRequestInitiator::initSem()
{
#if PENDINGREQ_HANDLING // To be enabled in future to follow a policy to attempt max to initiate a request
	//
	int ok = sem_init(&semReqInit, 0, 1 /* Initial value of one*/);
	if (ok == -1) {
	   std::cout << "*******CRequestInitiator::initSem - Could not create unnamed semaphore\n";
	   return false;
	}
	std::cout << "CRequestInitiator::initSem - success\n";
#endif
	return true;
}

void CRequestInitiator::addPendingReq(std::vector<std::string> a_vsRefID, unsigned int a_index)
{
#if PENDINGREQ_HANDLING // To be enabled in future to follow a policy to attempt max to initiate a request
	try
	{
		std::cout << "CRequestInitiator::addPendingReq\n";
		while(a_index < a_vsRefID.size())
		{
			string str = a_vsRefID[a_index];
			std::vector<std::string>::iterator itPendingVector =
					std::find(m_vsPrevPendingRequests.begin(), m_vsPrevPendingRequests.end(), str);
			if (m_vsPrevPendingRequests.end() != itPendingVector)
			{
				// Element in vector.
				// Send an error message on platform bus that request could not be initiated
				//std::cout << str << " :addPendingReq::element present \n";
#ifdef PERFTESTING // will be removed afterwards
				++reqCount;
#endif
				// Send an error message on platform bus
				RestRdPeriodicTagPart_t stRdPeriod;
				if(true == CMapHIDRdPeriod::instance().getElement(str, stRdPeriod))
				{
					json::value tempRPResponse;
					string strHID((const char*)stRdPeriod.m_stHaystackInfo.m_stHaystackId);
					tempRPResponse["haystack_id"] = json::value::string(strHID);

					string strKind((const char*)stRdPeriod.m_stHaystackInfo.m_stKind);
					tempRPResponse["kind"] = json::value::string(strKind);

					string l_stErrorString = "Next request is also pending like the earlier one";
					tempRPResponse["error"] = json::value::string(l_stErrorString);;
					/// Publishing error message to platformbus
					PlBusHandler::instance().publishPlBus(tempRPResponse, READ_PERIODIC_NUMBER_TOPIC.c_str());
				}

				// Remove the element from pending list
				//std::lock_guard<std::mutex> lock(m_mutexPrevReqVector);
				m_vsPrevPendingRequests.erase(itPendingVector);
			}
			else
			{
			//std::lock_guard<std::mutex> lock(m_mutexPrevReqVector);
			// Insert at the beginning
				m_vsPrevPendingRequests.insert(m_vsPrevPendingRequests.begin(), str);
			}
			++a_index;
		}
	}
	catch (exception &e)
	{
		std::cout << "Exception in CRequestInitiator::addPendingReq: " << e.what() << endl;
	}
#endif
}

void CRequestInitiator::threadRequestInit(std::vector<std::string> a_vsRefID)
{
	try
	{
		std::atomic<unsigned int> uiReqCount(0);
#if PENDINGREQ_HANDLING // To be enabled in future to follow a policy to attempt max to initiate a request
		++m_uiIsNextRequest;
		// Wait for current thread in execution to exit
		sem_wait(&semReqInit);
		--m_uiIsNextRequest;
#endif

		unsigned int index = 0;
		while(index < a_vsRefID.size())
		//for (std::vector<std::string>::iterator it = a_vsRefID.begin() ; it != a_vsRefID.end(); ++it)
		{
			string str = a_vsRefID[index];
#if PENDINGREQ_HANDLING // To be enabled in future to follow a policy to attempt max to initiate a request
			if(0 < m_uiIsNextRequest)
			{
				// Next thread is waiting. Exit this one.
				// But before that, add current requests into pending list
				addPendingReq(a_vsRefID, index);
				break;
			}
#endif
			RestRdPeriodicTagPart_t stRdPeriod;
			bool bIsFound = CMapHIDRdPeriod::instance().getElement(str, stRdPeriod);

#if PENDINGREQ_HANDLING // To be enabled in future to follow a policy to attempt max to initiate a request
			std::vector<std::string>::iterator itPendingVector =
					std::find(m_vsPrevPendingRequests.begin(), m_vsPrevPendingRequests.end(), str);
			if (m_vsPrevPendingRequests.end() != itPendingVector)
			{
				// Element in vector.
				// Send an error message on platform bus that request could not be initiated
				if (true == bIsFound)
				{
					json::value tempRPResponse;
					string strHID((const char*)stRdPeriod.m_stHaystackInfo.m_stHaystackId);
					tempRPResponse["haystack_id"] = json::value::string(strHID);

					string strKind((const char*)stRdPeriod.m_stHaystackInfo.m_stKind);
					tempRPResponse["kind"] = json::value::string(strKind);

					string l_stErrorString = "Next request to be sent before the pending one";
					tempRPResponse["error"] = json::value::string(l_stErrorString);;
					/// Publishing error message to platformbus
					PlBusHandler::instance().publishPlBus(tempRPResponse, READ_PERIODIC_NUMBER_TOPIC.c_str());
				}
				else
				{
					// No action - instance not found
				}
				// Remove the element from pending list
				//std::lock_guard<std::mutex> lock(m_mutexPrevReqVector);
				m_vsPrevPendingRequests.erase(itPendingVector);
			}
#endif

			if(true == bIsFound)
			{
				// The entry is found in map
				// Send a request
				if (true == sendRequest(stRdPeriod))
				{
					// Request is sent successfully
					// No action
					++uiReqCount;
#ifdef PERFTESTING // will be removed afterwards
					++reqCount;
#endif
					/**
					 * Commenting for UWC. no wait after 100 request.
					 */
					/*if(0 == uiReqCount%100)
					{
						uiReqCount.store(0);
						std::this_thread::sleep_for(std::chrono::milliseconds(90));
					}*/
#ifdef PERFTESTING // will be removed afterwards
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
#if PENDINGREQ_HANDLING // To be enabled in future to follow a policy to attempt max to initiate a request
					// Request could not be sent
					m_vsPrevPendingRequests.insert(m_vsPrevPendingRequests.begin(), str);
#endif
				}
			}
			else
			{
				// Request is not found in map
				// No action
			}
			++index;
		}
#if PENDINGREQ_HANDLING // To be enabled in future to follow a policy to attempt max to initiate a request
		//std::lock_guard<std::mutex> lock(m_mutexPrevReqVector);
		// All requests scheduled for this interval are complete
		// Now look at pending requests only if next thread is not waiting !
		while((0 == m_uiIsNextRequest) && (false == m_vsPrevPendingRequests.empty()))
		{
			// Get data from back
			std::string sRef = m_vsPrevPendingRequests.back();
			m_vsPrevPendingRequests.pop_back();


			RestRdPeriodicTagPart_t stRdPeriod;
			if(true == CMapHIDRdPeriod::instance().getElement(sRef, stRdPeriod))
			{
				// The entry is found in map
				// Send a request
				if (true == sendRequest(stRdPeriod))
				{
					// Request is sent successfully
					// No action
					++uiReqCount;
#ifdef PERFTESTING // will be removed afterwards
					++reqCount;
#endif
					/**
					 *
					if(0 == uiReqCount%100)
					{
						uiReqCount.store(0);
						std::this_thread::sleep_for(std::chrono::milliseconds(90));
					}
					*/
#ifdef PERFTESTING // will be removed afterwards
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
					// Request could not be sent
					m_vsPrevPendingRequests.insert(m_vsPrevPendingRequests.begin(), sRef);
				}
			}
			else
			{
				// Request is not found in map
				// No action
			}
		}
#endif
	}
	catch (exception &e)
	{
		std::cout << "Exception in CMapHIDRdPeriod::threadRequestInit: " << e.what() << endl;
	}

#if PENDINGREQ_HANDLING // To be enabled in future to follow a policy to attempt max to initiate a request
	// Allow next thread to execute
	sem_post(&semReqInit);
#endif
}

void CRequestInitiator::initiateRequests(std::vector<std::string> a_vsRefID)
{
	try
	{
#ifdef PERFTESTING // will be removed afterwards
		if(!stopPolling.load())
#endif
		{
			std::thread(&CRequestInitiator::threadRequestInit, this, a_vsRefID).detach();
		}
	}
	catch (exception &e)
	{
		std::cout << "Exception in CMapHIDRdPeriod::initiateRequests: " << e.what() << endl;
	}
}

CRequestInitiator::~CRequestInitiator()
{
	m_vsPrevPendingRequests.clear();
}

CRequestInitiator::CRequestInitiator() : m_uiIsNextRequest(0)
{
	try
	{
		initSem();
	}
	catch(const std::exception& e)
	{
		BOOST_LOG_SEV(lg, info) << "Exception CRequestInitiator ::" << __func__ << ": Unable to initiate instance: " << e.what();
		std::cout << "\nException CRequestInitiator ::" << __func__ << ": Unable to initiate instance: " << e.what();
	}
}

CTimeMapper::CTimeMapper()
{
	//m_thread =
	//std::thread(&CTimeMapper::ioPeriodicReadTimer, this, 5).detach();
	//std::cout << "\nCreated periodic read timer thread\n";
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
    std::vector<std::string> vsRefId;

    try
	{
    	std::lock_guard<std::mutex> lock(m_mapMutex);
		for (auto &element : m_mapTimeRecord)
		{
			CTimeRecord &a = element.second;
			if(0 == a.decrementTime(1))
			{
				std::vector<std::string> vsTemp = a.getRefIDList();
				vsRefId.insert(vsRefId.end(), vsTemp.begin(), vsTemp.end());
			}
		}

		if(vsRefId.size())
		{
			CRequestInitiator::instance().initiateRequests(vsRefId);
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

	auto sleepFor = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	/// Reschedule the timer for TIMER_DURATION (i.e. 1 min) in the future
	//t->expires_after(boost::asio::chrono::seconds(60 - sleepFor));
	t->expires_after(boost::asio::chrono::milliseconds(10-sleepFor));

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
	t.expires_after(boost::asio::chrono::seconds(1));	//TODO convert to millisecond
	t.async_wait(boost::bind(CTimeMapper::timerThreadFunc,
	        boost::asio::placeholders::error, &t));

	io.run();
}


bool CRequestInitiator::sendRequest(RestRdPeriodicTagPart_t &a_stRdPrdObj)
{
	MbusAPI_t stMbusApiPram = {};
	errno_t eSafeFunRetType;
	HaystackInfo_t stHaystackInfo = {};
	uint8_t u8ReturnType = MBUS_STACK_NO_ERROR;
	bool bRet = false;

	try
	{
		stMbusApiPram.m_u8DevId = a_stRdPrdObj.m_u8UnitId;
		eSafeFunRetType = memcpy_s(stMbusApiPram.m_u8IpAddr, sizeof(stMbusApiPram.m_u8IpAddr),
				a_stRdPrdObj.m_u8IpAddr, sizeof(stMbusApiPram.m_u8IpAddr));
		if(eSafeFunRetType != EOK)
		{
			BOOST_LOG_SEV(lg, debug) <<"fatal::" << __func__ <<
					"::IpAddr:memcpy_s return type:" << (unsigned)eSafeFunRetType;
		}

		stMbusApiPram.m_u16StartAddr = a_stRdPrdObj.m_u16StartAddr;
		stMbusApiPram.m_u16Quantity = a_stRdPrdObj.m_u16Quantity;
		stMbusApiPram.m_u16ByteCount = a_stRdPrdObj.m_u16ReqDataLen;
		stHaystackInfo = a_stRdPrdObj.m_stHaystackInfo;
		/*Enter TX Id*/
		//stMbusApiPram.m_u16TxId = (unsigned short)g_u16TxId.load();
		stMbusApiPram.m_u16TxId = (unsigned short)1;
#ifdef UNIT_TEST
		stMbusApiPram.m_u16TxId = 5;
#endif
		g_u16TxId++;
		a_stRdPrdObj.m_u16TxId = stMbusApiPram.m_u16TxId;

		//RequestMapper::instance().insertReadPeriodicHaystackStruct(stMbusApiPram.m_u16TxId, stHaystackInfo);

		u8ReturnType = Modbus_Stack_API_Call(a_stRdPrdObj.m_u8FunCode, &stMbusApiPram, (void *)readPeriodicCallBack);

		if(MBUS_STACK_NO_ERROR == u8ReturnType)
		{
			bRet = true;
		}
		else //BACDEL_SUCCESS != eReturnType
		{
			// In case of error, immediately retry in next iteration
			a_stRdPrdObj.bIsRespAwaited = false;
#if PENDINGREQ_HANDLING // To be enabled in future to follow a policy to attempt max to initiate a request
			a_stRdPrdObj.cntError++;

			// If number of consecutive errors are more, then try it after completing certain attemps
			// At present, till 40 sec time retry will be tried
			if(MAX_RDPRD_REQ_RETRIES < a_stRdPrdObj.cntError)
#endif
			{
				//json::value tempRPResponse;
				//string strHID((const char*)stHaystackInfo.m_stHaystackId);
				//tempRPResponse["haystack_id"] = json::value::string(stHaystackInfo.m_stHaystackId);

				//string strKind((const char*)stHaystackInfo.m_stKind);
				//tempRPResponse["kind"] = json::value::string(stHaystackInfo.m_stKind);

				string l_stErrorString = "Request initiation error:: "+ to_string(u8ReturnType)+" " + "DeviceID ::" + to_string(stMbusApiPram.m_u16TxId);
				//tempRPResponse["error"] = json::value::string(l_stErrorString);;
				/// Publishing error message to platformbus
				//PlBusHandler::instance().publishPlBus(tempRPResponse, stHaystackInfo.m_sMqttTopic.c_str());

				//cout << "RP request initiation failed in Periodic read property request with error :: "<< eReturnType << endl;
				bRet = true;
#if PENDINGREQ_HANDLING // To be enabled in future to follow a policy to attempt max to initiate a request
			a_stRdPrdObj.cntError = 0;
#endif

			}
#if PENDINGREQ_HANDLING // To be enabled in future to follow a policy to attempt max to initiate a request
			else
			{
				// Append to pending list for retry
				bRet = false;
			}
#endif
		}
	}
	catch(exception &e)
	{
		std::cout << "Exception in request init: " << e.what() << endl;
	}

	return bRet;
}


bool CTimeMapper::insert(uint32_t a_uTime, const std::string &a_sHID)
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
			it->second.add(a_sHID);
		}
		else
		{
			// Record does not exist
			CTimeRecord oTimeRecord(a_uTime, a_sHID);
			m_mapTimeRecord.emplace(a_uTime, oTimeRecord);
		}
	}
	catch (exception &e)
	{
		std::cout << "Time map insert failed: " << a_uTime << ", " << a_sHID << " : " << e.what() << endl;
		bRet = false;
	}
	return bRet;
}

bool CTimeMapper::remove(uint32_t a_uTime, const std::string &a_sHID)
{
	bool bRet = true;
	// 1. Check if record for this time exists or not
	// 2. If exists, then remove the given reference from existing list
	// 3. If does not exist, no action
	try
	{
		std::lock_guard<std::mutex> lock(m_mapMutex);
		std::map<uint32_t, CTimeRecord>::iterator it = m_mapTimeRecord.find(a_uTime);

		if(m_mapTimeRecord.end() != it)
		{
			// It means record exists
			bRet = it->second.remove(a_sHID);

			if (0 == it->second.size())
			{
				m_mapTimeRecord.erase(it);
			}
		}
		else
		{
			// No action
		}
	}
	catch (exception &e)
	{
		BOOST_LOG_SEV(lg, trace) << "Time map remove failed: " << a_uTime << ", " << a_sHID << " : " << e.what() << endl;
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
	try
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
	}
}

bool CTimeRecord::add(const std::string &a_sHID)
{
	bool bRet = true;
	// 1. Check if record for this time exists or not
	// 2. If exists, no action
	// 3. If does not exist, then add the reference
	try
	{
		std::lock_guard<std::mutex> lock(m_vectorMutex);
		if (std::find(m_vsRefID.begin(), m_vsRefID.end(), a_sHID) != m_vsRefID.end())
		{
		  // Element in vector.
		  // No action
		}
		else
		{
			// Add the reference
			m_vsRefID.push_back(a_sHID);
		}
	}
	catch (exception &e)
	{
		std::cout << "TimeRecord insert failed: " << a_sHID << " : " << e.what() << endl;
		bRet = false;
	}
	return bRet;
}

bool CTimeRecord::remove(const std::string &a_sHID)
{
	bool bRet = true;
	// 1. Check if record for this time exists or not
	// 2. If exists, remove the element
	// 3. If does not exist, no action
	try
	{
		std::lock_guard<std::mutex> lock(m_vectorMutex);
		std::vector<std::string>::iterator it = std::find(m_vsRefID.begin(), m_vsRefID.end(), a_sHID);
		if (it != m_vsRefID.end())
		{
			// Element in vector.
			m_vsRefID.erase(it);
		}
		else
		{
			// No action
		}
	}
	catch (exception &e)
	{
		std::cout << "TimeRecord remove failed: " << a_sHID << " : " << e.what() << endl;
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
		m_vsRefID.clear();
	}
	catch (exception &e)
	{
		std::cout << "Exception in TimeRecord Deletion: " << e.what() << endl;
	}
}

void CTimeRecord::print()
{
	try
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
	}
}

CMapHIDRdPeriod::~CMapHIDRdPeriod()
{
	try
	{
		std::lock_guard<std::mutex> lock(m_mapMutex);
		// For each element in map, remove it from timemapper
		for (auto &element : m_mapHID)
		{
			CTimeMapper::instance().remove(element.second.m_u32Interval, element.first);
		}

		// Clear map
		m_mapHID.clear();
	}
	catch (exception &e)
	{
		std::cout << "Exception in deletion of HID map: " << e.what() << endl;
	}
}

bool CMapHIDRdPeriod::removeElement(const string &a_sHID)
{
	std::lock_guard<std::mutex> lock(m_mapMutex);
	try {
		m_mapHID.erase(a_sHID);
		return true;
	}
	catch (exception &e) {
		std::cout << "Haystack ID not found: " << a_sHID << " : " << e.what() << endl;
	}
	return false;
}

bool CMapHIDRdPeriod::getElement(const string &a_sHID, RestRdPeriodicTagPart_t &a_stRdPeriod)
{
	std::lock_guard<std::mutex> lock(m_mapMutex);
	try {
		a_stRdPeriod = m_mapHID.at(a_sHID);
		return true;
	}
	catch (exception &e)
	{
		BOOST_LOG_SEV(lg, trace) << "trace::getElement" << a_sHID << " : " << e.what() << endl;
	}
	return false;
}

bool CMapHIDRdPeriod::processElement(string &a_sHID, RestRdPeriodicTagPart_t &a_stRdPeriod)
{
	bool bRet = true;
	try {
		//
		RestRdPeriodicTagPart_t *pstRdPeriod;
		{
			std::lock_guard<std::mutex> lock(m_mapMutex);
			pstRdPeriod = &m_mapHID.at(a_sHID);;
		}

		if (NULL == pstRdPeriod)
		{
			// Error.
			return false;
		}

		RestRdPeriodicTagPart_t &stRdPeriod = *pstRdPeriod;

		// Cases:
		// 1. Update interval time
		// 2. Unsubscribe
		// 3. No change

		// Case 1: Update interval time
		if((stRdPeriod.m_u32Interval != a_stRdPeriod.m_u32Interval) &&
			(true == a_stRdPeriod.IsSubscription))
		{
			// Remove element from time map for old entry
			bRet = CTimeMapper::instance().remove(stRdPeriod.m_u32Interval, a_sHID);

			stRdPeriod.m_u32Interval = a_stRdPeriod.m_u32Interval;
			// Make element entry into time map with new time
			bRet = CTimeMapper::instance().insert(a_stRdPeriod.m_u32Interval, a_sHID);
		}
		// Case 2: Unsubscribe
		else if(true != a_stRdPeriod.IsSubscription)
		{
			stRdPeriod.IsSubscription = false;
			// Remove element from time map for old entry
			bRet = CTimeMapper::instance().remove(stRdPeriod.m_u32Interval, a_sHID);

			// Remove element
			removeElement(a_sHID);
		}
		// 3. No change
		else
		{
			// No action needed
		}
	}
	catch(exception &e)
	{
		BOOST_LOG_SEV(lg, trace) << "Haystack ID not found: " << a_sHID << " : " << e.what() << endl;
	}

	return bRet;
}

bool CMapHIDRdPeriod::processElement(RestRdPeriodicTagPart_t &a_stRdPeriod)
{
	bool bRet = true;
	string sHID = a_stRdPeriod.m_stHaystackInfo.m_stHaystackId;
	try {
		// time is in milliseconds. Convert it to minute (whole number)
		/**
		 *  Commenting to keep time in millisecond
		 */
		/*unsigned int min = a_stRdPeriod.m_u32Interval/(1000*60);
		if (0 == min)
		{
			min = 1;
		}
		a_stRdPeriod.m_u32Interval = min;*/

		std::pair<std::map<std::string, RestRdPeriodicTagPart>::iterator, bool> ret;
		{
			std::lock_guard<std::mutex> lock(m_mapMutex);
			ret = m_mapHID.insert(std::pair<std::string, RestRdPeriodicTagPart> (sHID, a_stRdPeriod));
		}

		if (false == ret.second)
		{
			// element already exits, update it
			bRet = processElement(sHID, a_stRdPeriod);
		}
		else
		{
			// element is new, add to time map
			bRet = CTimeMapper::instance().insert(a_stRdPeriod.m_u32Interval, sHID);
		}

		//print();
		//CTimeMapper::instance().print();
	}
	catch (std::exception &e)
	{
		BOOST_LOG_SEV(lg, trace) << "trace::processElement" << e.what() << endl;
		bRet = false;
	}

	return bRet;
}

void CMapHIDRdPeriod::print()
{
	try
	{
		// Clear map
		std::lock_guard<std::mutex> lock(m_mapMutex);
		for (auto &element : m_mapHID)
		{
			RestRdPeriodicTagPart_t &a = element.second;
			std::cout << element.first << "=(" ;
			std::cout << a.m_u32Interval << ", " << a.IsSubscription << ", " << a.m_stHaystackInfo.m_stHaystackId;
			std::cout  << ")" << endl ;
		}
		std::cout << endl;
	}
	catch (exception &e)
	{
		std::cout << "Exception in TimeMapper Deletion: " << e.what() << endl;
	}
}
