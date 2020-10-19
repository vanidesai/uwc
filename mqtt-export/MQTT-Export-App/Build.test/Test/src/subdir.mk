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
../Test/src/Common_ut.cpp \
../Test/src/MQTTCallback_ut.cpp \
../Test/src/MQTTPublishHandler_ut.cpp \
../Test/src/MQTTSubscribeHandler_ut.cpp \
../Test/src/Main_ut.cpp \
../Test/src/QueueMgr_ut.cpp 

OBJS += \
./Test/src/Common_ut.o \
./Test/src/MQTTCallback_ut.o \
./Test/src/MQTTPublishHandler_ut.o \
./Test/src/MQTTSubscribeHandler_ut.o \
./Test/src/Main_ut.o \
./Test/src/QueueMgr_ut.o 

CPP_DEPS += \
./Test/src/Common_ut.d \
./Test/src/MQTTCallback_ut.d \
./Test/src/MQTTPublishHandler_ut.d \
./Test/src/MQTTSubscribeHandler_ut.d \
./Test/src/Main_ut.d \
./Test/src/QueueMgr_ut.d 


# Each subdirectory must supply rules for building sources it contributes
Test/src/%.o: ../Test/src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -lrt -std=c++11 -fpermissive -DUNIT_TEST=1 -I../$(PROJECT_DIR)/include -I/home/user/paho.mqtt.c/src -I/usr/local/include -O0 -g3 -ftest-coverage -fprofile-arcs -Wall -c -fmessage-length=0 -fPIE -O2 -D_FORTIFY_SOURCE=2 -static -fvisibility=hidden -fvisibility-inlines-hidden -Wformat -Wformat-security -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


