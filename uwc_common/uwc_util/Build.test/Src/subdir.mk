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
../Src/CommonDataShare.cpp \
../Src/ConfigManager.cpp \
../Src/EnvironmentVarHandler.cpp \
../Src/Logger.cpp \
../Src/MQTTPubSubClient.cpp \
../Src/NetworkInfo.cpp \
../Src/QueueHandler.cpp \
../Src/YamlUtil.cpp \
../Src/ZmqHandler.cpp 

OBJS += \
./Src/CommonDataShare.o \
./Src/ConfigManager.o \
./Src/EnvironmentVarHandler.o \
./Src/Logger.o \
./Src/MQTTPubSubClient.o \
./Src/NetworkInfo.o \
./Src/QueueHandler.o \
./Src/YamlUtil.o \
./Src/ZmqHandler.o 

CPP_DEPS += \
./Src/CommonDataShare.d \
./Src/ConfigManager.d \
./Src/EnvironmentVarHandler.d \
./Src/Logger.d \
./Src/MQTTPubSubClient.d \
./Src/NetworkInfo.d \
./Src/QueueHandler.d \
./Src/YamlUtil.d \
./Src/ZmqHandler.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++1z -DUNIT_TEST -I/usr/local/include -I../$(PROJECT_DIR)/include -O0 -g3 -ftest-coverage -fprofile-arcs -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


