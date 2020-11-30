/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "cjson/cJSON.h"
#include "Common.hpp"
#include "QueueMgr.hpp"
#include "Logger.hpp"

using namespace QMgr;

//initialize queue manager instances for on-demand read/write and real-time read/write

/**
 * Get reference of on-demand read operation instance
 * @param None
 * @return reference of on-demand read operation instance
 */
CQueueMgr& QMgr::getRead()
{
	static CQueueMgr ng_qRead(true, false);
	return ng_qRead;
}

/**
 * Get reference of on-demand write operation instance
 * @param None
 * @return reference of on-demand write operation instance
 */
CQueueMgr& QMgr::getWrite()
{
	static CQueueMgr ng_qWrite(false, false);
	return ng_qWrite;
}

/**
 * Get reference of on-demand real-time read operation instance
 * @param None
 * @return reference of on-demand real-time read operation instance
 */
CQueueMgr& QMgr::getRTRead()
{
	static CQueueMgr ng_qRTRead(true, true);
	return ng_qRTRead;
}

/**
 * Get reference of on-demand real-time write operation instance
 * @param None
 * @return reference of on-demand real-time write operation instance
 */
CQueueMgr& QMgr::getRTWrite()
{
	static CQueueMgr ng_qRTWrite(false, true);
	return ng_qRTWrite;
}

/**
 * Constructor of queue manager
 * @param isRead :[in] set if operational instance is for read operation
 * 					or write (toggle between read and write)
 * @param isRealTime :[in] set if operational instance is for real-time operation
 * 					or non-RT (toggle between RT and non-RT)
 * @return None
 */
QMgr::CQueueMgr::CQueueMgr(bool isRead, bool isRealTime)
{
	m_bIsRead = isRead;
	m_bIsRealTime = isRealTime;
}

/**
 * Destructor
 */
QMgr::CQueueMgr::~CQueueMgr()
{
}
