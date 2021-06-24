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

/*** Logger.hpp is used to log debug and error informations using `log4cpp` library*/

#ifndef INCLUDE_LOGGER_HPP_
#define INCLUDE_LOGGER_HPP_

#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>

#include <string>
#include <iostream>

#define LOGDETAILS(msg) "[ " + std::string(__FILE__) + " " + __func__ + " " + std::to_string(__LINE__) + "] " + std::string(msg)

#define DO_LOG_INFO(msg) { \
	if(CLogger::getInstance().isLevelSupported(log4cpp::Priority::INFO)) \
	{ \
		CLogger::getInstance().LogInfo(LOGDETAILS(msg)); \
	} \
}

#define DO_LOG_DEBUG(msg) { \
	if(CLogger::getInstance().isLevelSupported(log4cpp::Priority::DEBUG)) \
	{ \
		CLogger::getInstance().LogDebug(LOGDETAILS(msg)); \
	} \
}

#define DO_LOG_WARN(msg) { \
	if(CLogger::getInstance().isLevelSupported(log4cpp::Priority::WARN)) \
	{ \
		CLogger::getInstance().LogWarn(LOGDETAILS(msg)); \
	} \
}

#define DO_LOG_ERROR(msg) { \
	if(CLogger::getInstance().isLevelSupported(log4cpp::Priority::ERROR)) \
	{ \
		CLogger::getInstance().LogError(LOGDETAILS(msg)); \
	} \
}

#define DO_LOG_FATAL(msg) { \
	if(CLogger::getInstance().isLevelSupported(log4cpp::Priority::FATAL)) \
	{ \
		CLogger::getInstance().LogFatal(LOGDETAILS(msg)); \
	} \
}

/** class holds information of logger and log levels*/
class CLogger {
private:
	log4cpp::Category *logger; /** reference to the log4cpp*/
	bool m_bIsExternal; /** Is external or not(true or false)*/

	/** Private constructor so that no objects can be created.*/
	CLogger();
	CLogger(const CLogger & obj){logger = NULL;}
	CLogger& operator=(CLogger const&);
	bool configLogger(const char* a_pcLogPropsFilePath);

public:
	~CLogger();
	
	/**
	 * Static function to configures logger properties
	 * @param a_pcLogPropsFilePath :[in] path of logger properties file
	 * @return status: true - Success, false - error 
	 */
	static bool initLogger(const char* a_pcLogPropsFilePath)
	{
		CLogger& oLogger = getInstance();
		if(NULL != oLogger.logger)
		{
			std::cout << "Logger is already configured. Reconfiguration is not allowed.\n";
			return false;
		}	
		if(NULL != a_pcLogPropsFilePath)
		{
			return oLogger.configLogger(a_pcLogPropsFilePath);
		}
		else
		{
			std::cout << "Logger properties file path is not provided.\n";
			return false;
		}
		return true;
	}

	static CLogger& getInstance();

	void LogInfo(std::string msg);
	void LogDebug(std::string msg);
	void LogWarn(std::string msg);
	void LogError(std::string msg);
	void LogFatal(std::string msg);

	/** function to check level supported or not */
	bool isLevelSupported(int priority)
	{
		if(NULL != logger)
		{
			return logger->isPriorityEnabled(priority);
		}
		return false;
	}
};


#endif /* INCLUDE_LOGGER_HPP_ */
