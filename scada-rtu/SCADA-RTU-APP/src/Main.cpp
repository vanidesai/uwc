/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#include "Common.hpp"
#include "ConfigManager.hpp"
#include <iterator>
#include <vector>

#include "SCADAHandler.hpp"
#include "InternalMQTTSubscriber.hpp"
#include "SparkPlugDevMgr.hpp"

#ifdef UNIT_TEST
#include <gtest/gtest.h>
#endif

vector<std::thread> g_vThreads;

std::atomic<bool> g_shouldStop(false);

#define APP_VERSION "0.0.6.4"

/**
 * Process a message to be sent on internal MQTT broker
 * @param a_objCDataPointsMgr :[in] reference of data points manager class
 * @param a_qMgr :[in] reference of queue from which message is to be processed
 * @return none
 */
void processExternalMqttMsgs(CQueueHandler& a_qMgr)
{
	while (false == g_shouldStop.load())
	{
		CMessageObject recvdMsg{};
		if(true == a_qMgr.isMsgArrived(recvdMsg))
		{
			std::vector<stRefForSparkPlugAction> stRefActionVec;

			CSCADAHandler::instance().processDCMDMsg(recvdMsg, stRefActionVec);

			//prepare a sparkplug message only if there are values in map
			if(! stRefActionVec.empty())
			{
				CIntMqttHandler::instance().prepareCJSONMsg(stRefActionVec);
			}
		}
	}
}


/**
 * Process a message to be sent on external MQTT broker
 * @param a_objCDataPointsMgr :[in] reference of data points manager class
 * @param a_qMgr :[in] reference of queue from which message is to be processed
 * @return none
 */
void processInternalMqttMsgs(CQueueHandler& a_qMgr)
{
	string eisTopic = "";

	try
	{
		while (false == g_shouldStop.load())
		{
			CMessageObject recvdMsg{};
			if(true == a_qMgr.isMsgArrived(recvdMsg))
			{
				std::vector<stRefForSparkPlugAction> stRefActionVec;
				CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
						recvdMsg.getTopic(),
						recvdMsg.getStrMsg(),
						stRefActionVec);

				//prepare a sparkplug message only if there are values in map
				if(! stRefActionVec.empty())
				{
					CSCADAHandler::instance().prepareSparkPlugMsg(stRefActionVec);
				}
			}
		}
	}
	catch (const std::exception &e)
	{
		DO_LOG_FATAL(e.what());
	}
}

/**
 * Initialize data points repository by reading data points
 * from yaml file
 * @param none
 * @return true/false depending on success/failure
 */
bool initDataPoints()
{
	try
	{
		string strNetWorkType = EnvironmentInfo::getInstance().getDataFromEnvMap("NETWORK_TYPE");
		string strSiteListFileName = EnvironmentInfo::getInstance().getDataFromEnvMap("DEVICES_GROUP_LIST_FILE_NAME");
		string strAppId = "";

		if(strNetWorkType.empty() || strSiteListFileName.empty())
		{
			DO_LOG_ERROR("Network type or device list file name is not present");
			return false;
		}

		network_info::buildNetworkInfo(strNetWorkType, strSiteListFileName, strAppId);

		// Create SparkPlug devices corresponding to Modbus devices
		CSparkPlugDevManager::getInstance().addRealDevices();
	}
	catch(std::exception &ex)
	{
		DO_LOG_ERROR(ex.what());
	}
	return true;
}

/**
 * Main function of application
 * @param argc :[in] number of input parameters
 * @param argv :[in] input parameters
 * @return 	0/-1 based on success/failure
 */
int main(int argc, char *argv[])
{
	try
	{
		
		std::cout << __func__ << ":" << __LINE__ << " ------------- Starting SCADA RTU Container -------------" << std::endl;
		std::cout << "SCADA RTU container app version is set to :: "+  std::string(APP_VERSION) << std::endl;

		CLogger::initLogger(std::getenv("Log4cppPropsFile"));

		DO_LOG_DEBUG("Starting SCADA RTU ...");
		DO_LOG_INFO("SCADA RTU container app version is set to :: "+  std::string(APP_VERSION));
		if(!CCommon::getInstance().loadYMLConfig())
		{
			DO_LOG_ERROR("Please set the required config in scada_config.yml file and restart the container");
			std::cout << "Please set the required config in scada_config.yml file and restart the container" << std::endl;
		}

		//initialize CCommon class to get common variables
		string AppName = EnvironmentInfo::getInstance().getDataFromEnvMap("AppName");
		if(AppName.empty())
		{
			DO_LOG_ERROR("AppName Environment Variable is not set");
			std::cout << __func__ << ":" << __LINE__ << " Error : AppName Environment Variable is not set" <<  std::endl;

			return -1;
		}

		if(CCommon::getInstance().getExtMqttURL().empty() ||
				EnvironmentInfo::getInstance().getDataFromEnvMap("INTERNAL_MQTT_URL").empty())
		{
			while(true)
			{
				std::cout << "Waiting to set EXTERNAL_MQTT_URL/ INTERNAL_MQTT_URL variable in scada_config.yml file..." << std::endl;
				std::this_thread::sleep_for(std::chrono::seconds(300));
			};
		}
		else
		{
				{
					CSparkPlugDevManager::getInstance();

					initDataPoints();

					CSCADAHandler::instance();
					CIntMqttHandler::instance();
				}
		}


#ifdef UNIT_TEST
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
#endif
		g_vThreads.push_back(std::thread(processInternalMqttMsgs, std::ref(QMgr::getDatapointsQ())));
		g_vThreads.push_back(std::thread(processExternalMqttMsgs, std::ref(QMgr::getScadaSubQ())));

		for (auto &th : g_vThreads)
		{
			if (th.joinable())
			{
				th.join();
			}
		}

	}
	catch (std::exception &e)
	{
		DO_LOG_FATAL(e.what());
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
		return -1;
	}
	catch (...)
	{
		DO_LOG_FATAL("Unknown Exception Occurred. Exiting");
		std::cout << __func__ << ":" << __LINE__ << "Exception : Unknown Exception Occurred. Exiting" << std::endl;
		return -1;
	}

	std::cout << __func__ << ":" << __LINE__ << " ------------- Exiting SCADA RTU Container -------------" << std::endl;
	return 0;
}
