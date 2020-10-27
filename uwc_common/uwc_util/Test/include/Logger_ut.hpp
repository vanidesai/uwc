/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#ifndef TEST_INCLUDE_LOGGER_UT_HPP_
#define TEST_INCLUDE_LOGGER_UT_HPP_

#include "Logger.hpp"
#include <string>
#include <stdlib.h>
#include <map>
#include <thread>
#include "gtest/gtest.h"

class Logger_ut : public ::testing::Test {
protected:
	virtual void SetUp();
	virtual void TearDown();
public:
	std::string Infomsg = "Logger INFO";
	std::string Debugmsg = "Logger DEBUG";
	std::string Warnmsg = "Logger WARN";
	std::string Fatalmsg = "Logger FATAL";
	std::string Errormsg = "Logger ERROR";

};


#endif /* TEST_INCLUDE_LOGGER_UT_HPP_ */
