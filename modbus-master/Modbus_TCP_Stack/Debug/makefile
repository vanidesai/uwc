################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: libModbusTCPMasterStack.so

# Tool invocations
libModbusTCPMasterStack.so: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L../../bin/safestring/lib -fPIC -pthread -z noexecstack -z relro -z now -shared -o "libModbusTCPMasterStack.so" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(LIBRARIES)$(OBJS)$(C_DEPS) libModbusTCPMasterStack.so
	-@echo ' '

.PHONY: all clean dependents

-include ../makefile.targets
