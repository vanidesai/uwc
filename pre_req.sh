#!/bin/bash
################################################################################
# The source code contained or described herein and all documents related to
# the source code ("Material") are owned by Intel Corporation. Title to the
# Material remains with Intel Corporation.
#
# No license under any patent, copyright, trade secret or other intellectual
# property right is granted to or conferred upon you by disclosure or delivery of
# the Materials, either expressly, by implication, inducement, estoppel or otherwise.
################################################################################

working_dir=$(pwd)
RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
MAGENTA=$(tput setaf 5)
NC=$(tput sgr0)
BOLD=$(tput bold)
INFO=$(tput setaf 3)   # YELLOW (used for informative messages)

# ----------------------------
# Creating docker volume dir to store yaml files
# ----------------------------
createDockerVolumeDir()
{
    if [ ! -d /opt/intel/eis/uwc_data ]; then
    	echo "${GREEN}uwc_data directory is not present in /opt/intel/eis/ directory.${NC}"
    	echo "${GREEN}Creating /opt/intel/eis/uwc_data directory.${NC}"
    	mkdir -p /opt/intel/eis/uwc_data
	if [ "$?" -eq "0" ]; then
		echo "${GREEN}/opt/intel/eis/uwc_data is sucessfully created. ${NC}"
	else
        	echo "${RED}Failed to create docker volume directory${NC}"
		exit 1;
	fi
	
	rm -rf /opt/intel/eis/uwc_data/scada-rtu
    	mkdir -p /opt/intel/eis/uwc_data/scada-rtu
	if [ "$?" -eq "0" ]; then
		echo "${GREEN}/opt/intel/eis/uwc_data/scada-rtu is sucessfully created. ${NC}"
	else
        	echo "${RED}Failed to create docker volume directory${NC}"
		exit 1;
	fi
   fi
	echo "${GREEN}Deleting old /opt/intel/eis/container_logs directory.${NC}"
	rm -rf  /opt/intel/eis/container_logs
	echo "${GREEN}Done..${NC}"
	echo "${GREEN}Creating /opt/intel/eis/container_logs directory.${NC}"
	mkdir -p /opt/intel/eis/container_logs/modbus-tcp-master
	mkdir -p /opt/intel/eis/container_logs/modbus-rtu-master
	mkdir -p /opt/intel/eis/container_logs/mqtt-export
	mkdir -p /opt/intel/eis/container_logs/scada-rtu
    mkdir -p /opt/intel/eis/container_logs/kpi-tactic
	if [ "$?" -eq "0" ]; then
		echo "${GREEN}/opt/intel/eis/container_logs is sucessfully created. ${NC}"
	else
		echo "${RED}Failed to create docker volume directory${NC}"
		exit 1;
	fi
    if [ ! -d /opt/intel/eis/uwc_data/common_config ]; then
    	echo "${GREEN}common_config directory is not present in /opt/intel/eis/ directory.${NC}"
    	echo "${GREEN}Creating /opt/intel/eis/uwc_data/common_config directory.${NC}"
    	mkdir -p /opt/intel/eis/uwc_data/common_config
	if [ "$?" -eq "0" ]; then
		echo "${GREEN}/opt/intel/eis/uwc_data/common_config is sucessfully created. ${NC}"
	else
        	echo "${RED}Failed to create docker volume directory${NC}"
		exit 1;
	fi
    fi
}

addUWCContainersInEIS()
{
    echo "${INFO}Copying UWC Containers in EIS...${NC}"   
    cp -r Others/Config/UWC/Device_Config/* /opt/intel/eis/uwc_data
    cp Others/Config/UWC/Global_Config.yml /opt/intel/eis/uwc_data/common_config/Global_Config.yml
    copy_verification=$(echo $?)
    if [ "$copy_verification" -eq "0" ]; then
        echo "${GREEN}UWC containers are successfully copied ${NC}"
    else
        echo "${RED}failed to copy UWC containers.${NC}"
	    return 1
    fi
    return 0
}


echo "${GREEN}============================= Script START ============================================${NC}"

createDockerVolumeDir
addUWCContainersInEIS