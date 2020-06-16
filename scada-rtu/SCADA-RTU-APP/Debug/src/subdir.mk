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
../src/ConfigManager.cpp \
../src/Logger.cpp \
../src/MQTTCallback.cpp \
../src/Main.cpp \
../src/MqttHandler.cpp \
../src/NetworkInfo.cpp \
../src/Publisher.cpp \
../src/QueueMgr.cpp \
../src/SCADAHandler.cpp 

OBJS += \
./src/Common.o \
./src/ConfigManager.o \
./src/Logger.o \
./src/MQTTCallback.o \
./src/Main.o \
./src/MqttHandler.o \
./src/NetworkInfo.o \
./src/Publisher.o \
./src/QueueMgr.o \
./src/SCADAHandler.o 

CPP_DEPS += \
./src/Common.d \
./src/ConfigManager.d \
./src/Logger.d \
./src/MQTTCallback.d \
./src/Main.d \
./src/MqttHandler.d \
./src/NetworkInfo.d \
./src/Publisher.d \
./src/QueueMgr.d \
./src/SCADAHandler.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++11 -I../$(PROJECT_DIR)/include -I../$(PROJECT_DIR)/include/yaml-cpp -I../$(PROJECT_DIR)/include/utils -I../$(PROJECT_DIR)/include/tahu -I/home/user/paho.mqtt.c/src -I/usr/local/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


