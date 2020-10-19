####################################################################################
# The source code contained or described herein and all documents related to
# the source code ("Material") are owned by Intel Corporation. Title to the
# Material remains with Intel Corporation.
#
# No license under any patent, copyright, trade secret or other intellectual
# property right is granted to or conferred upon you by disclosure or delivery of
# the Materials, either expressly, by implication, inducement, estoppel or otherwise.
####################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/Common.cpp \
../src/Main.cpp \
../src/ModbusOnDemandHandler.cpp \
../src/ModbusStackInterface.cpp \
../src/PeriodicRead.cpp \
../src/PublishJson.cpp 

OBJS += \
./src/Common.o \
./src/Main.o \
./src/ModbusOnDemandHandler.o \
./src/ModbusStackInterface.o \
./src/PeriodicRead.o \
./src/PublishJson.o 

CPP_DEPS += \
./src/Common.d \
./src/Main.d \
./src/ModbusOnDemandHandler.d \
./src/ModbusStackInterface.d \
./src/PeriodicRead.d \
./src/PublishJson.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -lrt -std=c++11 -fpermissive -DMODBUS_STACK_TCPIP_ENABLED -DINSTRUMENTATION_LOG -I../$(PROJECT_DIR)/include -I/usr/local/include -I../$(PROJECT_DIR)/../bin/yaml-cpp/include -I../$(PROJECT_DIR)/include/utils -I../$(PROJECT_DIR)/../bin/safestring/include -O0 -g3 -Wall -c -fmessage-length=0 -fPIE -O2 -D_FORTIFY_SOURCE=2 -static -fvisibility=hidden -fvisibility-inlines-hidden -Wformat -Wformat-security  -fstack-protector-strong -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


