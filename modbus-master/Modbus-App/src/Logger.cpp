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

CLogger::CLogger() {

	logger = NULL;
	// TODO Auto-generated constructor stub
	const char* logPropsFile = std::getenv("Log4cppPropsFile");
	if(logPropsFile == NULL) {
		std::cout << __func__ << " Log4cppPropsFile Environment Variable is not set, exiting application" << std::endl;
		exit(-1);
	}

	log4cpp::PropertyConfigurator::configure(logPropsFile);

	log4cpp::Category& root = log4cpp::Category::getRoot();

/*	log4cpp::Category& mqttExport =
		log4cpp::Category::getInstance(std::string("MQTT_Export"));*/

	logger = &root;
}
CLogger::~CLogger() {
	// TODO Auto-generated destructor stub

	log4cpp::Category::shutdown();

	if(logger != NULL)
		delete logger;
}

void CLogger::LogInfo(std::string msg) {
	logger->info(msg);
}

void CLogger::LogDebug(std::string msg) {
	logger->debug(msg);
}

void CLogger::LogWarn(std::string msg) {
	logger->warn(msg);
}

void CLogger::LogError(std::string msg) {
	logger->error(msg);
}

void CLogger::LogFatal(std::string msg) {
	logger->fatal(msg);
}

void CLogger::log(LogLevel lvl, std::string msg) {
	try {
		if(logger == NULL)	{
			std::cout << __func__ << " null logger instance \n";
			return;
		}

		switch (lvl) {
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

	} catch (std::exception &e) {
		std::cout << __func__ << ": Exception: " << e.what() << std::endl;
	}
}

