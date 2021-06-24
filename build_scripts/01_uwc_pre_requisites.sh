#!/bin/bash
# Copyright (c) 2021 Intel Corporation.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

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
create_docker_volume_dir()
{
    if [ ! -d /opt/intel/eii/uwc_data ]; then
    	echo "${GREEN}uwc_data directory is not present in /opt/intel/eii/ directory.${NC}"
    	echo "${GREEN}Creating /opt/intel/eii/uwc_data directory.${NC}"
    	mkdir -p /opt/intel/eii/uwc_data
		if [ "$?" -eq "0" ]; then
			echo "${GREEN}/opt/intel/eii/uwc_data is sucessfully created. ${NC}"
		else
        	echo "${RED}Failed to create docker volume directory${NC}"
			exit 1;
		fi
	
		rm -rf /opt/intel/eii/uwc_data/sparkplug-bridge
    	mkdir -p /opt/intel/eii/uwc_data/sparkplug-bridge
		if [ "$?" -eq "0" ]; then
			echo "${GREEN}/opt/intel/eii/uwc_data/sparkplug-bridge is sucessfully created. ${NC}"
		else
        	echo "${RED}Failed to create docker volume directory${NC}"
			exit 1;
		fi
    fi
	echo "${GREEN}Deleting old /opt/intel/eii/container_logs directory.${NC}"
	rm -rf  /opt/intel/eii/container_logs
	echo "${GREEN}Done..${NC}"
	echo "${GREEN}Creating /opt/intel/eii/container_logs directory.${NC}"
	mkdir -p /opt/intel/eii/container_logs/modbus-tcp-master
	mkdir -p /opt/intel/eii/container_logs/modbus-rtu-master
	mkdir -p /opt/intel/eii/container_logs/mqtt-bridge
	mkdir -p /opt/intel/eii/container_logs/sparkplug-bridge
    mkdir -p /opt/intel/eii/container_logs/kpi-tactic
	if [ "$?" -eq "0" ]; then
		echo "${GREEN}/opt/intel/eii/container_logs is sucessfully created. ${NC}"
	else
		echo "${RED}Failed to create docker volume directory${NC}"
		exit 1;
	fi
    if [ ! -d /opt/intel/eii/uwc_data/common_config ]; then
    	echo "${GREEN}common_config directory is not present in /opt/intel/eii/ directory.${NC}"
    	echo "${GREEN}Creating /opt/intel/eii/uwc_data/common_config directory.${NC}"
    	mkdir -p /opt/intel/eii/uwc_data/common_config
		if [ "$?" -eq "0" ]; then
			echo "${GREEN}/opt/intel/eii/uwc_data/common_config is sucessfully created. ${NC}"
		else
        	echo "${RED}Failed to create docker volume directory${NC}"
			exit 1;
		fi
    fi
}

add_UWC_containers_In_EII()
{
    echo "${INFO}Copying UWC Containers in EII...${NC}"   
    cp -r ../Others/Config/UWC/Device_Config/* /opt/intel/eii/uwc_data
    cp ../Others/Config/UWC/Global_Config.yml /opt/intel/eii/uwc_data/common_config/Global_Config.yml
    copy_verification=$(echo $?)
    if [ "$copy_verification" -eq "0" ]; then
        echo "${GREEN}UWC containers are successfully copied ${NC}"
    else
        echo "${RED}failed to copy UWC containers.${NC}"
	    return 1
    fi
    return 0
}

modify_config()
{
	echo "Modifying the config file"
	search_dir="${working_dir}/../eii_configs/"
	for file_path in "$search_dir"/*
    do
     dir=$(basename "$file_path" .json)
	 if [ -d "${working_dir}/../../${dir}" ]; then
	     cp ${file_path} ${working_dir}/../../${dir}/config.json
		 echo "${GREEN}Done copying from ${file_path} to ${working_dir}/../../${dir}${NC}"
	 else
	     echo "${RED}${working_dir}/../../${dir} doesn't exists, Please check ${working_dir}/../eii_configs for valid config files${NC}"	 
	 fi	
    done
}

echo "${GREEN}============================= Script START ============================================${NC}"

create_docker_volume_dir
add_UWC_containers_In_EII
modify_config
