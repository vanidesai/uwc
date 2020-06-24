################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Common.cpp \
../src/ConfigManager.cpp \
../src/Logger.cpp \
../src/MQTTCallback.cpp \
../src/Main.cpp \
../src/NetworkInfo.cpp \
../src/Publisher.cpp \
../src/SCADAHandler.cpp 

OBJS += \
./src/Common.o \
./src/ConfigManager.o \
./src/Logger.o \
./src/MQTTCallback.o \
./src/Main.o \
./src/NetworkInfo.o \
./src/Publisher.o \
./src/SCADAHandler.o 

CPP_DEPS += \
./src/Common.d \
./src/ConfigManager.d \
./src/Logger.d \
./src/MQTTCallback.d \
./src/Main.d \
./src/NetworkInfo.d \
./src/Publisher.d \
./src/SCADAHandler.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++11 -DUNIT_TEST -DSCADA_RTU -I../$(PROJECT_DIR)/include -I../$(PROJECT_DIR)/include/yaml-cpp -I../$(PROJECT_DIR)/include/utils -I../$(PROJECT_DIR)/include/tahu -I/home/user/paho.mqtt.c/src -I/usr/local/include -O0 -g3 -ftest-coverage -fprofile-arcs -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


