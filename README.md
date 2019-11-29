The directory comprises of below file/directories:
* modbus-master:
  This directory hosts the modbus container sources and docker file for building the container.
* MQTT
  This directory contains MQTT container
* mqtt-export
  This directort contains mqtt-export as container
* Others
  This directory contains Configs required for provisioning
	* ETCD_Config
	  This sub directory contains Configuration files
	* UWC
	  This sub directory contains sample Yaml files
	* etcd_pre_load.json
	  This is master json file to load ETCD configurations
	* etcd_provision.py
	  This is modified script to load all configurations for UWC
* docker-compose.yml :
  This file will deploy the modbus container with EIS in DEV mode.

# Install pre-requisites
1. Use EISv2.1 RC4 Alpha release to deploy this build.
2. Set DEV_MODE=true in .env to enable dev mode in EIS.
3. Copy modbus-master,MQTT and mqtt-export directories in </IEdgeInsights-v2.1-Alpha-RC4> (i.e in base directory of EIS where all the containers directories are present).
4. Copy UWC directory and etcd_pre_load.json in </IEdgeInsights-v2.1-Alpha-RC4/docker_setup/provision/config/> directory in EIS.
5. Copy etcd_provision.py file in </IEdgeInsights-v2.1-Alpha-RC4/docker_setup/provision/dep/> directory in EIS.
6. Copy "docker-compose.yml" file in </IEdgeInsights-v2.1-Alpha-RC4/docker_setup> directory in EIS.


# Build and Run UWC containers
1. Follow steps metioned from #EIS Pre-requisites till end to provision and deploy.
