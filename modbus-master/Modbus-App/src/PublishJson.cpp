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

std::mutex publishJsonMutex;

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
 * @param msgbus_ctx	:[in] msgbus context to publish on
 * @param pub_ctx		:[in] pub context
 * @param a_sTopic		:[in] topic on which to publish
 * @return 	true : on success,
 * 			false : on error
 */
bool PublishJsonHandler::publishJson(msg_envelope_t* msg, void* msgbus_ctx, void* pub_ctx, const std::string a_sTopic)
{
	if((NULL == msg) || (NULL == msgbus_ctx) || (NULL == pub_ctx))
	{
		DO_LOG_ERROR(": Failed to publish message - Input parameters are NULL");
		return false;
	}
	std::lock_guard<std::mutex> lock(publishJsonMutex);

	msgbus_ret_t ret;

	DO_LOG_DEBUG("msg to publish :: Topic :: " + a_sTopic);

	ret = msgbus_publisher_publish(msgbus_ctx, (publisher_ctx_t*)pub_ctx, msg);
	if(ret != MSG_SUCCESS)
	{
		DO_LOG_ERROR(" Failed to publish message errno: " + std::to_string(ret));
		return false;
	}

	return true;
}
