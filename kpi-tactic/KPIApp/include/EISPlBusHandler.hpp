/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/
#ifndef INCLUDE_EISPLBUSHANDLER_HPP_
#define INCLUDE_EISPLBUSHANDLER_HPP_

#include <vector>
#include <thread>
#include "ZmqHandler.hpp"
#include "QueueMgr.hpp"
#include <functional>
#include <string>

class CEISPlBusHandler
{
    std::vector<std::thread> m_vThreads;

    void listenOnEIS(std::string topic, zmq_handler::stZmqContext context,
			zmq_handler::stZmqSubContext subContext, bool a_bIsPolling);
    
    bool processMsg(msg_envelope_t *msg, CQueueHandler &a_rQ,
		const std::function<bool(const std::string &)> &a_fPointListChecker);

public:
    bool initEISContext();
    void configEISListerners(bool a_bIsPollingRT, bool a_bIsWrOpRT);
    void stopEISListeners();
    bool publishWriteMsg(const std::string &a_sMsg);    
};

#endif
