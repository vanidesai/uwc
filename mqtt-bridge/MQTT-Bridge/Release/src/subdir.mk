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
../src/MQTTPublishHandler.cpp \
../src/MQTTSubscribeHandler.cpp \
../src/Main.cpp \
../src/QueueMgr.cpp 

OBJS += \
./src/Common.o \
./src/MQTTPublishHandler.o \
./src/MQTTSubscribeHandler.o \
./src/Main.o \
./src/QueueMgr.o 

CPP_DEPS += \
./src/Common.d \
./src/MQTTPublishHandler.d \
./src/MQTTSubscribeHandler.d \
./src/Main.d \
./src/QueueMgr.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -lrt -std=c++11 -fpermissive -I../$(PROJECT_DIR)/include -I/usr/local/include -O3 -Wall -c -fmessage-length=0 -fPIE -O2 -D_FORTIFY_SOURCE=2 -static -fvisibility=hidden -fvisibility-inlines-hidden -Wformat -Wformat-security -fstack-protector-strong -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


