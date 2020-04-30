/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#include "Logger.hpp"

/**
 * Constructor Initializes logger reading properties from .properties file
 * @param None
 * @return None
 */
CLogger::CLogger()
{
	logger = NULL;

	const char* logPropsFile = std::getenv("Log4cppPropsFile");
	if(logPropsFile == NULL)
	{
		std::cout << __func__ << " Log4cppPropsFile Environment Variable is not set, exiting application" << std::endl;
		exit(-1);
	}

	log4cpp::PropertyConfigurator::configure(logPropsFile);

	log4cpp::Category& root = log4cpp::Category::getRoot();

	std::cout << "Log level is set as: " << log4cpp::Priority::getPriorityName(root.getPriority()) << std::endl;

	logger = &root;
}

/**
 * Destructor remove all the appenders and shut down logger
 */
CLogger::~CLogger()
{
	if(NULL != logger)
	{
		logger->removeAllAppenders();
		logger->shutdownForced();
	}
}

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
