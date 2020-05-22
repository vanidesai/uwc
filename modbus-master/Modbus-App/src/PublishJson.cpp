/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#include "PublishJson.hpp"
#include "ZmqHandler.hpp"

#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <mutex>
#include "ZmqHandler.hpp"

#include "Logger.hpp"

using namespace zmq_handler;

/**
 * Constructor
 */
PublishJsonHandler::PublishJsonHandler()
{
	m_devMode = true;
	u32CutoffIntervalPercentage = 0;
}

/**
 * Return single instance of this class
 * @return
 */
PublishJsonHandler& PublishJsonHandler::instance()
{
	static PublishJsonHandler handler;
	return handler;
}

/**
 * Publish json
 * @param msg			:[in] message to publish
 * @param a_sTopic		:[in] topic on which to publish
 * @return 	true : on success,
 * 			false : on error
 */
bool PublishJsonHandler::publishJson(msg_envelope_t* msg, const std::string &a_sTopic)
{
	if(NULL == msg)
	{
		DO_LOG_ERROR(": Failed to publish message - Input message is NULL");
		return false;
	}

	DO_LOG_DEBUG("msg to publish :: Topic :: " + a_sTopic);

	zmq_handler::stZmqContext& msgbus_ctx = zmq_handler::getCTX(a_sTopic);
	void* pub_ctx = zmq_handler::getPubCTX(a_sTopic).m_pContext;
	if((NULL == msgbus_ctx.m_pContext) || (NULL == pub_ctx))
	{
		DO_LOG_ERROR(": Failed to publish message - context is NULL" + a_sTopic);
		return false;
	}

	msgbus_ret_t ret;

	{
		std::lock_guard<std::mutex> lock(msgbus_ctx.m_mutex);
		auto p1 = std::chrono::system_clock::now();
		unsigned long uTime = (unsigned long)(std::chrono::duration_cast<std::chrono::microseconds>(p1.time_since_epoch()).count());
		msg_envelope_elem_body_t* ptUsec = msgbus_msg_envelope_new_string((to_string(uTime)).c_str());
		if(NULL != ptUsec)
		{
			msgbus_msg_envelope_put(msg, "usec", ptUsec);
		}
		ret = msgbus_publisher_publish(msgbus_ctx.m_pContext, (publisher_ctx_t*)pub_ctx, msg);
	}

	if(ret != MSG_SUCCESS)
	{
		DO_LOG_ERROR(" Failed to publish message errno: " + std::to_string(ret));
		return false;
	}
	return true;
}
