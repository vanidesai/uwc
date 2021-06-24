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

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Common.cpp \
../src/InternalMQTTSubscriber.cpp \
../src/Main.cpp \
../src/Metric.cpp \
../src/QueueMgr.cpp \
../src/SCADAHandler.cpp \
../src/SparkPlugDevMgr.cpp \
../src/SparkPlugDevices.cpp \
../src/SparkPlugUDTMgr.cpp 

OBJS += \
./src/Common.o \
./src/InternalMQTTSubscriber.o \
./src/Main.o \
./src/Metric.o \
./src/QueueMgr.o \
./src/SCADAHandler.o \
./src/SparkPlugDevMgr.o \
./src/SparkPlugDevices.o \
./src/SparkPlugUDTMgr.o 

CPP_DEPS += \
./src/Common.d \
./src/InternalMQTTSubscriber.d \
./src/Main.d \
./src/Metric.d \
./src/QueueMgr.d \
./src/SCADAHandler.d \
./src/SparkPlugDevMgr.d \
./src/SparkPlugDevices.d \
./src/SparkPlugUDTMgr.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++1z -DSCADA_RTU -DUNIT_TEST -I../$(PROJECT_DIR)/include -I../$(PROJECT_DIR)/include/yaml-cpp -I../$(PROJECT_DIR)/include/tahu -I/usr/local/include -O0 -g3 -ftest-coverage -fprofile-arcs -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


