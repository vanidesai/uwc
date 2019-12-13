/********************************************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 **********************************************************************************************************************/

# Directory details
The directory comprises of following:
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/modbus-master">modbus-master</a> :
  This directory contains the modbus container sources and docker file for building the container. 
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/mqtt-export">mqtt-export</a> :
  This directory contains the mqtt-export container sources and docker file for building the container. 
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/MQTT">MQTT</a> :
  This directory contains the mqtt container sources and docker file for building the container.
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/Others">Others</a> :
  This directory contains configurations for ETCD required during provisioning. 
* <a href="https://github.impcloud.net/uwc/UWC-Core/blob/master/docker-compose.yml">docker-compose.yml</a> :
  This file will deploy the modbus container with EIS in prod mode.
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/docker-compose_DEV.yml">docker-compose_DEV.yml</a> :
  This file can be used to deploy UWC containers in dev mode.
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/01_pre-requisites.sh">01_pre-requisites.sh</a> :
  This script is use to install all pre-requisites.
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/02_provisionEIS.sh">02_provisionEIS.sh</a>  :
  This script is use to install provision EIS.
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/03_DeployEIS.sh">03_DeployEIS.sh</a> :
  This script is used to deploy UWC containers in dev mode.
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/04_uninstall_EIS.sh">04_uninstall_EIS.sh</a> :
  This script will uninstall all UWC containers.
* <a href="https://github.impcloud.net/uwc/UWC-Core/tree/master/create_uwc_package.sh">create_uwc_package.sh</a> :
  This script will be used to create UWC package

# Steps to create bundle out of sources
1. Run "sudo ./create_uwc_package.sh" script to create UWC bundle for deployment.

# Install pre-requisites
1. EdgeInsightsSoftware-v2.1-Alpha version of EIS should be available on deploy machine before deployment. 
2. Copy 01_pre-requisites.sh, 02_provisionEIS.sh, 03_DeployEIS.sh, 04_uninstall_EIS.sh and UWC.tar.gz(i.e. created in bundle create steps) files in EdgeInsightsSoftware-v2.1-Alpha/IEdgeInsights directory.
3. Go to EdgeInsightsSoftware-v2.1-Alpha/IEdgeInsights directory.
4. Uninstall previous containers using "sudo ./04_uninstall_EIS.sh" command.
5. Run "sudo ./01_pre-requisites.sh" script for installing all pre-requisites.

# Provision EIS
1. Run "sudo ./02_provisionEIS.sh" command to provision EIS.

# Build and Run all UWC containers
1. Run "sudo ./03_DeployEIS.sh" script for container deployment.

# Verify container status
1. Use "sudo docker ps" command to verify container status.

# Steps to check container logs
1. Syntax - sudo docker logs <container_name>
   E.g To check modbus-tcp-container logs execute "sudo docker logs modbus-tcp-container" command.
2. Command to check logs inside the container "sudo docker exec -it <container_name> bash"

# ETCD UI access
1. ETCD UI is available on "http://localhost:7070/etcdkeeper/" URL. (username - root , password- eis123)
