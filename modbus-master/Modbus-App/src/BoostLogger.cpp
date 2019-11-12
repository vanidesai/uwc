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

src::severity_logger< severity_level > lg;

/**
*
* DESCRIPTION
* function to initialized boost logging based on file

* @return [out] void/nothing.
*
*/
void initLogging()
{
    logging::add_file_log
    (
    	keywords::target = "logs",						/// logs will be stored in logs directory inside the container
        keywords::file_name = "modbus_%N.log",                                        /// file name pattern
        keywords::rotation_size = FILE_ROTATION_COUNT * FILE_SIZE_IN_BYTES * FILE_SIZE_IN_BYTES,   /// rotate files every 10 MiB
        //keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0), /// ...or at midnight
        keywords::format = "[%TimeStamp%]: %Message%",                           /// log record format
		keywords::max_size = 1*1024*1024,
		keywords::min_free_space = 1*1024 * 1024,
        keywords::auto_flush = true
    );

    logging::core::get()->set_filter
    (
        logging::trivial::severity >= logging::trivial::info
    );
}


