/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "PeriodicReadFeature.hpp"
#include "NetworkInfo.hpp"
#include "ZmqHandler.hpp"
#include "PublishJson.hpp"
#include "PeriodicRead.hpp"
#include <netdb.h>
#include <ifaddrs.h>
#include <string.h>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include "YamlUtil.hpp"
#include "ConfigManager.hpp"
#include "ModbusWriteHandler.hpp"
#include "Logger.hpp"
#ifdef UNIT_TEST
#include <gtest/gtest.h>
#endif

extern "C" {
#include <safe_lib.h>
}

std::mutex mtx;
std::condition_variable cv;
bool g_stop = false;

/// flag to stop all running threads
extern std::atomic<bool> g_stopThread;

void populatePollingRefData()
{
	string temp;
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start"));

	using network_info::CUniqueDataPoint;
	using network_info::eEndPointType;
	// 1. get unique point list
	// 2. check if polling is enabled for that point
	// 3. check if zmqcontext is available
	// 4. if 2 and 3 are yes, create polling ref data

	const std::map<std::string, CUniqueDataPoint> &mapUniquePoint = network_info::getUniquePointList();
	int iCount = 1;
	for(auto &pt: mapUniquePoint)
	{
		const CUniqueDataPoint &a = mapUniquePoint.at(pt.first);
		if(0 == a.getDataPoint().getPollingConfig().m_uiPollFreq)
		{
			temp = "Polling is not set for ";
			temp.append(a.getDataPoint().getID());

			CLogger::getInstance().log(INFO, LOGDETAILS(temp));
			// Polling frequency is not set
			continue; // go to next point
		}
		try
		{

			std::string sTopic = PublishJsonHandler::instance().getPolledDataTopic();

			temp = "Topic for context search: ";
			temp.append(sTopic);

			CLogger::getInstance().log(DEBUG, LOGDETAILS(temp));

			std::cout << "Point to poll: " << a.getID() << ", " << iCount++ << std::endl;
			zmq_handler::stZmqContext &busCTX = zmq_handler::getCTX(sTopic);
			zmq_handler::stZmqPubContext &pubCTX = zmq_handler::getPubCTX(sTopic);

			uint8_t uiFuncCode = 0;
			switch(a.getDataPoint().getAddress().m_eType)
			{
			case network_info::eEndPointType::eCoil:
				uiFuncCode = 1;
				break;
			case network_info::eEndPointType::eDiscrete_Input:
				uiFuncCode = 2;
				break;
			case network_info::eEndPointType::eHolding_Register:
				uiFuncCode = 3;
				break;
			case network_info::eEndPointType::eInput_Register:
				uiFuncCode = 4;
				break;
			}

			CRefDataForPolling objRefPolling{a, busCTX, pubCTX, uiFuncCode};

			CTimeMapper::instance().insert(a.getDataPoint().getPollingConfig().m_uiPollFreq, objRefPolling);

			temp = "Polling is set for ";
			temp.append(a.getDataPoint().getID());
			temp.append(", FunctionCode ");
			temp.append(to_string((unsigned)uiFuncCode));
			temp.append(", frequency ");
			temp.append(to_string(a.getDataPoint().getPollingConfig().m_uiPollFreq));

			CLogger::getInstance().log(INFO, LOGDETAILS(temp));
		}
		catch(std::exception &e)
		{

			temp = "Exception '";
			temp.append(e.what());
			temp.append("' in processing ");
			temp.append(a.getDataPoint().getID());

			CLogger::getInstance().log(FATAL, LOGDETAILS(temp));
		}
	}

	CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));
}

/**
 * Signal handler
 */
void signal_handler(int signo)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start"));

	CLogger::getInstance().log(INFO, LOGDETAILS("Signal Handler called.."));

    std::unique_lock<std::mutex> lck(mtx);

    /// stop all running threads
    g_stopThread = true;
    AppMbusMaster_StackDeInit();

    /// stop the read periodic timer
    bool retVal = LinuxTimer::stop_timer();

    (retVal == true) ? std::cout<< "Periodic Timer is stopped successfully\n":std::cout<< "Error while stopping Periodic timer\n";

    CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));

    // Signal main to stop
    cv.notify_one();
}

/// function to check Main thread exit
bool exitMainThread(){return g_stopThread;};

/**
 *
 * DESCRIPTION
 * This function is used to check keys from given JSON
 *
 * @param root [in] argument count
 * @param a_sKeyName [in] key from JSON
 *
 * @return boolean [out] return true on success and false on failure
 */
bool isElementExistInJson(cJSON *root, std::string a_sKeyName)
{
	bool bRetVal = false;
	cJSON *pBaseptr = cJSON_GetObjectItem(root, a_sKeyName.c_str());
	if(NULL != pBaseptr)
	{
		// key found in json
		bRetVal = true;
	}
	return bRetVal;
}

/** This function is used to read environment variable
 *
 * @sEnvVarName : environment variable to be read
 * @storeVal : variable to store env variable value
 * @return: true/false based on success or error
 */
bool CommonUtils::readEnvVariable(const char *pEnvVarName, string &storeVal)
{
	bool bRetVal = false;
	char *cEvar = getenv(pEnvVarName);
	if (NULL != cEvar)
	{
		bRetVal = true;
		std::string tmp (cEvar);
		storeVal = tmp;
		CLogger::getInstance().log(INFO, LOGDETAILS(std::string(pEnvVarName) + " environment variable is set to ::" + storeVal));
		std::cout << std::string(pEnvVarName) + " environment variable is set to ::" + storeVal << endl;
	}
	else
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS(std::string(pEnvVarName) + " environment variable is not found"));
		cout << std::string(pEnvVarName) + " environment variable is not found" <<endl;
	}
	return bRetVal;
}

/** This function is used to read common environment variables
 *
 * @return: true/false based on success or error
 */
bool CommonUtils::readCommonEnvVariables()
{
	bool bRetVal = false;
	std::list<std::string> topicList{"PolledData", "ReadResponse", "WriteResponse",
		"ReadRequest", "WriteRequest", "SITE_LIST_FILE_NAME"};
	std::map <std::string, std::string> envTopics;

	for (auto topic : topicList)
	{
		std::string envVar = "";
		bRetVal = readEnvVariable(topic.c_str(), envVar);
		if(!bRetVal)
		{
			return false;
		}
		else
		{
			envTopics.emplace(topic, envVar);
		}
	}

	PublishJsonHandler::instance().setPolledDataTopic(envTopics.at("PolledData"));
	PublishJsonHandler::instance().setSReadResponseTopic(envTopics.at("ReadResponse"));
	PublishJsonHandler::instance().setSWriteResponseTopic(envTopics.at("WriteResponse"));
	PublishJsonHandler::instance().setSReadRequestTopic(envTopics.at("ReadRequest"));
	PublishJsonHandler::instance().setSWriteRequestTopic(envTopics.at("WriteRequest"));
	PublishJsonHandler::instance().setSiteListFileName(envTopics.at("SITE_LIST_FILE_NAME"));

	return bRetVal;
}
/**
 *
 * DESCRIPTION
 * This function is entry point for application
 *
 * @param argc [in] argument count
 * @param argv [in] argument value
 *
 * @return int [out] return 1 on success
 */
int main(int argc, char* argv[])
{
	CLogger::getInstance().log(DEBUG, "Start");
	string temp;

	try
	{
		CLogger::getInstance().log(DEBUG, LOGDETAILS("Starting Modbus_App ..."));

		string sAppVersion;
		if(!CommonUtils::readEnvVariable("APP_VERSION", sAppVersion))
		{
			exit(1);
		}

		CLogger::getInstance().log(INFO, LOGDETAILS("Modbus container app version is set to :: " + sAppVersion));
		cout <<"\nModbus container app version is :: " + sAppVersion << "\n"<<endl;

		string sAppName;
		if(!CommonUtils::readEnvVariable("AppName", sAppName))
		{
			exit(1);
		}

		PublishJsonHandler::instance().setAppName(sAppName);

		if (!CommonUtils::readCommonEnvVariables())
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("Required env variables are not set."));
			std::cout << "Required common env variables are not set.\n";
			exit(1);
		}

#ifndef MODBUS_STACK_TCPIP_ENABLED

		string sPortName, sBaudrate, sParity, sStopBit;
		if (!((CommonUtils::readEnvVariable("PORT_NAME", sPortName)) &&
				(CommonUtils::readEnvVariable("BAUD_RATE", sBaudrate)) &&
				(CommonUtils::readEnvVariable("PARITY", sParity)) &&
				(CommonUtils::readEnvVariable("STOPBIT", sStopBit))))
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("Required environment variables are not found for RTU"));
			std::cout << "Required environment variables are not found for RTU\n";
			exit(1);
		}
		else
		{
			CLogger::getInstance().log(INFO, LOGDETAILS("Required environment variables are found for RTU"));
		}
		cout << "********************************************************************"<<endl;
		cout << "Modbus RTU container is running with below configuration.."<<endl;
		cout<<"Baud rate = "<< stoi(sBaudrate)<< endl;
		cout<<"Port Name = "<< sPortName<< endl;
		cout<<"Parity = "<< stoi(sParity)<< endl;
		cout<<"StopBit = "<< stoi(sStopBit)<< endl;
		cout << "********************************************************************"<<endl;

		CLogger::getInstance().log(INFO, LOGDETAILS("Modbus RTU container is running with below configuration.."));

		CLogger::getInstance().log(INFO, LOGDETAILS("Baud rate = " + sBaudrate + " \n" +
				"Port Name = " + sPortName + " \n" + "Parity = " + sParity + " \n" + "Stop Bit =" + sStopBit));

		int fd = initSerialPort((uint8_t*)(sPortName.c_str()),
				stoi(sBaudrate),
				stoi(sParity),
				stoi(sStopBit));
		if(fd < 0)
		{
			cout << "Failed to initialize serial port for RTU."<<endl;
			cout << "Connect the RTU device to serial port"<<endl;
			cout << "Container will restart until the serial port is connected."<<endl;
			CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to initialize serial port for RTU."));

			CLogger::getInstance().log(ERROR, LOGDETAILS(temp = "File descriptor is set to ::" + to_string(fd)));

			cout << "Error:: File descriptor is set to :: " << fd << endl;
			return -1;
		}
		else
		{
			cout << "Initialize serial port for RTU is successful"<<endl;
			CLogger::getInstance().log(INFO, LOGDETAILS(temp = "File descriptor is set to ::" + to_string(fd)));
			cout << "File descriptor is set to :: " << fd << endl;
		}
#endif

		// Setup signal handlers
		signal(SIGUSR1, signal_handler);
		signal(SIGALRM, LinuxTimer::timer_callback);

		uint8_t	u8ReturnType = AppMbusMaster_StackInit();
		if(0 != u8ReturnType)
		{
			temp = "Exiting. Failed to initialize modbus stack:";
			temp.append(to_string(u8ReturnType));
			CLogger::getInstance().log(ERROR, LOGDETAILS(temp));

			std::cout << "Error: Exiting. Failed to initialize modbus stack:" << u8ReturnType << std::endl;
			exit(1);
		}

		// Initializing all the pub/sub topic base context for ZMQ
		if(const char* pcPubTopic = std::getenv("PubTopics"))
		{
			CLogger::getInstance().log(INFO, LOGDETAILS("List of topic configured for Pub are :: " + std::string(pcPubTopic)));

			bool bRes = zmq_handler::prepareCommonContext("pub");
			if(!bRes)
			{
				CLogger::getInstance().log(ERROR, LOGDETAILS("Context creation failed for pub topic "));
			}
		}
		if(const char* pcSubTopic = std::getenv("SubTopics"))
		{
			CLogger::getInstance().log(INFO, LOGDETAILS("List of topic configured for Sub are :: " + std::string(pcSubTopic)));

			bool bRetVal = zmq_handler::prepareCommonContext("sub");
			if(!bRetVal)
			{
				CLogger::getInstance().log(ERROR, LOGDETAILS("Context creation failed for sub topic "));
			}
		}

#ifdef MODBUS_STACK_TCPIP_ENABLED
		/// store the yaml files in data structures
		network_info::buildNetworkInfo(true);
		CLogger::getInstance().log(INFO, LOGDETAILS("Modbus container application is set to TCP mode"));
		cout << "Modbus container application is set to TCP mode.." << endl;
#else

		// Setting RTU mode
		network_info::buildNetworkInfo(false);
		CLogger::getInstance().log(INFO, LOGDETAILS("Modbus container application is set to RTU mode"));
		cout << "Modbus container application is set to RTU mode.." << endl;
#endif

		if(false == modWriteHandler::Instance().isWriteInitialized())
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("modWriteHandler is not initialized"));
		}
		else
		{
			CLogger::getInstance().log(DEBUG, LOGDETAILS("modWriteHandler is properly initialized"));
			/// Write request initializer thread.
			modWriteHandler::Instance().initWriteHandlerThreads();
		}
		/// listening thread for write
		modWriteHandler::Instance().createWriteListener();

		// ZMQ contexts are built
		// Network device data and unique point data are also available
		// Lets build: reference data for polling
		populatePollingRefData();

		if(false == CPeriodicReponseProcessor::Instance().isInitialized())
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("CPeriodicReponseProcessor is not initialized"));
		}
		else
		{
			CLogger::getInstance().log(INFO, LOGDETAILS("CPeriodicReponseProcessor is properly initialized"));
			CPeriodicReponseProcessor::Instance().initRespHandlerThreads();
		}

		CLogger::getInstance().log(INFO, LOGDETAILS("Configuration done. Starting operations."));
		CTimeMapper::instance().initTimerFunction();

		if(true == LinuxTimer::start_timer(1))
		{
			std::cout << "\nSuccessfully started read periodic timer\n";
		}
		else
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to start periodic read timer."));
			std::cout << "Error: Failed to start periodic read timer" << std::endl;
		}

#ifdef UNIT_TEST

		::testing::InitGoogleTest(&argc, argv);
		return RUN_ALL_TESTS();

#endif

		std::unique_lock<std::mutex> lck(mtx);
		cv.wait(lck,exitMainThread);

		CLogger::getInstance().log(INFO, LOGDETAILS("Condition variable is set for application exit."));

		CLogger::getInstance().log(INFO, LOGDETAILS("Exiting the application gracefully."));
		cout << "************************************************************************************************" <<endl;
		cout << "********************** Exited Modbus container to apply new configurations from ETCD ***********" <<endl;
		cout << "************************************************************************************************" <<endl;

		return EXIT_SUCCESS;
	}
	catch (const std::exception &e)
	{
		temp = "fatal::Error in getting arguments: ";
		temp.append(e.what());
		CLogger::getInstance().log(FATAL, LOGDETAILS(temp));

		return EXIT_FAILURE;
	}
	CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));

	return EXIT_SUCCESS;
}
