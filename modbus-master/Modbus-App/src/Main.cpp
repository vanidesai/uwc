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
#include <signal.h>
#include "ModbusOnDemandHandler.hpp"
#include "YamlUtil.hpp"
#include "ConfigManager.hpp"
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

#define APP_VERSION "0.0.5.5"
#define TIMER_TICK_FREQ 1000 // in microseconds

/// flag to stop all running threads
extern std::atomic<bool> g_stopThread;

/**
 * Populate polling data
 */
void populatePollingRefData()
{
	DO_LOG_DEBUG("Start");

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
			DO_LOG_INFO("Polling is not set for "+ a.getDataPoint().getID());
			// Polling frequency is not set
			continue; // go to next point
		}
		try
		{
			std::cout << iCount++ << ". Point to poll: " << a.getID() << ", RT: "
					<< a.getDataPoint().getPollingConfig().m_bIsRealTime << ", Freq: "
					<< a.getDataPoint().getPollingConfig().m_uiPollFreq << std::endl;

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

			CRefDataForPolling objRefPolling{a, uiFuncCode};

			CTimeMapper::instance().insert(a.getDataPoint().getPollingConfig().m_uiPollFreq, objRefPolling);

			DO_LOG_INFO("Polling is set for " +
						a.getDataPoint().getID() +
						", FunctionCode " +
						to_string((unsigned)uiFuncCode) +
						", frequency " +
						to_string(a.getDataPoint().getPollingConfig().m_uiPollFreq) +
						", RT " +
						to_string(a.getDataPoint().getPollingConfig().m_bIsRealTime));
		}
		catch(std::exception &e)
		{
			DO_LOG_FATAL("Exception " +
						(string)e.what() +
						"in processing " +
						a.getDataPoint().getID());
		}
	}

	DO_LOG_DEBUG("End");
}

/**
 * checks whether to stop thread execution
 * @return 	true : on success,
 * 			false : on error
 */
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
	if(NULL == pEnvVarName)
	{
		return false;
	}
	bool bRetVal = false;
	char *cEvar = getenv(pEnvVarName);
	if (NULL != cEvar)
	{
		bRetVal = true;
		std::string tmp (cEvar);
		storeVal = tmp;
		DO_LOG_INFO(std::string(pEnvVarName) + " environment variable is set to ::" + storeVal);
		std::cout << std::string(pEnvVarName) + " environment variable is set to ::" + storeVal << endl;
	}
	else
	{
		DO_LOG_ERROR(std::string(pEnvVarName) + " environment variable is not found");
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
	string devMode;
	std::list<std::string> topicList{"DEVICES_GROUP_LIST_FILE_NAME", "DEV_MODE", "NETWORK_TYPE"};
	std::map <std::string, std::string> envTopics;

	try
	{
		for (auto &topic : topicList)
		{
			std::string envVar = "";
			bRetVal = readEnvVariable(topic.c_str(), envVar);
			if(!bRetVal)
			{
				//return false;
				bRetVal = true;
			}
			else
			{
				envTopics.emplace(topic, envVar);
			}
		}

		if(envTopics.find("DEVICES_GROUP_LIST_FILE_NAME") != envTopics.end())
		{
			PublishJsonHandler::instance().setSiteListFileName(envTopics.at("DEVICES_GROUP_LIST_FILE_NAME"));
		}
		if(envTopics.find("NETWORK_TYPE") != envTopics.end())
		{
			PublishJsonHandler::instance().setnetworkType(envTopics.at("NETWORK_TYPE"));
		}
		if(envTopics.find("DEV_MODE") != envTopics.end())
		{
			devMode = envTopics.at("DEV_MODE");
		}

		transform(devMode.begin(), devMode.end(), devMode.begin(), ::toupper);

		if (devMode == "TRUE")
		{
			PublishJsonHandler::instance().setDevMode(true);
			DO_LOG_INFO("DEV_MODE is set to true");
			cout << "DEV_MODE is set to true\n";

		}
		else if (devMode == "FALSE")
		{
			PublishJsonHandler::instance().setDevMode(false);
			DO_LOG_INFO("DEV_MODE is set to false");
			cout << "DEV_MODE is set to false\n";
		}
		else
		{
			/// default set to false
			DO_LOG_ERROR("Invalid value for DEV_MODE env variable");
			DO_LOG_INFO("Set the dev mode to default (i.e. true)");
			cout << "DEV_MODE is set to default false\n";
		}
	}
	catch (std::exception &e)
	{
		DO_LOG_ERROR("Error while reading env. variable. "+ std::string(e.what()));
		bRetVal = false;
	}

	return bRetVal;
}

/**
 * Populate device contexts
 */
void setDevContexts()
{
	DO_LOG_DEBUG("Start");

	using network_info::eNetworkType;
	using network_info::CWellSiteDevInfo;
	using network_info::CWellSiteInfo;
	// 1. Set context for each device - TCP and RTU
	// 2. For RTU, for each network context is obtained and then set in each RTU device.
	// 3. For TCP, for each device, a different context is set.
	auto &siteList = network_info::getWellSiteList();
	for(auto &site: siteList)
	{
		auto &listDev = site.second.getDevices();
		for(auto &dev : listDev)
		{
			if(network_info::eNetworkType::eTCP == dev.getAddressInfo().m_NwType)
			{
#ifdef MODBUS_STACK_TCPIP_ENABLED
				int iCtx = 0;
				/// create context for unique ip-address and port number.
				stCtxInfo objCtxInfo{0};
				unsigned char	u8IpAddr[4];
				CommonUtils::ConvertIPStringToCharArray(dev.getAddressInfo().m_stTCP.m_sIPAddress, &(u8IpAddr[0]));

				objCtxInfo.u16Port = dev.getAddressInfo().m_stTCP.m_ui16PortNumber;
				objCtxInfo.pu8SerIpAddr = u8IpAddr;
				eStackErrorCode retValue = getTCPCtx(&iCtx, &objCtxInfo);
				if(STACK_NO_ERROR != retValue)
				{
					std::cout << dev.getID() << ": Unable to create context. Error: " << retValue << std::endl;
					DO_LOG_ERROR(dev.getID() + ": Unable to create context. Error: " + to_string(retValue));
				}
				else
				{
					dev.setCtxInfo(iCtx);
					DO_LOG_INFO(dev.getID() + ": Context is set");
				}
#endif
			}
			else
			{
#ifndef MODBUS_STACK_TCPIP_ENABLED
				eParity parity = eNone;
				const string sParity = dev.getRTUNwInfo().getParity();
				int iRTUCTX;
				if(!(sParity.empty() && dev.getRTUNwInfo().getPortName().empty()) && dev.getRTUNwInfo().getBaudRate() >= 0)
				{
					if(!(sParity == "N" || sParity == "n" ||
							sParity == "E" || sParity == "e" ||
							sParity == "O" || sParity == "o"))
					{
						DO_LOG_ERROR("Set Parity is wrong for RTU. Set correct parity N/E/O");
						std::cout << "Set Parity \"" << sParity << "\" is wrong for RTU. Set correct parity N/E/O" << endl;
					}
					else
					{
						parity = (sParity == "N" || sParity == "n") ? eNone : (sParity == "O" || sParity == "o") ? eOdd : eEven;
						
						stCtxInfo objCtxInfo{0};
						objCtxInfo.m_eParity = parity;
						objCtxInfo.m_lInterframeDelay = dev.getRTUNwInfo().getInterframeDelay();
						objCtxInfo.m_lRespTimeout = dev.getRTUNwInfo().getResTimeout();
						objCtxInfo.m_u32baudrate = dev.getRTUNwInfo().getBaudRate();
						objCtxInfo.m_u8PortName = (uint8_t*)((dev.getRTUNwInfo().getPortName()).c_str());

						eStackErrorCode retValue = getRTUCtx(&iRTUCTX, &objCtxInfo);

						if(STACK_NO_ERROR != retValue)
						{
							std::cout << "RTU: Unable to create context. Error: " << retValue << std::endl;
							DO_LOG_ERROR("RTU: Unable to create context. Error: " + to_string(retValue));
						}
						else
						{
							dev.setCtxInfo(iRTUCTX);
							DO_LOG_INFO(dev.getID() + ": Context is set");
						}
					}
				}
				else
				{
					std::cout << "RTU: configuration is not proper." << std::endl;
					DO_LOG_ERROR("RTU: configuration is not proper.");
				}
#endif
			}
		}
	}

	DO_LOG_DEBUG("End");
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
	DO_LOG_DEBUG("Start");

	try
	{
		DO_LOG_DEBUG("Starting Modbus_App ...");

		DO_LOG_INFO("Modbus container app version is set to :: " + std::string(APP_VERSION));
		cout <<"\nModbus container app version is :: " + std::string(APP_VERSION) << "\n"<<endl;

		// load global configuration for container real-time setting
		bool bRetVal = globalConfig::loadGlobalConfigurations();
		if(!bRetVal)
		{
			DO_LOG_INFO("Global configuration is set with some default parameters");
			cout << "\nGlobal configuration is set with some default parameters\n\n";
		}
		else
		{
			DO_LOG_INFO("Global configuration is set successfully");
			cout << "\nGlobal configuration for container real-time is set successfully\n\n";
		}

		string sAppName;
		if(!CommonUtils::readEnvVariable("AppName", sAppName))
		{
			exit(1);
		}

		PublishJsonHandler::instance().setAppName(sAppName);

		if (!CommonUtils::readCommonEnvVariables())
		{
			DO_LOG_ERROR("Required env variables are not set.");
			std::cout << "Required common env variables are not set.\n";
		}

		string cutOff;
		if(!CommonUtils::readEnvVariable("CUTOFF_INTERVAL_PERCENTAGE", cutOff))
		{
			DO_LOG_INFO("CUTOFF_INTERVAL_PERCENTAGE env variables are not set.");
			cout << "CUTOFF_INTERVAL_PERCENTAGE env variables are not set."<<endl;
			std::cout << "setting it to default i.e. 90 \n";
			DO_LOG_INFO("setting it to default i.e. 90");
			PublishJsonHandler::instance().setCutoffIntervalPercentage(90);
		}
		else
		{
			PublishJsonHandler::instance().setCutoffIntervalPercentage(atoi(cutOff.c_str()));
		}
		std::cout << "Cutoff is set to: "
				<< PublishJsonHandler::instance().getCutoffIntervalPercentage() << std::endl;

		// Initializing all the pub/sub topic base context for ZMQ
		if(const char* pcPubTopic = std::getenv("PubTopics"))
		{
			DO_LOG_INFO("List of topic configured for Pub are :: " + std::string(pcPubTopic));

			bool bRes = zmq_handler::prepareCommonContext("pub");
			if(!bRes)
			{
				DO_LOG_ERROR("Context creation failed for pub topic ");
			}
		}
		if(const char* pcSubTopic = std::getenv("SubTopics"))
		{
			DO_LOG_INFO("List of topic configured for Sub are :: " + std::string(pcSubTopic));

			bool bRetVal = zmq_handler::prepareCommonContext("sub");
			if(!bRetVal)
			{
				DO_LOG_ERROR("Context creation failed for sub topic ");
			}
		}

#ifdef MODBUS_STACK_TCPIP_ENABLED
		/// store the yaml files in data structures
		//network_info::buildNetworkInfo(true);

		// Setting RTU mode
		if(!PublishJsonHandler::instance().getSiteListFileName().empty())
		{
			network_info::buildNetworkInfo(PublishJsonHandler::instance().getnetworkType(),
					PublishJsonHandler::instance().getSiteListFileName());
			DO_LOG_INFO("Modbus container application is set to TCP mode");
			cout << "Modbus container application is set to TCP mode.." << endl;
		}
		else
		{
			cout << "Devices group list is not present\n";
			DO_LOG_INFO("Devices group list is not present");
		}

#else

		// Setting RTU mode
		if(!PublishJsonHandler::instance().getSiteListFileName().empty())
		{
			network_info::buildNetworkInfo(PublishJsonHandler::instance().getnetworkType(),
					PublishJsonHandler::instance().getSiteListFileName());
			DO_LOG_INFO("Modbus container application is set to RTU mode");
			cout << "Modbus container application is set to RTU mode.." << endl;
		}
		else
		{
			cout << "Devices group list is not present\n";
			DO_LOG_INFO("Devices group list is not present");
		}

#endif

		// set TCP/RTU context.
		setDevContexts();

		// get interframe delay and response timeout
		long lInterfameDelay = 0, lRespTimeout = 80;
		auto &siteList = network_info::getWellSiteList();
		for(auto &site: siteList)
		{
			auto &listDev = site.second.getDevices();
			for(auto &dev : listDev)
			{
				if(network_info::eNetworkType::eTCP == dev.getAddressInfo().m_NwType)
				{
					lInterfameDelay = dev.getTcpMasterInfo().m_lInterframeDelay;
					lRespTimeout = dev.getTcpMasterInfo().m_lResTimeout;
				}
			}
		}
		stDevConfig_t stDevConf;
		stDevConf.m_lInterframedelay = lInterfameDelay;
		stDevConf.m_lResponseTimeout = lRespTimeout;
		if(STACK_NO_ERROR != AppMbusMaster_SetStackConfigParam(&stDevConf))
		{
			std::cout << "Error: Exiting. Failed to set stack  config parameters"<< std::endl;
			DO_LOG_ERROR("Failed to set stack  config parameters");
			return -1;
		}
		else
		{
			std::cout << "Success :: modbus stack set config successful" << std::endl;
			DO_LOG_INFO("modbus stack set config successful");
		}

		uint8_t	u8ReturnType = AppMbusMaster_StackInit();
		if(0 != u8ReturnType)
		{
			DO_LOG_ERROR("Exiting. Failed to initialize modbus stack:" +
						to_string(u8ReturnType));

			std::cout << "Error: Exiting. Failed to initialize modbus stack:" << (unsigned)u8ReturnType << std::endl;
			exit(1);
		}
		else
		{
			std::cout << "\nSuccess :: modbus stack initialization successful" << std::endl;
			DO_LOG_INFO("modbus stack initialization successful");
		}

		if(false == onDemandHandler::Instance().isWriteInitialized())
		{
			DO_LOG_ERROR("modWriteHandler is not initialized");
		}
		else
		{
			DO_LOG_DEBUG("modWriteHandler is properly initialized");
			/// On-Demand request initializer thread.
			onDemandHandler::Instance().createOnDemandListener();
		}

		// ZMQ contexts are built
		// Network device data and unique point data are also available
		// Lets build: reference data for polling
		populatePollingRefData();

		if(false == CPeriodicReponseProcessor::Instance().isInitialized())
		{
			DO_LOG_ERROR("CPeriodicReponseProcessor is not initialized");
		}
		else
		{
			DO_LOG_INFO("CPeriodicReponseProcessor is properly initialized");
			CPeriodicReponseProcessor::Instance().initRespHandlerThreads();
		}


#ifdef UNIT_TEST
		//::testing::GTEST_FLAG(filter) ="*CConfigManager_ut*";
		::testing::InitGoogleTest(&argc, argv);
		return RUN_ALL_TESTS();
#endif

		DO_LOG_INFO("Configuration done. Starting operations.");
		CTimeMapper::instance().initTimerFunction();

		// Get best possible polling frequency
		uint32_t ulMinFreq = CTimeMapper::instance().getMinTimerFrequency();
		std::cout << "Best possible minimum frequency in milliseconds: " << ulMinFreq << std::endl;
		std::cout << "Starting periodic timer\n";
		PeriodicTimer::timer_start(ulMinFreq);
		std::cout << "Timer is started..\n";

		std::unique_lock<std::mutex> lck(mtx);
		cv.wait(lck,exitMainThread);

		DO_LOG_INFO("Condition variable is set for application exit.");

		DO_LOG_INFO("Exiting the application gracefully.");
		cout << "************************************************************************************************" <<endl;
		cout << "********************** Exiting modbus container ***********" <<endl;
		cout << "************************************************************************************************" <<endl;

		return EXIT_SUCCESS;
	}
	catch (const std::exception &e)
	{
		std::cout << "Exception in main::"<<"fatal::Error in getting arguments: "<<e.what()<< endl;
		DO_LOG_FATAL("fatal::Error in getting arguments: " +
					(string)e.what());

		return EXIT_FAILURE;
	}
	DO_LOG_DEBUG("End");

	return EXIT_SUCCESS;
}
