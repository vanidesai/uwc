# Universal Wellpad Controller (UWC)

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
## Directory details
The directory comprises of following:
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/modbus-master">modbus-master</a> :
  This directory contains the modbus TCP1 and RTU1 containers sources and docker file for building the container
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/mqtt-export">mqtt-export</a> :
  This directory contains the mqtt-export container sources and docker file for building the container 
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/MQTT">MQTT</a> :
  This directory contains the mqtt container sources and docker file for building the container.
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/Others">Others</a> :
  This directory contains configurations for ETCD required during provisioning
* <a href="https://github.impcloud.net/uwc/UWC-Core/blob/master/docker-compose.yml">docker-compose.yml</a> :
  This file will deploy the modbus container with EIS in IPC with DEV mode
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/docker-compose_IPC_DEV.yml">docker-compose_IPC_DEV.yml</a> :
  This file is used to deploy UWC containers in IPC with DEV mode
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/docker-compose_TCP_DEV.yml">docker-compose_TCP_DEV.yml</a> :
  This file is used to deploy UWC containers in TCP with DEV mode
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/docker-compose_TCP_PROD.yml">docker-compose_TCP_PROD.yml</a> :
  This file is used to deploy UWC containers in TCP with PROD mode.
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/docker-compose_unit_test.yml">docker-compose_unit_test.yml</a> :
  This file is used to run unit test cases
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/Release">Release</a> :
  This directory contains UWC bundle for deployement
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/create_uwc_package.sh">create_uwc_package.sh</a> :
  This script will be used to create UWC package
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/uwc_common">uwc_common</a> :
  This directory contains common dockefiles for UWC

## Steps to install UWC along with EIS Installer 
```
1. EdgeInsightsSoftware-v2.1-PV version of EIS should be available on deploy machine before deployment. 
2. Copy files from "Release" diectory (e.g. all shell scripts, tar.gz file, etc.) into "EdgeInsightsSoftware-v2.2-PV/IEdgeInsights" directory. Please ensure that shell scripts have "execute" permission (sudo chmod +x <script name>).
3. Open a terminal and go to EdgeInsightsSoftware-v2.2-PV/IEdgeInsights directory.
4. Run below command on terminal to install all pre-requisites required to deploy UWC containers.
sudo ./ConfigureUWC.sh 
```

## Build and Run all UWC containers
```
Follow EIS installer process for further deployment.
Go to EdgeInsightsSoftware-v2.2-PV/installer/installation and then execute setup.sh script.
```

## Verify container status
```
Execute below command on terminal to verify container status.
sudo docker ps
```

## Steps to run unit test cases
```
1. Installed all the pre-requisites mentioned in ##Install pre-requisites section
2. Run 06_UnitTestRun.sh script using "sudo ./06_UnitTestRun.sh" command to run unit test cases.
3. Check the reports in <EIS>/docker_setup/unit_test_reports directory.
```

## Debugging steps
```
1. Checking container logs 
   Syntax - sudo docker logs <container_name>
   E.g. To check modbus-tcp-container logs execute "sudo docker logs modbus-tcp-container" command.
2. Command to check logs inside the container "sudo docker exec -it <container_name> bash"
3. Use "cat <log_file_name>" to see log file inside the container
4. Copying logs from container to host machine
   Syntax - docker cp <container_name>:<file to copy from container> <file to be copied i.e. host directory>
5. To check the IP address of machine, use "ifconfig" command.
6. For Modbus RTU, to check attached COM port for serial communication, use "dmesg | grep tty" command.

## Redirect docker logs to file including errors
```
docker logs modbus-tcp-container > docker.log 2>&1
```

## ETCD UI access (*Note : This is not required since we are not storing any configuration data for UWC containers in ETCD)
```
1. ETCD UI is available on `http://localhost:7070/etcdkeeper/` URL. (username - root , password- eis123)
```

## Steps to create bundle out of sources - Optional 
```
Execute below script to create UWC bundle for deployment.
sudo ./create_uwc_package.sh
```

