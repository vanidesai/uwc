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

 # Variable to store user proxy provided by command line
USER_PROXY=""
eis_working_dir="$Current_Dir/docker_setup"

# ----------------------------
# Set dev mode to true
# ----------------------------
setHostIP()
{
   echo "${INFO}Setting dev mode...${NC}"    
   sed -i 's/DEV_MODE=true/DEV_MODE=false/g' $working_dir/docker_setup/.env
   if [ "$?" -ne "0" ]; then
	echo "${RED}Failed to set dev mode."
	echo "${GREEN}Kinldy set DEV_MODE to false manualy in .env file and then re--run this script"
	exit 1
   else
	echo "${GREEN}Dev Mode is set.${NC}"
   fi
   echo "${INFO}Setting HOST_IP to 127.0.0.1 in .env file of EIS..${NC}"
   if grep -Fxq 'HOST_IP=127.0.0.1' $working_dir/docker_setup/.env
   then
   	echo "${GREEN}HOST_IP=127.0.0.1 variable already present in <>docker_setup/.env${NC}"
   else
   	echo 'HOST_IP=127.0.0.1' >> $working_dir/docker_setup/.env
   fi
   echo "${GREEN}HOST_IP=127.0.0.1 is set in .env file of EIS${NC}"

   return 0
}

# ----------------------------
# Set TTY ports for EIS user
# ----------------------------
configureRTUPorts()
{
	echo "${INFO}Checking TTY port setting..${NC}"
        rm -rf /etc/udev/rules.d/50-myusb.rules 
	if [ ! -f "/etc/udev/rules.d/50-myusb.rules" ];then
		cat >> /etc/udev/rules.d/50-myusb.rules <<EOF
KERNEL=="ttyUSB[0-9]*",MODE="0666"
KERNEL=="ttyS[0-31]*",MODE="0666"
KERNEL=="tty[0-63]*",MODE="0666"
EOF
                udevadm control --reload-rules && udevadm trigger
		#echo "KERNEL==\"ttyUSB[0-9]*\",MODE=\"0666\"" > /etc/udev/rules.d/50-myusb.rules
		#echo "KERNEL==\"ttyACM[0-9]*\",MODE=\"0666\"" > /etc/udev/rules.d/50-myusb.rules
		echo "${GREEN}TTY ports are configured successfully...${NC}"
		echo "${GREEN}Unplug the device and replug it, if connected...${NC}"
    	else
		# Files do not exist, display error message
		echo "${GREEN}TTY ports are already configured.${NC}" 
    	fi
}

# ----------------------------
# Checking for root user
# ----------------------------
checkrootUser()
{
   echo "${INFO}Checking for root user...${NC}"    
   if [[ $EUID -ne 0 ]]; then
    	echo "${RED}This script must be run as root.${NC}"
	echo "${GREEN}E.g. sudo ./<script_name>${NC}"
    	exit 1
   else
   	echo "${GREEN}Script is running with root user.${NC}"
	dpkg --configure -a
   fi
   return 0
}

# ----------------------------
# Changing file permissions
# ----------------------------
changeFilePermissions()
{
    chown -R ${SUDO_USER}:${SUDO_USER} *
}

verifyDirectory()
{
    echo "${INFO}Verifying the directory...${NC}"
    if [ ! -d ${working_dir}/docker_setup ]; then
    	echo "${RED}UWC installer files are not placed in right directory.${NC}"
	echo "${GREEN}Copy UWC installer files in EdgeInsightsSoftware-v2.2-PV/IEdgeInsights directory and re-run this script.${NC}"
	exit 1
    fi
    if [ ! -f UWC.tar.gz ] && [ ! f ${working_dir}/CopyUWCFiles.sh ]]; then
    	echo "${RED}UWC.tar.gz, CopyUWCFiles.sh files are not present in required directory.${NC}"
	echo "${GREEN}Copy all UWC installer files in EdgeInsightsSoftware-v2.2-PV/IEdgeInsights directory and re-run this script.${NC}"
	exit 1
    else
	echo "${GREEN}UWC files are present in current directory${NC}"
    fi

}

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

# ----------------------------
# Copying UWC Containers in EIS
# ----------------------------
addUWCContainersInEIS()
{
    echo "${INFO}Copying UWC Containers in EIS...${NC}"
    rm -rf UWC/ && mkdir UWC
    tar -xzvf UWC.tar.gz -C UWC > /dev/null 2>&1
    cd UWC
    cp -r modbus-master/ MQTT/ uwc_common/ mqtt-export/ ../
    cp docker-compose.yml ../docker_setup/samples/docker-compose-uwc.yml
    cp -r Others/Config/UWC/Device_Config/* /opt/intel/eis/uwc_data
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

#------------------------------------------------------------------------------ 
# copyInstallerFiles
#
# Description: 
#       copies EIS scripts and config for uwc to insights folder
# Args:
#       None
# Return:
#       None
# Usage:
#       copyInstallerFiles
#------------------------------------------------------------------------------
copyInstallerFiles()
{
	cd ${working_dir}/UWC
	cp Others/Config/UWC/EIS_Changes/* ../../installer/installation/src/insights/
    	echo "${GREEN}>>>>>${NC}"
   	echo "${GREEN}************************* UWC container configuration is sucessfully done ***************************************"
    	echo ""
    	echo "${GREEN}************************* Follow EIS Installer steps for further deployment *************************************"
	chown -R $SUDO_USER:$SUDO_USER ${working_dir}/../installer/
    	echo "${INFO}To execute unit test cases for UWC, run 06_UnitTestRun.sh script ${NC}"
	cd ${working_dir}/ && rm -rf UWC/
}

# Internal function calls to setup uwc containers
verifyDirectory
checkrootUser
setHostIP
configureRTUPorts
createDockerVolumeDir
addUWCContainersInEIS
copyInstallerFiles
changeFilePermissions

cd "${working_dir}"
exit 0
