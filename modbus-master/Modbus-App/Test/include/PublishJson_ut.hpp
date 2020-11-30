/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#ifndef TEST_INCLUDE_PUBLISHJSON_UT_HPP_
#define TEST_INCLUDE_PUBLISHJSON_UT_HPP_


#include <mutex>
#include <string.h>
#include <stdlib.h>
#include "PublishJson.hpp"
#include "ZmqHandler.hpp"
#include "PeriodicReadFeature.hpp"
#include "PeriodicRead.hpp"
#include "NetworkInfo.hpp"
#include "Common.hpp"
#include "API.h"
#include "gtest/gtest.h"

extern bool isElementExistInJson(cJSON *root, std::string a_sKeyName);



class PublishJson_ut : public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();

public:

	msg_envelope_t *msg;

	msg_envelope_t* g_msg = NULL;
	stStackResponse res;
	std::string sRespTopic = "Modbus-TCP-Master_ReadRequest";


};


#endif /* TEST_INCLUDE_PUBLISHJSON_UT_HPP_ */
