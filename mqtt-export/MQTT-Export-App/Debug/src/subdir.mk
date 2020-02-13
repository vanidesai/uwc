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
../src/ConfigManager.cpp \
../src/EISMsgbusHandler.cpp \
../src/Logger.cpp \
../src/MQTTCallback.cpp \
../src/MQTTHandler.cpp \
../src/MQTT_Export.cpp \
../src/TopicMapper.cpp 

OBJS += \
./src/ConfigManager.o \
./src/EISMsgbusHandler.o \
./src/Logger.o \
./src/MQTTCallback.o \
./src/MQTTHandler.o \
./src/MQTT_Export.o \
./src/TopicMapper.o 

CPP_DEPS += \
./src/ConfigManager.d \
./src/EISMsgbusHandler.d \
./src/Logger.d \
./src/MQTTCallback.d \
./src/MQTTHandler.d \
./src/MQTT_Export.d \
./src/TopicMapper.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -lrt -std=c++11 -fpermissive -DINSTRUMENTATION_LOG -I../$(PROJECT_DIR)/include -I/usr/local/include -O0 -g3 -Wall -c -fmessage-length=0 -fPIE -O2 -D_FORTIFY_SOURCE=2 -static -fvisibility=hidden -fvisibility-inlines-hidden -Wformat -Wformat-security -fstack-protector-strong -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


