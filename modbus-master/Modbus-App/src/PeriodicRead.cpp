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
#include "utils/YamlUtil.hpp"
#include <sstream>
#include <ctime>
#include <chrono>
#include <functional>
#include <sys/timerfd.h>
#include <unistd.h>

/// flag to check thread stop condition
std::atomic<bool> g_stopThread;

/// flag to check timer stop condition
std::atomic<bool> g_stopTimer(false);

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


/// variable to store timer instance
timer_t gTimerid;

/**
 * Get time based parameters
 * @param a_objReqData	:[in] request data
 * @param a_sTimeStamp	:[out] time stamp
 * @param a_sUsec		:[out] usec
 * @param a_sTxID		:[out] transaction id
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
 * get time in nano-seconds
 * @param ts	:[in] time to convert to nano-seconds
 * @return	time in nano-seconds
 */
static unsigned long get_nanos(struct timespec ts) {
    return (unsigned long)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

/**
 * Prepare response json
 * @param a_pMsg		:[in] pointer to message envelope to fill up
 * @param a_objReqData	:[in] request data
 * @param a_stResp		:[in] response data
 * @return 	true : on success,
 * 			false : on error
 */
BOOLEAN CPeriodicReponseProcessor::prepareResponseJson(msg_envelope_t** a_pMsg, const CRefDataForPolling* a_objReqData, stStackResponse a_stResp)
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
		stOnDemandRequest onDemandReqData;
		std::string sTimestamp, sUsec, sTxID;
		msg_envelope_elem_body_t* ptTopic = NULL;
		msg_envelope_elem_body_t* ptWellhead = NULL;
		msg_envelope_elem_body_t* ptMetric = NULL;
		msg_envelope_elem_body_t* ptRealTime = NULL;
		msg = msgbus_msg_envelope_new(CT_JSON);
		
		if(MBUS_CALLBACK_POLLING == a_stResp.m_operationType || MBUS_CALLBACK_POLLING_RT == a_stResp.m_operationType)
		{
			getTimeBasedParams(*a_objReqData, sTimestamp, sUsec, sTxID);

			msg_envelope_elem_body_t* ptDriverSeq = msgbus_msg_envelope_new_string(sTxID.c_str());
			string sTopic = a_objReqData->getDataPoint().getID() + SEPARATOR_CHAR + PERIODIC_GENERIC_TOPIC;
			ptTopic = msgbus_msg_envelope_new_string(sTopic.c_str());
			ptWellhead = msgbus_msg_envelope_new_string(a_objReqData->getDataPoint().getWellSite().getID().c_str());
			ptMetric = msgbus_msg_envelope_new_string(a_objReqData->getDataPoint().getDataPoint().getID().c_str());
			ptRealTime =  msgbus_msg_envelope_new_string(std::to_string(a_objReqData->getDataPoint().getDataPoint().getPollingConfig().m_bIsRealTime).c_str());

			msgbus_msg_envelope_put(msg, "driver_seq", ptDriverSeq);

			bIsByteSwap = a_objReqData->getDataPoint().getDataPoint().getAddress().m_bIsByteSwap;
			bIsWordSwap = a_objReqData->getDataPoint().getDataPoint().getAddress().m_bIsWordSwap;
		}
		else
		{
			bool bRetVal = common_Handler::getOnDemandReqData(a_stResp.u16TransacID, onDemandReqData);
			if(false == bRetVal)
			{
				CLogger::getInstance().log(FATAL, LOGDETAILS("Could not get data in map for on-demand request"));
				return FALSE;
			}
			common_Handler::getTimeParams(sTimestamp, sUsec);

			/// application sequence
			msg_envelope_elem_body_t* ptAppSeq = msgbus_msg_envelope_new_string(onDemandReqData.m_strAppSeq.c_str());
			/// topic
			ptTopic = msgbus_msg_envelope_new_string(onDemandReqData.m_strTopic.append("Response").c_str());
			/// wellhead
			ptWellhead = msgbus_msg_envelope_new_string(onDemandReqData.m_strWellhead.c_str());
			/// metric
			ptMetric = msgbus_msg_envelope_new_string(onDemandReqData.m_strMetric.c_str());
			/// RealTime
			ptRealTime =  msgbus_msg_envelope_new_string(to_string(onDemandReqData.m_isRT).c_str());
			// add timestamps for req recvd by app
			msg_envelope_elem_body_t* ptAppTSReqRcvd = msgbus_msg_envelope_new_string( (to_string(get_nanos(onDemandReqData.m_obtReqRcvdTS))).c_str() );
			// message received from MQTT Time
			msg_envelope_elem_body_t* ptMqttTime = msgbus_msg_envelope_new_string(onDemandReqData.m_strMqttTime.c_str());
			// message received from MQTT Time
			msg_envelope_elem_body_t* ptEisTime = msgbus_msg_envelope_new_string(onDemandReqData.m_strEisTime.c_str());

			msgbus_msg_envelope_put(msg, "reqRcvdByApp", ptAppTSReqRcvd);
			msgbus_msg_envelope_put(msg, "app_seq", ptAppSeq);
			msgbus_msg_envelope_put(msg, "tsMsgRcvdFromMQTT", ptMqttTime);
			msgbus_msg_envelope_put(msg, "tsMsgPublishOnEIS", ptEisTime);

			bIsByteSwap = onDemandReqData.m_isByteSwap;
			bIsWordSwap = onDemandReqData.m_isWordSwap;
		}

		msg_envelope_elem_body_t* ptVersion = msgbus_msg_envelope_new_string("2.0");
		msg_envelope_elem_body_t* ptTimeStamp = msgbus_msg_envelope_new_string(sTimestamp.c_str());
		msg_envelope_elem_body_t* ptUsec = msgbus_msg_envelope_new_string(sUsec.c_str());

		// add timestamps from stack
		msg_envelope_elem_body_t* ptStackTSReqRcvd = msgbus_msg_envelope_new_string( (to_string(get_nanos(a_stResp.m_objStackTimestamps.tsReqRcvd))).c_str() );
		msg_envelope_elem_body_t* ptStackTSReqSent = msgbus_msg_envelope_new_string( (to_string(get_nanos(a_stResp.m_objStackTimestamps.tsReqSent))).c_str() );
		msg_envelope_elem_body_t* ptStackTSRespRcvd = msgbus_msg_envelope_new_string( (to_string(get_nanos(a_stResp.m_objStackTimestamps.tsRespRcvd))).c_str() );
		msg_envelope_elem_body_t* ptStackTSRespPosted = msgbus_msg_envelope_new_string( (to_string(get_nanos(a_stResp.m_objStackTimestamps.tsRespSent))).c_str() );
		//msg_envelope_elem_body_t* ptPriority =  msgbus_msg_envelope_new_string(to_string(a_stResp.m_lPriority).c_str());

		msgbus_msg_envelope_put(msg, "version", ptVersion);
		msgbus_msg_envelope_put(msg, "timestamp", ptTimeStamp);
		msgbus_msg_envelope_put(msg, "usec", ptUsec);
		msgbus_msg_envelope_put(msg, "topic", ptTopic);
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
						// save last known response
						(const_cast<CRefDataForPolling*>(a_objReqData))->saveGoodResponse(sVal, sUsec);
					}

					msg_envelope_elem_body_t* ptValue = msgbus_msg_envelope_new_string(sVal.c_str());
					msgbus_msg_envelope_put(msg, "value", ptValue);
					msg_envelope_elem_body_t* ptStatus = msgbus_msg_envelope_new_string("Good");
					msgbus_msg_envelope_put(msg, "status", ptStatus);
				}
				else
				{
					msg_envelope_elem_body_t* ptStatus = msgbus_msg_envelope_new_string("Bad");
					msg_envelope_elem_body_t* ptErrorDetails = msgbus_msg_envelope_new_string(((to_string(a_stResp.m_stException.m_u8ExcCode)) + ", " +  (to_string(a_stResp.m_stException.m_u8ExcCode))).c_str());
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
				msg_envelope_elem_body_t* ptErrorDetails = msgbus_msg_envelope_new_string(((to_string(a_stResp.m_stException.m_u8ExcCode)) + ", " +  (to_string(a_stResp.m_stException.m_u8ExcStatus))).c_str());
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
				msg_envelope_elem_body_t* ptErrorDetails = msgbus_msg_envelope_new_string(((to_string(a_stResp.m_stException.m_u8ExcCode)) + ", " +  (to_string(a_stResp.m_stException.m_u8ExcStatus))).c_str());
				msgbus_msg_envelope_put(msg, "error_code", ptErrorDetails);
			}
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

/**
 * Post response json
 * @param a_stResp		:[in] response data
 * @param a_objReqData	:[in] request data
 * @return 	true : on success,
 * 			false : on error
 */
BOOLEAN CPeriodicReponseProcessor::postResponseJSON(stStackResponse& a_stResp, const CRefDataForPolling* a_objReqData)
{
	if((MBUS_CALLBACK_POLLING == a_stResp.m_operationType || MBUS_CALLBACK_POLLING_RT == a_stResp.m_operationType)
			&& NULL == a_objReqData)
	{
		return FALSE;
	}
	msg_envelope_t* g_msg = NULL;

	try
	{
		if(FALSE == prepareResponseJson(&g_msg, a_objReqData, a_stResp))
		{
			CLogger::getInstance().log(INFO, LOGDETAILS( " Error in preparing response"));
			return FALSE;
		}
		else
		{
			if(MBUS_CALLBACK_POLLING == a_stResp.m_operationType || MBUS_CALLBACK_POLLING_RT == a_stResp.m_operationType)
			{
				if(true == PublishJsonHandler::instance().publishJson(g_msg, a_objReqData->getBusContext().m_pContext,
						a_objReqData->getPubContext().m_pContext,
						a_stResp.m_strResponseTopic))
				{
					CLogger::getInstance().log(DEBUG, LOGDETAILS("Msg published successfully"));
				}
			}
			else
			{
				zmq_handler::stZmqContext msgbus_ctx = zmq_handler::getCTX(a_stResp.m_strResponseTopic);
				zmq_handler::stZmqPubContext pubCtx = zmq_handler::getPubCTX(a_stResp.m_strResponseTopic);

				PublishJsonHandler::instance().publishJson(g_msg, msgbus_ctx.m_pContext, pubCtx.m_pContext, a_stResp.m_strResponseTopic);
			}

#ifdef INSTRUMENTATION_LOG
			msg_envelope_serialized_part_t* parts = NULL;
			int num_parts = msgbus_msg_envelope_serialize(g_msg, &parts);
			if(num_parts > 0)
			{
				if(NULL != parts[0].bytes)
				{
					std::string s(parts[0].bytes);

					CLogger::getInstance().log(DEBUG,
							LOGDETAILS("TxID:" + to_string(a_stResp.u16TransacID)
									+ ", Msg: " + s));
				}
				msgbus_msg_envelope_serialize_destroy(parts, num_parts);
			}
#endif
		}
	}
	catch(const std::exception& e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}

	if(NULL != g_msg)
	{
		msgbus_msg_envelope_destroy(g_msg);
	}

	// return true on success
	return TRUE;
}

/**
 * Post dummy bad response
 * @param a_objReqData	:[in] request for which to send dummy response
 * @param m_stException	:[in] exception
 * @return 	true : on success,
 * 			false : on error
 */
BOOLEAN CPeriodicReponseProcessor::postDummyBADResponse(CRefDataForPolling& a_objReqData,
		const stException_t m_stException)
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
		stResp.m_lPriority = a_objReqData.getDataPoint().getDataPoint().getPollingConfig().m_uiPollFreq;

		// Post it
		postResponseJSON(stResp, &a_objReqData);

		// Response is posted. Mark the flag
		a_objReqData.setResponsePosted(true);
	}
	catch(const std::exception& e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}

	// return true on success
	return TRUE;
}

/**
 * Post response json
 * @param a_stResp	:[in] response data
 * @return 	true : on success,
 * 			false : on error
 */
BOOLEAN CPeriodicReponseProcessor::postResponseJSON(stStackResponse& a_stResp)
{
	try
	{
		common_Handler::removeReqData(a_stResp.u16TransacID);	/// remove retry structure from map
		if(MBUS_CALLBACK_POLLING == a_stResp.m_operationType || MBUS_CALLBACK_POLLING_RT == a_stResp.m_operationType)
		{
			CRefDataForPolling& objReqData = CRequestInitiator::instance().getTxIDReqData(a_stResp.u16TransacID);
			// Node is found in transaction map. Remove it now.
			CRequestInitiator::instance().removeTxIDReqData(a_stResp.u16TransacID);
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
		CLogger::getInstance().log(FATAL, LOGDETAILS(to_string(a_stResp.u16TransacID) + e.what()));
	}

	// return true on success
	return TRUE;
}

/**
 * Initialize semaphore
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
 * Response process threads
 * @return appropriate error code
 */
eMbusStackErrorCode CPeriodicReponseProcessor::respProcessThreads(eMbusCallbackType operationCallbackType,
		sem_t& a_refSem,
		globalConfig::COperation& a_refOps)
{
	eMbusStackErrorCode eRetType = MBUS_STACK_NO_ERROR;
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
				//CRequestInitiator::instance().removeTxIDReqData(res.u16TransacID);
				res.m_Value.clear();
			}
			catch(const std::exception& e)
			{
#ifdef PERFTESTING
				other_status++;
#endif
				CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
				//return FALSE;
			}
		}while(0);
	}

	return eRetType;
}

/**
 * Push response to queue
 * @param stStackResNode :[in] response node
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
			CLogger::getInstance().log(FATAL, "Invalid callback called.");
			break;
		}

		return true;
	}
	catch(const std::exception& e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}

	return false;
}

BOOLEAN CPeriodicReponseProcessor::checkForRetry(struct stStackResponse &a_stStackResNode, eMbusCallbackType operationCallbackType)
{
	bool retValue = false;
	try
	{
		MbusAPI_t reqData;
		eMbusStackErrorCode eFunRetType = MBUS_STACK_NO_ERROR;
		if(a_stStackResNode.m_stException.m_u8ExcCode == STACK_ERROR_RECV_TIMEOUT &&
				a_stStackResNode.m_stException.m_u8ExcStatus == 2)
		{
			common_Handler::getReqData(a_stStackResNode.u16TransacID, reqData);
			if(reqData.m_nRetry > 0)
			{
				CLogger::getInstance().log(INFO, LOGDETAILS("Retry called for transaction id:: "+to_string(reqData.m_u16TxId)));
				/// decrement retry value by 1
				reqData.m_nRetry--;
				/// updating structure in map with decremented retry value
				common_Handler::updateReqData(a_stStackResNode.u16TransacID, reqData);

				void* ptrAppCallback = NULL;
				getCallbackForRetry(&ptrAppCallback, operationCallbackType);

				eFunRetType = (eMbusStackErrorCode) Modbus_Stack_API_Call(
						a_stStackResNode.m_u8FunCode,
						&reqData,
						ptrAppCallback);
				if(MBUS_STACK_NO_ERROR != eFunRetType)
				{
					CLogger::getInstance().log(INFO, LOGDETAILS("Retry stack call failed. error::  "+ to_string(eFunRetType)));
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
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}

	return retValue;
}

/**
 * Get data to process
 * @param a_stStackResNode :[in] response node
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
				CLogger::getInstance().log(DEBUG, LOGDETAILS("Response queue is empty"));
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
				CLogger::getInstance().log(DEBUG, LOGDETAILS("Response queue is empty"));
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
				CLogger::getInstance().log(DEBUG, LOGDETAILS("Response queue is empty"));
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
				CLogger::getInstance().log(DEBUG, LOGDETAILS("Response queue is empty"));
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
				CLogger::getInstance().log(DEBUG, LOGDETAILS("Response queue is empty"));
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
				CLogger::getInstance().log(DEBUG, LOGDETAILS("Response queue is empty"));
				return false;
			}
			a_stStackResNode = stackOnDemandRTWriteResQ.front();
			stackOnDemandRTWriteResQ.pop();
		}
		break;
		default:
			CLogger::getInstance().log(FATAL, LOGDETAILS("Invaid callback to get data from queue."));
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
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}

	return retValue;
}

/**
 * Handle response
 * @param pstMbusAppCallbackParams :[in] response received from stack
 * @param respType :[in] To identify polling or on-demand response
 */
void CPeriodicReponseProcessor::handleResponse(stMbusAppCallbackParams_t *pstMbusAppCallbackParams,
												//eMbusResponseType respType,
												eMbusCallbackType operationCallbackType,
												string strResponseTopic)
{
	if(pstMbusAppCallbackParams == NULL)
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS("Response received from stack is null hence discarding"));
		return;
	}

	struct stStackResponse stStackResNode;
	stStackResNode.bIsValPresent = false;
	stStackResNode.u8Reason = 0;
	stStackResNode.m_operationType = operationCallbackType;
	stStackResNode.m_u8FunCode = pstMbusAppCallbackParams->m_u8FunctionCode;
	stStackResNode.m_strResponseTopic = strResponseTopic;

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
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}
}

/**
 * function to receive RP-A call back
 * @param pstMbusAppCallbackParams :[in] parameters received from stack
 * @return appropriate error code
 */
eMbusStackErrorCode readPeriodicCallBack(stMbusAppCallbackParams_t *pstMbusAppCallbackParams, uint16_t uTxID)
{
	if(pstMbusAppCallbackParams == NULL)
	{
		CLogger::getInstance().log(DEBUG, LOGDETAILS("Response received from stack is null"));
		return MBUS_STACK_ERROR_RECV_FAILED;
	}

	//handle response
	CPeriodicReponseProcessor::Instance().handleResponse(pstMbusAppCallbackParams,
														MBUS_CALLBACK_POLLING,
														PublishJsonHandler::instance().getPolledDataTopic());
	return MBUS_STACK_NO_ERROR;
}

/**
 * function to receive RP-A call back for RT requests
 * @param pstMbusAppCallbackParams :[in] parameters received from stack
 * @return appropriate error code
 */
eMbusStackErrorCode readPeriodicRTCallBack(stMbusAppCallbackParams_t *pstMbusAppCallbackParams, uint16_t uTxID)
{
	if(pstMbusAppCallbackParams == NULL)
	{
		CLogger::getInstance().log(DEBUG, LOGDETAILS("Response received from stack is null"));
		return MBUS_STACK_ERROR_RECV_FAILED;
	}

	//handle response
	CPeriodicReponseProcessor::Instance().handleResponse(pstMbusAppCallbackParams,
														MBUS_CALLBACK_POLLING_RT,
														PublishJsonHandler::instance().getPolledDataTopicRT());
	return MBUS_STACK_NO_ERROR;
}

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
 * Constructor
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
		CLogger::getInstance().log(FATAL, LOGDETAILS("Unable to initiate instance"));
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		std::cout << "\nException CPeriodicReponseProcessor ::" << __func__ << ": Unable to initiate instance: " << e.what();
	}
}

/**
 * Return single instance of this class
 * @return
 */
CPeriodicReponseProcessor& CPeriodicReponseProcessor::Instance()
{
	static CPeriodicReponseProcessor _self;
	return _self;
}

/**
 * Initialize response handler threads
 */
void CPeriodicReponseProcessor::initRespHandlerThreads()
{
	static bool bSpawned = false;
	try
	{
		if(false == bSpawned)
		{
			// Spawn 5 thread to process responses
			//for (int i = 0; i < 5; i++)
			{
				//std::thread{std::bind(&CPeriodicReponseProcessor::respProcessThreads, std::ref(*this))}.detach();
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
		CLogger::getInstance().log(FATAL, LOGDETAILS("Unable to initiate instance"));
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		std::cout << "\nException CPeriodicReponseProcessor ::" << __func__ << ": Unable to initiate instance: " << e.what();
	}
}

/**
 * Initiate request initiator
 * @return 	true : on success,
 * 			false : on error
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
 * Push polling frequency to queue
 * @param a_uiRef	:[in] frequency
 * @param a_objTimeRecord :[in] reference of TimeRecord class
 * @param a_bIsReq: [in] bool variable to differentiate between request and response
 * @return 	true : on success,
 * 			false : on error
 */
bool CRequestInitiator::pushPollFreqToQueue(uint32_t &a_uiRef, CTimeRecord &a_objTimeRecord, bool a_bIsReq)
{
	try
	{
		if (true == a_bIsReq)
		{
			std::lock_guard<std::mutex> lock(m_mutexReqFreqQ);

			// Check for availability of RT list
			if(true == a_objTimeRecord.isRTListAvailable())
			{
				m_qReqFreqRT.push(a_uiRef);
				// Signal response process thread
				sem_post(&semaphoreRTReqProcess);
			}

			// Check for availability of non-RT list
			if(true == a_objTimeRecord.isNonRTListAvailable())
			{
				m_qReqFreq.push(a_uiRef);
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
				m_qRespFreqRT.push(a_uiRef);
				// Signal response process thread
				sem_post(&semaphoreRTRespProcess);
			}

			// Check for availability of non-RT list
			if(true == a_objTimeRecord.isNonRTListAvailable())
			{
				m_qRespFreq.push(a_uiRef);
				// Signal response process thread
				sem_post(&semaphoreRespProcess);
			}
		}

		return true;
	}
	catch(const std::exception& e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}

	return false;
}

/**
 * Get polling frequency
 * @param a_uiRef	:[in] reference to store polling frequency
 * @param a_bIsRT	:[in] bool variable to differentiate between RT/Non-RT
 * @param a_bIsReq	:[in] bool variable to differentiate between request and response
 * @return 	true : on success,
 * 			false : on error
 */
bool CRequestInitiator::getFreqRefForPollCycle(uint32_t &a_uiRef, bool a_bIsRT, bool a_bIsReq)
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
				a_uiRef = m_qReqFreqRT.front();
				m_qReqFreqRT.pop();
			}
			else
			{
				a_uiRef = m_qReqFreq.front();
				m_qReqFreq.pop();
			}
		}
		else
		{
			// Refer response related data
			std::lock_guard<std::mutex> lock(m_mutexRespFreqQ);
			if(true == a_bIsRT)
			{
				a_uiRef = m_qRespFreqRT.front();
				m_qRespFreqRT.pop();
			}
			else
			{
				a_uiRef = m_qRespFreq.front();
				m_qRespFreq.pop();
			}
		}

		return true;
	}
	catch(const std::exception& e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}

	return false;
}

/**
 * Initiate request to get processed
 * @param a_vReqData	:[in] reference to store polling frequency
 * @param isRTRequest	:[in] boolean variable to distinguish between RT/Non-RT requests
 */
void CRequestInitiator::initiateRequest(std::vector<CRefDataForPolling>& a_vReqData, bool isRTRequest)
{
	for(auto &objReqData: a_vReqData)
	{
		//CRefDataForPolling& objReqData = a_oReqData;
		// Check if a response is already awaited
		if(true == objReqData.getDataPoint().isIsAwaitResp())
		{
			// waiting for response. Send BAD response
			stException_t m_stException = {};
			m_stException.m_u8ExcCode = 100;
			m_stException.m_u8ExcStatus = 100;
			uint16_t lastTxID = objReqData.getReqTxID();
			CLogger::getInstance().log(INFO,
					LOGDETAILS("Post dummy response as response not received for - Point: " + objReqData.getDataPoint().getID()
						+ ", LastTxID: " + to_string(lastTxID)));
			CPeriodicReponseProcessor::Instance().postDummyBADResponse(objReqData, m_stException);

			if(false == CRequestInitiator::instance().isTxIDPresent(lastTxID))
			{
				CLogger::getInstance().log(INFO,
					LOGDETAILS("TxID is not present in map.Resetting the response status"));
				objReqData.getDataPoint().setIsAwaitResp(false);
			}
			continue;
		}

		//if(true == bIsFound)
		{
			// Response is awaited. Mark the flag
			objReqData.getDataPoint().setIsAwaitResp(true);
			// Response is not posted. Mark the flag
			objReqData.setResponsePosted(false);

			// generate the TX ID
			uint16_t m_u16TxId = PublishJsonHandler::instance().getTxId();

			// set txid
			objReqData.setReqTxID(m_u16TxId);

			CLogger::getInstance().log(INFO,
				LOGDETAILS("Trying to send request for - Point: " + objReqData.getDataPoint().getID()
					+ ", with TxID: " + to_string(m_u16TxId)));
			// The entry is found in map
			// Send a request
			if (true == sendRequest(objReqData, m_u16TxId, isRTRequest))
			{
				// Request is sent successfully
				// No action
#ifdef PERFTESTING // will be removed afterwards
				++reqCount;
				if(reqCount == total_read_periodic.load())
				{
					reqCount = 0;
					CLogger::getInstance().log(ERROR, LOGDETAILS("Polling is stopped ...."));
					stopPolling.store(true);
					break;
				}
#endif
			}
			else
			{
				objReqData.getDataPoint().setIsAwaitResp(false);
				stException_t m_stException = {};
				m_stException.m_u8ExcCode = 101;
				m_stException.m_u8ExcStatus = 101;
				CPeriodicReponseProcessor::Instance().postDummyBADResponse(objReqData, m_stException);

				/// remove node from TxID map
				CRequestInitiator::instance().removeTxIDReqData(m_u16TxId);
				// reset txid
				objReqData.setReqTxID(0);
				CLogger::getInstance().log(ERROR, LOGDETAILS("sendRequest failed"));
			}
		}
	}
}

/**
 * Thread function to initiate requests
 * @param isRTPoint	:[in] bool variable to differentiate between RT/Non-RT
 */
void CRequestInitiator::threadReqInit(bool isRTPoint,
		const globalConfig::COperation& a_refOps)
{
	try
	{
		// set the thread priority
		globalConfig::set_thread_sched_param(a_refOps);

		globalConfig::display_thread_sched_attr("threadReqInit param::");

		sem_t *pSem = NULL;
		if(true == isRTPoint)
		{
			pSem = &semaphoreRTReqProcess;
		}
		else
		{
			pSem = &semaphoreReqProcess;
		}
		if(NULL == pSem)
		{
			// Error condition
			CLogger::getInstance().log(FATAL, LOGDETAILS("Semaphore is null"));
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
				uint32_t uiRef;
				if(false == getFreqRefForPollCycle(uiRef, isRTPoint, true))
				{
					break;
				}
				std::vector<CRefDataForPolling>& vReqData = CTimeMapper::instance().getPolledPointList(uiRef, isRTPoint);
				initiateRequest(vReqData, isRTPoint);
			} while(0);
		}
	}
	catch (exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}
}

/**
 * Thread function to send pending responses
 * @param isRTPoint	:[in] bool variable to differentiate between RT/Non-RT
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
			CLogger::getInstance().log(FATAL, LOGDETAILS("Semaphore is null"));
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
				uint32_t uiRef;
				if(false == getFreqRefForPollCycle(uiRef, isRTPoint, false))
				{
					break;
				}
				std::vector<CRefDataForPolling>& vReqData =
						CTimeMapper::instance().getPolledPointList(uiRef, isRTPoint);

				// Check if responses are sent
				for(auto &objPolledPoint : vReqData)
				{
					if(true == objPolledPoint.isResponsePosted())
					{
						CLogger::getInstance().log(DEBUG, LOGDETAILS(objPolledPoint.getDataPoint().getID()
											+ ": Response posted. No action in cutoff"));
					}
					else
					{
						// waiting for response. Send BAD response
						stException_t m_stException = {0};
						m_stException.m_u8ExcCode = 102;
						m_stException.m_u8ExcStatus = 102;
						if(true == objPolledPoint.isLastRespAvailable())
						{
							// Last response is available. Use it.
							CLogger::getInstance().log(DEBUG, LOGDETAILS(objPolledPoint.getDataPoint().getID()
																+ ": Using last response"));

							m_stException.m_u8ExcCode = 103;
							m_stException.m_u8ExcStatus = 103;
						}
						else
						{
							// Last response is not available. Send dummy response
							CLogger::getInstance().log(DEBUG, LOGDETAILS(objPolledPoint.getDataPoint().getID()
									+ ": Response NOT posted. Sending dummy response"));
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
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}
}

/**
 * Initiate messages
 * @param a_uiRef	:[in] polling frequency
 * @param a_objTimeRecord	:[in] reference of TimeRecord
 * @param a_bIsReq	:[in] bool variable to differentiate between request and response
 */
void CRequestInitiator::initiateMessages(uint32_t a_uiRef, CTimeRecord &a_objTimeRecord, bool a_bIsReq)
{
	try
	{
#ifdef PERFTESTING // will be removed afterwards
		if(!stopPolling.load())
#endif
		{
			pushPollFreqToQueue(a_uiRef, a_objTimeRecord, a_bIsReq);
		}
	}
	catch (exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}
}

/**
 * Destructor
 */
CRequestInitiator::~CRequestInitiator()
{
}

/**
 * Constructor
 */
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
	}
}

/**
 * Get point information corresponding to a transaction id for requested data
 * @param tokenId	:[in] get request for request with token
 * @return point reference
 */
CRefDataForPolling& CRequestInitiator::getTxIDReqData(unsigned short tokenId)
{
	/// Ensure that only on thread can execute at a time
	std::lock_guard<std::mutex> lock(m_mutextTxIDMap);

	// return the request ID
	return m_mapTxIDReqData.at(tokenId);
}

/**
 * Check if a give txid is presnet in txid map
 * @param tokenId	:[in] check request for request with token
 * @return true: present, false: absent
 */
bool CRequestInitiator::isTxIDPresent(unsigned short tokenId)
{
	/// Ensure that only on thread can execute at a time
	std::lock_guard<std::mutex> lock(m_mutextTxIDMap);

	// return the request ID
	if(m_mapTxIDReqData.end() == m_mapTxIDReqData.find(tokenId))
	{
		// token id is not found
		return false;
	}
	return true;
}

/**
 * insert new entry in map
 * @param token 		:[in] token
 * @param objRefData	:[in] reference to polling data
 */
void CRequestInitiator::insertTxIDReqData(unsigned short token, CRefDataForPolling &objRefData)
{
	/// Ensure that only on thread can execute at a time
	std::lock_guard<std::mutex> lock(m_mutextTxIDMap);

	// insert the data in request map
	//m_mapTxIDReqData.insert(pair <unsigned short, CRefDataForPolling> (token, objRefData));
	m_mapTxIDReqData.emplace(token, objRefData);
}

/**
 * Remove entry from the map once reply is sent
 * @param tokenId :[in] remove entry with given token id
 */
void CRequestInitiator::removeTxIDReqData(unsigned short tokenId)
{
	/// Ensure that only on thread can execute at a time
	std::lock_guard<std::mutex> lock(m_mutextTxIDMap);
	m_mapTxIDReqData.erase(tokenId);
}

/**
 * Constructor
 */
CTimeMapper::CTimeMapper()
{
}

/**
 * Initiate timer function
 */
void CTimeMapper::initTimerFunction()
{
	//m_thread =
	//std::thread(&CTimeMapper::ioPeriodicReadTimer, this, 5).detach();
	// Init CRequestInitiator
	CRequestInitiator::instance();
	//std::cout << "\nCreated periodic read timer thread\n";
}

/**
 * Check timer
 */
void CTimeMapper::checkTimer(uint32_t a_uiInterval)
{
    try
	{
    	std::lock_guard<std::mutex> lock(m_mapMutex);
		for (auto &element : m_mapTimeRecord)
		{
			CTimeRecord &a = element.second;
			// First decrement timer counters
			a.decrementTimerCounters(a_uiInterval);
			// Now check cutoff timer interval
			if(true == a.isCutoffNow())
			{
				/*const auto p1 = std::chrono::system_clock::now();

				unsigned long long int u64{
						(unsigned long long int)(std::chrono::duration_cast<std::chrono::microseconds>(p1.time_since_epoch()).count())
					};

				std::cout << u64
						<< ": cutoff is hit for:" << a.getInterval() << std::endl;
				*/

				//std::cout << "---------------------------------\n";
				CRequestInitiator::instance().initiateMessages(element.first, a, false);
			}

			// Check interval timer counters
			if(true == a.isPollingNow())
			{
				/*const auto p1 = std::chrono::system_clock::now();

				unsigned long long int u64{
						(unsigned long long int)(std::chrono::duration_cast<std::chrono::microseconds>(p1.time_since_epoch()).count())
					};

				std::cout << u64
						<< ": interval is hit for:" << a.getInterval() << std::endl;
				*/
				//std::cout << "******************************\n";
				// Polling time: send requests
				CRequestInitiator::instance().initiateMessages(element.first, a, true);
			}
		}
	}
	catch (exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}
}

/**
 * Send request
 * @param a_stRdPrdObj	:[in] request to send
 * @param m_u16TxId	:	[in] TxID
 * @return 	true : on success,
 * 			false : on error
 */
bool CRequestInitiator::sendRequest(CRefDataForPolling &a_stRdPrdObj, uint16_t &m_u16TxId, bool isRTRequest)
{
	MbusAPI_t stMbusApiPram = {};
	uint8_t u8ReturnType = MBUS_STACK_NO_ERROR;
	bool bRet = false;
	void* ptrCallbackFunc = NULL;

	try
	{
		stMbusApiPram.m_u16StartAddr = a_stRdPrdObj.getDataPoint().getDataPoint().getAddress().m_iAddress;
		stMbusApiPram.m_u16Quantity = a_stRdPrdObj.getDataPoint().getDataPoint().getAddress().m_iWidth;
		stMbusApiPram.m_u16ByteCount = a_stRdPrdObj.getDataPoint().getDataPoint().getAddress().m_iWidth;

		if(true == a_stRdPrdObj.getDataPoint().getDataPoint().getPollingConfig().m_bIsRealTime)
		{
			stMbusApiPram.m_lPriority = 3;
		}
		else
		{
			stMbusApiPram.m_lPriority = 4;
		}

		// Coil and discrete input are single bytes. All others are 2 byte registers
		if((network_info::eEndPointType::eCoil != a_stRdPrdObj.getDataPoint().getDataPoint().getAddress().m_eType) &&
		(network_info::eEndPointType::eDiscrete_Input != a_stRdPrdObj.getDataPoint().getDataPoint().getAddress().m_eType))
		{
			// as default value for register is 2 bytes
			stMbusApiPram.m_u16ByteCount = stMbusApiPram.m_u16Quantity *2;
		}

		/*Enter TX Id*/
		//stMbusApiPram.m_u16TxId = PublishJsonHandler::instance().getTxId();
		stMbusApiPram.m_u16TxId = m_u16TxId;

#ifdef UNIT_TEST
		stMbusApiPram.m_u16TxId = 5;
#endif
		//a_stRdPrdObj.m_u16TxId = stMbusApiPram.m_u16TxId;

		CRequestInitiator::instance().insertTxIDReqData(stMbusApiPram.m_u16TxId, a_stRdPrdObj);
#ifdef MODBUS_STACK_TCPIP_ENABLED
		// fill the unit ID
		stMbusApiPram.m_u8DevId = a_stRdPrdObj.getDataPoint().getWellSiteDev().getAddressInfo().m_stTCP.m_uiUnitID;
		std::string sIPAddr{a_stRdPrdObj.getDataPoint().getWellSiteDev().getAddressInfo().m_stTCP.m_sIPAddress};

		CommonUtils::ConvertIPStringToCharArray(sIPAddr,stMbusApiPram.m_u8IpAddr);

		/// fill tcp port
		stMbusApiPram.m_u16Port = a_stRdPrdObj.getDataPoint().getWellSiteDev().getAddressInfo().m_stTCP.m_ui16PortNumber;

		if(true == isRTRequest)
		{
			stMbusApiPram.m_nRetry = globalConfig::CGlobalConfig::getInstance().getOpPollingOpConfig().getRTConfig().getRetries();
			ptrCallbackFunc = (void*)readPeriodicRTCallBack;
		}
		else
		{
			stMbusApiPram.m_nRetry = globalConfig::CGlobalConfig::getInstance().getOpPollingOpConfig().getNonRTConfig().getRetries();
			ptrCallbackFunc = (void*)readPeriodicCallBack;
		}

		if(false == common_Handler::insertReqData(stMbusApiPram.m_u16TxId, stMbusApiPram))
		{
			CLogger::getInstance().log(WARN, LOGDETAILS("Failed to add MbusAPI_t data to map."));
		}

		u8ReturnType = Modbus_Stack_API_Call(a_stRdPrdObj.getFunctionCode(), &stMbusApiPram, ptrCallbackFunc);

#else
		stMbusApiPram.m_u8DevId = a_stRdPrdObj.getDataPoint().getWellSiteDev().getAddressInfo().m_stRTU.m_uiSlaveId;

		if(true == isRTRequest)
		{
			stMbusApiPram.m_nRetry = globalConfig::CGlobalConfig::getInstance().getOpPollingOpConfig().getRTConfig().getRetries();
			ptrCallbackFunc = (void*)readPeriodicRTCallBack;
		}
		else
		{
			stMbusApiPram.m_nRetry = globalConfig::CGlobalConfig::getInstance().getOpPollingOpConfig().getNonRTConfig().getRetries();
			ptrCallbackFunc = (void*)readPeriodicCallBack;
		}

		if(false == common_Handler::insertReqData(stMbusApiPram.m_u16TxId, stMbusApiPram))
		{
			CLogger::getInstance().log(WARN, LOGDETAILS("Failed to add MbusAPI_t data to map."));
		}
		u8ReturnType = Modbus_Stack_API_Call(a_stRdPrdObj.getFunctionCode(), &stMbusApiPram, ptrCallbackFunc);
#endif

		if(MBUS_STACK_NO_ERROR == u8ReturnType)
		{
			bRet = true;
		}
		else
		{
			// In case of error, immediately retry in next iteration
			//a_stRdPrdObj.bIsRespAwaited = false;
			string l_stErrorString = "Request initiation error:: "+ to_string(u8ReturnType)+" " + "DeviceID ::" + to_string(stMbusApiPram.m_u16TxId);
			bRet = false;
		}
	}
	catch(exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}

	return bRet;
}

/**
 * TimeRecord Constructor
 * @param a_u32Interval	:[in] data point polling frequency
 * @param a_oPoint	:[in] reference of RefDataForPolling class
 */
CTimeRecord::CTimeRecord(uint32_t a_u32Interval, CRefDataForPolling &a_oPoint)
	: m_u32Interval(a_u32Interval), m_u32RemainingInterval(a_u32Interval),
	  m_u32CutoffInterval(a_u32Interval), m_u32RemainingCutoffInterval(a_u32Interval*2),
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
 * Insert time
 * @param a_uTime	:[in] time
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
		CLogger::getInstance().log(FATAL, LOGDETAILS(tempw0));
		bRet = false;
	}
	return bRet;
}

/**
 * Destructor
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
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}
}

/**
 * Find GCD of given numbers
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
 * Gets minimum time frequency that can be used based on current records
 * @param
 * @return 	number : minimum polling frequency in milliseconds
 */
uint32_t CTimeMapper::getMinTimerFrequency()
{
	// This function tries to find GCD (greatest common divisor) of all frequencies
	// This gcd is used as a timer frequency
	// E.g. for frequencies 250, 500, 1000 - gcd is 250
	// for frequencies 250, 400, 500 - gcd is 50
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
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Minimum frequency: " + to_string(ulMinFreq)));
	return ulMinFreq;
}

/**
 * Add time record
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
		CLogger::getInstance().log(FATAL, LOGDETAILS(tempw));
		bRet = false;
	}
	return bRet;
}

/**
 * Destructor
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
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
	}
}

/**
 *Function to get timer callback
 *@param : [in] interval in milliseconds
 *@return : Nothing
 */
void PeriodicTimer::timerThread(uint32_t interval)
{
	struct itimerspec new_value;
	int fd;
	struct timespec now;

	// set thread priority
	globalConfig::set_thread_sched_param(
			globalConfig::CGlobalConfig::getInstance().getOpPollingOpConfig().getNonRTConfig(),
			65,
			globalConfig::threadScheduler::RR,
			true);

	globalConfig::display_thread_sched_attr("timerThread param::");

	if(0 == interval)
	{
		interval = 1;
	}

	if (clock_gettime(CLOCK_MONOTONIC, &now) == -1)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS("Unable to get current time: "
				+ to_string(errno)));
		return;
	}

	/* Create a CLOCK_REALTIME absolute timer with initial
       expiration and interval as specified in command line */
	new_value.it_value.tv_sec = now.tv_sec + 1;
	new_value.it_value.tv_nsec = 0;
	new_value.it_interval.tv_sec = interval / 1000;
	new_value.it_interval.tv_nsec = (interval % 1000) * 1000 * 1000;

	fd = timerfd_create(CLOCK_MONOTONIC, 0);
	if (fd < 0)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS("Timer could not be created: "
				+ to_string(errno)));
		return;
	}

	if (timerfd_settime(fd, TFD_TIMER_ABSTIME, &new_value, NULL) < 0)
		//if (timerfd_settime(fd, 0, &new_value, NULL) < 0)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS("Timer timing could not be set: "
				+ to_string(errno)));
		close(fd);
		return;
	}

	while(!g_stopTimer)
	{
		uint64_t uiTimerHitCount;
		ssize_t s = read(fd, &uiTimerHitCount, sizeof(uint64_t));
		if (s != sizeof(uint64_t))
		{
			CLogger::getInstance().log(FATAL, LOGDETAILS("Failed to read timer status: "
					+ to_string(errno)));
			continue;
		}
		CTimeMapper::instance().checkTimer(interval * uiTimerHitCount);
	}
}

/**
 * Function to start Linux timer
 * @param [in] : interval in milliseconds
 * @return : nothing
 */
void PeriodicTimer::timer_start(uint32_t interval)
{
	/// create thread for timer routine
	std::thread(std::bind(timerThread, interval)).detach();
}

/**
 * Function to stop Linux timer
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
		m_objDataPoint{a_refPolling.m_objDataPoint}, m_objBusContext{a_refPolling.m_objBusContext}
		, m_objPubContext{a_refPolling.m_objPubContext}, m_uiFuncCode{a_refPolling.m_uiFuncCode}
		, m_bIsRespPosted{false}, m_bIsLastRespAvailable{false}
{
	m_oLastGoodResponse.m_sValue = "";
	m_oLastGoodResponse.m_sLastUsec = "";
}

/**
 * Constructor
 * @param CUniqueDataPoint	:[in] reference CUniqueDataPoint object
 * @param stZmqContext		:[in] reference to ZMQ bus context
 * @param stZmqPubContext	:[in] reference to pub topic context
 * @param uint8_t			:[in] function code for this point
 */
CRefDataForPolling::CRefDataForPolling(const CUniqueDataPoint &a_objDataPoint, struct stZmqContext& a_objBusContext, struct stZmqPubContext& a_objPubContext, uint8_t a_uiFuncCode) :
				m_objDataPoint{a_objDataPoint}, m_objBusContext{a_objBusContext}, m_objPubContext{a_objPubContext}, m_uiFuncCode{a_uiFuncCode}
				, m_bIsRespPosted{false}, m_bIsLastRespAvailable{false}
{
	m_oLastGoodResponse.m_sValue = "";
	m_oLastGoodResponse.m_sLastUsec = "";
}

/**
 * Saves last known good response data
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
 * Get last response
 * @param nothing
 * @return 	stLastGoodResponse : Last known good response
 */
stLastGoodResponse CRefDataForPolling::getLastGoodResponse()
{
	std::lock_guard<std::mutex> lock(m_mutexLastResp);
	stLastGoodResponse objStackResp = m_oLastGoodResponse;
	return objStackResp;
}
