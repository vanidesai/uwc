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
	m_bIsExternal = false;
	logger = NULL;
}

/**
 * Configures logger properties
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
			return true;
		}
		catch(exception& ex)
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
{
	if(NULL != logger)
	{
		if(false == m_bIsExternal)
		{
			//logger->removeAllAppenders();
			//logger->shutdownForced();
		}
		else
		{
			// No Action. This is external logger.
		}
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
