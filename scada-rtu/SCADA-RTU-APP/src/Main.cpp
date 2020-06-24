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
#include "Publisher.hpp"

#ifdef UNIT_TEST
#include <gtest/gtest.h>
#endif

vector<std::thread> g_vThreads;

std::atomic<bool> g_shouldStop(false);

#define APP_VERSION "0.0.5.3"

/**
 * Function to keep running this application and check NBIRTH and NDEATH messages
 * where we stop and start the container
 * @param none
 * @return none
 */
void updateDataPoints()
{
	while (false == g_shouldStop.load())
	{
		//keep on working till the application is not stopped
	}
}

/**
 * Main function of application
 * @param argc :[in] number of input parameters
 * @param argv :[in] input parameters
 * @return 	0/-1 based on success/failure
 */
int main(int argc, char *argv[])
{
	DO_LOG_DEBUG("Starting SCADA RTU ...");
	std::cout << __func__ << ":" << __LINE__ << " ------------- Starting SCADA RTU Container -------------" << std::endl;

	try
	{
		//initialize CCommon class to get common variables
		string AppName = CCommon::getInstance().getStrAppName();
		if(AppName.empty())
		{
			DO_LOG_ERROR("AppName Environment Variable is not set");
			std::cout << __func__ << ":" << __LINE__ << " Error : AppName Environment Variable is not set" <<  std::endl;
			return -1;
		}

		DO_LOG_INFO("SCADA RTU container app version is set to :: "+  std::string(APP_VERSION));
		cout << "SCADA RTU container app version is set to :: "+  std::string(APP_VERSION) << endl;

		cout << "MQTT URL : " << CCommon::getInstance().getStrMqttURL() << endl;

		CPublisher::instance();

		if( false == CPublisher::instance().isPublisherConnected())
		{
			std::cout << "Publisher failed to connect with MQTt broker" << endl;
			exit(-1);
		}
		CSCADAHandler::instance();

#ifdef UNIT_TEST
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
#endif

		//send messages for SCADA
		g_vThreads.push_back(std::thread(updateDataPoints));

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
