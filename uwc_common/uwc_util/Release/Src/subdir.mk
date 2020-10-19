################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Src/CommonDataShare.cpp \
../Src/ConfigManager.cpp \
../Src/EnvironmentVarHandler.cpp \
../Src/Logger.cpp \
../Src/NetworkInfo.cpp \
../Src/QueueHandler.cpp \
../Src/ZmqHandler.cpp  

OBJS += \
./Src/CommonDataShare.o \
./Src/ConfigManager.o \
./Src/EnvironmentVarHandler.o \
./Src/Logger.o \
./Src/NetworkInfo.o \
./Src/QueueHandler.o \
./Src/ZmqHandler.o 

CPP_DEPS += \
./Src/CommonDataShare.d \
./Src/ConfigManager.d \
./Src/EnvironmentVarHandler.d \
./Src/Logger.d \
./Src/NetworkInfo.d \
./Src/QueueHandler.d \
./Src/ZmqHandler.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++11 -I../$(PROJECT_DIR)/include -I../$(PROJECT_DIR)/../bin/yaml-cpp/include -I/usr/local/include -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


