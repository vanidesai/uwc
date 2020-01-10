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

PublishJsonHandler::PublishJsonHandler()
{}

PublishJsonHandler& PublishJsonHandler::instance()
{
	static PublishJsonHandler handler;
	return handler;
}

BOOLEAN PublishJsonHandler::publishJson(msg_envelope_t* msg, void* msgbus_ctx, const std::string a_sTopic)
{
	if((NULL == msg) || (NULL == msgbus_ctx))
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS(": Failed to publish message - Input parameters are NULL"));
		return false;
	}
	std::lock_guard<std::mutex> lock(publishJsonMutex);

	msgbus_ret_t ret;
	zmq_handler::stZmqPubContext busPubCTX;

	CLogger::getInstance().log(DEBUG, LOGDETAILS("msg to publish :: Topic :: " + a_sTopic));

	try
	{
		/// get the publisher context
		busPubCTX = zmq_handler::getPubCTX(a_sTopic);
	}
	catch (std::exception &e) {
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		stZmqPubContext objPubContext;
		publisher_ctx_t *pub_ctx = NULL;
		ret = msgbus_publisher_new(msgbus_ctx,a_sTopic.c_str(), &pub_ctx);

		if(ret != MSG_SUCCESS)
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS(" Failed to initialize publisher errno: " + std::to_string(ret)));
			return false;
		}

		objPubContext.m_pContext = pub_ctx;
		if(true == zmq_handler::insertPubCTX(a_sTopic, objPubContext))
		{
			/// get the publisher context
			busPubCTX = zmq_handler::getPubCTX(a_sTopic);
		}
		else
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS(" Failed to obtain publish context " + std::to_string(ret)));
			return false;
		}
	}

	//void *pubContext = busPubCTX.m_pContext;
	ret = msgbus_publisher_publish(msgbus_ctx, (publisher_ctx_t*)busPubCTX.m_pContext, msg);
	if(ret != MSG_SUCCESS)
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS(" Failed to publish message errno: " + std::to_string(ret)));
		return false;
	}

	return true;
}

msg_envelope_t* PublishJsonHandler::initialize_message(std::string strMsg)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start:: " + strMsg));

    // Creating message to be published
    msg_envelope_elem_body_t* tempMsg = msgbus_msg_envelope_new_string(strMsg.c_str());

    msg_envelope_t* msg = msgbus_msg_envelope_new(CT_JSON);

    msgbus_msg_envelope_put(msg, "", tempMsg);

    CLogger::getInstance().log(DEBUG, LOGDETAILS("End:: " + strMsg));

    return msg;
}



