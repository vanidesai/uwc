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

/*** QueueMgr.hpp for queue managing operations*/

#ifndef QMGR_HPP_
#define QMGR_HPP_

#include "QueueHandler.hpp"
#include "ControlLoopHandler.hpp"

/**
 * namespace for Queue manager
 */
namespace QMgr
{
/**
 * Queue manager class to manage instances of on-demand operations for msg handling
 */
	CQueueHandler& PollMsgQ();
	CQueueHandler& WriteRespMsgQ();
}

/** nmaespace for Bus manager*/
namespace PlBusMgr
{
	void initPlatformBusHandler(bool a_bIsMQTTMode);
	bool publishWriteReq(const CControlLoopOp& a_rCtrlLoop, 
			const std::string &a_sWrSeq);
	void stopListeners();
}
#endif
