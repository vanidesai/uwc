#!/bin/bash
# Copyright (c) 2021 Intel Corporation.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

export LD_LIBRARY_PATH="/MQTT-Bridge/lib/:/usr/local/lib/"
env
./MQTT_Bridge_test > /reports/mqtt-bridge/mqtt-bridge_test_status.log 2>&1

# Run GCovr command
gcovr --html -f "../src/Common.cpp" -f "../src/Main.cpp" -f "../src/MQTTPublishHandler.cpp" -f "../src/MQTTSubscribeHandler.cpp" -f "../src/QueueMgr.cpp" -f "../include/Common.hpp" -f "../include/MQTTPublishHandler.hpp" -f "../include/MQTTSubscribeHandler.hpp" -f "../include/QueueMgr.hpp" --exclude-throw-branches -o /reports/mqtt-bridge/MQTT-Bridge_Report.html -r .. .
