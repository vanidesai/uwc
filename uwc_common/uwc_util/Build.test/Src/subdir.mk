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
../Src/CommonDataShare.cpp \
../Src/ConfigManager.cpp \
../Src/EnvironmentVarHandler.cpp \
../Src/Logger.cpp \
../Src/MQTTPubSubClient.cpp \
../Src/NetworkInfo.cpp \
../Src/QueueHandler.cpp \
../Src/YamlUtil.cpp \
../Src/ZmqHandler.cpp 

OBJS += \
./Src/CommonDataShare.o \
./Src/ConfigManager.o \
./Src/EnvironmentVarHandler.o \
./Src/Logger.o \
./Src/MQTTPubSubClient.o \
./Src/NetworkInfo.o \
./Src/QueueHandler.o \
./Src/YamlUtil.o \
./Src/ZmqHandler.o 

CPP_DEPS += \
./Src/CommonDataShare.d \
./Src/ConfigManager.d \
./Src/EnvironmentVarHandler.d \
./Src/Logger.d \
./Src/MQTTPubSubClient.d \
./Src/NetworkInfo.d \
./Src/QueueHandler.d \
./Src/YamlUtil.d \
./Src/ZmqHandler.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++1z -DUNIT_TEST -I/usr/local/include -I../$(PROJECT_DIR)/include -O0 -g3 -ftest-coverage -fprofile-arcs -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


