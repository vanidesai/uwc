/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#ifndef TEST_INCLUDE_ZMQHANDLER_UT_HPP_
#define TEST_INCLUDE_ZMQHANDLER_UT_HPP_

#include <string>
#include <stdlib.h>
#include <map>
#include <thread>
#include "gtest/gtest.h"

#include "ConfigManager.hpp"
#include "NetworkInfo.hpp"
#include "ZmqHandler.hpp"
#include "NetworkInfo.hpp"

extern void populatePollingRefData();

class ZmqHandler_ut : public ::testing::Test {
protected:
	virtual void SetUp();
	virtual void TearDown();
public:
	msgbus_ret_t retVal = MSG_SUCCESS;
	bool retValue = false;
	recv_ctx_t* sub_ctx = NULL;
	zmq_handler::stZmqSubContext stsub_ctx;
	 std::vector<std::string> Topics;
	 zmq_handler::stZmqPubContext busPubCTX;
	 zmq_handler::stZmqSubContext busSubCTX;
	 zmq_handler::stZmqPubContext objPubContext;
	 std::string stValue = "0xFF00FF00";
	 std::string TValue = "0xFFFF";
	 unsigned char byte1;


	 zmq_handler::stZmqSubContext objTempSubCtx;
	 zmq_handler::stZmqSubContext *TempSubCtx = &objTempSubCtx;
	// objTempSubCtx.sub_ctx= sub_ctx;
	 publisher_ctx_t *pub_ctx = NULL;
	 uint16_t u16TransacID = 1;
//	 cJSON *root = cJSON_Parse(msg.c_str());
	virtual ~ZmqHandler_ut()
	{ };

};


#endif /* TEST_INCLUDE_ZMQHANDLER_UT_HPP_ */
