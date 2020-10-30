/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "Common.hpp"
#include "QueueMgr.hpp"
#include "MqttHandler.hpp"
#include "EISPlBusHandler.hpp"
#include "KPIAppConfigMgr.hpp"
#include "ControlLoopHandler.hpp"
#include "ZmqHandler.hpp"

static CEISPlBusHandler& getEISPlBusHandler()
{
	static CEISPlBusHandler _obj;
	return _obj;
}

/**
 * Get reference of polled message
 * @param None
 * @return reference of on-demand read operation instance
 */
CQueueHandler& QMgr::PollMsgQ()
{
	static CQueueHandler ng_qPollMsgs;
	return ng_qPollMsgs;
}

/**
 * Get reference of write response message
 * @param None
 * @return reference of on-demand read operation instance
 */
CQueueHandler& QMgr::WriteRespMsgQ()
{
	static CQueueHandler ng_qWrRespMsgs;
	return ng_qWrRespMsgs;
}

/**
 * Get reference of write response message
 * @param None
 * @return reference of on-demand read operation instance
 */
void PlBusMgr::initPlatformBusHandler(bool a_bIsMQTTMode)
{
	try
	{
		if(a_bIsMQTTMode)
		{
			// Prepare MQTT for publishing & subscribing
			// subscribing to topics happens in callback of connect()
			CMqttHandler::instance();
		}
		else
		{
			// It is ZMQ mode. Prepare contexts and create threads
			getEISPlBusHandler().initEISContext();

			getEISPlBusHandler().configEISListerners(CKPIAppConfig::getInstance().isRTModeForPolledPoints(),
				CKPIAppConfig::getInstance().isRTModeForWriteOp());
		}
	}
	catch(const std::exception& e)
	{
		DO_LOG_ERROR(e.what());
	}
}

/**
 * Forms and publishes write request on MQTT
 * @param a_sPubTopic	[in]: Topic for publishing 
 * @param a_sMsg		[in]: Message to be published
 * @return true/false based on success/failure
 */
bool publishWriteReqOnMQTT(const std::string &a_sPubTopic, const std::string &a_sMsg)
{
	try
	{
		return CMqttHandler::instance().publishMsg(a_sMsg, a_sPubTopic);
	}
	catch(const std::exception& e)
	{
		DO_LOG_ERROR(a_sPubTopic + ": Write request error: " +  e.what());
	}
	
	return false;
}

/**
 * Forms and publishes write request on MQTT
 * @param a_rCtrlLoop	[in]: Control loop for which write needs to be published
 * @param a_sWrSeq		[in]: Sequence number to be used in write request
 * @param a_oPollMsg	[in]: Poll message for which write is being sent
 * @return true/false based on success/failure
 */
bool PlBusMgr::publishWriteReq(const CControlLoopOp& a_rCtrlLoop, 
			const std::string &a_sWrSeq, CMessageObject &a_oPollMsg)
{
	try
	{
		std::string sWrRT{"0"};
		if(true == CKPIAppConfig::getInstance().isRTModeForWriteOp())
		{
			sWrRT.assign("1");
		}
		std::string sPubTopic{a_rCtrlLoop.getWritePoint()+"/write"};
		std::string sMsg{""};
		if(false == commonUtilKPI::createWriteRequest(sMsg, a_sWrSeq, 
			a_rCtrlLoop.getWellHeadNameForWrReq(), 
			a_rCtrlLoop.getPointNameForWrReq(), a_rCtrlLoop.getValue(), sWrRT,
			sPubTopic, CKPIAppConfig::getInstance().isMQTTModeOn()))
		{
			DO_LOG_ERROR(a_oPollMsg.getStrMsg() + ": Unable to create a write request. Ignoring the message");
			return false;
		}

		if(true == CKPIAppConfig::getInstance().isMQTTModeOn())
		{
			return publishWriteReqOnMQTT(sPubTopic, sMsg);
		}
		else
		{
			return getEISPlBusHandler().publishWriteMsg(sMsg);
		}
	}
	catch(const std::exception& e)
	{
		DO_LOG_ERROR(a_rCtrlLoop.getWritePoint() + ": Write request error: " +  e.what());
	}
	
	return false;
}

/**
 * Stops listener threads
 * @return None
 */
void PlBusMgr::stopListeners()
{
	try
	{
		if(true == CKPIAppConfig::getInstance().isMQTTModeOn())
		{
			;
		}
		else
		{
			getEISPlBusHandler().stopEISListeners();
		}
	}
	catch(const std::exception& e)
	{
		DO_LOG_ERROR(e.what());
	}
}
