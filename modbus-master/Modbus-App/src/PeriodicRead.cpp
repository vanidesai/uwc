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
#include "ConfigManager.hpp"
#include "ModbusOnDemandHandler.hpp"
#include "utils/YamlUtil.hpp"
#include <sstream>
#include <ctime>
#include <chrono>
#include <functional>
#include <sys/timerfd.h>
#include <poll.h>
#include <unistd.h>
#include <time.h>

/// flag to check thread stop condition
std::atomic<bool> g_stopThread;

#define ERORR_MULTIPLIER 1000

/// flag to check timer stop condition
std::atomic<bool> g_stopTimer(false);

extern "C" {
	#include <safe_lib.h>
}

#define TIMER_THREAD_PRIORITY 65
#define TIMER_THREAD_SCHEDULER globalConfig::threadScheduler::RR

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


/// variable to store timer instance
timer_t gTimerid;

/**
 * Get time based parameters like usec, timestamp, transaction id, etc. based on current time
 * @param a_objReqData	:[in] request data
 * @param a_sTimeStamp	:[out] time stamp
 * @param a_sUsec		:[out] usec
 * @param a_sTxID		:[out] transaction id
 * @return none
 */
void getTimeBasedParams(const CRefDataForPolling& a_objReqData, std::string &a_sTimeStamp, std::string &a_sUsec, std::string &a_sTxID)
{
	a_sTxID.clear();
	common_Handler::getTimeParams(a_sTimeStamp, a_sUsec);
	const auto p1 = std::chrono::system_clock::now();

	{
		unsigned long long int u64{
			(unsigned long long int)(std::chrono::duration_cast<std::chrono::milliseconds>(p1.time_since_epoch()).count())
		};
		std::stringstream ss;
		ss << (unsigned long long)(((unsigned long long)(a_objReqData.getDataPoint().getMyRollID()) << 48) | u64);
		a_sTxID.insert(0, ss.str());
	}
}

/**
 * Gets timestmp in nano-seconds from give timepsec structure
 * @param ts	:[in] time to convert to nano-seconds
 * @return	time in micro-seconds
 */
static unsigned long get_micros(struct timespec ts) {
	return (unsigned long)ts.tv_sec * 1000000L + ts.tv_nsec/1000;
}

/**
 * Prepare response json using EIS APIs
 * @param a_pMsg		:[in] pointer to message envelope to fill up
 * @param a_objReqData	:[in] request data
 * @param a_stResp		:[in] response data
 * @param a_pstTsPolling:[in] polling timestamp, if any
 * @return 	true : on success,
 * 			false : on error
 */
bool CPeriodicReponseProcessor::prepareResponseJson(msg_envelope_t** a_pMsg, const CRefDataForPolling* a_objReqData, stStackResponse a_stResp, struct timespec *a_pstTsPolling = NULL)
{
	if((MBUS_CALLBACK_POLLING == a_stResp.m_operationType || MBUS_CALLBACK_POLLING_RT == a_stResp.m_operationType) &&
			NULL == a_objReqData)
	{
		return FALSE;
	}

	bool bRetValue = true;
	bool bIsByteSwap = false;
	bool bIsWordSwap = false;
	msg_envelope_t *msg = NULL;
	try
	{
		MbusAPI_t stMbusApiPram = {};
		std::string sTimestamp, sUsec, sTxID;
		std::string sPolllingVal = "";
		sPolllingVal.clear();
		msg_envelope_elem_body_t* ptTopic = NULL;
		msg_envelope_elem_body_t* ptWellhead = NULL;
		msg_envelope_elem_body_t* ptMetric = NULL;
		msg_envelope_elem_body_t* ptRealTime = NULL;
		msg = msgbus_msg_envelope_new(CT_JSON);
		
		if(MBUS_CALLBACK_POLLING == a_stResp.m_operationType || MBUS_CALLBACK_POLLING_RT == a_stResp.m_operationType)
		{

			string sTopic = a_objReqData->getDataPoint().getID() + SEPARATOR_CHAR + PERIODIC_GENERIC_TOPIC;
			ptTopic = msgbus_msg_envelope_new_string(sTopic.c_str());
			ptWellhead = msgbus_msg_envelope_new_string(a_objReqData->getDataPoint().getWellSite().getID().c_str());
			ptMetric = msgbus_msg_envelope_new_string(a_objReqData->getDataPoint().getDataPoint().getID().c_str());
			ptRealTime =  msgbus_msg_envelope_new_string(std::to_string(a_objReqData->getDataPoint().getDataPoint().getPollingConfig().m_bIsRealTime).c_str());

			// Polling time is explicitly given, use that
			if(NULL != a_pstTsPolling)
			{
				msg_envelope_elem_body_t* ptPollingTS = msgbus_msg_envelope_new_string( (to_string(get_micros(*a_pstTsPolling))).c_str() );
				msgbus_msg_envelope_put(msg, "tsPollingTime", ptPollingTS);
			}
			else
			{
				// Polling time is not given, use one from reference polling point
				msg_envelope_elem_body_t* ptPollingTS = msgbus_msg_envelope_new_string( (to_string(get_micros(a_objReqData->getTimestampOfPollReq()))).c_str() );
				msgbus_msg_envelope_put(msg, "tsPollingTime", ptPollingTS);
			}

			// Request retried timestamp
			//msg_envelope_elem_body_t* ptRetryTS = msgbus_msg_envelope_new_string( (to_string(get_micros(a_objReqData->getTsForRetry()))).c_str() );
			//msgbus_msg_envelope_put(msg, "tsRetry", ptRetryTS);

			// Request retried count
			//msg_envelope_elem_body_t* ptRetrycount = msgbus_msg_envelope_new_string( (to_string(a_objReqData->getRetriedCount()).c_str() ));
			//msgbus_msg_envelope_put(msg, "ReqRetriedCount", ptRetrycount);

			//msgbus_msg_envelope_put(msg, "driver_seq", ptDriverSeq);

			bIsByteSwap = a_objReqData->getDataPoint().getDataPoint().getAddress().m_bIsByteSwap;
			bIsWordSwap = a_objReqData->getDataPoint().getDataPoint().getAddress().m_bIsWordSwap;

			// Point data type
			string aDataType = a_objReqData->getDataPoint().getDataPoint().getAddress().m_sDataType;
			if(!aDataType.empty())
			{
				msg_envelope_elem_body_t* ptDataType = msgbus_msg_envelope_new_string(aDataType.c_str());
				msgbus_msg_envelope_put(msg, "datatype", ptDataType);
			}

			// Persistence
			msg_envelope_elem_body_t* ptPersistence = msgbus_msg_envelope_new_bool(a_objReqData->getDataPoint().getDataPoint().getPersistence());
			msgbus_msg_envelope_put(msg, "persistence", ptPersistence);
		}
		else
		{
			bool bRetVal = common_Handler::getReqData(a_stResp.u16TransacID, stMbusApiPram);
			if(false == bRetVal)
			{
				DO_LOG_FATAL("Could not get data in map for on-demand request");
				return FALSE;
			}

			/// application sequence
			msg_envelope_elem_body_t* ptAppSeq = msgbus_msg_envelope_new_string(stMbusApiPram.m_stOnDemandReqData.m_strAppSeq.c_str());
			/// topic
			ptTopic = msgbus_msg_envelope_new_string(stMbusApiPram.m_stOnDemandReqData.m_strTopic.append("Response").c_str());
			/// wellhead
			ptWellhead = msgbus_msg_envelope_new_string(stMbusApiPram.m_stOnDemandReqData.m_strWellhead.c_str());
			/// metric
			ptMetric = msgbus_msg_envelope_new_string(stMbusApiPram.m_stOnDemandReqData.m_strMetric.c_str());
			/// RealTime
			ptRealTime =  msgbus_msg_envelope_new_string(to_string(stMbusApiPram.m_stOnDemandReqData.m_isRT).c_str());
			/// add timestamps for req recvd by app
			msg_envelope_elem_body_t* ptAppTSReqRcvd = msgbus_msg_envelope_new_string( (to_string(get_micros(stMbusApiPram.m_stOnDemandReqData.m_obtReqRcvdTS))).c_str() );
			/// message received from MQTT Time
			msg_envelope_elem_body_t* ptMqttTime = msgbus_msg_envelope_new_string(stMbusApiPram.m_stOnDemandReqData.m_strMqttTime.c_str());
			/// message received from MQTT Time
			msg_envelope_elem_body_t* ptEisTime = msgbus_msg_envelope_new_string(stMbusApiPram.m_stOnDemandReqData.m_strEisTime.c_str());

			msgbus_msg_envelope_put(msg, "reqRcvdByApp", ptAppTSReqRcvd);
			msgbus_msg_envelope_put(msg, "app_seq", ptAppSeq);
			msgbus_msg_envelope_put(msg, "tsMsgRcvdFromMQTT", ptMqttTime);
			msgbus_msg_envelope_put(msg, "tsMsgPublishOnEIS", ptEisTime);

			bIsByteSwap = stMbusApiPram.m_stOnDemandReqData.m_isByteSwap;
			bIsWordSwap = stMbusApiPram.m_stOnDemandReqData.m_isWordSwap;
		}

		msg_envelope_elem_body_t* ptVersion = msgbus_msg_envelope_new_string("2.0");

		// add timestamps from stack
		msg_envelope_elem_body_t* ptStackTSReqRcvd = msgbus_msg_envelope_new_string( (to_string(get_micros(a_stResp.m_objStackTimestamps.tsReqRcvd))).c_str() );
		msg_envelope_elem_body_t* ptStackTSReqSent = msgbus_msg_envelope_new_string( (to_string(get_micros(a_stResp.m_objStackTimestamps.tsReqSent))).c_str() );
		msg_envelope_elem_body_t* ptStackTSRespRcvd = msgbus_msg_envelope_new_string( (to_string(get_micros(a_stResp.m_objStackTimestamps.tsRespRcvd))).c_str() );
		msg_envelope_elem_body_t* ptStackTSRespPosted = msgbus_msg_envelope_new_string( (to_string(get_micros(a_stResp.m_objStackTimestamps.tsRespSent))).c_str() );
		//msg_envelope_elem_body_t* ptPriority =  msgbus_msg_envelope_new_string(to_string(a_stResp.m_lPriority).c_str());

		msgbus_msg_envelope_put(msg, "version", ptVersion);
		msgbus_msg_envelope_put(msg, "data_topic", ptTopic);
		msgbus_msg_envelope_put(msg, "wellhead", ptWellhead);
		msgbus_msg_envelope_put(msg, "metric", ptMetric);
		msgbus_msg_envelope_put(msg, "realtime", ptRealTime);

		// add timestamps
		msgbus_msg_envelope_put(msg, "reqRcvdInStack", ptStackTSReqRcvd);
		msgbus_msg_envelope_put(msg, "reqSentByStack", ptStackTSReqSent);
		msgbus_msg_envelope_put(msg, "respRcvdByStack", ptStackTSRespRcvd);
		msgbus_msg_envelope_put(msg, "respPostedByStack", ptStackTSRespPosted);

		//// fill value
		*a_pMsg = msg;

		if( a_stResp.m_u8FunCode == READ_COIL_STATUS ||
				a_stResp.m_u8FunCode == READ_HOLDING_REG ||
				a_stResp.m_u8FunCode == READ_INPUT_STATUS ||
				a_stResp.m_u8FunCode == READ_INPUT_REG)
		{
			if(TRUE == a_stResp.bIsValPresent)
			{
				std::vector<uint8_t> vt = a_stResp.m_Value;
				if(0 != vt.size())
				{
					std::string sVal = "";
					sVal = common_Handler::swapConversion(vt, bIsByteSwap, bIsWordSwap);
					if(MBUS_CALLBACK_POLLING == a_stResp.m_operationType || MBUS_CALLBACK_POLLING_RT == a_stResp.m_operationType)
					{
						sPolllingVal  = sVal;
					}

					msg_envelope_elem_body_t* ptValue = msgbus_msg_envelope_new_string(sVal.c_str());
					msgbus_msg_envelope_put(msg, "value", ptValue);
					msg_envelope_elem_body_t* ptStatus = msgbus_msg_envelope_new_string("Good");
					msgbus_msg_envelope_put(msg, "status", ptStatus);
				}
				else
				{
					msg_envelope_elem_body_t* ptStatus = msgbus_msg_envelope_new_string("Bad");

					int iErrCode = a_stResp.m_stException.m_u8ExcStatus * ERORR_MULTIPLIER + a_stResp.m_stException.m_u8ExcCode;
					msg_envelope_elem_body_t* ptErrorDetails = msgbus_msg_envelope_new_string(to_string(iErrCode).c_str());
					msgbus_msg_envelope_put(msg, "status", ptStatus);
					msgbus_msg_envelope_put(msg, "error_code", ptErrorDetails);

					// Use last known value for polling
					if(MBUS_CALLBACK_POLLING == a_stResp.m_operationType || MBUS_CALLBACK_POLLING_RT == a_stResp.m_operationType)
					{
						stLastGoodResponse objLastResp =
								(const_cast<CRefDataForPolling*>(a_objReqData))->getLastGoodResponse();

						msg_envelope_elem_body_t* ptValue = msgbus_msg_envelope_new_string(objLastResp.m_sValue.c_str());
						msgbus_msg_envelope_put(msg, "value", ptValue);

						msg_envelope_elem_body_t* ptLastUsec = msgbus_msg_envelope_new_string(objLastResp.m_sLastUsec.c_str());
						msgbus_msg_envelope_put(msg, "lastGoodUsec", ptLastUsec);
					}
					else
					{
						// it is on-demand read response
						msg_envelope_elem_body_t* ptValue = msgbus_msg_envelope_new_string("");
						msgbus_msg_envelope_put(msg, "value", ptValue);
					}
				}
			}
			else
			{
				msg_envelope_elem_body_t* ptStatus = msgbus_msg_envelope_new_string("Bad");
				int iErrCode = a_stResp.m_stException.m_u8ExcStatus * ERORR_MULTIPLIER + a_stResp.m_stException.m_u8ExcCode;
									msg_envelope_elem_body_t* ptErrorDetails =
											msgbus_msg_envelope_new_string(to_string(iErrCode).c_str());
				msgbus_msg_envelope_put(msg, "status", ptStatus);
				msgbus_msg_envelope_put(msg, "error_code", ptErrorDetails);

				// Use last known value for polling
				if(MBUS_CALLBACK_POLLING == a_stResp.m_operationType || MBUS_CALLBACK_POLLING_RT == a_stResp.m_operationType)
				{
					stLastGoodResponse objLastResp =
							(const_cast<CRefDataForPolling*>(a_objReqData))->getLastGoodResponse();
					msg_envelope_elem_body_t* ptValue = msgbus_msg_envelope_new_string(objLastResp.m_sValue.c_str());
					msgbus_msg_envelope_put(msg, "value", ptValue);

					msg_envelope_elem_body_t* ptLastUsec = msgbus_msg_envelope_new_string(objLastResp.m_sLastUsec.c_str());
					msgbus_msg_envelope_put(msg, "lastGoodUsec", ptLastUsec);
				}
				else
				{
					// it is on-demand read response
					msg_envelope_elem_body_t* ptValue = msgbus_msg_envelope_new_string("");
					msgbus_msg_envelope_put(msg, "value", ptValue);
				}
			}
		}
		else
		{
			msg_envelope_elem_body_t* ptStatus = NULL;
			if(a_stResp.m_stException.m_u8ExcCode == 0 && a_stResp.m_stException.m_u8ExcStatus ==0)
			{
				ptStatus = msgbus_msg_envelope_new_string("Good");
			}
			else
			{
				ptStatus = msgbus_msg_envelope_new_string("Bad");
				int iErrCode = a_stResp.m_stException.m_u8ExcStatus * ERORR_MULTIPLIER + a_stResp.m_stException.m_u8ExcCode;
				msg_envelope_elem_body_t* ptErrorDetails = msgbus_msg_envelope_new_string(to_string(iErrCode).c_str());
				msgbus_msg_envelope_put(msg, "error_code", ptErrorDetails);
			}
			msgbus_msg_envelope_put(msg, "status", ptStatus);
		}

		// Adding timestamp at last
		if(MBUS_CALLBACK_POLLING == a_stResp.m_operationType || MBUS_CALLBACK_POLLING_RT == a_stResp.m_operationType)
		{
			getTimeBasedParams(*a_objReqData, sTimestamp, sUsec, sTxID);

			msg_envelope_elem_body_t* ptDriverSeq = msgbus_msg_envelope_new_string(sTxID.c_str());
			msgbus_msg_envelope_put(msg, "driver_seq", ptDriverSeq);

			if(false == sPolllingVal.empty())
			{
				// save last known response
				(const_cast<CRefDataForPolling*>(a_objReqData))->saveGoodResponse(sPolllingVal, sUsec);
			}
		}
		else
		{
			common_Handler::getTimeParams(sTimestamp, sUsec);
		}

		msg_envelope_elem_body_t* ptTimeStamp = msgbus_msg_envelope_new_string(sTimestamp.c_str());
		msgbus_msg_envelope_put(msg, "timestamp", ptTimeStamp);
	}
    catch(const std::exception& e)
	{
    	DO_LOG_FATAL(e.what());
		bRetValue = false;
	}

	return bRetValue;
}

/**
 * Prepare and post response json to ZMQ
 * @param a_stResp		:[in] response data
 * @param a_objReqData	:[in] request data
 * @param a_pstTsPolling:[in] polling timestamp, if any
 * @return 	true : on success,
 * 			false : on error
 */
bool CPeriodicReponseProcessor::postResponseJSON(stStackResponse& a_stResp, const CRefDataForPolling* a_objReqData, struct timespec *a_pstTsPolling = NULL)
{
	if((MBUS_CALLBACK_POLLING == a_stResp.m_operationType || MBUS_CALLBACK_POLLING_RT == a_stResp.m_operationType)
			&& NULL == a_objReqData)
	{
		return FALSE;
	}
	msg_envelope_t* g_msg = NULL;

	try
	{
		if(FALSE == prepareResponseJson(&g_msg, a_objReqData, a_stResp, a_pstTsPolling))
		{
			DO_LOG_INFO( " Error in preparing response");
			return FALSE;
		}
		else
		{
			if(MBUS_CALLBACK_POLLING == a_stResp.m_operationType || MBUS_CALLBACK_POLLING_RT == a_stResp.m_operationType)
			{
			}
			else
			{
				common_Handler::removeReqData(a_stResp.u16TransacID);	/// removing request structure from map
			}
			if(true == PublishJsonHandler::instance().publishJson(g_msg, a_stResp.m_strResponseTopic))
			{
				DO_LOG_DEBUG("Msg published successfully");
			}

#ifdef INSTRUMENTATION_LOG
			if(NULL != g_msg)
			{
				msg_envelope_serialized_part_t* parts = NULL;
				int num_parts = msgbus_msg_envelope_serialize(g_msg, &parts);
				if(num_parts > 0)
				{
					if(NULL != parts[0].bytes)
					{
						std::string s(parts[0].bytes);

						DO_LOG_DEBUG("TxID:" + to_string(a_stResp.u16TransacID)
										+ ", Msg: " + s);
					}
					msgbus_msg_envelope_serialize_destroy(parts, num_parts);
				}
			}
#endif
		}
	}
	catch(const std::exception& e)
	{
		DO_LOG_FATAL(e.what());
	}

	if(NULL != g_msg)
	{
		msgbus_msg_envelope_destroy(g_msg);
	}

	// return true on success
	return TRUE;
}

/**
 * Post dummy bad response as actual response is not received
 * @param a_objReqData	:[in] request for which to send dummy response
 * @param m_stException	:[in] exception
 * @param a_pstRefPollTime:[in] Polling timestamp for given request
 * @return 	true : on success,
 * 			false : on error
 */
bool CPeriodicReponseProcessor::postDummyBADResponse(CRefDataForPolling& a_objReqData,
		const stException_t m_stException, struct timespec *a_pstRefPollTime = NULL)
{
	try
	{
		// Prepare dummy response
		stStackResponse stResp = {};
		stResp.bIsValPresent = false;
		stResp.m_Value.clear();
		stResp.u16TransacID = 0;
		stResp.u8Reason = m_stException.m_u8ExcCode;
		stResp.m_stException.m_u8ExcCode = m_stException.m_u8ExcCode;
		stResp.m_stException.m_u8ExcStatus = m_stException.m_u8ExcStatus;
		stResp.m_operationType = MBUS_CALLBACK_POLLING;
		stResp.m_u8FunCode = a_objReqData.getFunctionCode();
		//Set polling frequency as priority
		//stResp.m_lPriority = a_objReqData.getDataPoint().getDataPoint().getPollingConfig().m_uiPollFreq;

		// Post it
		postResponseJSON(stResp, &a_objReqData, a_pstRefPollTime);

		// Response is posted. Mark the flag
		a_objReqData.setResponsePosted(true);
	}
	catch(const std::exception& e)
	{
		DO_LOG_FATAL(e.what());
	}

	// return true on success
	return TRUE;
}

/**
 * Post response json to ZMQ using ggiven response data
 * @param a_stResp	:[in] response data
 * @return 	true : on success,
 * 			false : on error
 */
bool CPeriodicReponseProcessor::postResponseJSON(stStackResponse& a_stResp)
{
	try
	{
		if(MBUS_CALLBACK_POLLING == a_stResp.m_operationType || MBUS_CALLBACK_POLLING_RT == a_stResp.m_operationType)
		{
			CRefDataForPolling& objReqData = CRequestInitiator::instance().getTxIDReqData(a_stResp.u16TransacID, a_stResp.m_bIsRT);
			// Node is found in transaction map. Remove it now.
			CRequestInitiator::instance().removeTxIDReqData(a_stResp.u16TransacID, a_stResp.m_bIsRT);
			// Response is received. Reset response awaited status
			objReqData.getDataPoint().setIsAwaitResp(false);
			// reset txid
			objReqData.setReqTxID(0);

			if(TRUE == postResponseJSON(a_stResp, &objReqData))
			{
				// Response is posted. Mark the flag
				objReqData.setResponsePosted(true);
			}
		}
		else
		{
			postResponseJSON(a_stResp, NULL);
		}
	}
	catch(const std::exception& e)
	{
		DO_LOG_FATAL(to_string(a_stResp.u16TransacID) + e.what());
	}

	// return true on success
	return TRUE;
}

/**
 * Initialize semaphore for all RT and Non-RT operations for response processing
 * @return 	true : on success,
 * 			false : on error
 */
bool CPeriodicReponseProcessor::initSem()
{
	//
	int okPolling = sem_init(&semPollingRespProcess, 0, 0 /* Initial value of zero*/);
	int okPollingRT = sem_init(&semRTPollingRespProcess, 0, 0 /* Initial value of zero*/);
	int okODRead = sem_init(&semODReadRespProcess, 0, 0 /* Initial value of zero*/);
	int okODReadRT = sem_init(&semRTODReadRespProcess, 0, 0 /* Initial value of zero*/);
	int okODWrite = sem_init(&semODWriteRespProcess, 0, 0 /* Initial value of zero*/);
	int okODWriteRT = sem_init(&semRTODWriteRespProcess, 0, 0 /* Initial value of zero*/);
	if (okPolling == -1 || okPollingRT == -1 || okODRead == -1 || okODReadRT == -1 || okODWrite == -1 || okODWriteRT ==-1)
	{
	   std::cout << "*******Could not create unnamed semaphore\n";
	   return false;
	}
	return true;
}

/**
 * Response process thread
 * @param operationCallbackType	:[in] operation type, polling, on-demand, RT/Non_RT
 * @param a_refSem: [in] semaphore applicable for given operation type for listening on response data from a queue
 * @param a_refOps: [in] global config reference for given operation
 * @return appropriate error code
 */
eMbusAppErrorCode CPeriodicReponseProcessor::respProcessThreads(eMbusCallbackType operationCallbackType,
		sem_t& a_refSem,
		globalConfig::COperation& a_refOps)
{
	eMbusAppErrorCode eRetType = APP_SUCCESS;
	sem_t& l_sem = a_refSem;

	// set the thread priority
	globalConfig::set_thread_sched_param(a_refOps);

	globalConfig::display_thread_sched_attr("respProcessThreads param::");


	while(false == g_stopThread.load())
	{
		/// iterate Queue one by one to send message on Pl-bus
		do
		{
			if((sem_wait(&l_sem)) == -1 && errno == EINTR)
			{
				continue;	// Continue if interrupted by handler
			}

			stStackResponse res;
			try
			{
				if(false == getDataToProcess(res, operationCallbackType))
				{
					break;
				}

				// fill the response in JSON
				postResponseJSON(res);
				// Cases:
				// 1. Success / Error response received from end device
				// 2. Error response received from stack (e.g. request time-out)
#ifdef PERFTESTING
				if(1 == res.u8Reason)	// 1 means success i.e. response received.
				{
					// Case 1. Success / Error response received from end device
					//if(PDU_TYPE_COMPLEX_ACK == res.stIpArgs.m_stNPDUData.m_stAPDUData.m_ePduType)
					if(true == res.bIsValPresent)
					{
						++iNumOfAck;
						//cout << asctime(ti) << "Number of Complex-Ack received :: "<< ++iNumOfAck << endl;

						if(iNumOfAck == total_read_periodic.load())
						{
							iNumOfAck = 0;
						}

						success_count++;

						// set the RETURN type
						eRetType = APP_SUCCESS;
					}
					else
					{
						error_count++;

						// set the RETURN type
						eRetType = APP_ERROR_EMPTY_DATA_RECVD_FROM_STACK;
					}
				}
				else
				{
					other_status++;
				}
#endif
				/// remove node from TxID map
				//CRequestInitiator::instance().removeTxIDReqData(res.u16TransacID);
				res.m_Value.clear();
			}
			catch(const std::exception& e)
			{
#ifdef PERFTESTING
				other_status++;
#endif
				DO_LOG_FATAL(e.what());
				//return FALSE;
			}
		}while(0);
	}

	return eRetType;
}

/**
 * Push response to queue
 * @param stStackResNode :[in] response node
 * @param operationCallbackType: [in] operation type (polling/on-demand/RT/Non-RT) defines queue to be used
 * @return 	true : on success,
 * 			false : on error
 */
bool CPeriodicReponseProcessor::pushToQueue(struct stStackResponse &stStackResNode, eMbusCallbackType operationCallbackType)
{
	try
	{
		switch(operationCallbackType)
		{
		case MBUS_CALLBACK_POLLING:
		{
			std::lock_guard<std::mutex> lock(__resQPollingMutex);
			stackPollingResQ.push(stStackResNode);
			sem_post(&semPollingRespProcess);	// Signal response process thread
		}
			break;
		case MBUS_CALLBACK_POLLING_RT:
		{
			std::lock_guard<std::mutex> lock(__resQRTPollingMutex);
			stackRTPollingResQ.push(stStackResNode);
			sem_post(&semRTPollingRespProcess);	// Signal response process thread
		}
			break;
		case MBUS_CALLBACK_ONDEMAND_READ:
		{
			std::lock_guard<std::mutex> lock(__resQODReadMutex);
			stackOnDemandReadResQ.push(stStackResNode);
			sem_post(&semODReadRespProcess);	// Signal response process thread
		}
			break;
		case MBUS_CALLBACK_ONDEMAND_READ_RT:
		{
			std::lock_guard<std::mutex> lock(__resQODRTReadMutex);
			stackOnDemandRTReadResQ.push(stStackResNode);
			sem_post(&semRTODReadRespProcess);	// Signal response process thread
		}
			break;
		case MBUS_CALLBACK_ONDEMAND_WRITE:
		{
			std::lock_guard<std::mutex> lock(__resQODWriteMutex);
			stackOnDemandWriteResQ.push(stStackResNode);
			sem_post(&semODWriteRespProcess);	// Signal response process thread
		}
			break;
		case MBUS_CALLBACK_ONDEMAND_WRITE_RT:
		{
			std::lock_guard<std::mutex> lock(__resQODRTWriteMutex);
			stackOnDemandRTWriteResQ.push(stStackResNode);
			sem_post(&semRTODWriteRespProcess);	// Signal response process thread
		}
			break;
		default:
			DO_LOG_FATAL("Invalid callback called.");
			break;
		}

		return true;
	}
	catch(const std::exception& e)
	{
		DO_LOG_FATAL(e.what());
	}

	return false;
}

/**
 * In case of response timeout reply, checks if retry is applicable.
 * Retries the request, if applicable
 * @param stStackResNode :[in] response node
 * @param operationCallbackType: [in] operation type (polling/on-demand/RT/Non-RT) defines queue to be used
 * @return 	true : if no retry is applicable
 * 			false : in case of error or retry is done
 */
bool CPeriodicReponseProcessor::checkForRetry(struct stStackResponse &a_stStackResNode, eMbusCallbackType operationCallbackType)
{
	bool retValue = false;
	try
	{
		MbusAPI_t *pReqData = NULL;
		MbusAPI_t tempData;
		eMbusAppErrorCode eFunRetType = APP_SUCCESS;
		if(a_stStackResNode.m_stException.m_u8ExcCode == STACK_ERROR_RECV_TIMEOUT &&
				a_stStackResNode.m_stException.m_u8ExcStatus == 2)
		{
			if((MBUS_CALLBACK_POLLING == operationCallbackType) ||
					(MBUS_CALLBACK_POLLING_RT == operationCallbackType))
			{
				bool bIsPresent = CRequestInitiator::instance().isTxIDPresent(a_stStackResNode.u16TransacID, a_stStackResNode.m_bIsRT);
				if(true == bIsPresent)
				{
					CRefDataForPolling &oRef =
							CRequestInitiator::instance().getTxIDReqData(a_stStackResNode.u16TransacID, a_stStackResNode.m_bIsRT);
					MbusAPI_t &refReq = oRef.getMBusReq();
					pReqData = &refReq;

					if(refReq.m_nRetry > 0)
					{
						struct timespec ts;
						timespec_get(&ts, TIME_UTC);
						oRef.flagRetry(ts);
					}
				}
			}
			else
			{
				if(true == common_Handler::getReqData(a_stStackResNode.u16TransacID, tempData))
				{
					pReqData = &tempData;
				}
			}

			if(NULL == pReqData)
			{
				return false;
			}
			MbusAPI_t &reqData = *pReqData;
			if(reqData.m_nRetry > 0)
			{
				DO_LOG_INFO("Retry called for transaction id:: "+to_string(reqData.m_u16TxId));
				/// decrement retry value by 1
				reqData.m_nRetry--;
				if((MBUS_CALLBACK_POLLING != operationCallbackType) &&
									(MBUS_CALLBACK_POLLING_RT != operationCallbackType))
				{
					/// updating structure in map with decremented retry value
					common_Handler::updateReqData(a_stStackResNode.u16TransacID, reqData);
				}

				void* ptrAppCallback = NULL;
				getCallbackForRetry(&ptrAppCallback, operationCallbackType);

				eFunRetType = (eMbusAppErrorCode) Modbus_Stack_API_Call(
						a_stStackResNode.m_u8FunCode,
						&reqData,
						ptrAppCallback);
				if(APP_SUCCESS != eFunRetType)
				{
					DO_LOG_INFO("Retry stack call failed. error::  "+ to_string(eFunRetType));
				}
			}
			else
			{
				retValue = true;
			}
		}
		else
		{
			retValue = true;
		}
	}
	catch(const std::exception& e)
	{
		DO_LOG_FATAL(e.what());
	}

	return retValue;
}

/**
 * Get response data to process from queue
 * @param a_stStackResNode :[out] response node
 * @param operationCallbackType: [in] operation type (polling/on-demand/RT/Non-RT) defines queue to be used
 * @return 	true : on success,
 * 			false : on error
 */
bool CPeriodicReponseProcessor::getDataToProcess(struct stStackResponse &a_stStackResNode, eMbusCallbackType operationCallbackType)
{
	bool retValue = false;
	try
	{
		switch (operationCallbackType)
		{
		case MBUS_CALLBACK_POLLING:
		{
			std::lock_guard<std::mutex> lock(__resQPollingMutex);
			if(stackPollingResQ.empty())
			{
				DO_LOG_DEBUG("Response queue is empty");
				return false;
			}
			a_stStackResNode = stackPollingResQ.front();
			stackPollingResQ.pop();
		}
		break;
		case MBUS_CALLBACK_POLLING_RT:
		{
			std::lock_guard<std::mutex> lock(__resQRTPollingMutex);
			if(stackRTPollingResQ.empty())
			{
				DO_LOG_DEBUG("Response queue is empty");
				return false;
			}
			a_stStackResNode = stackRTPollingResQ.front();
			stackRTPollingResQ.pop();
		}
		break;
		case MBUS_CALLBACK_ONDEMAND_READ:
		{
			std::lock_guard<std::mutex> lock(__resQODReadMutex);
			if(stackOnDemandReadResQ.empty())
			{
				DO_LOG_DEBUG("Response queue is empty");
				return false;
			}
			a_stStackResNode = stackOnDemandReadResQ.front();
			stackOnDemandReadResQ.pop();
		}
		break;
		case MBUS_CALLBACK_ONDEMAND_READ_RT:
		{
			std::lock_guard<std::mutex> lock(__resQODRTReadMutex);
			if(stackOnDemandRTReadResQ.empty())
			{
				DO_LOG_DEBUG("Response queue is empty");
				return false;
			}
			a_stStackResNode = stackOnDemandRTReadResQ.front();
			stackOnDemandRTReadResQ.pop();
		}
		break;
		case MBUS_CALLBACK_ONDEMAND_WRITE:
		{
			std::lock_guard<std::mutex> lock(__resQODWriteMutex);
			if(stackOnDemandWriteResQ.empty())
			{
				DO_LOG_DEBUG("Response queue is empty");
				return false;
			}
			a_stStackResNode = stackOnDemandWriteResQ.front();
			stackOnDemandWriteResQ.pop();
		}
		break;
		case MBUS_CALLBACK_ONDEMAND_WRITE_RT:
		{
			std::lock_guard<std::mutex> lock(__resQODRTWriteMutex);
			if(stackOnDemandRTWriteResQ.empty())
			{
				DO_LOG_DEBUG("Response queue is empty");
				return false;
			}
			a_stStackResNode = stackOnDemandRTWriteResQ.front();
			stackOnDemandRTWriteResQ.pop();
		}
		break;
		default:
			DO_LOG_FATAL("Invaid callback to get data from queue.");
			break;
		}

		if(false == checkForRetry(a_stStackResNode, operationCallbackType))
		{
			return false;
		}

		retValue = true;
	}
	catch(const std::exception& e)
	{
		DO_LOG_FATAL(e.what());
	}

	return retValue;
}

/**
 * Receives raw response data and pushes to queue for processing
 * @param pstMbusAppCallbackParams :[in] response received from stack
 * @param operationCallbackType :[in] Operation type - polling/on-demand/RT/Non-RT
 * @param strResponseTopic: [in] ZMQ topic to be used. Defined by calling callback function
 * @param a_bIsRT: [in] indicates whether it is a RT request
 * @return none
 */
void CPeriodicReponseProcessor::handleResponse(stMbusAppCallbackParams_t *pstMbusAppCallbackParams,
												//eMbusResponseType respType,
												eMbusCallbackType operationCallbackType,
												string strResponseTopic,
												bool a_bIsRT)
{
	if(pstMbusAppCallbackParams == NULL)
	{
		DO_LOG_ERROR("Response received from stack is null hence discarding");
		return;
	}

	struct stStackResponse stStackResNode;
	stStackResNode.bIsValPresent = false;
	stStackResNode.u8Reason = 0;
	stStackResNode.m_operationType = operationCallbackType;
	stStackResNode.m_u8FunCode = pstMbusAppCallbackParams->m_u8FunctionCode;
	stStackResNode.m_strResponseTopic = strResponseTopic;
	stStackResNode.m_bIsRT = a_bIsRT;

	try
	{
		stStackResNode.m_stException.m_u8ExcCode = pstMbusAppCallbackParams->m_u8ExceptionExcCode;
		stStackResNode.m_stException.m_u8ExcStatus = pstMbusAppCallbackParams->m_u8ExceptionExcStatus;

		stStackResNode.u16TransacID = pstMbusAppCallbackParams->m_u16TransactionID;

		memcpy_s((&stStackResNode.m_objStackTimestamps),
				sizeof(stTimeStamps),
				&pstMbusAppCallbackParams->m_objTimeStamps,
				sizeof(pstMbusAppCallbackParams->m_objTimeStamps));

		stStackResNode.m_lPriority = pstMbusAppCallbackParams->m_lPriority;

		if((0 == pstMbusAppCallbackParams->m_u8ExceptionExcStatus) &&
				(0 == pstMbusAppCallbackParams->m_u8ExceptionExcCode))
		{
			stStackResNode.u8Reason = 1;
			stStackResNode.bIsValPresent = true;
			for (uint8_t index=0; index < pstMbusAppCallbackParams->m_u8MbusRXDataLength ; index++)
			{
				stStackResNode.m_Value.push_back(pstMbusAppCallbackParams->m_au8MbusRXDataDataFields[index]);
			}
		}
		else
		{
			stStackResNode.u8Reason = 0;
			stStackResNode.bIsValPresent = false;
		}

		pushToQueue(stStackResNode, operationCallbackType);
	}
	catch(const std::exception& e)
	{
		DO_LOG_FATAL(e.what());
	}
}

/**
 * Function to receive read callback for Non-RT polling
 * @param pstMbusAppCallbackParams :[in] parameters received from stack
 * @param uTxID: [in] transaction id for matching request and response
 * @return appropriate error code
 */
eMbusAppErrorCode readPeriodicCallBack(stMbusAppCallbackParams_t *pstMbusAppCallbackParams, uint16_t uTxID)
{
	if(pstMbusAppCallbackParams == NULL)
	{
		DO_LOG_DEBUG("Response received from stack is null");
		return APP_ERROR_EMPTY_DATA_RECVD_FROM_STACK;
	}

	//handle response
	CPeriodicReponseProcessor::Instance().handleResponse(pstMbusAppCallbackParams,
														MBUS_CALLBACK_POLLING,
														PublishJsonHandler::instance().getPolledDataTopic(),
														false);
	return APP_SUCCESS;
}

/**
 * Function to receive read callback for RT polling
 * @param pstMbusAppCallbackParams :[in] parameters received from stack
 * @param uTxID: [in] transaction id for matching request and response
 * @return appropriate error code
 */
eMbusAppErrorCode readPeriodicRTCallBack(stMbusAppCallbackParams_t *pstMbusAppCallbackParams, uint16_t uTxID)
{
	if(pstMbusAppCallbackParams == NULL)
	{
		DO_LOG_DEBUG("Response received from stack is null");
		return APP_ERROR_EMPTY_DATA_RECVD_FROM_STACK;
	}

	//handle response
	CPeriodicReponseProcessor::Instance().handleResponse(pstMbusAppCallbackParams,
														MBUS_CALLBACK_POLLING_RT,
														PublishJsonHandler::instance().getPolledDataTopicRT(),
														true);
	return APP_SUCCESS;
}

/**
 * Function to identify callback function based on operation type for calling stack APIs
 * @param callbackFunc :[out] callback function to be used for calling stack APIs
 * @param operationCallbackType: [in] operation type for which callback function needs to be identified
 * @return none
 */
void CPeriodicReponseProcessor::getCallbackForRetry(void**callbackFunc, eMbusCallbackType operationCallbackType)
{
	switch(operationCallbackType)
	{
	case MBUS_CALLBACK_POLLING:
	{
		*callbackFunc = (void*)readPeriodicCallBack;
	}
	break;
	case MBUS_CALLBACK_POLLING_RT:
	{
		*callbackFunc = (void*)readPeriodicRTCallBack;
	}
	break;
	case MBUS_CALLBACK_ONDEMAND_READ:
	{
		*callbackFunc = (void*)OnDemandRead_AppCallback;
	}
	break;
	case MBUS_CALLBACK_ONDEMAND_READ_RT:
	{
		*callbackFunc = (void*)OnDemandReadRT_AppCallback;
	}
	break;
	case MBUS_CALLBACK_ONDEMAND_WRITE:
	{
		*callbackFunc = (void*)OnDemandWrite_AppCallback;
	}
	break;
	case MBUS_CALLBACK_ONDEMAND_WRITE_RT:
	{
		*callbackFunc = (void*)OnDemandWriteRT_AppCallback;
	}
	break;
	default:
		break;
	}
}

/**
 * Constructor: Creates instance of CPeriodicReponseProcessor to process responses
 * received from network. This is a singleton class. This instance initiates semaphores
 * which are used to signal different threads on receiving responses.
 * @param None
 * @return none
 */
CPeriodicReponseProcessor::CPeriodicReponseProcessor() : m_bIsInitialized(false)
{
	try
	{
		initSem();
		m_bIsInitialized = true;
	}
	catch(const std::exception& e)
	{
		DO_LOG_FATAL("Unable to initiate instance");
		DO_LOG_FATAL(e.what());
		std::cout << "\nException CPeriodicReponseProcessor ::" << __func__ << ": Unable to initiate instance: " << e.what();
	}
}

/**
 * This is a singleton class. Returns singleton instance of response processor.
 * @return singleton instance
 */
CPeriodicReponseProcessor& CPeriodicReponseProcessor::Instance()
{
	static CPeriodicReponseProcessor _self;
	return _self;
}

/**
 * Initiates number of threads to process responses from network for various operations.
 * It passes parameters based on operation type to thread init function.
 * Operations: Polling, Polling-RT, On-demand-read, On-demand-read-RT, On-demand-write, On-demand-write-RT
 * @param None
 * @return none
 */
void CPeriodicReponseProcessor::initRespHandlerThreads()
{
	static bool bSpawned = false;
	try
	{
		if(false == bSpawned)
		{
			{
				std::thread(&CPeriodicReponseProcessor::respProcessThreads, std::ref(*this),
						MBUS_CALLBACK_POLLING, std::ref(semPollingRespProcess),std::ref(globalConfig::CGlobalConfig::getInstance().getOpPollingOpConfig().getNonRTConfig())).detach();
				std::thread(&CPeriodicReponseProcessor::respProcessThreads, std::ref(*this),
						MBUS_CALLBACK_POLLING_RT, std::ref(semRTPollingRespProcess),std::ref(globalConfig::CGlobalConfig::getInstance().getOpPollingOpConfig().getRTConfig())).detach();
				std::thread(&CPeriodicReponseProcessor::respProcessThreads, std::ref(*this),
						MBUS_CALLBACK_ONDEMAND_READ, std::ref(semODReadRespProcess),std::ref(globalConfig::CGlobalConfig::getInstance().getOpOnDemandReadConfig().getNonRTConfig())).detach();
				std::thread(&CPeriodicReponseProcessor::respProcessThreads, std::ref(*this),
						MBUS_CALLBACK_ONDEMAND_READ_RT, std::ref(semRTODReadRespProcess),std::ref(globalConfig::CGlobalConfig::getInstance().getOpOnDemandReadConfig().getRTConfig())).detach();
				std::thread(&CPeriodicReponseProcessor::respProcessThreads, std::ref(*this),
						MBUS_CALLBACK_ONDEMAND_WRITE, std::ref(semODWriteRespProcess),std::ref(globalConfig::CGlobalConfig::getInstance().getOpOnDemandWriteConfig().getNonRTConfig())).detach();
				std::thread(&CPeriodicReponseProcessor::respProcessThreads, std::ref(*this),
						MBUS_CALLBACK_ONDEMAND_WRITE_RT, std::ref(semRTODWriteRespProcess),std::ref(globalConfig::CGlobalConfig::getInstance().getOpOnDemandWriteConfig().getRTConfig())).detach();
			}
			bSpawned = true;
		}
	}
	catch(const std::exception& e)
	{
		DO_LOG_FATAL("Unable to initiate instance");
		DO_LOG_FATAL(e.what());
		std::cout << "\nException CPeriodicReponseProcessor ::" << __func__ << ": Unable to initiate instance: " << e.what();
	}
}

/**
 * Initiate request initiator
 * @return 	true : on success,
 * 			false : on error
 */
/**
 * This is a singleton class. Used to send periodic requests and check if
 * response is received at given cutoff time.
 * Initiates number of threads, semaphores to requests and cutoff processing.
 * Separate threads are created each for RT and Non-RT.
 * @param None
 * @return true on successful init
 */
bool CRequestInitiator::init()
{
	// Initiate semaphore for requests
	int retVal = sem_init(&semaphoreReqProcess, 0, 0 /* Initial value of zero*/);
	if (retVal == -1)
	{
		std::cout << "*******Could not create unnamed semaphore for non-RT\n";
		return false;
	}
	std::thread{std::bind(&CRequestInitiator::threadReqInit,
			std::ref(*this),
			false,
			std::ref(globalConfig::CGlobalConfig::getInstance().getOpPollingOpConfig().getNonRTConfig()))}.detach();

	retVal = sem_init(&semaphoreRTReqProcess, 0, 0 /* Initial value of zero*/);
	if (retVal == -1)
	{
		std::cout << "*******Could not create unnamed semaphore for RT\n";
		return false;
	}
	std::thread{std::bind(&CRequestInitiator::threadReqInit,
			std::ref(*this),
			true,
			std::ref(globalConfig::CGlobalConfig::getInstance().getOpPollingOpConfig().getRTConfig()))}.detach();

	// Initiate semaphore for responses
	retVal = sem_init(&semaphoreRTRespProcess, 0, 0 /* Initial value of zero*/);
	if (retVal == -1)
	{
		std::cout << "*******Could not create unnamed semaphore for RT responses\n";
		return false;
	}
	std::thread{std::bind(&CRequestInitiator::threadCheckCutoffRespInit,
			std::ref(*this),
			true,
			std::ref(globalConfig::CGlobalConfig::getInstance().getOpPollingOpConfig().getRTConfig()))}.detach();

	retVal = sem_init(&semaphoreRespProcess, 0, 0 /* Initial value of zero*/);
	if (retVal == -1)
	{
		std::cout << "*******Could not create unnamed semaphore for non-RT response\n";
		return false;
	}
	std::thread{std::bind(&CRequestInitiator::threadCheckCutoffRespInit,
			std::ref(*this),
			false,
			std::ref(globalConfig::CGlobalConfig::getInstance().getOpPollingOpConfig().getNonRTConfig()))}.detach();


	return true;
}

/**
 * Request initiation is done by different threads. Polling interval is passed to these threads.
 * For polling operation, based on polling interval, the points are fetched.
 * This function enqueues the polling interval and different threads are signaled.
 * @param a_stPollRef: [in] polling timestamp and frequency
 * @param a_objTimeRecord :[in] reference of TimeRecord class
 * @param a_bIsReq: [in] bool variable to differentiate between request and response
 * @return 	true : on success,
 * 			false : on error
 */
bool CRequestInitiator::pushPollFreqToQueue(struct StPollingInstance &a_stPollRef, CTimeRecord &a_objTimeRecord, bool a_bIsReq)
{
	try
	{
		if (true == a_bIsReq)
		{
			std::lock_guard<std::mutex> lock(m_mutexReqFreqQ);

			// Check for availability of RT list
			if(true == a_objTimeRecord.isRTListAvailable())
			{
				m_qReqFreqRT.push(a_stPollRef);
				// Signal response process thread
				sem_post(&semaphoreRTReqProcess);
			}

			// Check for availability of non-RT list
			if(true == a_objTimeRecord.isNonRTListAvailable())
			{
				m_qReqFreq.push(a_stPollRef);
				// Signal response process thread
				sem_post(&semaphoreReqProcess);
			}
		}
		else
		{
			std::lock_guard<std::mutex> lock(m_mutexRespFreqQ);

			// Check for availability of RT list
			if(true == a_objTimeRecord.isRTListAvailable())
			{
				m_qRespFreqRT.push(a_stPollRef);
				// Signal response process thread
				sem_post(&semaphoreRTRespProcess);
			}

			// Check for availability of non-RT list
			if(true == a_objTimeRecord.isNonRTListAvailable())
			{
				m_qRespFreq.push(a_stPollRef);
				// Signal response process thread
				sem_post(&semaphoreRespProcess);
			}
		}

		return true;
	}
	catch(const std::exception& e)
	{
		DO_LOG_FATAL(e.what());
	}

	return false;
}

/**
 * Retrieve polling interval from queue for initiating requests for polling and cutoff.
 * Depending RT/Non-RT, polling/cutoff - different threads, semaphores ae used.
 * @param a_stPollRef :[out] reference to polling interval to be used for polling
 * @param a_bIsRT	:[in] bool variable to differentiate between RT/Non-RT
 * @param a_bIsReq	:[in] bool variable to differentiate between request and response
 * @return 	true : on success,
 * 			false : on error
 */
bool CRequestInitiator::getFreqRefForPollCycle(struct StPollingInstance &a_stPollRef, bool a_bIsRT, bool a_bIsReq)
{
	try
	{
		// Check if it is a request
		if(true == a_bIsReq)
		{
			// Refer request related data
			std::lock_guard<std::mutex> lock(m_mutexReqFreqQ);
			if(true == a_bIsRT)
			{
				a_stPollRef = m_qReqFreqRT.front();
				m_qReqFreqRT.pop();
			}
			else
			{
				a_stPollRef = m_qReqFreq.front();
				m_qReqFreq.pop();
			}
		}
		else
		{
			// Refer response related data
			std::lock_guard<std::mutex> lock(m_mutexRespFreqQ);
			if(true == a_bIsRT)
			{
				a_stPollRef = m_qRespFreqRT.front();
				m_qRespFreqRT.pop();
			}
			else
			{
				a_stPollRef = m_qRespFreq.front();
				m_qRespFreq.pop();
			}
		}

		return true;
	}
	catch(const std::exception& e)
	{
		DO_LOG_FATAL(e.what());
	}

	return false;
}

/**
 * Initiate request for polling
 * @param a_stPollTimestamp:[in] timestamp at which polling interval triggered
 * @param a_vReqData	:[in] List of points to be polled
 * @param isRTRequest	:[in] boolean variable to distinguish between RT/Non-RT requests
 * @param a_lPriority	:[in] priority assigned to message when sending a request
 * @param a_nRetry		:[in] request retries to be performed in case of timeout
 * @param a_ptrCallbackFunc	:[in] callback function to be called by stack to send response
 * @return none
 */
void CRequestInitiator::initiateRequest(struct timespec &a_stPollTimestamp, std::vector<CRefDataForPolling>& a_vReqData,
		bool isRTRequest,
		const long a_lPriority,
		int a_nRetry,
		void* a_ptrCallbackFunc)
{
	for(auto &objReqData: a_vReqData)
	{
		// Check if a response is already awaited
		if(true == objReqData.getDataPoint().isIsAwaitResp())
		{
			// waiting for response. Send BAD response
			stException_t m_stException = {};
			m_stException.m_u8ExcCode = APP_ERROR_DUMMY_RESPONSE;
			m_stException.m_u8ExcStatus = 0;
			uint16_t lastTxID = objReqData.getReqTxID();
			DO_LOG_INFO("Post dummy response as response not received for - Point: " + objReqData.getDataPoint().getID()
						+ ", LastTxID: " + to_string(lastTxID));
			CPeriodicReponseProcessor::Instance().postDummyBADResponse(objReqData, m_stException, &a_stPollTimestamp);

			if(false == CRequestInitiator::instance().isTxIDPresent(lastTxID, isRTRequest))
			{
				DO_LOG_INFO("TxID is not present in map.Resetting the response status");
				objReqData.getDataPoint().setIsAwaitResp(false);
			}
			continue;
		}

		//if(true == bIsFound)
		{
			// generate the TX ID
			//uint16_t m_u16TxId = PublishJsonHandler::instance().getTxId();
			uint16_t m_u16TxId = objReqData.getDataPoint().getMyRollID();

			// Set data for this polling request
			objReqData.setDataForNewReq(m_u16TxId, a_stPollTimestamp);

			DO_LOG_INFO("Trying to send request for - Point: " + 
						objReqData.getDataPoint().getID() +
						", with TxID: " + to_string(m_u16TxId));

			// Send a request
			if (true == sendRequest(objReqData, m_u16TxId, isRTRequest, a_lPriority, a_nRetry, a_ptrCallbackFunc))
			{
				// Request is sent successfully
				// No action
#ifdef PERFTESTING // will be removed afterwards
				++reqCount;
				if(reqCount == total_read_periodic.load())
				{
					reqCount = 0;
					DO_LOG_ERROR("Polling is stopped ....");
					stopPolling.store(true);
					break;
				}
#endif
			}
			else
			{
				objReqData.getDataPoint().setIsAwaitResp(false);
				stException_t m_stException = {};
				m_stException.m_u8ExcCode = APP_ERROR_REQUEST_SEND_FAILED;
				m_stException.m_u8ExcStatus = 0;
				CPeriodicReponseProcessor::Instance().postDummyBADResponse(objReqData, m_stException);

				/// remove node from TxID map
				CRequestInitiator::instance().removeTxIDReqData(m_u16TxId, isRTRequest);
				// reset txid
				objReqData.setReqTxID(0);
				DO_LOG_ERROR("sendRequest failed");
			}
		}
	}
}

/**
 * Thread function to initiate requests for polling.
 * It listens on a semaphore to retrieve polling interval to be used to send requests.
 * @param isRTPoint	:[in] bool variable to differentiate between RT/Non-RT
 * @param a_refOps  :[in] reference to global configuration given operation type
 * @return none
 */
void CRequestInitiator::threadReqInit(bool isRTPoint,
		const globalConfig::COperation& a_refOps)
{

		// set the thread priority
		globalConfig::set_thread_sched_param(a_refOps);
		long l_reqPriority = common_Handler::getReqPriority(a_refOps);

		globalConfig::display_thread_sched_attr("threadReqInit param::");

		sem_t *pSem = NULL;
		int nRetry = 0;
		void* ptrCallbackFunc = NULL;
		if(true == isRTPoint)
		{
			pSem = &semaphoreRTReqProcess;
			nRetry = globalConfig::CGlobalConfig::getInstance().getOpPollingOpConfig().getRTConfig().getRetries();
			ptrCallbackFunc = (void*)readPeriodicRTCallBack;
		}
		else
		{
			pSem = &semaphoreReqProcess;
			nRetry = globalConfig::CGlobalConfig::getInstance().getOpPollingOpConfig().getNonRTConfig().getRetries();
			ptrCallbackFunc = (void*)readPeriodicCallBack;
		}
		if(NULL == pSem)
		{
			// Error condition
			DO_LOG_FATAL("Semaphore is null");
			return;
		}

		while(false == g_stopThread.load())
		{
			try
			{
				do
				{
					if((sem_wait(pSem)) == -1 && errno == EINTR)
					{
						// Continue if interrupted by handler
						continue;
					}
					if(true == g_stopThread.load())
					{
						break;
					}
					struct StPollingInstance stPollRef = {0};
					if(false == getFreqRefForPollCycle(stPollRef, isRTPoint, true))
					{
						break;
					}
					std::vector<CRefDataForPolling>& vReqData = CTimeMapper::instance().getPolledPointList(stPollRef.m_uiPollInterval, isRTPoint);
					initiateRequest(stPollRef.m_tsPollTime, vReqData, isRTPoint, (CTimeMapper::instance().getFreqIndex(stPollRef.m_uiPollInterval) +
							l_reqPriority + 1), nRetry, ptrCallbackFunc);
				} while(0);

			}
			catch (exception &e)
			{
				DO_LOG_FATAL("failed to initiate request :: " + std::string(e.what()));
			}
		}
}

/**
 * Thread function to send pending responses
 * @param isRTPoint	:[in] bool variable to differentiate between RT/Non-RT
 */
/**
 * Thread function to check cutoff time for sent requests and send error response.
 * It listens on a semaphore to retrieve polling interval to be used to check for cutoff.
 * @param isRTPoint	:[in] bool variable to differentiate between RT/Non-RT
 * @param a_refOps  :[in] reference to global configuration given operation type
 * @return none
 */
void CRequestInitiator::threadCheckCutoffRespInit(bool isRTPoint,
		const globalConfig::COperation& a_refOps)
{
	try
	{
		// set the thread priority
		globalConfig::set_thread_sched_param(a_refOps);

		globalConfig::display_thread_sched_attr("threadCheckCutoffRespInit param::");

		sem_t *pSem = NULL;
		if(true == isRTPoint)
		{
			pSem = &semaphoreRTRespProcess;
		}
		else
		{
			pSem = &semaphoreRespProcess;
		}
		if(NULL == pSem)
		{
			// Error condition
			DO_LOG_FATAL("Semaphore is null");
			return;
		}

		while(false == g_stopThread.load())
		{
			do
			{
				if((sem_wait(pSem)) == -1 && errno == EINTR)
				{
					// Continue if interrupted by handler
					continue;
				}

				if(true == g_stopThread.load())
				{
					break;
				}
				struct StPollingInstance stPollRef = {0};
				if(false == getFreqRefForPollCycle(stPollRef, isRTPoint, false))
				{
					break;
				}
				std::vector<CRefDataForPolling>& vReqData =
						CTimeMapper::instance().getPolledPointList(stPollRef.m_uiPollInterval, isRTPoint);

				// Check if responses are sent
				for(auto &objPolledPoint : vReqData)
				{
					if(true == objPolledPoint.isResponsePosted())
					{
						DO_LOG_DEBUG(objPolledPoint.getDataPoint().getID()
											+ ": Response posted. No action in cutoff");
					}
					else
					{
						// waiting for response. Send BAD response
						stException_t m_stException = {0};
						m_stException.m_u8ExcCode = APP_ERROR_CUTOFF_TIME_INTERVAL;
						m_stException.m_u8ExcStatus = 0;
						if(true == objPolledPoint.isLastRespAvailable())
						{
							// Last response is available. Use it.
							DO_LOG_DEBUG(objPolledPoint.getDataPoint().getID()
																+ ": Using last response");

							/*m_stException.m_u8ExcCode = 103;
							m_stException.m_u8ExcStatus = 0;*/
						}
						else
						{
							// Last response is not available. Send dummy response
							DO_LOG_DEBUG(objPolledPoint.getDataPoint().getID()
									+ ": Response NOT posted. Sending dummy response");
						}
						CPeriodicReponseProcessor::Instance().postDummyBADResponse(objPolledPoint, m_stException);
						// Response is posted. Mark the flag
						objPolledPoint.setResponsePosted(true);
					}
				}
			} while(0);
		}
	}
	catch (exception &e)
	{
		DO_LOG_FATAL(e.what());
	}
}

/**
 * Adds polling interval to queue for triggering request initiation for polling
 * @param a_stPollRef	:[in] polling time and polling frequency
 * @param a_objTimeRecord	:[in] reference of TimeRecord
 * @param a_bIsReq	:[in] bool variable to differentiate between request and cutoff
 * @return none
 */
void CRequestInitiator::initiateMessages(struct StPollingInstance &a_stPollRef, CTimeRecord &a_objTimeRecord, bool a_bIsReq)
{
	try
	{
#ifdef PERFTESTING // will be removed afterwards
		if(!stopPolling.load())
#endif
		{
			pushPollFreqToQueue(a_stPollRef, a_objTimeRecord, a_bIsReq);
		}
	}
	catch (exception &e)
	{
		DO_LOG_FATAL(e.what());
	}
}

/**
 * Destructor: Clears all data structures
 * @param none
 * @return none
 */
CRequestInitiator::~CRequestInitiator()
{
	{
		// Clear Non-RT structures
		std::lock_guard<std::mutex> lock(m_mutextTxIDMap);
		m_mapTxIDReqData.clear();
	}
	{
		// Clear RT structures
		std::lock_guard<std::mutex> lock(m_mutextTxIDMapRT);
		m_mapTxIDReqDataRT.clear();
	}
	sem_destroy(&semaphoreReqProcess);
	sem_destroy(&semaphoreRespProcess);
	sem_destroy(&semaphoreRTReqProcess);
	sem_destroy(&semaphoreRTRespProcess);
}

/**
 * Constructor: This is a singleton class used to send requests for polling.
 * Constructor initiates the data.
 * @param: none
 * @return none
 */
CRequestInitiator::CRequestInitiator() : m_uiIsNextRequest(0)
{
	try
	{
		init();
	}
	catch(const std::exception& e)
	{
		DO_LOG_FATAL("Unable to initiate instance: Exception:");
		DO_LOG_FATAL(e.what());
	}
}

/**
 * Get point information corresponding to a transaction id for requested data
 * @param tokenId	:[in] get request for request with token
 * @param a_bIsRT	:[in] indicates whether it is a RT request
 * @return point reference
 */
CRefDataForPolling& CRequestInitiator::getTxIDReqData(unsigned short tokenId, bool a_bIsRT)
{
	if(true == a_bIsRT)
	{
		// This is RT request. Use RT-related structures
		/// Ensure that only on thread can execute at a time
		std::lock_guard<std::mutex> lock(m_mutextTxIDMapRT);

		// return the request ID
		return m_mapTxIDReqDataRT.at(tokenId);
	}

	// This is Non-RT request. Use Non-RT-related structures
	/// Ensure that only one thread can execute at a time
	std::lock_guard<std::mutex> lock(m_mutextTxIDMap);

	// return the request ID
	return m_mapTxIDReqData.at(tokenId);
}

/**
 * Check if a given txid is present in txid map
 * @param tokenId	:[in] check request for request with token
 * @param a_bIsRT	:[in] indicates whether it is a RT request
 * @return true: present, false: absent
 */
bool CRequestInitiator::isTxIDPresent(unsigned short tokenId, bool a_bIsRT)
{
	if(true == a_bIsRT)
	{
		// This is RT request. Use RT-related structures
		/// Ensure that only one thread can execute at a time
		std::lock_guard<std::mutex> lock(m_mutextTxIDMapRT);

		// check if data is present
		if(m_mapTxIDReqDataRT.end() != m_mapTxIDReqDataRT.find(tokenId))
		{
			// token id is found
			return true;
		}
	}
	else
	{
		// This is Non-RT request. Use Non-RT-related structures
		/// Ensure that only one thread can execute at a time
		std::lock_guard<std::mutex> lock(m_mutextTxIDMap);

		// check if data is present
		if(m_mapTxIDReqData.end() != m_mapTxIDReqData.find(tokenId))
		{
			// token id is found
			return true;
		}
	}
	// Token id is not found
	return false;
}

/**
 * Insert new TxID and point reference entry in map
 * @param token 		:[in] token
 * @param objRefData	:[in] reference to polling data
 * @param a_bIsRT		:[in] defines whether it is a RT request or not
 * @return none
 */
void CRequestInitiator::insertTxIDReqData(unsigned short token, CRefDataForPolling &objRefData, bool a_bIsRT)
{
	if(true  == a_bIsRT)
	{
		// This is RT request. Use RT-related structures
		/// Ensure that only on thread can execute at a time
		std::lock_guard<std::mutex> lock(m_mutextTxIDMapRT);

		// insert the data in request map
		//m_mapTxIDReqData.insert(pair <unsigned short, CRefDataForPolling> (token, objRefData));
		m_mapTxIDReqDataRT.emplace(token, objRefData);
	}
	else
	{
		// This is Non-RT request. Use Non-RT-related structures
		/// Ensure that only on thread can execute at a time
		std::lock_guard<std::mutex> lock(m_mutextTxIDMap);

		// insert the data in request map
		//m_mapTxIDReqData.insert(pair <unsigned short, CRefDataForPolling> (token, objRefData));
		m_mapTxIDReqData.emplace(token, objRefData);
	}
}

/**
 * Removes entry from the map once data is posted on ZMQ for polling.
 * @param tokenId :[in] remove entry with given token id
 * @param a_bIsRT :[in] indicates whether it is a RT request
 * @return none
 */
void CRequestInitiator::removeTxIDReqData(unsigned short tokenId, bool a_bIsRT)
{
	if(true == a_bIsRT)
	{
		// This is RT request. Use RT-related structures
		/// Ensure that only one thread can execute at a time
		std::lock_guard<std::mutex> lock(m_mutextTxIDMapRT);
		m_mapTxIDReqDataRT.erase(tokenId);
	}
	else
	{
		// This is Non-RT request. Use Non-RT-related structures
		/// Ensure that only one thread can execute at a time
		std::lock_guard<std::mutex> lock(m_mutextTxIDMap);
		m_mapTxIDReqData.erase(tokenId);
	}
}

/**
 * Constructor: This is a singleton class. Used to keep records of all
 * CTimeRecord objects and polling time of those intervals
 * @param none
 * @return none
 */
CTimeMapper::CTimeMapper()
{
}

/**
 * This singleton class initiates CRequestInitiator object for sending requests
 * @param none
 * @return none
 */
void CTimeMapper::initTimerFunction()
{
	// Init CRequestInitiator
	CRequestInitiator::instance();
}

/**
 * This function checks if for given timer-counter, is there any polling activity or
 * cutoff check needed. If yes, the function accordingly initiates a process to signal respective threads
 * If a polling has to be triggered for certain interval, the function sets next applicable trigger point
 * in terms of future counter-timer value.
 * @param a_uiMaxCounter:[in] Maximum counter used for timer tracking
 * @param a_uiCounter:[in] current counter value
 * @param a_tsPollTime:[in] current polling timestamp
 * @return none
 */
void CTimeMapper::checkTimer(const uint32_t &a_uiMaxCounter, uint32_t a_uiCounter, struct timespec& a_tsPollTime)
{
    try
	{
    	std::vector<StPollingTracker> listPollTracker;
    	if(true == getPollingTrackerList(a_uiCounter, listPollTracker))
    	{
    		std::vector<std::reference_wrapper<CTimeRecord>> listPolledTimeRecords;
    		listPolledTimeRecords.clear();
    		struct StPollingInstance stPollRef;
    		stPollRef.m_tsPollTime = a_tsPollTime;
    		// list found for counter
    		for(auto &pollInterval: listPollTracker)
    		{
    			CTimeRecord &a = pollInterval.m_objTimeRecord.get();
				stPollRef.m_uiPollInterval = a.getInterval();
				if(true == pollInterval.m_bIsPolling)
				{
					CRequestInitiator::instance().initiateMessages(stPollRef, a, true);
					listPolledTimeRecords.push_back(a);
				}
				else
				{
					CRequestInitiator::instance().initiateMessages(stPollRef, a, false);
				}
    		}

    		// set future counter-timer value for polled intervals
    		for(auto &elememt: listPolledTimeRecords)
    		{
    			CTimeRecord &a = elememt.get();
				// If it is polling interval, calculate next polling and cutoff intervals
				uint32_t uiNextPolling = (a_uiCounter + a.getInterval());
				uint32_t uiNextCutoff = (a_uiCounter + a.getCutoffInterval());
				if((a_uiCounter == a_uiMaxCounter)
						&& (a_uiMaxCounter == a.getInterval()))
				{
					uiNextPolling = a_uiMaxCounter;
				}
				if(uiNextPolling > a_uiMaxCounter)
				{
					uiNextPolling = uiNextPolling % a_uiMaxCounter;
				}
				if(uiNextCutoff > a_uiMaxCounter)
				{
					uiNextCutoff = uiNextCutoff % a_uiMaxCounter;
				}
				// set next polling interval
				addToPollingTracker(uiNextPolling, a, true);
				// set next cutoff interval
				addToPollingTracker(uiNextCutoff, a, false);
    		}

    		listPolledTimeRecords.clear();
    	}
    	else
    	{
    		/// No polling is needed for counter
    		//std::cout << "No polling for: " << a_uiCounter << std::endl;
    	}
	}
	catch (exception &e)
	{
		DO_LOG_FATAL(e.what());
	}
}

/**
 * Initializes data structure to send request for polling.
 * Adds a point in a map for request-response matching in future.
 * @param a_stRdPrdObj:	[in] request to send
 * @param m_u16TxId	:	[in] TxID
 * @param isRTRequest: 	[in] RT/Non-Rt
 * @param a_lPriority: 	[in] priority to be used for sending request
 * @param a_nRetry:		[in] request retries to be performed in case of timeout
 * @param a_ptrCallbackFunc	:[in] callback function to be called by stack to send response
 * @return 	true : on success,
 * 			false : on error
 */
bool CRequestInitiator::sendRequest(CRefDataForPolling &a_stRdPrdObj,
		uint16_t &m_u16TxId,
		bool isRTRequest,
		const long a_lPriority,
		int a_nRetry,
		void* a_ptrCallbackFunc)
{
	uint8_t u8ReturnType = APP_SUCCESS;
	bool bRet = false;

	try
	{
		MbusAPI_t& stMbusApiPram = a_stRdPrdObj.getMBusReq();

		// set the priority
		stMbusApiPram.m_lPriority = a_lPriority;

		/*Enter TX Id*/
		//stMbusApiPram.m_u16TxId = PublishJsonHandler::instance().getTxId();
		stMbusApiPram.m_u16TxId = m_u16TxId;

		// Set retries to be done in case of timeout
		stMbusApiPram.m_nRetry = a_nRetry;


#ifdef UNIT_TEST
		stMbusApiPram.m_u16TxId = 5;
#endif
		CRequestInitiator::instance().insertTxIDReqData(stMbusApiPram.m_u16TxId, a_stRdPrdObj, isRTRequest);
#ifdef MODBUS_STACK_TCPIP_ENABLED
		u8ReturnType = Modbus_Stack_API_Call(a_stRdPrdObj.getFunctionCode(), &stMbusApiPram, a_ptrCallbackFunc);
#else
		u8ReturnType = Modbus_Stack_API_Call(a_stRdPrdObj.getFunctionCode(), &stMbusApiPram, a_ptrCallbackFunc);
#endif

		if(APP_SUCCESS == u8ReturnType)
		{
			bRet = true;
		}
		else
		{
			// In case of error, immediately retry in next iteration
			//a_stRdPrdObj.bIsRespAwaited = false;
			string l_stErrorString = "Request initiation error:: "+ to_string(u8ReturnType)+" " + "Tx ID ::" + to_string(stMbusApiPram.m_u16TxId);
			DO_LOG_ERROR(l_stErrorString);
			bRet = false;
		}
	}
	catch(exception &e)
	{
		DO_LOG_FATAL(e.what());
	}

	return bRet;
}

/**
 * TimeRecord Constructor. Used to keep a list of points eligible for one polling interval
 * @param a_u32Interval	:[in] data point polling frequency
 * @param a_oPoint	:[in] reference of RefDataForPolling class
 */
CTimeRecord::CTimeRecord(uint32_t a_u32Interval, CRefDataForPolling &a_oPoint)
	: m_u32Interval(a_u32Interval), m_u32CutoffInterval(a_u32Interval),
	  m_bIsRTAvailable(false), m_bIsNonRTAvailable(false)
{
	std::cout << PublishJsonHandler::instance().getCutoffIntervalPercentage() << ": value of cutoff %\n";
	m_u32CutoffInterval.store(a_u32Interval *
			(double)(PublishJsonHandler::instance().getCutoffIntervalPercentage())/100.0);
	if(0 == m_u32CutoffInterval)
	{
		// Use 90% as default one, if cutoff value is 0
		std::cout << "CTimeRecord: Constructor: 90% cutoff is used by default\n";
		m_u32CutoffInterval.store(a_u32Interval * 0.9);
	}
	this->add(a_oPoint);
}

/**
 * Insert a point to be polled into appropriate TimeRecord object
 * @param a_uTime	:[in] polling interval
 * @param a_oPointD	:[in] point for which to add time
 * @return 	true : on success,
 * 			false : on error
 */
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
		DO_LOG_FATAL(tempw0);
		bRet = false;
	}
	return bRet;
}

/**
 * Destructor: Deinit data, i.e. TimeRecord map
 *
 */
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
		DO_LOG_FATAL(e.what());
	}
}

/**
 * Finds GCD of 2 given numbers. This is used to find smallest timer tick for polling
 * @param num1
 * @param num2
 * @return 	number : GCD of two numbers
 */
uint32_t CTimeMapper::gcd(uint32_t num1, uint32_t num2)
{
	while(0 != num1)
	{
		uint32_t temp = num1;
		num1 = num2 % num1;
		num2 = temp;
	}
	return num2;
}

/**
 * Get index of given polling interval in TimeRecord map
 * @param a_uFreq: [in]: frequency to find index
 * @return 	uint32_t : [out] returns actual index at given frequency
 */
uint32_t CTimeMapper::getFreqIndex(const uint32_t a_uFreq)
{
	int index = 0;
	for (auto itr = m_mapTimeRecord.begin(); itr != m_mapTimeRecord.end(); itr++)
	{
		index++;
		if(itr->first == a_uFreq)
		{
			// index found
			break;
		}
	}
	return index;
}

/**
 * Gets minimum time frequency that can be used based on current records
 * for timer tick for polling.
 * @param none
 * @return 	number : minimum polling frequency in milliseconds
 */
uint32_t CTimeMapper::getMinTimerFrequency()
{
	uint32_t ulMinFreq = 1;
	try
	{
		std::lock_guard<std::mutex> lock(m_mapMutex);
		if(false == m_mapTimeRecord.empty())
		{
			std::map<uint32_t, CTimeRecord>::iterator it = m_mapTimeRecord.begin();
			CTimeRecord &objTimeRecord = it->second;
			// get the first minimum frequency of interval and
			std::cout << "timerecord - interval: " << objTimeRecord.getInterval() << ", cutoff:" << objTimeRecord.getCutoffInterval() << std::endl;
			ulMinFreq = gcd(objTimeRecord.getInterval(), objTimeRecord.getCutoffInterval());
			it++;
			while (it != m_mapTimeRecord.end())
			{
				CTimeRecord &objTimeRecord = it->second;
				std::cout << "timerecord - interval: " << objTimeRecord.getInterval() << ", cutoff:" << objTimeRecord.getCutoffInterval() << std::endl;
				// First find min frequency using interval
				ulMinFreq = gcd(objTimeRecord.getInterval(), ulMinFreq);
				if(1 == ulMinFreq)
				{
					break;
				}

				// now check with cutoffinterval
				ulMinFreq = gcd(objTimeRecord.getCutoffInterval(), ulMinFreq);
				if(1 == ulMinFreq)
				{
					break;
				}
				it++;
			}
		}
	}
	catch (exception &e)
	{
		DO_LOG_FATAL(e.what());
	}
	DO_LOG_DEBUG("Minimum frequency: " + to_string(ulMinFreq));
	return ulMinFreq;
}

/**
 * Add the point under RT or Non-RT polling list depending on polling type
 * @param a_oPoint
 * @return 	true : on success,
 * 			false : on error
 */
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
			if(a_oPoint.getDataPoint().getRTFlag())
			{
				m_vPolledPointsRT.push_back(a_oPoint);
				m_bIsRTAvailable = true;
			}
			else
			{
				m_vPolledPoints.push_back(a_oPoint);
				m_bIsNonRTAvailable = true;
			}
		}
	}
	catch (exception &e)
	{
		//std::cout << "TimeRecord insert failed: " << a_sHID << " : " << e.what() << endl;
		string tempw = " Exception in CTimeRecord:";
		tempw.append(e.what());
		tempw.append(", Point: ");
		tempw.append(a_oPoint.getDataPoint().getID());
		DO_LOG_FATAL(tempw);
		bRet = false;
	}
	return bRet;
}

/**
 * Destructor; Clears lists for RT and Non-RT
 */
CTimeRecord::~CTimeRecord()
{
	try
	{
		// Clear vector of reference IDs
		std::lock_guard<std::mutex> lock(m_vectorMutex);
		m_vPolledPoints.clear();
		m_vPolledPointsRT.clear();
	}
	catch (exception &e)
	{
		DO_LOG_FATAL(e.what());
	}
}

/**
 * Prepares time tracker data for polling operation.
 * Sets first timer-counter value for given polling intervals.
 * @param none
 * @return 	Maximum polling interval
 */
uint32_t CTimeMapper::preparePollingTracker()
{
	// This function prepares a tracker list for polling and returns maximum polling interval.
	uint32_t ulMaxPollInterval = 0;
	try
	{
		std::lock_guard<std::mutex> lock(m_mapMutex);
		for(auto &it: m_mapTimeRecord)
		{
			ulMaxPollInterval = it.first;

			// set polling interval
			addToPollingTracker(ulMaxPollInterval, it.second, true);
		}
	}
	catch (exception &e)
	{
		DO_LOG_FATAL(e.what());
	}
	DO_LOG_DEBUG("Maximum poll interval: " + to_string(ulMaxPollInterval));
	return ulMaxPollInterval;
}

/**
 * Compares 2 polling intervals. Used to sort the list to ensure lower polling interval
 * occurs first.
 * @param i1: [in] argument 1: Polling interval 1
 * @param i2: [in] argument 2: Polling interval 2
 * @return 	true if 1st argument is smaller that 2nd argument
 */
bool compareInterval(StPollingTracker i1, StPollingTracker i2)
{
    return (i1.m_uiPollInterval < i2.m_uiPollInterval);
}

/**
 * Adds given reference data of polling interval to the tracker for timer-counter
 * @param a_uiCounter: Timer-counter against which data needs to be added
 * @param a_objTimeRecord: Reference of TimeRecord object corresponding to teh polling interval
 * @param a_bIsPolling: True: Polling interval, False: Cutoff Interval
 * @return 	none
 */
void CTimeMapper::addToPollingTracker(uint32_t a_uiCounter, CTimeRecord &a_objTimeRecord, bool a_bIsPolling)
{
	// This function adds given reference data to polling interval to the tracker
	try
	{
		struct StPollingTracker stTemp{a_objTimeRecord.getInterval(), a_objTimeRecord, a_bIsPolling};
		auto itr = m_mapPollingTracker.find(a_uiCounter);
		if(itr == m_mapPollingTracker.end())
		{
			std::vector<StPollingTracker> listTemp;
			listTemp.push_back(stTemp);
			m_mapPollingTracker.insert(
				std::pair<uint32_t, std::vector<StPollingTracker>> (a_uiCounter, listTemp));
		}
		else
		{
			auto &list = itr->second;
			list.push_back(stTemp);
			std::sort(list.begin(), list.end(), compareInterval);
		}
	}
	catch (exception &e)
	{
		DO_LOG_FATAL(e.what());
	}
	return ;
}

/**
 * Exracts list of polling intervals corresponding to given timer-counter
 * @param a_uiCounter: Timer-counter for which data needs to be extracted
 * @param a_listPollInterval: Out parrameter: List of polling intervals
 * @return 	true: if data is available
 * 			false: no data is available
 */
bool CTimeMapper::getPollingTrackerList(uint32_t a_uiCounter, std::vector<StPollingTracker> &a_listPollInterval)
{
	try
	{
		auto itr = m_mapPollingTracker.find(a_uiCounter);
		if(itr != m_mapPollingTracker.end())
		{
			a_listPollInterval = itr->second;
			itr->second.clear();
			//m_mapPollingTracker.erase(itr);
			return true;
		}
	}
	catch (exception &e)
	{
		DO_LOG_FATAL(e.what());
	}
	return false;
}

/**
 * Function to track timer-counter for polling operations
 * @param : [in] interval in milliseconds: Minimum timer tick value
 * @return : none
 */
void PeriodicTimer::timerThread(uint32_t interval)
{
	// set thread priority
	globalConfig::set_thread_sched_param(
			globalConfig::CGlobalConfig::getInstance().getOpPollingOpConfig().getNonRTConfig(),
			TIMER_THREAD_PRIORITY,
			TIMER_THREAD_SCHEDULER,
			true);

	globalConfig::display_thread_sched_attr("timerThread param::");

	uint32_t uiMaxCounter = CTimeMapper::instance().preparePollingTracker();
	uint32_t uiCurCounter = 0;
	std::cout << "Maximum counter = " << uiMaxCounter << std::endl;

	// checking ounter value
	if(0 == uiMaxCounter)
	{
		uiMaxCounter = 1;
		std::cout << "Maximum counter was 0. Now set as = " << uiMaxCounter << std::endl;
	}
	// interval is in milliseconds
	if(0 == interval)
	{
		interval = 1;
	}

	struct timespec ts;
	int rc = clock_getres(CLOCK_MONOTONIC, &ts);
	if(0 != rc)
	{
		std::cout << "Error: clock_getres failed: " << errno << std::endl;
		std::cout << "Continuing further\n";
	}
	else
	{
		std::cout << "Clock resolution: " << (long)ts.tv_sec << " seconds, " << (long)ts.tv_nsec << " nanoseconds \n";
	}

	uint32_t uiMsecInterval = interval;

	// for following calculation, convert interval to nanoseconds
	interval = interval*1000*1000;
	rc = clock_gettime(CLOCK_MONOTONIC, &ts);
	if(0 != rc)
	{
		std::cout << "Fatal error: polling timer: clock_gettime failed: " << errno << std::endl;
		return;
	}
	while(!g_stopTimer)
	{
		unsigned long next_tick = (ts.tv_sec * 1000000000L + ts.tv_nsec) + interval;
		ts.tv_sec = next_tick / 1000000000L;
		ts.tv_nsec = next_tick % 1000000000L;

		do
		{
			rc = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL);
		} while(EINTR == rc);

		if(0 == rc)
		{
			struct timespec tsPoll = {0};
			rc = clock_gettime(CLOCK_REALTIME, &tsPoll);
			if(0 != rc)
			{
				std::cout << "Fatal error: polling timer: clock_gettime failed in polling: " << errno << std::endl;
				//return;
			}
			uiCurCounter = uiCurCounter + uiMsecInterval;
			/// call timer function
			CTimeMapper::instance().checkTimer(uiMaxCounter, uiCurCounter, tsPoll);

			// reset the counter
			if(uiCurCounter == uiMaxCounter)
			{
				uiCurCounter = 0;
			}
		}
		else
		{
			DO_LOG_FATAL("Polling timer error:" + to_string(rc));
		}
	}
}


/**
 * Function to initiate thread for timer operation
 * @param [in] : interval in milliseconds
 * @return : nothing
 */
void PeriodicTimer::timer_start(uint32_t interval)
{
	/// create thread for timer routine
	std::thread(std::bind(timerThread, interval)).detach();
}

/**
 * Function to stop timer
 * @param : Nothing
 * @return : Nothing
 */
void PeriodicTimer::timer_stop(void)
{
	g_stopTimer = true;
}

/**
 * Copy Constructor
 * @param a_objPt 		:[in] reference CRefDataForPolling object for copy constructor
 */
CRefDataForPolling::CRefDataForPolling(const CRefDataForPolling &a_refPolling) :
		m_objDataPoint{a_refPolling.m_objDataPoint}, m_uiFuncCode{a_refPolling.m_uiFuncCode}
		, m_bIsRespPosted{false}, m_bIsLastRespAvailable{false}
		, m_stPollTsForReq{a_refPolling.m_stPollTsForReq}, m_stMBusReq{a_refPolling.m_stMBusReq}
		, m_stRetryTs{0}, m_iReqRetriedCnt{0}
{
	m_oLastGoodResponse.m_sValue = "";
	m_oLastGoodResponse.m_sLastUsec = "";
}

/**
 * Constructor: It constructs data which is used for polling. This is one time activity.
 * @param CUniqueDataPoint	:[in] reference CUniqueDataPoint object
 * @param uint8_t			:[in] function code for this point
 */
CRefDataForPolling::CRefDataForPolling(const CUniqueDataPoint &a_objDataPoint, uint8_t a_uiFuncCode) :
				m_objDataPoint{a_objDataPoint}, m_uiFuncCode{a_uiFuncCode}
				, m_bIsRespPosted{false}, m_bIsLastRespAvailable{false}, m_stPollTsForReq{0}, m_stMBusReq{0}
				, m_stRetryTs{0}, m_iReqRetriedCnt{0}
{
	m_oLastGoodResponse.m_sValue = "";
	m_oLastGoodResponse.m_sLastUsec = "";

	// Pre-Build parameters of request structure for later use
	m_stMBusReq.m_u16StartAddr = m_objDataPoint.getDataPoint().getAddress().m_iAddress;
	m_stMBusReq.m_u16Quantity = m_objDataPoint.getDataPoint().getAddress().m_iWidth;
	m_stMBusReq.m_u16ByteCount = m_objDataPoint.getDataPoint().getAddress().m_iWidth;

	// Coil and discrete input are single bytes. All others are 2 byte registers
	if((network_info::eEndPointType::eCoil != m_objDataPoint.getDataPoint().getAddress().m_eType) &&
	(network_info::eEndPointType::eDiscrete_Input != m_objDataPoint.getDataPoint().getAddress().m_eType))
	{
		// as default value for register is 2 bytes
		m_stMBusReq.m_u16ByteCount = m_stMBusReq.m_u16Quantity * 2;
	}

#ifdef MODBUS_STACK_TCPIP_ENABLED
	// fill the unit ID
	m_stMBusReq.m_u8DevId = m_objDataPoint.getWellSiteDev().getAddressInfo().m_stTCP.m_uiUnitID;
	std::string sIPAddr{m_objDataPoint.getWellSiteDev().getAddressInfo().m_stTCP.m_sIPAddress};

	CommonUtils::ConvertIPStringToCharArray(sIPAddr,m_stMBusReq.m_u8IpAddr);

	/// fill tcp port
	m_stMBusReq.m_u16Port = m_objDataPoint.getWellSiteDev().getAddressInfo().m_stTCP.m_ui16PortNumber;

#else
	m_stMBusReq.m_u8DevId = m_objDataPoint.getWellSiteDev().getAddressInfo().m_stRTU.m_uiSlaveId;
#endif

}

/**
 * Saves last known good polling response data for given point
 * @param a_sValue	:[in] data value
 * @param a_sUsec	:[in] associated timestamp
 * @return 	true : on success,
 * 			false : on error
 */
bool CRefDataForPolling::saveGoodResponse(const std::string& a_sValue, const std::string& a_sUsec)
{
	std::lock_guard<std::mutex> lock(m_mutexLastResp);
	m_oLastGoodResponse.m_sValue = a_sValue;
	m_oLastGoodResponse.m_sLastUsec = a_sUsec;
	m_bIsLastRespAvailable.store(true);
	return true;
}

/**
 * Get last saved good response (if any) for given polling point
 * @param nothing
 * @return 	stLastGoodResponse : Last known good response
 */
stLastGoodResponse CRefDataForPolling::getLastGoodResponse()
{
	std::lock_guard<std::mutex> lock(m_mutexLastResp);
	stLastGoodResponse objStackResp = m_oLastGoodResponse;
	return objStackResp;
}

/**
 * Set data structures for new polling request for this point
 * @param a_uTxID	:[in] TxID of new polling request for this point
 * @param a_tsPoll	:[in] Timestamp of new polling request for this point
 * @return 	nothing
 */
void CRefDataForPolling::setDataForNewReq(uint16_t a_uTxID, struct timespec& a_tsPoll)
{
	// Set following for new request being sent

	m_stPollTsForReq = a_tsPoll;
	m_uReqTxID.store(a_uTxID);
	m_bIsRespPosted.store(false);
	m_objDataPoint.setIsAwaitResp(true);
	m_stRetryTs = {0};
	m_iReqRetriedCnt = 0;
}
