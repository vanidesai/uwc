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

export LD_LIBRARY_PATH="/Modbus-App/lib/:/usr/local/lib/"
env
./ModbusMaster_test > /reports/modbus-tcp-master/modbus-tcp-master_test_status.log 2>&1

# Run GCovr command
gcovr --html -e "../Test" -e "../src/ModbusStackInterfaceCaller.cpp" -e"../include/ModbusStackInterfaceCaller.hpp" -e"../include/MsgbusManager.hpp" -e"../include/MsgbusEnvelope.hpp" -e"../src/MsgbusManager.cpp" -e"../src/SerialInterface.cpp" -e"../src/SerialPortSetting.cpp" -e"../src/MsgbusEnvelope.cpp" -e"../src/ModBus_EnvConfig_Caller.cpp" -e"../src/config_Caller.cpp" -e ../../bin   -o /reports/modbus-tcp-master/ModbusTCP_report.html -r .. .
