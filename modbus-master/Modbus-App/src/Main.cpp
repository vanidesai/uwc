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
#include "DataPoll.hpp"
#include "BoostLogger.hpp"
#include "PeriodicReadFeature.hpp"

#include <netdb.h>
#include <ifaddrs.h>
#include <string.h>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include <safe_lib.h>
}

// for boost logging
extern src::severity_logger< severity_level > lg;

int main(int argc, char* argv[])
{
	try
	{
		PeriodicDataPoll objDataPoll;

		initLogging();
		logging::add_common_attributes();

		uint8_t	u8ReturnType = AppMbusMaster_StackInit();

		CTimeMapper::instance().initTimerFunction();


		objDataPoll.initDataPoll();

		while(1)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
	catch (const std::exception &e)
	{
		BOOST_LOG_SEV(lg, info) << "fatal::Error in getting arguments: "<<e.what();
		return 0;
	}

	return EXIT_SUCCESS;
}




