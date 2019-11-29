/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/


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

#include "BoostLogger.hpp"
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
#include "YamlUtil.h"
#include "ConfigManager.hpp"
#include "ModbusWriteHandler.hpp"

extern "C" {
#include <safe_lib.h>
}

// for boost logging
extern src::severity_logger< severity_level > lg;

void populatePollingRefData()
{
	BOOST_LOG_SEV(lg, debug) << __func__ << "Start";
	
	using network_info::CUniqueDataPoint;
	using network_info::eEndPointType;
	// 1. get unique point list
	// 2. check if polling is enabled for that point
	// 3. check if zmqcontext is available
	// 4. if 2 and 3 are yes, create polling ref data

	const std::map<std::string, CUniqueDataPoint> &mapUniquePoint = network_info::getUniquePointList();
	for(auto pt: mapUniquePoint)
	{
		const CUniqueDataPoint &a = mapUniquePoint.at(pt.first);
		if(0 == a.getDataPoint().getPollingConfig().m_uiPollFreq)
		{
			BOOST_LOG_SEV(lg, info) << __func__ << "Polling is not set for " << a.getDataPoint().getID();
			// Polling frequency is not set
			continue; // go to next point
		}
		try
		{
			std::string sTopic(a.getWellSite().getID() + SEPARATOR_CHAR +
								a.getWellSiteDev().getID());
			BOOST_LOG_SEV(lg, debug) << __func__ << "Topic for context search: " << sTopic;
			std::cout << "Topic for context search: " << sTopic << std::endl;
			zmq_handler::stZmqContext &busCTX = zmq_handler::getCTX(sTopic);

			uint8_t uiFuncCode;
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

			CRefDataForPolling objRefPolling{mapUniquePoint.at(pt.first), busCTX, uiFuncCode};

			CTimeMapper::instance().insert(a.getDataPoint().getPollingConfig().m_uiPollFreq, objRefPolling);
			BOOST_LOG_SEV(lg, info) << __func__ << "Polling is set for " << a.getDataPoint().getID() << ", FunctionCode " << (unsigned)uiFuncCode
						<< ", frequency " << a.getDataPoint().getPollingConfig().m_uiPollFreq;
		}
		catch(std::exception &e)
		{
			BOOST_LOG_SEV(lg, error) << __func__ << "Exception '" << e.what() << "' in processing " << a.getDataPoint().getID();
			std::cout << e.what() << std::endl;
		}
	}
	
	BOOST_LOG_SEV(lg, debug) << __func__ << "End";
}

int main(int argc, char* argv[])
{
	BOOST_LOG_SEV(lg, debug) << __func__ << "Start";
	try
	{
		initLogging();
		logging::add_common_attributes();

		uint8_t	u8ReturnType = AppMbusMaster_StackInit();
		if(0 != u8ReturnType)
		{
			BOOST_LOG_SEV(lg, error) << __func__ << "Exiting. Failed to initialize modbus stack:" << u8ReturnType;
			std::cout << "Error: Exiting. Failed to initialize modbus stack:" << u8ReturnType << std::endl;
			exit(1);
		}

		// Initializing all the pub/sub topic base context for ZMQ
		if(getenv("PubTopics") != NULL)
		{
			BOOST_LOG_SEV(lg, error) << __func__ << " List of topic configured for Pub are :: " << getenv("PubTopics");
			bool bRes = zmq_handler::prepareCommonContext("pub");
			if(!bRes)
			{
				BOOST_LOG_SEV(lg, error) << __func__ << " Context creation failed for pub topic ";
			}
		}
		if(getenv("SubTopics") != NULL)
		{
			BOOST_LOG_SEV(lg, error) << __func__ << " List of topic configured for Sub are :: " << getenv("SubTopics");
			bool bRetVal = zmq_handler::prepareCommonContext("sub");
			if(!bRetVal)
			{
				BOOST_LOG_SEV(lg, error) << __func__ << " Context creation failed for sub topic ";
			}
		}

		std::string AppName(APP_NAME);
		std::string sDirToRegister = "/" + AppName + DIR_PATH;

		/// register callback for ETCD
		CfgManager::Instance().registerCallbackOnChangeDir(sDirToRegister.c_str());

		// get the environment variable
		if(const char* env_p = std::getenv("TCP_ENABLED"))
		{
			int32_t imode = atoi(env_p);
			if(imode == 1)
			{
				/// store the yaml files in data structures
				network_info::buildNetworkInfo(true);
				BOOST_LOG_SEV(lg, info) << __func__ << "Modbus container application is set to TCP mode";
				cout << "Modbus container application is set to TCP mode.." << endl;
			}
			else if (imode == 0)
			{
				BOOST_LOG_SEV(lg, fatal) << __func__ << "RTU mode is not supported currently..Use TCP mode only";
				cout << "RTU mode is not supported currently.. Use TCP mode only...Exiting ..." << endl;
				exit(1);
			}
			else
			{
				BOOST_LOG_SEV(lg, fatal) << __func__ << "Invalid value for TCP_ENABLED environment variable..it should be 0/1";
				cout << "Invalid value for TCP_ENABLED environment variable..it should be 0 or 1"
						"(i.e. 0 for RTU and 1 for TCP mode )......Exiting ..." << endl;
				exit(1);
			}
		}
		else
		{
			BOOST_LOG_SEV(lg, fatal) << __func__ << "TCP_ENABLED Environment variable is not set..Exiting";
			cout << "TCP_ENABLED Environment variable is not set..Exiting" << endl;
			exit(1);
		}

		// ZMQ contexts are built
		// Network device data and unique point data are also available
		// Lets build: reference data for polling
		populatePollingRefData();


		if(false == CPeriodicReponseProcessor::Instance().isInitialized())
		{
			BOOST_LOG_SEV(lg, debug) << __func__ << "CPeriodicReponseProcessor is not initialized";
		}
		else
		{
			BOOST_LOG_SEV(lg, debug) << __func__ << "CPeriodicReponseProcessor is properly initialized";
			CPeriodicReponseProcessor::Instance().initRespHandlerThreads();
		}

		BOOST_LOG_SEV(lg, info) << __func__ << "Configuration done. Starting operations.";
		CTimeMapper::instance().initTimerFunction();

		modWriteHandler::Instance().initZmqReadThread();

		while(1)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
	catch (const std::exception &e)
	{
		BOOST_LOG_SEV(lg, error) << __func__ << "fatal::Error in getting arguments: ";
		return 0;
	}
	BOOST_LOG_SEV(lg, debug) << __func__ << "End";

	return EXIT_SUCCESS;
}
