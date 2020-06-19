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

## Install pre-requisites for scada-rtu
```
1. EdgeInsightsSoftware-v2.2-PV version of EIS should be available on deploy machine before deployment. 
2. Copy files from "Release" diectory (e.g. all shell scripts, tar.gz file, etc.) into "EdgeInsightsSoftware-v2.2-PV/IEdgeInsights" directory. Please ensure that shell scripts have "execute" permission (sudo chmod +x <script name>).
3. Open a terminal and go to EdgeInsightsSoftware-v2.2-PV/IEdgeInsights directory.
4. Open docker-compose.yml file and go to section "scada-rtu". Then uncomment the entire "scada-rtu" section ( line numbers 321 to 361), and also uncomment certicates for scada-rtu (line numbers 396 to 399). Then save the file.
5. Open docker-compose_unit_test.yml file and go to section "scada-rtu-test". Then uncomment entire section (line numbers 245 to 279).
5. Run below commands on terminal to deploy all UWC containers.
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

## SparkPlug Messages 
```
1. NBIRTH Message
NBIRTH is Node-Birth.
On start-up, SCADA-RTU module publishes this message over MQTT broker configured in environment variable. The message is published in SparkPlug encoded format.
Following are sample contents in simplified JSON format:

Topic: spBv1.0/Sparkplug B Devices/NBIRTH/SCADA-RTU
Message: 
{
  "timestamp": 1592549911034,
  "metrics": [
    {
      "name": "Name",
      "alias": 10,
      "timestamp": 1592549911034,
      "dataType": "String",
      "value": "SCADA-RTU"
    }
  ],
  "seq": 0,
  "uuid": "SCADA-RTU"
}

2. NDEATH Message

NDEATH is Node-Death.
Whenever SCADA-RTU module’s connection with MQTT broker breaks, the MQTT broker publishes this message. The message is published in SparkPlug encoded format.
Following are sample contents in simplified JSON format:

Topic: spBv1.0/Sparkplug B Devices/NDEATH/SCADA-RTU
Message: 
{
  "timestamp": 1592306298537,
  "metrics": [
    {
      "name": "bdSeq",
      "alias": 10,
      "timestamp": 1592306298537,
      "dataType": "UInt64",
      "value": 0
    }
  ],
  "seq": 0
}

```
