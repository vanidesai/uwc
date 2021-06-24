/********************************************************************************
* Copyright (c) 2021 Intel Corporation.

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*********************************************************************************/


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

