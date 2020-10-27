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

#include "QueueHandler.hpp"

/**
 * namespace for Queue manager
 */
namespace QMgr
{
/**
 * Queue manager class to manage instances of on-demand operations for msg handling
 */
class CQueueMgr : public CQueueHandler
{
	bool m_bIsRead;
	bool m_bIsRealTime;
public:
	CQueueMgr(bool isRead, bool isRealTime);
	~CQueueMgr();

	bool isRead() {return m_bIsRead;};
	bool isRealTime() {return m_bIsRealTime;};
};

//functions to get on-demand operation instances
CQueueMgr& getRead();
CQueueMgr& getWrite();
CQueueMgr& getRTRead();
CQueueMgr& getRTWrite();
}
#endif
