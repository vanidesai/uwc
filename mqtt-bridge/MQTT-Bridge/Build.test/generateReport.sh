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

export LD_LIBRARY_PATH="/MQTT-Bridge/lib/:/usr/local/lib/"
env
./MQTT_Bridge_test > /reports/mqtt-bridge/mqtt-bridge_test_status.log 2>&1

# Run GCovr command
gcovr --html -f "../src/Common.cpp" -f "../src/Main.cpp" -f "../src/MQTTPublishHandler.cpp" -f "../src/MQTTSubscribeHandler.cpp" -f "../src/QueueMgr.cpp" -f "../include/Common.hpp" -f "../include/MQTTPublishHandler.hpp" -f "../include/MQTTSubscribeHandler.hpp" -f "../include/QueueMgr.hpp" --exclude-throw-branches -o /reports/mqtt-bridge/MQTT-Bridge_Report.html -r .. .
