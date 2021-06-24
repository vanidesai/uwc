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
../Test/Src/Common_ut.cpp \
../Test/Src/InternalMQTTSubscriber_ut.cpp \
../Test/Src/Main_ut.cpp \
../Test/Src/Metric_ut.cpp \
../Test/Src/SCADAHandler_ut.cpp \
../Test/Src/SparkPlugDevices_ut.cpp \
../Test/Src/SparkPlugUDTMgr_ut.cpp \
../Test/Src/SparklugDevMgr_ut.cpp 

OBJS += \
./Test/Src/Common_ut.o \
./Test/Src/InternalMQTTSubscriber_ut.o \
./Test/Src/Main_ut.o \
./Test/Src/Metric_ut.o \
./Test/Src/SCADAHandler_ut.o \
./Test/Src/SparkPlugDevices_ut.o \
./Test/Src/SparkPlugUDTMgr_ut.o \
./Test/Src/SparklugDevMgr_ut.o 

CPP_DEPS += \
./Test/Src/Common_ut.d \
./Test/Src/InternalMQTTSubscriber_ut.d \
./Test/Src/Main_ut.d \
./Test/Src/Metric_ut.d \
./Test/Src/SCADAHandler_ut.d \
./Test/Src/SparkPlugDevices_ut.d \
./Test/Src/SparkPlugUDTMgr_ut.d \
./Test/Src/SparklugDevMgr_ut.d 


# Each subdirectory must supply rules for building sources it contributes
Test/Src/%.o: ../Test/Src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++1z -DSCADA_RTU -DUNIT_TEST -I../$(PROJECT_DIR)/include -I../$(PROJECT_DIR)/include/yaml-cpp -I../$(PROJECT_DIR)/include/tahu -I/usr/local/include -O0 -g3 -ftest-coverage -fprofile-arcs -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


