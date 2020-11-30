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
 * Set the topic name as per specified patterns
 * @param a_sTopic		:[in] Topic for which pub or sub context needs to be created
 * @return 	true : on success,
 * 			false : on error
 */
bool PublishJsonHandler::setTopicForOperation(std::string a_sTopic)
{
	bool bRet = true;
	if(regExFun(a_sTopic, READ_RES))
	{
		PublishJsonHandler::instance().setSReadResponseTopic(a_sTopic);
		std::cout << "read Res topic = " << a_sTopic << std::endl;
		DO_LOG_INFO("read res topic = " + a_sTopic);
	}
	else if(regExFun(a_sTopic, READ_RES_RT))
	{
		PublishJsonHandler::instance().setSReadResponseTopicRT(a_sTopic);
		std::cout << "read Res RT topic = " << a_sTopic << std::endl;
		DO_LOG_INFO("read res RT topic = " + a_sTopic);
	}
	else if(regExFun(a_sTopic, WRITE_RES))
	{
		PublishJsonHandler::instance().setSWriteResponseTopic(a_sTopic);
		std::cout << "write res topic = " << a_sTopic << std::endl;
		DO_LOG_INFO("write res topic = " + a_sTopic);
	}
	else if(regExFun(a_sTopic, WRITE_RES_RT))
	{
		PublishJsonHandler::instance().setSWriteResponseTopicRT(a_sTopic);
		std::cout << "write res RT topic = " << a_sTopic << std::endl;
		DO_LOG_INFO("write res RT topic = " + a_sTopic);
	}
	else if(regExFun(a_sTopic, POLLDATA))
	{
		PublishJsonHandler::instance().setPolledDataTopic(a_sTopic);
		std::cout << "poll topic = " << a_sTopic << std::endl;
		DO_LOG_INFO("poll topic = " + a_sTopic);
	}
	else if(regExFun(a_sTopic, POLLDATA_RT))
	{
		PublishJsonHandler::instance().setPolledDataTopicRT(a_sTopic);
		std::cout << "poll topic RT = " << a_sTopic << std::endl;
		DO_LOG_INFO("poll topic RT = " + a_sTopic);
	}
	else if(regExFun(a_sTopic, READ_REQ) or
			regExFun(a_sTopic, READ_REQ_RT) or
			regExFun(a_sTopic, WRITE_REQ) or
			regExFun(a_sTopic, WRITE_REQ_RT))
	{
		// Do nothing it just to check the correct pattern
	}
	else
	{
		DO_LOG_ERROR("Invalid topic name in SubTopics/PubTopics. hence ignoring :: " + a_sTopic);
		std::cout << "Invalid topic name in SubTopics/PubTopics. hence ignoring :: " << a_sTopic << std::endl;
		DO_LOG_ERROR("Kindly specify correct topic name as per specification :: " + a_sTopic);
		std::cout << "Kindly specify correct topic name as per specification :: " << a_sTopic << std::endl;
		bRet = false;
	}
	return bRet;
}
