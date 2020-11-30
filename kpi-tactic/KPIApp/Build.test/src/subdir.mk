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
../src/ControlLoopHandler.cpp \
../src/EISPlBusHandler.cpp \
../src/KPIAppConfigMgr.cpp \
../src/Main.cpp \
../src/MqttHandler.cpp \
../src/QueueMgr.cpp 

OBJS += \
./src/Common.o \
./src/ControlLoopHandler.o \
./src/EISPlBusHandler.o \
./src/KPIAppConfigMgr.o \
./src/Main.o \
./src/MqttHandler.o \
./src/QueueMgr.o 

CPP_DEPS += \
./src/Common.d \
./src/ControlLoopHandler.d \
./src/EISPlBusHandler.d \
./src/KPIAppConfigMgr.d \
./src/Main.d \
./src/MqttHandler.d \
./src/QueueMgr.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DUNIT_TEST=1 -I../$(PROJECT_DIR)/include -I/usr/local/include -I../$(PROJECT_DIR)/../bin/yaml-cpp/include -O0 -g3 -ftest-coverage -fprofile-arcs -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


