/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "ControlLoopHandler.hpp"
#include "QueueMgr.hpp"
#include "Common.hpp"
#include "KPIAppConfigMgr.hpp"
#include <algorithm>
#include "eis/msgbus/msg_envelope.h"
#include "ZmqHandler.hpp"

extern std::atomic<bool> g_stopThread;

/**
 * This function posts a dummy analysis message in case of error
 * @param a_sAppSeq	[in]: Probable app-seq number for write op
 * @param a_sError	[in]: Error info 
 * @return none
 */
void CControlLoopOp::postDummyAnalysisMsg(const std::string &a_sAppSeq, const std::string &a_sError) const
{
	try
	{
		struct stPollWrData oTempData{};
		CPollNWriteReqMapper::getInstace().getForProcessing(a_sAppSeq, oTempData);
		std::string sDummyErrorRep;
		commonUtilKPI::addFieldToMsg(sDummyErrorRep, "app_seq", a_sAppSeq, false);
		commonUtilKPI::addFieldToMsg(sDummyErrorRep, "data_topic", m_sWritePointFullPath + "/writeResponse", false);
		commonUtilKPI::addFieldToMsg(sDummyErrorRep, "error_code", a_sError + "/writeResponse", true);
		sDummyErrorRep = "{" + sDummyErrorRep + "}";
		CMessageObject oDummyMsg{"errorDummyTopic", sDummyErrorRep};
		commonUtilKPI::logAnalysisMsg(oTempData, oDummyMsg);
	}
	catch (exception &ex)
	{
		DO_LOG_ERROR((std::string)ex.what());
	}
}

/**
 * Thread function for a control loop to check polling message and send a write request
 * @return true/false based on success/failure
 */
void CControlLoopOp::threadPollMonitoring()
{
	std::cout << "Started new thread for : " << m_sPolledTopic 
		<< ", write point: " << m_sWritePointFullPath 
		<< ", ID: " << m_sId << std::endl;
	
	DO_LOG_INFO("Started new thread for : " + m_sPolledTopic 
		+ ", write point: " + m_sWritePointFullPath);
	std::string sLastWrSeqVal{""};
	try
	{
		while (false == g_stopThread.load())
		{
			CMessageObject recvdMsg{};
			if(true == m_q.isMsgArrived(recvdMsg))
			{
				// Message is received
				// Check if writeResponse for last message was received
				if(true == CPollNWriteReqMapper::getInstace().isPresent(sLastWrSeqVal))
				{
					postDummyAnalysisMsg(sLastWrSeqVal, "WrRespNotRcvd");
				}
				
				std::string sWrSeqVal{commonUtilKPI::getValueofKeyFromJSONMsg(recvdMsg.getStrMsg(), "driver_seq")};
				if(true == sWrSeqVal.empty())
				{
					DO_LOG_ERROR(recvdMsg.getStrMsg() + ": driver_seq key not found. Ignoring the message");
					continue;
				}
				sWrSeqVal.append("-" + EnvironmentInfo::getInstance().getDataFromEnvMap("AppName")+ "-" + m_sId);
				// Sleep for configured delay
				this_thread::sleep_for(std::chrono::milliseconds(m_uiDelayMs));

				CKPIAppConfig::getInstance().getControlLoopMapper().publishWriteReq(*this, sWrSeqVal, recvdMsg);
				sLastWrSeqVal.assign(sWrSeqVal);
			}
		}
		std::cout << "Exiting thread::threadPollMonitoring - id " << m_sId << "\n";
	}
	catch (const std::exception &e)
	{
		DO_LOG_ERROR(e.what());
	}
}

/**
 * Starts control loop thread
 * @return true/false based on success/failure
 */
bool CControlLoopOp::startThread()
{
	//
	m_thread = std::thread(&CControlLoopOp::threadPollMonitoring, this);
	return true;
}

/**
 * Stops control loop thread
 * @return true/false based on success/failure
 */
bool CControlLoopOp::stopThread()
{
	try
	{
		m_q.breakWaitOnQ();
		if (m_thread.joinable())
		{
			m_thread.join();
		}
	}
	catch (exception &ex)
	{
		DO_LOG_ERROR((std::string)ex.what());
	}
	return true;
}

/**
 * Verifies whether point path is in correct format
 * @param a_sFullPath 		[in]: Point's full path
 * @param a_sDevName 		[out]: extracted dev name
 * @param a_sWellHeadName 	[out]: extracted well head name
 * @param a_sPointName 		[out]: extracted point name
 * @return true/false based on success/failure
 */
bool CControlLoopMapper::verifyPointFullPath(const std::string &a_sFullPath, std::string &a_sDevName, 
						std::string &a_sWellHeadName, std::string &a_sPointName)
{
	//
	bool bRet{false};
	try
	{
		do 
		{
			// Format: /dev_name/wellhead/point-name e.g. /iou/PL0/AVal
			// Split it to get individual fields
			std::string delimiter = "/";
			size_t last = 0;
			size_t next = 0;
			std::vector<std::string> vsTopicParts;
			int iDelimiterCnt = 0;
			// Extract dev name
			while ((next = a_sFullPath.find(delimiter, last)) != std::string::npos)
			{
				++iDelimiterCnt;
				std::string sTemp(a_sFullPath.substr(last, next - last));
				if(false == sTemp.empty())
				{
					vsTopicParts.push_back(sTemp);
				}
				last = next + 1;
			}
			vsTopicParts.push_back(a_sFullPath.substr(last));
			if((vsTopicParts.size() == 3) && (3 == iDelimiterCnt))
			{
				a_sDevName.assign(vsTopicParts[0]);
				a_sWellHeadName.assign(vsTopicParts[1]);
				a_sPointName.assign(vsTopicParts[2]);

				bRet = true;
			}		
			
		} while(0);
	}
	catch (const std::exception &e)
	{
		DO_LOG_ERROR(e.what());		
	}

	return bRet;
}

/**
 * Adds a control loop daat entry into map being maintained
 * @param a_sPolledTopic	:[in] Topic to be monitored for polling
 * @param a_sWriteTopic		:[in] Topic to be used for sending write request
 * @param a_uiDelayMs		:[in] Delay to be used before sending a write request
 * @param a_sVal			:[in] Value to be sent in the write request
 * @return true/false based on success/failure
 */
bool CControlLoopMapper::insertControlLoopData(const std::string &a_sPolledTopic, const std::string &a_sWriteTopic, 
					uint32_t a_uiDelayMs, const std::string &a_sVal)
{
	try
	{
		// Verify point names first
		std::string sDevName{""}, sWellHeadName{""}, sPointName{""};
		bool bRet = verifyPointFullPath(a_sPolledTopic, sDevName, sWellHeadName, sPointName);
		if(false == bRet)
		{
			// Log error 
			DO_LOG_ERROR(a_sPolledTopic + ": Incorrect polling point name.");
			DO_LOG_ERROR(a_sPolledTopic + ": " + a_sWriteTopic + ": Control loop cannot be executed.");
			return false;
		}
		sDevName.clear(); sWellHeadName.clear(); sPointName.clear();
		bRet = verifyPointFullPath(a_sWriteTopic, sDevName, sWellHeadName, sPointName);
		if(false == bRet)
		{
			// Log error 
			DO_LOG_ERROR(a_sWriteTopic + ": Incorrect write point name.");
			DO_LOG_ERROR(a_sPolledTopic + ": " + a_sWriteTopic + ": Control loop cannot be executed.");
			return false;
		}
		
		std::string sPollKey{a_sPolledTopic + "/update"}; 
		auto itr = m_oControlLoopMap.find(sPollKey);
		if (m_oControlLoopMap.end() == itr)
		{
			std::vector<CControlLoopOp> oTemp;
			m_oControlLoopMap.emplace(sPollKey, oTemp);
			itr = m_oControlLoopMap.find(sPollKey);
		}
		if (m_oControlLoopMap.end() != itr)
		{
			++m_uiCtrlLoopCnt;
			CControlLoopOp oLoop{m_uiCtrlLoopCnt, sPollKey, a_sWriteTopic, sDevName, sWellHeadName, sPointName, a_uiDelayMs, a_sVal};
			itr->second.push_back(oLoop);
			m_vsPollTopics.push_back(sPollKey);
			m_vsWrRspTopics.push_back(a_sWriteTopic + "/writeResponse");
		}
	}
	catch(exception &e)
	{
		DO_LOG_ERROR(e.what());
		return false;
	}
	return true;
}

/**
 * Trigger associated control loops on receiving a polling message
 * @param a_sPolledPoint	:[in] Point being polled
 * @param a_oMsg			:[in] Received message
 * @return true/false based on success/failure
 */
bool CControlLoopMapper::triggerControlLoops(std::string& a_sPolledPoint, CMessageObject a_oMsg)
{
	std::vector<std::string> sPolledTopicVector;
	try
	{
		auto &vControlLoop = m_oControlLoopMap.at(a_sPolledPoint);
		for (auto &itr : vControlLoop) 
		{
			itr.getQueue().pushMsg(a_oMsg);
		}
	}
	catch(exception &ex)
	{
		DO_LOG_ERROR(ex.what());
		return false;
	}
	return true;
}

/**
 * Starts control loop threads
 * @param a_bIsRTWrite	[in]: It indicates whether write op is RT
 * @return true/false based on success/failure
 */
bool CControlLoopMapper::configControlLoopOps(bool a_bIsRTWrite)
{
	//
	for (auto& itr : m_oControlLoopMap) 
	{
		try
		{
			auto &listControlLoop = itr.second;
			DO_LOG_INFO(itr.first + " - Polled Topic, Control loops:" + std::to_string(listControlLoop.size()));
			for (auto& rCtrlLoop : listControlLoop) 
			{
				rCtrlLoop.startThread();
			}
		}
		catch(const std::exception& e)
		{
			DO_LOG_ERROR(itr.first + ": error while creating control loop thread for polled point. " + e.what());
		}
	}

	std::cout << "Control loop threads are set. Now start listening\n";
	return true;
}

/**
 * Destroys the sub contexts
 * @return true/false based on success/failure
 */
bool CControlLoopMapper::destroySubCtx()
{
	bool retVal = false;
	try
	{
		std::vector<std::string> vFullTopics = CcommonEnvManager::Instance().getTopicList();
		for(auto& sTopic : vFullTopics)
		{
			zmq_handler::stZmqContext& ctx = zmq_handler::getCTX(sTopic);
			zmq_handler::stZmqSubContext& subContext = zmq_handler::getSubCTX(sTopic);
			if(NULL != subContext.sub_ctx && NULL != ctx.m_pContext)
			{
				zmq_handler::removeSubCTX(sTopic);
				msgbus_recv_ctx_destroy(ctx.m_pContext, subContext.sub_ctx);
			}
			/// free msg bus context
			if(ctx.m_pContext != NULL)
			{
				zmq_handler::removeCTX(sTopic);
				msgbus_destroy(ctx.m_pContext);
				ctx.m_pContext = NULL;
			}
		}
		retVal = true;
	}
	catch(const std::exception& e)
	{
		DO_LOG_ERROR(e.what());
		std::cout << "Exception while destroying contexts..." << std::endl;
		retVal = false;
	}

	return retVal;
}

/**
 * Stops control loop threads
 * @return true/false based on success/failure
 */
bool CControlLoopMapper::stopControlLoopOps()
{
	//
	for (auto& itr : m_oControlLoopMap) 
	{
		try
		{
			auto &listControlLoop = itr.second;
			for (auto& rCtrlLoop : listControlLoop) 
			{
				rCtrlLoop.stopThread();
			}

			PlBusMgr::stopListeners();
		}
		catch(const std::exception& e)
		{
			DO_LOG_ERROR(itr.first + ": error while stopping control loop thread for polled point. " + e.what());
		}
	}

	std::cout << "Control loop threads are signalled for stopping.\n";
	return true;
}

/**
 * Checks whether given polling topic (e.g. "/iou/PL0/AVal/update")
 * is a part of one of the control loops
 * @param a_sPollTopic	[in]: Poll topic to be checked 
 * @return true = yes, false = no
 */
bool CControlLoopMapper::isControlLoopPollPoint(const std::string &a_sPollTopic)
{
	if (std::find(m_vsPollTopics.begin(), m_vsPollTopics.end(), a_sPollTopic) != m_vsPollTopics.end()) 
	{
		return true;
	}
	return false;
}

/**
 * Checks whether given write response topic (e.g. "/iou/PL0/AVal/writeResponse")
 * is a part of one of the control loops
 * @param a_sPollTopic	[in]: Write response topic to be checked 
 * @return true = yes, false = no
 */
bool CControlLoopMapper::isControlLoopWrRspPoint(const std::string &a_sWrRspTopic)
{
	if (std::find(m_vsWrRspTopics.begin(), m_vsWrRspTopics.end(), a_sWrRspTopic) != m_vsWrRspTopics.end()) 
	{
		return true;
	}
	return false;
}

/**
 * Checks whether given write response topic (e.g. "/iou/PL0/AVal/writeResponse")
 * is a part of one of the control loops
 * @param a_rCtrlLoop	[in]: Control loop for which write needs to be published
 * @param a_sWrSeq		[in]: Sequence number to be used in write request
 * @param a_oPollMsg	[in]: Poll message for which write is being sent
 * @return true/false based on success/failure
 */
bool CControlLoopMapper::publishWriteReq(const CControlLoopOp& a_rCtrlLoop, 
			const std::string &a_sWrSeq, CMessageObject &a_oPollMsg)
{
	try
	{
		struct timespec tsStartWrReqCreate;
		timespec_get(&tsStartWrReqCreate, TIME_UTC);
		struct stPollWrData oTemp{a_oPollMsg, tsStartWrReqCreate};
		CPollNWriteReqMapper::getInstace().insertForTracking(a_sWrSeq, oTemp);
		if(false == PlBusMgr::publishWriteReq(a_rCtrlLoop, a_sWrSeq, a_oPollMsg))
		{
			DO_LOG_ERROR("Failed publishing for point: " + a_rCtrlLoop.getWritePoint());
			a_rCtrlLoop.postDummyAnalysisMsg(a_sWrSeq, "WrReqInitFailed");
			return false;
		}
		return true;
	}
	catch(const std::exception& e)
	{
		DO_LOG_ERROR(a_rCtrlLoop.getWritePoint() + ": Write request error: " +  e.what());
	}
	
	return false;
}
