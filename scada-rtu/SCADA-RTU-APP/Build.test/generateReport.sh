#!/bin/bash
################################################################################
# The source code contained or described herein and all documents related to
# the source code ("Material") are owned by Intel Corporation. Title to the
# Material remains with Intel Corporation.
#
# No license under any patent, copyright, trade secret or other intellectual
# property right is granted to or conferred upon you by disclosure or delivery of
# the Materials, either expressly, by implication, inducement, estoppel or otherwise.
################################################################################

export LD_LIBRARY_PATH="/SCADA-RTU-APP/lib:/usr/local/lib"
env
./SCADARTU_test > /reports/scada-rtu/SCADARTU_status.log 2>&1

# Run GCovr command
gcovr --html -e "../Test" -e "../include/utils" -e "../include/tahu" -e "../include/yaml-cpp" -e "../src/utils" -e "../include/log4cpp/Appender.hh" --exclude-throw-branches -o /reports/scada-rtu/SCADARTU_Report.html -r .. .
