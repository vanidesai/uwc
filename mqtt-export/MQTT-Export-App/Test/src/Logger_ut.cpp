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

TEST(Logger_ut, Log_Fatal)
{
	CLogger::getInstance().log(FATAL, "Fstal Error");
}

TEST(Logger_ut, Log_Default)
{
	CLogger::getInstance().log(23, "Default");
}
