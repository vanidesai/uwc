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

3. [Steps to compile modbus-tcp-master](#Steps-to-compile-modbus-tcp-master)

4. [Steps to compile modbus-rtu-master](#Steps-to-compile-modbus-rtu-master)

5. [Steps to run modbus executable on machine](#Steps-to-run-modbus-executable-on-machine)

6. [Steps to deploy sources inside container](#Steps-to-deploy-sources-inside-container)

7. [Steps to run unit test cases](#Steps-to-run-unit-testcases)

8. [Steps to enable/disable instrumentation logs](#Steps-to-enable/disable-instrumentation-logs)


# Directory and file details
Section to describe all directory contents and it's uses.

1. Modbus-App - This directory contains sources for Modbus App implementation for TCP and RTU containers
	1. `.settings` - Eclipse project configuration files
	2. `Build.test` - Unit test configuration to run unit test cases and generating code coverage
	3. `Config` - Contains logger configuration directory
	4. `Debug` - Build configuration for Debug mode
	5. `include` - This directory contains all the header files (i.e. .hpp) required to compile modbus container
	6. `lib` - This directory used to keep all the third party libraries (i.e. pthread , cjson, etc. ) required for modbus container. 
	7. `Release` - Build configuration for Release mode. (This mode will be used for final deployment mode)
	8. `src` - This directory contains all .cpp files.
	9. `Test` - Contains unit test cases files. (.hpp, .cpp, etc.)
	10. `.cproject` - Eclipse project configuration files
	11. `.project` - Eclipse project configuration files
	12. `sonar-project.properties` - This file is required for Softdel CICD process for sonar qube analysis
	13. modbus_RTU: This folder is in parallel to Modbus-App & contains indivisual docker-compose & config files for modbus-RTU.
	14. modbus-TCP: This folder is in parallel to Modbus-App & contains indivisual docker-compose & config files for modbus-RTU.

2. SoftMod_Stack - This directory contains sources for Modbus Stack implementation for TCP and RTU 
	1. `.settings` - Eclipse project configuration files
	2. `Debug` - Build configuration for Debug mode
	3. `include` - This directory contains all the header files (i.e. .hpp) required to compile modbus container
	4. `lib` - This directory used to keep all the third party libraries (i.e. pthread , cjson, etc. ) required for modbus container. 
	5. `Release` - Build configuration for Release mode. (This mode will be used for final deployment mode)
	6. `src` - This directory contains all .cpp files.
	7. `.cproject` - Eclipse project configuration files
	8. `.project` - Eclipse project configuration files
	9. `sonar-project.properties` - This file is required for Softdel CICD process for sonar qube analysis
3. `Dockerfile` - Dockerfile to build modbus-tcp-master container.
4. `Dockerfile_RTU` - Dockerfile to build modbus-rtu-master container.
5. `Dockerfile_UT` - Dockerfile to build unit test container for modbus-tcp-master sources testing.
6. `Dockerfile_UT_RTU` - Dockerfile to build unit test container for modbus-rtu-master sources testing.
		
# Pre-requisites Installation
For compiling modbus container sources on machine without container, following pre-requisites needs to installed on machine,
1. Install make and cmake 
2. Install wget by using command "sudo apt-get install wget".
3. Install Git by using command "sudo apt install git"
4. Install all the EIS libraries on host by running the shell script -- `sudo -E ./eis_libs_installer.sh`. Refre the README.md from  `IEdgeInsights\common\README.md` for details.
5. Install log4cpp (version - 1.1.3, link - https://sourceforge.net/projects/log4cpp/files/latest/download/log4cpp-1.1.3.tar.gz) library under /usr/local/ directory.
6. Install yaml-cpp library (branch - yaml-cpp-0.6.3, version - 0.6.3, link - https://github.com/jbeder/yaml-cpp.git) libraries on host under /usr/local/ directory.
7. Install paho-cpp (branch develop https://github.com/eclipse/paho.mqtt.c.git) libraries on host under /usr/local/ directory.
8. Install uwc_common library refering to `README_UWC_Common.md` of `uwc_common` folder and copy `uwc_common/uwc_util/lib/libuwc-common.so` and `uwc_common/uwc_util/include/all .hpp files` in `Sourcecode\modbus-master/Modbus-App/lib ` and `Sourcecode/modbus-master/Modbus-App/include` directory respectively.
	
# Steps to compile modbus-tcp-master 
1. Compile SoftMod_Stack with following steps,
	1. Go to `Sourcecode\modbus-master\SoftMod_Stack\Release` directory and open a bash terminal.
	2. Execute `$make clean all` command.
	3. After successful completion of step 2, libModbusMasterStack.so library must be created in same directory.
	4. Copy libModbusMasterStack.so librray in `Sourcecode\modbus-master\Modbus-App\lib `directory
2. Go to `Sourcecode\modbus-master\Modbus-App\Release directory` and open a terminal.
3. Execute ``$make clean all`` command
4. After successfull execution of step 3, application executable must be generated in current directory (i.e. Release).

# Steps to compile modbus-rtu-master
1. Compile SoftMod_Stack with following steps,
	1. Go to `Sourcecode\modbus-master\SoftMod_Stack\Release` directory and open a bash terminal.
	2. Remove `-DMODBUS_STACK_TCPIP_ENABLED` macro from `Sourcecode\modbus-master\SoftMod_Stack\Release\Src\subdir.mk` file.
	3. Execute `$make clean all` command.
	4. After successful completion of step 2, libModbusMasterStack.so library must be created in same directory.
	5. Copy libModbusMasterStack.so library in `Sourcecode\modbus-master\Modbus-App\lib` directory
2. Go to `Sourcecode\modbus-master\Modbus-App\Release` directory and open a terminal.
3. Remove `-DMODBUS_STACK_TCPIP_ENABLED` macro from `Sourcecode\modbus-master\Modbus-App\Release\src\subdir.mk` file.
4. Execute `$make clean all` command.
5. After successful execution of step 2, application executable (with name `ModbusMaster`) must be generated in current directory (i.e. Release).

Notes : Above instructions are specified to build the sources in "Release" mode. To build sources in "Debug" mode, please execute the same steps in "Debug" folder inside `Sourcecode\modbus-master\` folder. 

# Steps to run modbus executable on machine
1. Deploy ia_etcd container with dev mode using following steps. 
	1. Run `preReq.sh` script as explained in the main uwc/README.md.
	2. Add `network_mode: host` option in two containers present in IEdgeInsights\build\provision\dep\docker-compose-provision.yml file.
	3. Run the eprovisioning command script to deploy ia_etcd container as explainedin main uwc/README/md.
2. Go to `Sourcecode\modbus-master\Modbus-App\Release` directory and open bash terminal.
3. Set EIS specific environment variables using below command.
	`source <Complete Path of .env file present inside IEdgeInsights/build directory>`
	For example `source /home/intel/uwc-releases/IEdgeInsights/build/.env`
4. Export all other environment variables required for modbus container. Refer environment section from modbus-tcp-master service present inside docker-compose.yml file (E.g. `export AppName="TCP"` for exporting AppName variable likewise all other variables needed to be exported in the same terminal) for modbus-tcp-master container and for RTU refer same section from modbus-rtu-master container 
5. After successful compilation, run the application binary with following command,
	`./ModbusMaster`
	
# Steps to deploy sources inside container
Kindly Refer UWC user guide for container deployments 

# Steps to run unit test cases 
1. Pre-requisites Installation for unit test execution
	1. Install latest version of "gcovr" tool for coverage report generation by using command "pip install gcovr".
	2. Install gtest (wget https://github.com/google/googletest/archive/release-1.8.0.tar.gz) libraries on host under /usr/local/ directory.
	3. All other Pre-requisites to be installed mentioned in section # Pre-requisites Installation
2. Follow same steps mentioned in section # Steps to compile modbus-tcp-master and # Steps to compile modbus-rtu-master, step 1 to step 4/5, but in "Build.test" directory of `Sourcecode/modbus-master/Modbus-App` instead of "Release" directory.
3. Run unit test cases
	1. Deploy ia_etcd container with dev mode using following steps. 
		1. Run `01_pre-requisites.sh --isTLS=no  --brokerAddr="mqtt_test_container" --brokerPort="11883" --qos=1 --deployMode=IPC_DEV` script
		2. Add `network_mode: host` option in two containers present in EdgeInsightsSoftware-v2.2-PV\IEdgeInsights\docker_setup\provision\dep\docker-compose-provision.yml file.
		3. Run `02_provisionEIS.sh` script to deploy ia_etcd container
	2. Go to `Sourcecode/modbus-master/Modbus-App/Build.test` directory and open bash terminal.
	3. Follow steps 3 and 4 of `# Steps to run modbus executable on machine` section
	4. After successful compilation, run the application binary with following command,
	`./ModbusMaster_test > modbus-tcp-master_test_status.log` for modbus-tcp and `./ModbusMaster_test > modbus-rtu-master_test_status.log` for modbus-rtu.
	5. After successful execution of step 4, unit test log file `modbus-tcp-master_test_status.log` and `modbus-rtu-master_test_status.log` must be generated in the same folder that is Build.test
4. Generate unit test coverage report
	1. Go to `Sourcecode/modbus-master/Modbus-App/Build.test` directory and open bash terminal.
	2. Run the command,
		`gcovr --html -e "../Test" -e "../include/log4cpp" -e ../../bin --exclude-throw-branches -o ModbusTCP_report.html -r .. .` and `gcovr --html -e "../Test" -e "../include/log4cpp" -e ../../bin --exclude-throw-branches -o ModbusRTU_report.html -r .. .` for modbus-tcp and modbus-rtu respectively.
	3. After successful execution of step 2, unit test coverage report file `ModbusTCP_report.html/ModbusRTU_report.html` must be generated.
5. Run unit test cases inside container
	1. Kindly follow the steps mentioned in section `## Steps to run unit test cases` of file `README.md` in Sourcecode directory.

#Steps to enable/disable instrumentation logs
1. By default the instrumentation logs are enabled for debug mode & disabled for release mode. 
2. Go to `Sourcecode\modbus-master\Modbus-App\Release\src` directory and open subdir.mk file.
3. To enable the instrumentation logs, go to g++ command at line number 41 & add the option "-DINSTRUMENTATION_LOG".
4. To disable the instrumentation logs,go to g++ command at line number 41 check & remove the option "-DINSTRUMENTATION_LOG" if found.

