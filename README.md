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
* modbus-master :
  This directory contains the modbus TCP1 and RTU1 containers sources and docker file for building the container. It also has the ingredient docker-compose.yml & config.json for modbus TCP & RTU services.
  For detail, please refer to `README-modbus-master.md` file of modbus-master folder
* mqtt-export :
  This directory contains the mqtt-export container sources and docker file for building the container. It also has the ingredient docker-compose.yml & config.json for mqtt-export service. 
  For detail, please refer to `README-mqtt-export.md` file of mqtt-export folder
* MQTT:
  This directory contains the mqtt container sources and docker file for building the container. It also has the ingredient docker-compose.yml & config.json for MQTT internal broker service.
  For detail, please refer to `README-MQTT.md` file of MQTT folder
* scada-rtu:
  This directory contains the scada-rtu sources and docker file for building the container. It also has the ingredient docker-compose.yml & config.json for SCADA-RTU service.
  For detail, please refer to `README_SCADA_RTU.md` file of scada-rtu folder
* uwc-common:
  This directory contains sources for uwc common library container & Dockerfile to install all the dependencies and libraries needed by all the containers. For detail, please refer to `README-UWC_Common.md` file of uwc_common folder
* kpi-tactic:
  This directory contains the kpi-tactic container sources and docker file for building the container 
  For detail, please refer to `README-kpi-tactic.md` file of kpi-tactic folder
* Others:
  This directory contains configurations for ETCD required during provisioning
* uwc_common:
  This directory contains common dockefiles for UWC

## Install pre-requisites
```
1. After having successfully repo-synched the uwc.xml recipe from eis-manifest repo
Generate the consolidated docker-compose.yml & eis_config.json using the eis_builder utility from the eis-core repo. Can either use "build/uwc-pipeline-without-scada.yml" , "build/uwc-pipeline-with-scada.yml", "build/uwc-pipeline-with-kpi-no-scada.yml" with eis-builder based on the services needed for use case. Also, the recipe YML files can be edited as per the services needed.
Steps for running this eis_builder utility can be obtained from "https://github.impcloud.net/uwc/eii-core#eis-pre-requisites".
An example to use eis_builder utility is also shown as below. This generates consolidated docker-compose.yml & eis_config.json with the services mentioned in the "services recipe YML file" uwc-pipeline-with-kpi-no-scada.yml:
$cd "<working-dir>"/IEdgeInsights/<build>
$python3.6 eis_builder.py -f uwc-pipeline-with-kpi-no-scada.yml

2. Run "sudo sh pre_req.sh" from "<working-dir>/IEdgeInsights/uwc" directory. This creates necessary directories in /opt/intel/eis/ for log files storage & also copies UWC YAML configuration files. 
3. cd to "<working-dir>/IEdgeInsights/build" & then open .env. 
4. Update the key "DEV_MODE=true/false" based on the intended mode of running the services in either DEVELOPMENT (DEV_MODE=true) or PRODUCTION MODE (DEV_MODE=false).
5. Update the key "MQTT_PROTOCOL=tcp/ssl" based on the intended mode of running the services in either DEVELOPMENT (MQTT_PROTOCOL=tcp) or PRODUCTION MODE (MQTT_PROTOCOL=ssl).
6. Default is prod mode.
7. NOTE: In prod mode, the "Certificates" directory in "<working-dir>/IEdgeInsights/build/provision" needs 'sudo su" to be accessed. i.e. to open Certificates folder do the following:
  a. cd <working-dir>/IEdgeInsights/build/provision
  b. sudo su
  c. cd Certificates
8. After accessing Certificates, enter "exit" command & terminal would return back to normal mode.
9. IMPORTANT NOTE: EVERYTIME THE MODE IS SWITCHED (DEV<->PROD) BY MAKING CHANGES IN .ENV FILE AS ABOVE, MAKE SURE TO RE-RUN THE "eis_builder.py" utility as mentioned above, again, even if the recipe hasn't changed. This is needed to re-generate consolidated docker-compose.yml file in build directory as per dev/prod mode.
```

## Provision EIS & UWC services
```
Execute below command on terminal for provisioning EIS & UWC services.
1. cd <working-dir>/IEdgeInsights/build/provision
2. sudo ./provision_eis.sh ../docker-compose.yml
```

## Build and Run all UWC & EIS containers
```
Execute below command on terminal for container deployment.
1. cd <working-dir>/IEdgeInsights/build/
2. docker-compose up --build -d

```

## Verify container status
```
Execute below command on terminal to verify container status.
sudo docker ps
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
docker logs modbus-tcp-container > docker.log 2>&1
```

1. ETCD UI is available on `http://localhost:7070/etcdkeeper/` URL for dev mode & `https://localhost:7071/etcdkeeper/` for prod mode . (username - root , password- eis123)
2. EIS Message Bus related interface configurations (including secrets) for all UWC & EII containers are stored in ia_etcd server, which can be accessed using EtcdUI as mentioned above.

## Steps to apply new configuration (i.e. YML files or docker-compose.yml)
  Once YML files/docker-compose.yml are changed/Modified in /opt/intel/eis/uwc_data directory then execute following command to apply new configurations,
 ```
  1. cd <working-dir>/IEdgeInsights/build
  2. docker-compose down && docker-compose up --build -d
```
## How to bring up/down UWC containers
```
cd <working-dir>/IEdgeInsights/build
docker-compose down  - bring down all containers
docker-compose up -d - bring up all containers in background

## Notes
*  If docker-compose.yml is modified then bring down the containers & then bring them up as mentioned above.
*  If previous containers are running on deploy machine, then stop those containers using command for bringing down containers as mentioned above.
*  Can use MQTT.FX or any other MQTT client to verify teh flow or all 6 operations for RT/Non-RT, read/write,polling operations
