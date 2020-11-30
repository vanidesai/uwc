```
********************************************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 **********************************************************************************************************************
```

Modbus containers and Modbus stack sources details and build and Run instructions
This is applicable for TCP as well as RTU.

# Contents:

1. [Directory and file details](#All-internal-directory-file-details)

2. [Pre-requisites](#Pre-requisites-Installation)

3. [Steps to compile SoftMod_Stack for TCP](#Steps-to-compile-SoftMod_Stack-tcp)

4. [Steps to compile SoftMod_Stack for RTU](#Steps-to-compile-SoftMod_Stack-rtu)


# Directory and file details
Section to describe all directory contents and it's uses.

1. SoftMod_Stack - This directory contains sources for Modbus Stack implementation for TCP and RTU 
	1. `.settings` - Eclipse project configuration files
	2. `Debug` - Build configuration for Debug mode
	3. `include` - This directory contains all the header files (i.e. .hpp) required to compile modbus container
	4. `lib` - This directory used to keep all the third party libraries (i.e. pthread , cjson, etc. ) required for modbus container. 
	5. `Release` - Build configuration for Release mode. (This mode will be used for final deployment mode)
	6. `src` - This directory contains all .cpp files.
	7. `.cproject` - Eclipse project configuration files
	8. `.project` - Eclipse project configuration files
	9. `sonar-project.properties` - This file is required for Softdel CICD process for sonar qube analysis
		
# Pre-requisites Installation
For compiling modbus container sources on machine without container, following pre-requisites needs to installed on machine,
1. Install make and cmake 
	
# Steps to compile SoftMod_Stack for TCP

1. Go to `Sourcecode\modbus-master\SoftMod_Stack\Release` directory and open a bash terminal.
2. Execute `$make clean all` command.
3. After successful completion of step 2, libModbusMasterStack.so library must be created in same directory.
4. Copy libModbusMasterStack.so librray in `Sourcecode\modbus-master\Modbus-App\lib `directory

# Steps to compile SoftMod_Stack for RTU

1. Go to `Sourcecode\modbus-master\SoftMod_Stack\Release` directory and open a bash terminal.
2. Remove `-DMODBUS_STACK_TCPIP_ENABLED` macro from `Sourcecode\modbus-master\SoftMod_Stack\Release\Src\subdir.mk` file.
3. Execute `$make clean all` command.
4. After successful completion of step 2, libModbusMasterStack.so library must be created in same directory.
5. Copy libModbusMasterStack.so library in `Sourcecode\modbus-master\Modbus-App\lib` directory

Notes : Above instructions are specified to build the sources in "Release" mode. To build sources in "Debug" mode, please execute the same steps in "Debug" folder inside `Sourcecode\modbus-master\` folder.
