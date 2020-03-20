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
 * Constructor
 */
CLogger::CLogger()
{
	logger = NULL;
	// TODO Auto-generated constructor stub
	const char* logPropsFile = std::getenv("Log4cppPropsFile");
	if(logPropsFile == NULL)
	{
		std::cout << __func__ << " Log4cppPropsFile Environment Variable is not set, exiting application" << std::endl;
		exit(-1);
	}

	log4cpp::PropertyConfigurator::configure(logPropsFile);

	log4cpp::Category& root = log4cpp::Category::getRoot();

	logger = &root;
}

/**
 * Destructor
 */
CLogger::~CLogger()
{
	// TODO Auto-generated destructor stub
	logger->removeAllAppenders();

	logger->shutdownForced();
}

/**
 * Write statement with info level
 * @param msg :[in] statement to log
 */
void CLogger::LogInfo(std::string msg)
{
	logger->info(msg);
}

/**
 * Write statement with debug level
 * @param msg :[in] statement to log
 */
void CLogger::LogDebug(std::string msg)
{
	logger->debug(msg);
}

/**
 * Write statement with warn level
 * @param msg :[in] statement to log
 */
void CLogger::LogWarn(std::string msg)
{
	logger->warn(msg);
}

/**
 * Write statement with error level
 * @param msg :[in] statement to log
 */
void CLogger::LogError(std::string msg)
{
	logger->error(msg);
}

/**
 * Write statement with fatal level
 * @param msg :[in] statement to log
 */
void CLogger::LogFatal(std::string msg)
{
	logger->fatal(msg);
}

/**
 * Write statements in log file with given log level
 * @param lvl :[in] log level
 * @param msg :[in] statement to log
 */
void CLogger::log(LogLevel lvl, std::string msg)
{
	try
	{
		if(logger == NULL)
		{
			std::cout << __func__ << " null logger instance \n";
			return;
		}

		switch (lvl)
		{
		case INFO:
			logger->info(msg);
			break;
		case DEBUG:
			logger->debug(msg);
			break;
		case WARN:
			logger->warn(msg);
			break;
		case ERROR:
			logger->error(msg);
			break;
		case FATAL:
			logger->fatal(msg);
			break;
		default:
			std::cout << __func__ << " Invalid log level \n";
		}

	}
	catch (std::exception &e)
	{
		std::cout << __func__ << ":" << __LINE__ << "Exception : " << e.what() << std::endl;
	}
}
