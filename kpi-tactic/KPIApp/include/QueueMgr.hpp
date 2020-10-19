/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef QMGR_HPP_
#define QMGR_HPP_

#include "QueueHandler1.hpp"
#include "ControlLoopHandler.hpp"

using namespace std;
/**
 * namespace for Queue manager
 */
namespace QMgr
{
/**
 * Queue manager class to manage instances of on-demand operations for msg handling
 */
	CQueueHandler1& PollMsgQ();
	CQueueHandler1& WriteRespMsgQ();
}

namespace PlBusMgr
{
	void initPlatformBusHandler(bool a_bIsMQTTMode);
	bool publishWriteReq(const CControlLoopOp& a_rCtrlLoop, 
			const std::string &a_sWrSeq, CMessageObject &a_oPollMsg);
	void stopListeners();
}
#endif
