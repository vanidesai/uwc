#*************************************************************************
#                   Copyright (c) by Softdel Systems              
#                                                                       
#   This software is copyrighted by and is the sole property of Softdel
#   Systems. All rights, title, ownership, or other interests in the
#   software remain the property of Softdel Systems. This software
#   may only be used in accordance with the corresponding license
#   agreement. Any unauthorized use, duplication, transmission,
#   distribution, or disclosure of this software is expressly forbidden. 
#                                                                       
#   This Copyright notice may not be removed or modified without prior   
#   written consent of Softdel Systems.                               
#                                                                       
#   Softdel Systems reserves the right to modify this software       
#   without notice.                                                      
#************************************************************************/

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/ClientSocket.c \
../Src/ModbusExportedAPI.c \
../Src/SessionControl.c \
../Src/osalLinux.c 

OBJS += \
./Src/ClientSocket.o \
./Src/ModbusExportedAPI.o \
./Src/SessionControl.o \
./Src/osalLinux.o 

C_DEPS += \
./Src/ClientSocket.d \
./Src/ModbusExportedAPI.d \
./Src/SessionControl.d \
./Src/osalLinux.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I../../bin/safestring/include -I../Inc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -pthread  -O2 -D_FORTIFY_SOURCE=2 -static -fvisibility=hidden -Wformat -Wformat-security -fstack-protector-strong -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


