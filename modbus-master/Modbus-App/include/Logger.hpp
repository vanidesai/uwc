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

enum LogLevel{
	INFO,
	DEBUG,
	WARN,
	ERROR,
	FATAL
};

#define LOGDETAILS(msg) "[ " + std::string(__FILE__) + " " + __func__ + " " + std::to_string(__LINE__) + "] " + std::string(msg)

class CLogger {
private:
	log4cpp::Category *logger;

	// Private constructor so that no objects can be created.
	CLogger();
	CLogger(const CLogger & obj){logger = NULL;}
	CLogger& operator=(CLogger const&);

public:
	virtual ~CLogger();

	static CLogger& getInstance() {
		static CLogger _self;
			return _self;
	}

	void log(LogLevel lvl, string msg);
	void LogInfo(std::string msg);
	void LogDebug(std::string msg);
	void LogWarn(std::string msg);
	void LogError(std::string msg);
	void LogFatal(std::string msg);

};


#endif /* INCLUDE_LOGGER_HPP_ */
