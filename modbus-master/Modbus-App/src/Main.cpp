/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

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
#include "YamlUtil.hpp"
#include "ConfigManager.hpp"
#include "ModbusWriteHandler.hpp"

extern "C" {
#include <safe_lib.h>
}

std::mutex mtx;
std::condition_variable cv;
bool g_stop = false;

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

/**
 * Signal handler
 */
void signal_handler(int signo)
{
	BOOST_LOG_SEV(lg, debug) << __func__ << " Start";

	BOOST_LOG_SEV(lg, info) << __func__ << " Signal Handler called..";

    std::unique_lock<std::mutex> lck(mtx);

    // Set stop flag
    g_stop = true;

    BOOST_LOG_SEV(lg, debug) << __func__ << "End";

    // Signal main to stop
    cv.notify_one();
}

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
	BOOST_LOG_SEV(lg, debug) << __func__ << "Start";
	try
	{
		initLogging();
		logging::add_common_attributes();

#ifndef MODBUS_STACK_TCPIP_ENABLED
		if(!CfgManager::Instance().IsClientCreated())
		{
			std::cout << __func__ << "ETCD client is not created ." <<endl;
			BOOST_LOG_SEV(lg, error) << __func__ << " ETCD client is not created";
			return -1;
		}
		char *cEtcdValue  = CfgManager::Instance().getETCDValuebyKey("/RTU_Master_Config");
		if((NULL == cEtcdValue) || (0 == *cEtcdValue))
		{
			std::cout << __func__ << "NULL JSON received from ETCD while reading RTU configuration data.." <<endl;
			BOOST_LOG_SEV(lg, error) << __func__ << " NULL JSON received from ETCD while reading RTU configuration data..";
			return -1;
		}
		else
		{
			//parse from root element
			cJSON *root = cJSON_Parse(cEtcdValue);
			if(NULL == root)
			{
				std::cout << __func__ << "Failed to parse JSON received from ETCD." <<endl;
				BOOST_LOG_SEV(lg, error) << __func__ << " Failed to parse JSON received from ETCD.";
				return -1;
			}

			BOOST_LOG_SEV(lg, info) << " Parsing RTU configuration..";
			cout<<"Parsing RTU configuration.."<<endl;
			if(!(isElementExistInJson(root,"buad_rate") && isElementExistInJson(root,"parity")
					&&isElementExistInJson(root,"stop_bit")))
			{
				BOOST_LOG_SEV(lg, error) << __func__ << " Required element are missing from RTU_Master_Config JSON.";
				cout << "Required keys are missing from RTU_Master_Config JSON."<<endl;
				return -1;
			}
			string portName = cJSON_GetObjectItem(root,"port_name")->valuestring;
			int baudrate = cJSON_GetObjectItem(root,"buad_rate")->valueint;
			int parity = cJSON_GetObjectItem(root,"parity")->valueint;
			int stopBit = cJSON_GetObjectItem(root,"stop_bit")->valueint;
			cout<<"Done"<<endl;
			BOOST_LOG_SEV(lg, info) << " Done";

			cout << "********************************************************************"<<endl;
			cout << "Modbus RTU container is running with below configuration.."<<endl;
			cout<<"Port Name = "<< portName<< endl;
			cout<<"Baud rate = "<< baudrate<< endl;
			cout<<"Parity = "<< parity<< endl;
			cout<<"StopBit = "<< stopBit<< endl;
			cout << "********************************************************************"<<endl;

			BOOST_LOG_SEV(lg, info) << "Modbus RTU container is running with below configuration..";
			BOOST_LOG_SEV(lg, info) << __func__ << " Port Name == "<< portName;
			BOOST_LOG_SEV(lg, info) << __func__ <<" Baud rate == "<< baudrate;
			BOOST_LOG_SEV(lg, info) << __func__ <<" Parity == "<< parity<< endl;
			BOOST_LOG_SEV(lg, info) << __func__ <<" StopBit == "<< stopBit;

			int fd = initSerialPort(portName.c_str(), baudrate, parity, stopBit);
			if(fd < 0)
			{
				cout << "Failed to initialize serial port for RTU."<<endl;
				cout << "Connect the RTU device to serial port"<<endl;
				cout << "Container will restart until the serial port is connected."<<endl;
				BOOST_LOG_SEV(lg, error) << __func__ << " Failed to initialize serial port for RTU.";
				BOOST_LOG_SEV(lg, error) << __func__ << " File descriptor is set to ::" << fd;
				cout << "Error:: File descriptor is set to :: " << fd << endl;
				return -1;
			}
			else
			{
				cout << "Initialize serial port for RTU is successful"<<endl;
				BOOST_LOG_SEV(lg, info) << __func__ << " File descriptor is set to ::" << fd;
				cout << "File descriptor is set to :: " << fd << endl;
			}
		}
#endif

	    // Setup signal handlers
	    signal(SIGUSR1, signal_handler);

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
		std::string sDirToRegister = "/" + AppName + "/";

		/// register callback for ETCD
		CfgManager::Instance().registerCallbackOnChangeDir(const_cast<char *>(sDirToRegister.c_str()));

#ifdef MODBUS_STACK_TCPIP_ENABLED
		/// store the yaml files in data structures
		network_info::buildNetworkInfo(true);
		BOOST_LOG_SEV(lg, info) << __func__ << "Modbus container application is set to TCP mode";
		cout << "Modbus container application is set to TCP mode.." << endl;
#else

		// Setting RTU mode
		network_info::buildNetworkInfo(false);
		BOOST_LOG_SEV(lg, info) << __func__ << "Modbus container application is set to RTU mode";
		cout << "Modbus container application is set to RTU mode.." << endl;
#endif


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

		std::unique_lock<std::mutex> lck(mtx);
		while (!g_stop)
		{
			BOOST_LOG_SEV(lg, info) << __func__ << " Condition variable is set for application exit.";
			cv.wait(lck);
		}
		BOOST_LOG_SEV(lg, info) << __func__ << " Exiting the application gracefully.";
		cout << "Exited application gracefully. "<< endl;

	    return EXIT_SUCCESS;
	}
	catch (const std::exception &e)
	{
		BOOST_LOG_SEV(lg, error) << __func__ << "fatal::Error in getting arguments: ";
		return EXIT_FAILURE;
	}
	BOOST_LOG_SEV(lg, debug) << __func__ << "End";

	return EXIT_SUCCESS;
}
