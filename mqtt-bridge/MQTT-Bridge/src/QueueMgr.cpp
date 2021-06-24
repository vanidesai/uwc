/********************************************************************************
* Copyright (c) 2021 Intel Corporation.

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*********************************************************************************/

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
