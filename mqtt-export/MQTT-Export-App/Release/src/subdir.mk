################################################################################
# The source code contained or described herein and all documents related to
# the source code ("Material") are owned by Intel Corporation. Title to the
# Material remains with Intel Corporation.
#
# No license under any patent, copyright, trade secret or other intellectual
# property right is granted to or conferred upon you by disclosure or delivery of
# the Materials, either expressly, by implication, inducement, estoppel or otherwise.
################################################################################


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
	g++ -lrt -std=c++11 -fpermissive -DINSTRUMENTATION_LOG -I../$(PROJECT_DIR)/include -I/usr/local/include -O3 -Wall -c -fmessage-length=0 -fPIE -O2 -D_FORTIFY_SOURCE=2 -static -fvisibility=hidden -fvisibility-inlines-hidden -Wformat -Wformat-security -fstack-protector-strong -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


