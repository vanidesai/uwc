/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/



#ifndef INCLUDE_BOOSTLOGGER_HPP_
#define INCLUDE_BOOSTLOGGER_HPP_

//#define BOOST_LOG_DYN_LINK 1
#define FILE_ROTATION_COUNT 1
#define FILE_SIZE_IN_BYTES 1024

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sinks/text_file_backend.hpp>

// function to initialized boost logging based on file
void initLogging();

// namespace used in boost logging based on file
namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

using namespace logging::trivial;

/// boost logger handle
extern src::severity_logger< severity_level > lg;

#ifdef LOG_ENABLED
#define logMessage(level, msg, ...) BOOST_LOG_SEV(lg, level) << msg << ##__VA_ARGS__;
#else
#define logMessage(level, msg);
#endif

#endif /* INCLUDE_BOOSTLOGGER_HPP_ */
