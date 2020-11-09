/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/


#include "../include/Logger_ut.hpp"


void Logger_ut::SetUp()
{
	// Setup code
}

void Logger_ut::TearDown()
{
	// TearDown code
}

/**Test for initLogger()**/
TEST_F(Logger_ut, initLogger)
{
	CLogger::getInstance().initLogger("Config/log4cpp.properties");

	/// to handle else block of Logger
	CLogger::getInstance().initLogger("Config/log4cpp_1.properties");

	/// to handle scenario where logger is already created
	CLogger::getInstance().initLogger("Config/log4cpp.properties");
}

/**Test for LogInfo()**/
TEST_F(Logger_ut, logINFO)
{
	CLogger::getInstance().LogInfo(Infomsg);
}

/**Test for LogDebug()**/
TEST_F(Logger_ut, logDEBUG)
{
	CLogger::getInstance().LogDebug(Debugmsg);
}

/**Test for LogWarn()**/
TEST_F(Logger_ut, logWARN)
{
	CLogger::getInstance().LogWarn(Warnmsg);
}

/**Test for LogFatal()**/
TEST_F(Logger_ut, logFATAL)
{
	CLogger::getInstance().LogFatal(Fatalmsg);
}

/**Test for LogError()**/
TEST_F(Logger_ut, logERROR)
{
	CLogger::getInstance().LogError(Errormsg);
}

