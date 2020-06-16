/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#ifndef INCLUDE_LOGGER_HPP_
#define INCLUDE_LOGGER_HPP_

#include <log4cpp/Category.hh>
#include <log4cpp/PropertyConfigurator.hh>

#include <string>

using namespace std;

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

class CLogger {
private:
	log4cpp::Category *logger;

	// Private constructor so that no objects can be created.
	CLogger();
	CLogger(const CLogger & obj){logger = NULL;}
	CLogger& operator=(CLogger const&);

public:
	~CLogger();

	static CLogger& getInstance() {
		static CLogger _self;
			return _self;
	}

	void LogInfo(std::string msg);
	void LogDebug(std::string msg);
	void LogWarn(std::string msg);
	void LogError(std::string msg);
	void LogFatal(std::string msg);

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
