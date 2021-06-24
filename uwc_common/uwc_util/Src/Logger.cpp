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

#include "Logger.hpp"

/**
 * Constructor Initializes common variables
 * @param None
 * @return None
 */
CLogger::CLogger()
{
	m_bIsExternal = false;
	logger = NULL;
}

/**
 * Function for singleton instance.
 * @param None
 * @return singleton instance of logger
 */
CLogger& CLogger::getInstance() 
{
	static CLogger _self;
	return _self;
}

/**
 * Configures logger properties from log4cpp.properties file
 * @param a_pcLogPropsFilePath :[in] path of logger properties file
 * @return status: true - Success, false - error 
 */
bool CLogger::configLogger(const char* a_pcLogPropsFilePath)
{
	logger = NULL;

	if(NULL != a_pcLogPropsFilePath)
	{
		try
		{
			log4cpp::PropertyConfigurator::configure(a_pcLogPropsFilePath);

			log4cpp::Category& root = log4cpp::Category::getRoot();

			std::cout << "Log level is set as: " << log4cpp::Priority::getPriorityName(root.getPriority()) << std::endl;

			logger = &root;
			m_bIsExternal = false;
			DO_LOG_INFO("Log level is set to ..." + log4cpp::Priority::getPriorityName(root.getPriority()));
			return true;
		}
		catch(std::exception& ex)
		{
			std::cout << "Exception in logger creation: " << ex.what() << "\nLogging functionality will not work.\n";
		}
	}
	else
	{
		std::cout << __func__ << " Path to Log4cppPropsFile is not set.\nLogging functionality will not work.\n";
	}
	return false;
}

/**
 * Destructor remove all the appenders and shut down logger
 */
CLogger::~CLogger()
{}

/**
 * Write statement with info level
 * @param msg :[in] statement to log
 * @return None
 */
void CLogger::LogInfo(std::string msg)
{
	if(NULL != logger)
	{
		logger->info(msg);
	}
}

/**
 * Write statement with debug level
 * @param msg :[in] statement to log
 * @return None
 */
void CLogger::LogDebug(std::string msg)
{
	if(NULL != logger)
	{
		logger->debug(msg);
	}
}

/**
 * Write statement with warn level
 * @param msg :[in] statement to log
 * @return None
 */
void CLogger::LogWarn(std::string msg)
{
	if(NULL != logger)
	{
		logger->warn(msg);
	}
}

/**
 * Write statement with error level
 * @param msg :[in] statement to log
 * @return None
 */
void CLogger::LogError(std::string msg)
{
	if(NULL != logger)
	{
		logger->error(msg);
	}
}

/**
 * Write statement with fatal level
 * @param msg :[in] statement to log
 * @return None
 */
void CLogger::LogFatal(std::string msg)
{
	if(NULL != logger)
	{
		logger->fatal(msg);
	}
}
