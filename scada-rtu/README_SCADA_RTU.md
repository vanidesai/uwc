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
UWC supports Eclipse Foundation’s SparkPlug standard to expose data to SCADA systems over MQTT. SCADA-RTU implements the standard and enables communication with SCADA system. Please note SCADA-RTU feature is under development feature. 
The directory comprises of following:
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/scada-rtu">scada-rtu</a> :
  This directory contains the scada-rtu sources and docker file for building the container.

## Pre-requisite to capture Scada-rtu messages
```
The scada-rtu uses non-secure mode only to publish message on MQTT broker. So, to capture the messages user needs to configure parameter "MQTT_URL" in docker-compose.yml file at line no. 355. User can assign any non-secure MQTT broker URL to this parameter.
e.g.
   MQTT_URL: "tcp://127.0.0.1:1883"

This MQTT broker acts as a client for Scada-rtu messages. Please run MQTT.FX and connect to this MQTT broker URL. Once the connection is
established, check the lower right corner of the MQTT.FX UI. Please set "Payload decoded by" option to "Sparkplug Decoder". This shall
display the SparkPlug encoded message in human-readable format in MQTT.FX.

To set scada-rtu container in unit testing mode, please assign MQTT broker URL in same way as above line number 271.
```

## Install pre-requisites for scada-rtu
```
1. EdgeInsightsSoftware-v2.2-PV version of EIS should be available on deploy machine before deployment. 
2. Copy files from "Release" diectory (e.g. all shell scripts, tar.gz file, etc.) into "EdgeInsightsSoftware-v2.2-PV/IEdgeInsights" directory. Please ensure that shell scripts have "execute" permission (sudo chmod +x <script name>).
3. Open a terminal and go to EdgeInsightsSoftware-v2.2-PV/IEdgeInsights directory.
4. Open docker-compose.yml file and go to section "scada-rtu". Then uncomment the entire "scada-rtu" section ( line numbers 336 to 380), and also uncomment certicates for scada-rtu (line numbers 415 to 418). Go to line no 364 and add MQTT broker URL (please refer section "Pre-requisite to capture Scada-rtu messages"). Then save the file.
5. Open docker-compose_unit_test.yml file and go to section "scada-rtu-test". Then uncomment entire section (line numbers 245 to 283).
6. Run below commands on terminal to deploy all UWC containers.
a)Provision for EIS, run below command:
sudo ./02_provisionEIS.sh
b)Then, deploy all the containers:
sudo ./03_DeployEIS.sh
```

## Verify container status
```
Execute below command on terminal to verify container status.
sudo docker ps
```

## Steps to run unit test cases
```
1. Install all the pre-requisites mentioned in ##Install pre-requisites section
2. Run 06_UnitTestRun.sh script using "sudo ./06_UnitTestRun.sh" command to run unit test cases.
3. Check the reports in <EIS>/docker_setup/unit_test_reports directory.
```

## How to start/stop scada-rtu container manually
```
cd EIS..../docker_setup
export PWD=$(pwd)
docker start scada-rtu  - to start the scada-rtu container
docker stop scada-rtu  - to stop the scada-rtu container
```

## Edge node id configuration
```
The edge_node_id in all the scada-rtu messages, a combination of mentioned device model ("nodeName" parameter in "/opt/intel/eis/uwc_data/common_config/Global_Config.yml" at line no 90) and MAC address, e.g. RBOX510-XX:XX:XX:XX:XX:XX. The default interface name provided in Global_Configuration.yml file is "eth0". If this interface is available in the host machine, a MAC address gets appended in the edge_node_id. In case if no MAC address is found, 00 gets appended in the edge_node_id.

User can configure the topic name to have MAC address or not. 
Please follow below steps if user wishes not to have MAC address in topic name:
1) Please go to "/opt/intel/eis/uwc_data/common_config/Global_Config.yml". 
2) At line number 91, keep "generateUniqueName" parameter value as false, (by default it is always true, so always MAC address of "eth0" interface will get added in the topic-name). Save the file.
3) Go to IEdgeInsight directory and run script "05_applyConfigChanges.sh".

Please follow below steps if user wishes to have MAC address in topic name:
1) Please go to "/opt/intel/eis/uwc_data/common_config/Global_Config.yml". 
2) Go to line number 92 and change the value of parameter "interface_name", (by default "eth0" is mentioned).
3) Go to IEdgeInsight directory and run script "05_applyConfigChanges.sh".

```

## SparkPlug Messages 
```
1. NBIRTH Message
NBIRTH is Node-Birth.
On start-up, SCADA-RTU module publishes this message over MQTT broker configured in environment variable. The message is published in SparkPlug encoded format. 

Following are sample contents in simplified JSON format:

Topic: spBv1.0/UWC nodes/NBIRTH/RBOX510-00
Message: 
{
  "timestamp": 1593071037176,
  "metrics": [
    {
      "name": "Name",
      "timestamp": 1593071037176,
      "dataType": "String",
      "value": "RBOX510-00"
    }
  ],
  "seq": 0,
  "uuid": "SCADA_RTU"
}

2. DBIRTH Message
DBIRTH is device birth message. This message gets published after NBIRTH message.
The message publishes information about a device along with its data-points, in SparkPlug encoded format. Following is sample contents in simplified JSON format:

Topic: spBv1.0/UWC nodes/DBIRTH/RBOX510-00/flowmeter/PL0
Message:
    {
      "name": "Inputs/D1",
      "timestamp": 1593071037176,
      "dataType": "String",
      "properties": {
        "Pollinterval": {
          "type": "UInt32",
          "value": 1000
        },
        "Realtime": {
          "type": "Boolean",
          "value": false
        }
      }
    }

3. NDEATH Message

NDEATH is Node-Death.
Whenever SCADA-RTU module’s connection with MQTT broker breaks, the MQTT broker publishes this message. The message is published in SparkPlug encoded format.
Following are sample contents in simplified JSON format:

Topic: spBv1.0/UWC nodes/NDEATH/RBOX510-00
Message: 
{
  "timestamp": 1593072212656,
  "metrics": [
    {
      "name": "bdSeq",
      "timestamp": 1593072212656,
      "dataType": "UInt64",
      "value": 0
    }
  ],
  "seq": 0
}

```
