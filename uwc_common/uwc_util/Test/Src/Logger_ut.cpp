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

/*********In Logger functions there is nothing to check through the unit test cases
 therefore functions are just only called in the unit test cases for the seck of coverage of uncovered
 functions from Logger.cpp file.

 *********/

TEST_F(Logger_ut, initLogger)
{
	CLogger::getInstance().initLogger("Config/log4cpp.properties");

	/// to handle else block of Logger
	CLogger::getInstance().initLogger("Config/log4cpp_1.properties");

	/// to handle scenario where logger is already created
	CLogger::getInstance().initLogger("Config/log4cpp.properties");

	CLogger::getInstance().initLogger(CLogger::getInstance());
}

TEST_F(Logger_ut, logINFO)
{
	CLogger::getInstance().LogInfo(Infomsg);
}


TEST_F(Logger_ut, logDEBUG)
{
	CLogger::getInstance().LogDebug(Debugmsg);
}

TEST_F(Logger_ut, logWARN)
{
	CLogger::getInstance().LogWarn(Warnmsg);
}

TEST_F(Logger_ut, logFATAL)
{
	CLogger::getInstance().LogFatal(Fatalmsg);
}

TEST_F(Logger_ut, logERROR)
{
	CLogger::getInstance().LogError(Errormsg);
}

