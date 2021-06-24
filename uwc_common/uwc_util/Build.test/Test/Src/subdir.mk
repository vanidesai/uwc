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
../Test/Src/CConfigManager_ut.cpp \
../Test/Src/CommonDataShare_ut.cpp \
../Test/Src/EnvironmentVarHandler_ut.cpp \
../Test/Src/Logger_ut.cpp \
../Test/Src/MQTTPubSubClient_ut.cpp \
../Test/Src/NetworkInfo_ut.cpp \
../Test/Src/QueueHandler_ut.cpp \
../Test/Src/ZmqHandler_ut.cpp 

OBJS += \
./Test/Src/CConfigManager_ut.o \
./Test/Src/CommonDataShare_ut.o \
./Test/Src/EnvironmentVarHandler_ut.o \
./Test/Src/Logger_ut.o \
./Test/Src/MQTTPubSubClient_ut.o \
./Test/Src/NetworkInfo_ut.o \
./Test/Src/QueueHandler_ut.o \
./Test/Src/ZmqHandler_ut.o 

CPP_DEPS += \
./Test/Src/CConfigManager_ut.d \
./Test/Src/CommonDataShare_ut.d \
./Test/Src/EnvironmentVarHandler_ut.d \
./Test/Src/Logger_ut.d \
./Test/Src/MQTTPubSubClient_ut.d \
./Test/Src/NetworkInfo_ut.d \
./Test/Src/QueueHandler_ut.d \
./Test/Src/ZmqHandler_ut.d 


# Each subdirectory must supply rules for building sources it contributes
Test/Src/%.o: ../Test/Src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++1z -DUNIT_TEST -I/usr/local/include -I../$(PROJECT_DIR)/include -O0 -g3 -ftest-coverage -fprofile-arcs -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


