/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef TEST_INCLUDE_EISPLBUSHANDLER_UT_HPP_
#define TEST_INCLUDE_EISPLBUSHANDLER_UT_HPP_

#include "EISPlBusHandler.hpp"
#include "gtest/gtest.h"


class EISPlBusHandler_ut : public ::testing::Test{
protected:
	virtual void SetUp();
	virtual void TearDown();
public:
	CEISPlBusHandler CEISPlBusHandler_obj;
	std::string strMsg = "{ 	\"value\": \"0xFF00\", 	\"command\": \"Pointname\", 	\"app_seq\": \"1234\" }";

};

#endif /* TEST_INCLUDE_EISPLBUSHANDLER_UT_HPP_ */
