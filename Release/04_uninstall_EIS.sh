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

Current_Dir=$(pwd)
RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
MAGENTA=$(tput setaf 5)
NC=$(tput sgr0)
SUCCESS='\033[0;32m'
INFO='\033[0;34m'

DOCKER_REPO="deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"
DOCKER_GPG_KEY="0EBFCD88"
EIS_USER_NAME=eisuser
EIS_UID=5319
EIS_INSTALL_PATH=/opt/intel/eis

turtlecreek_dir="$Current_Dir/../turtlecreek"

clean_exit(){
    echo -e "${ERROR}------------------------------Uninstallation failed. Exiting--------------------------------${NC}"
    exit 1
}

del_file() 
{
    if [[ -f $1 ]]; then
        rm -rf $1
        if [[ $? -ne 0 ]]; then
            clean_exit
        fi
    fi 
}

verifyDirectory()
{
    echo "Verifying the directory..."    
    if [ ! -d ${Current_Dir}/docker_setup ]; then
    	echo "${RED}UWC installer files are not placed in right directory.${NC}"
	echo "${GREEN}Copy UWC installer files in EdgeInsightsSoftware-v2.1-Alpha/IEdgeInsights directory and re-run this script.${NC}"
	exit 1
    fi
    if [ ! -f UWC.tar.gz ] && [ ! f ${Current_Dir}/01_pre-requisites.sh ] && [ ! f ${Current_Dir}/02_provisionEIS.sh.sh ] && [ ! f ${Current_Dir}/03_DeployEIS.sh.sh ] &&  [ ! f ${Current_Dir}/04_uninstall_EIS.sh ]; then
    	echo "${RED}UWC.tar.gz, 01_pre-requisites.sh, 02_provisionAndDeployEIS.sh, 03_uninstall_EIS.sh files are not present in required directory.${NC}"
	echo "${GREEN}Copy all UWC installer files in EdgeInsightsSoftware-v2.1-Alpha/IEdgeInsights directory and re-run this script.${NC}"
	exit 1
    else
	echo "${GREEN}UWC files are present in current directory${NC}"
    fi

}

# ----------------------------
# Checking for root user
# ----------------------------
checkrootUser()
{
   echo "Checking for root user..."    
   if [[ $EUID -ne 0 ]]; then
    	echo "${RED}This script must be run as root.${NC}"
	echo "${GREEN}E.g. sudo ./<script_name>${NC}"
    	exit 1
   else
   	echo "${GREEN}Script is running with root user.${NC}"
   fi
   return 0
}

function uninstallDockerCompose()
{
    # UNINSTALLING DOCKER-COMPOSE
    echo -e "${INFO}--------------------------------------Uninstalling Docker-compose--------------------------------${NC}"
    rm -rf $(which docker-compose)
    pip3 uninstall -y docker-compose >>/dev/null
    echo -e "${SUCCESS}---------------------------Docker-compose uninstalled successfully-------------------------------${NC}"   
}

function uninstallDocker()
{
    # UNINSTALLING DOCKER 
    echo -e "${INFO}---------------------------------------Uninstalling Docker---------------------------------------${NC}"
    
    dpkg --purge --force-all docker-ce docker-ce-cli containerd.io
    apt-get purge -y docker docker.io
    
    # Removing Docker GPG and removing the repository from sources
    apt-key del $DOCKER_GPG_KEY
    add-apt-repository --remove "$DOCKER_REPO"
    echo -e "${SUCCESS}-------------------------------Docker uninstalled successfully-----------------------------------${NC}" 
    
    #Delete UWC config
    echo -e "${INFO}---------------------------------------Removing UWC config-----------------------${NC}"
    del_file /opt/intel/eis/uwc_data
    del_file /opt/intel/eis/uwc_data/common_config
    echo -e "${SUCCESS}-------------------------------Removed all UWC config------------------------------------${NC}"
    
    #RESET THE PROXY SETTING 
    echo -e "${INFO}---------------------------------------Resetting proxy setting-----------------------------------${NC}"
    del_file /etc/docker/daemon.json
    del_file /etc/systemd/system/docker.service.d/http-proxy.conf
    del_file /etc/systemd/system/docker.service.d/https-proxy.conf      
    del_file $HOME/.docker/config.json
    del_file /etc/systemd/system/docker.service.d
    echo -e "${SUCCESS}-------------------------------Proxy setting reset to default------------------------------------${NC}"
    rm -rf  /opt/intel
}

removeEISUser()
{
    echo -e "${INFO}Removing eisuser if present. ${NC}"
    deluser eisuser
    groupdel eisuser
    echo -e "${GREEN}Done.${NC}"
}
uninstallContainers()
{
    echo "${INFO}Removing all containers from this machine${NC}"
	docker rm -f $(docker ps -a -q)
	docker system prune -af --volumes
	rm -rf UWC*
	rm -rf modbus-master/ MQTT/ mqtt-export/
    rm -rf docker_setup/provision/config/UWC/
	if [ "$?" -ne "0" ];then
		echo "${RED}Failed to uninstall UWC contatiners.${NC}"
		exit 1
	else
		echo "${GREEN}Removed all the UWC containers.${NC}"
	fi
	return 0
}

function removeTurtleCreek()
{
    echo -e "${INFO}---------------------------------------Uninstalling TC -----------------------------------${NC}"
    cd ${turtlecreek_dir}
    ./uninstall-tc.sh
    echo -e "${INFO}---------------------------------------Uninstalling TC Completed-----------------------------------${NC}"
}

cd ${Current_Dir}
verifyDirectory
checkrootUser
uninstallContainers
removeTurtleCreek
uninstallDocker
uninstallDockerCompose
#removeEISUser
echo ""
echo -e "${GREEN}---------------------------------------Uninstallation Completed-----------------------------------${NC}"
