/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#ifndef INCLUDE_PUBLISHJSON_HPP_
#define INCLUDE_PUBLISHJSON_HPP_

#include "BoostLogger.hpp"
#include "Common.hpp"
#include <eis/utils/thread_safe_queue.h>
#include <eis/utils/config.h>
#include <eis/utils/json_config.h>
#include <eis/msgbus/msgbus.h>
#include <eis/config_manager/env_config.h>
#include <eis/config_manager/config_manager.h>

/**
 * Structure to contain state for a publisher thread
 */
typedef struct {
    msg_envelope_t* msg;
    publisher_ctx_t* pub_ctx;
} pub_thread_ctx_t;

class PublishJsonHandler
{
	/// delete copy and move constructors and assign operators
	PublishJsonHandler(const PublishJsonHandler&) = delete;	 			/// Copy construct
	PublishJsonHandler& operator=(const PublishJsonHandler&) = delete;	/// Copy assign

	/// onstructor
	PublishJsonHandler();

public:
	// function to get single instance of this class
	static PublishJsonHandler& instance();
	BOOLEAN publishJson(msg_envelope_t* msg, void* msgbus_ctx, const std::string str_Topic);
	msg_envelope_t* initialize_message(std::string strMsg);
};


#endif /* INCLUDE_PUBLISHJSON_HPP_ */
