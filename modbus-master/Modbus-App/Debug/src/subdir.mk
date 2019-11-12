################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/BoostLogger.cpp \
../src/DataPoll.cpp \
../src/Main.cpp \
../src/ModbusStackInterface.cpp \
../src/PeriodicRead.cpp 

OBJS += \
./src/BoostLogger.o \
./src/DataPoll.o \
./src/Main.o \
./src/ModbusStackInterface.o \
./src/PeriodicRead.o 

CPP_DEPS += \
./src/BoostLogger.d \
./src/DataPoll.d \
./src/Main.d \
./src/ModbusStackInterface.d \
./src/PeriodicRead.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -lrt -std=c++11 -fpermissive -DBOOST_LOG_DYN_LINK=1 -I../$(PROJECT_DIR)/include -I../$(PROJECT_DIR)/../bin/boost/include -I../$(PROJECT_DIR)/../bin/safestring/include -I../$(PROJECT_DIR)/../bin/ssl/include -O0 -g3 -Wall -c -fmessage-length=0 -fPIE -O2 -D_FORTIFY_SOURCE=2 -static -fvisibility=hidden -fvisibility-inlines-hidden -Wformat -Wformat-security -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


