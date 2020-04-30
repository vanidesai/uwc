/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include <gtest/gtest.h>
#include "Logger.hpp"



TEST(Logger_ut, LogInfo)
{
	CLogger::getInstance().LogInfo("LogInfo_TestMsg");
}

TEST(Logger_ut, LogDebug)
{
	CLogger::getInstance().LogDebug("LogDebug_TestMsg");
}

TEST(Logger_ut, LogWarn)
{
	CLogger::getInstance().LogWarn("LogWarn_TestMsg");
}

TEST(Logger_ut, LogError)
{
	CLogger::getInstance().LogError("LogError_TestMsg");
}

TEST(Logger_ut, LogFatal)
{
	CLogger::getInstance().LogFatal("LogFatal_TestMsg");
}
