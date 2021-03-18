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

MQTT-Export containers sources details and build and Run instructions

# Contents:

1. [Directory and file details](#All-internal-directory-file-details)

2. [Pre-requisites](#Pre-requisites-Installation)

3. [Steps to compile MQTT-Export](#Steps-to-compile-MQTT-Export)

4. [Steps to run MQTT-Export executable on machine](#Steps-to-run-MQTT-Export-executable-on-machine)

5. [Steps to deploy sources inside container](#Steps-to-deploy-sources-inside-container)

6. [Steps to run unit test cases](#Steps-to-run-unit-testcases)

7. [Steps to enable/disable instrumentation logs](#Steps-to-enable/disable-instrumentation-logs)

# Directory and file details
Section to describe all directory contents and it's uses.

1. MQTT-Export-App - This directory contains sources for MQTT-Export App implementation 
	1. `.settings` - Eclipse project configuration files
	2. `Build.test` - Unit test configuration to run unit test cases and generating code coverage
	3. `Config` - Contains logger configuration directory
	4. `Debug` - Build configuration for Debug mode
	5. `include` - This directory contains all the header files (i.e. .hpp) required to compile kpi container
	6. `lib` - This directory used to keep all the third party libraries (i.e. pthread , cjson, etc. ) required for MQTT-Export container. 
	7. `Release` - Build configuration for Release mode. (This mode will be used for final deployment mode)
	8. `src` - This directory contains all .cpp files.
	9. `Test` - Contains unit test cases files. (.hpp, .cpp, etc.)
	10. `.cproject` - Eclipse project configuration files
	11. `.project` - Eclipse project configuration files
	12. `sonar-project.properties` - This file is required for Softdel CICD process for sonar qube analysis
2. `Dockerfile` - Dockerfile to build MQTT-Export container.
3. `Dockerfile_UT` - Dockerfile to build unit test container for MQTT-Export sources testing.
4. docker-compose.yml -- Ingredient docker-compose.yml for mqtt-export micro service.
5. config.json - Ingredient config.json for mqtt-export micro service.

# Pre-requisites Installation

For compiling MQTT-Export sources on machine without container, following pre-requisites needs to installed on machine,
1. Install make and cmake
2. Install wget by using command "sudo apt-get install wget".
3. Install Git by using command "sudo apt install git"
4. Install all the EIS libraries on host by running the shell script -- `sudo -E ./eis_libs_installer.sh`. Refre the README.md from  `IEdgeInsights\common\README.md` for details.
5. Install log4cpp (version - 1.1.3, link - https://sourceforge.net/projects/log4cpp/files/latest/download/log4cpp-1.1.3.tar.gz) library under /usr/local/ directory.
6. Install yaml-cpp (branch - yaml-cpp-0.6.3, version - 0.6.3, link - https://github.com/jbeder/yaml-cpp.git) libraries on host under /usr/local/ directory.
7. Install paho-cpp (branch develop https://github.com/eclipse/paho.mqtt.c.git) libraries on host under /usr/local/ directory.
8. Install eclipse-tahu (branch develop https://github.com/eclipse/tahu) libraries on host under /usr/local/ directory.
9. Install ssl (https://www.openssl.org/source/openssl-1.1.1g.tar.gz) libraries on host under /usr/local/ directory.
10. Install uwc_common library refering to `README_UWC_Common.md` of `uwc_common` and copy `uwc_common/uwc_util/lib/libuwc-common.so` and 'uwc_common/uwc_util/include' (.hpp) in `Sourcecode\mqtt-export\MQTT-Export-App\lib` and `Sourcecode\mqtt-export\MQTT-Export-App\include` directory respectively.

# Steps to compile MQTT-Export

1. Go to `Sourcecode\mqtt-export\MQTT-Export-App\Release` directory and open a terminal.
2. Execute `$make clean all` command.
3. After successful execution of step 2, application executable (with name `MQTT_Export`) must be generated in current directory (i.e. Release).

Notes : Above instructions are specified to build the sources in "Release" mode. To build sources in "Debug" mode, please execute the same steps in "Debug" folder inside `Sourcecode\mqtt-export\` folder.

# Steps to run MQTT-Export executable on machine
1. Deploy ia_etcd container with dev mode using following steps. 
	1. Run `preReq.sh` script as explained in the main uwc/README.md.
	2. Add `network_mode: host` option in two containers present in IEdgeInsights\build\provision\dep\docker-compose-provision.yml file.
	3. Run th eprovisioning command script to deploy ia_etcd container as explainedin main uwc/README/md.
2. Go to `Sourcecode\mqtt-export\MQTT-Export-App\Release` directory and open bash terminal.
3. Set EIS specific environment variables using below command.
	`source <Complete Path of .env file present inside IEdgeInsights/build directory>`
	For example `source /home/intel/uwc-releases/IEdgeInsights/build/.env`
4. Export all environment variables required for mqtt-export-test container. Refer environment section from mqtt-export-test service present inside docker-compose.yml of mqtt-export service file (E.g. `export AppName="MQTT-Export"` for exporting AppName variable likewise all other variables needed to be exported in the same terminal). 
5. After successful compilation, run the application binary with following command,
	`./MQTT_Export`

# Steps to deploy sources inside container
Kindly Refer UWC user guide for container deployments

# Steps to run unit test cases
1. Pre-requisites Installation for unit test execution
    1. Install latest version of "gcovr" tool for coverage report generation by using command "pip install gcovr".
    2. Install gtest (wget https://github.com/google/googletest/archive/release-1.8.0.tar.gz) libraries on host under /usr/local/ directory.
    3. All other Pre-requisites to be installed mentioned in section # Pre-requisites Installation
2. Follow same steps mentioned in section # Steps to compile MQTT-Export, step 1 to step 3, but in "Build.test" directory of `Sourcecode\mqtt-export\MQTT-Export-App` instead of "release" directory.
3. Run unit test cases
    1. Deploy ia_etcd container with dev mode using following steps. 
        1. Run `01_pre-requisites.sh --deployMode=IPC_DEV --withoutScada=yes` script
	2. Add `network_mode: host` option in two containers present in IEdgeInsights\build\provision\dep\docker-compose-provision.yml file.
	3. Run `02_provisionEIS.sh` script to deploy ia_etcd container
    2. Go to `Sourcecode\mqtt-export\MQTT-Export-App\Build.test` directory and open bash terminal.
    3. Export all environment variables required for mqtt-export-test container. Refer environment section from mqtt-export-test service present inside docker-compose_unit_test.yml file (E.g. `export AppName="MQTT-Export"` for exporting AppName variable likewise all other variables needed to be exported in the same terminal) 
    4. After successful compilation, run the application binary with following command,
    `./MQTT_Export_test > mqtt-export_test_status.log` 
    5. After successful execution of step 4, unit test log file `mqtt-export_test_status.log` must be generated in the same folder that is Build.test
4. Generate unit test coverage report
    1. Go to `Sourcecode\mqtt-export\MQTT-Export-App\Build.test` directory and open bash terminal.
    2. Run the command,
        `gcovr --html -f "../src/Common.cpp" -f "../src/Main.cpp" -f "../src/MQTTPublishHandler.cpp" -f "../src/MQTTSubscribeHandler.cpp" -f "../src/QueueMgr.cpp" -f "../include/Common.hpp" -f "../include/MQTTPublishHandler.hpp" -f "../include/MQTTSubscribeHandler.hpp" -f "../include/QueueMgr.hpp" --exclude-throw-branches -o MQTT-Export_Report.html -r .. .`
    3. After successful execution of step 2, unit test coverage report file `MQTT-Export_Report.html` must be generated.


Notes : Above steps are to run KPIApp unit test locally. In order to run unit test in container, please follow the steps mentioned in section `## Steps to run unit test cases` of file `README.md` in Sourcecode directory. 

#Steps to enable/disable instrumentation logs
1. By default the instrumentation logs are enabled for debug mode & disabled for release mode. 
2. Go to `Sourcecode\mqtt-export\MQTT-Export-App\Release\src` directory and open subdir.mk file.
3. To enable the instrumentation logs, go to g++ command at line number 39 & add the option "-DINSTRUMENTATION_LOG".
4. To disable the instrumentation logs,go to g++ command at line number 39 check & remove the option "-DINSTRUMENTATION_LOG" if found.



