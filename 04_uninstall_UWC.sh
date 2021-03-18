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

Current_Dir=$(pwd)/../build/
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

source uwc_common_lib.sh

clean_exit()
{
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

function uninstall_docker_compose()
{
    	# UNINSTALLING DOCKER-COMPOSE
    	echo -e "${INFO}--------------------------------------Uninstalling Docker-compose--------------------------------${NC}"
    	rm -rf $(which docker-compose)

	    pip3 show docker-compose 1>/dev/null
	    if [ $? == 0 ]; then
		    pip3 uninstall -y docker-compose >>/dev/null
	    else
	        echo "" 
	    fi

        #pip3 uninstall -y docker-compose >>/dev/null
        echo -e "${SUCCESS}---------------------------Docker-compose uninstalled successfully-------------------------------${NC}"   
}

function uninstall_docker()
{
    # UNINSTALLING DOCKER 
    echo -e "${INFO}---------------------------------------Uninstalling Docker---------------------------------------${NC}"
    
    dpkg --purge --force-all docker-ce docker-ce-cli containerd.io
    apt-get purge -y docker docker.io
    
    # Removing Docker GPG and removing the repository from sources
    apt-key del $DOCKER_GPG_KEY
    add-apt-repository --remove "$DOCKER_REPO"
    echo -e "${SUCCESS}-------------------------------Docker uninstalled successfully-----------------------------------${NC}" 
}
 
function removeUWCConfig
{
    #Delete UWC config
    echo -e "${INFO}---------------------------------------Removing UWC config-----------------------${NC}"
    del_file /opt/intel/eis/uwc_data
    del_file /opt/intel/eis/uwc_data/common_config
    del_file /opt/intel/eis/container_logs
    echo -e "${SUCCESS}-------------------------------Removed all UWC config------------------------------------${NC}"
    
    #RESET THE PROXY SETTING 
    echo -e "${INFO}---------------------------------------Resetting proxy setting-----------------------------------${NC}"
    del_file /etc/docker/daemon.json
    del_file /etc/systemd/system/docker.service.d/http-proxy.conf
    del_file /etc/systemd/system/docker.service.d/https-proxy.conf      
    del_file $HOME/.docker/config.json
    del_file /etc/systemd/system/docker.service.d
    echo -e "${SUCCESS}-------------------------------Proxy setting reset to default------------------------------------${NC}"
}

uninstall_containers()
{
    echo "${INFO}Bringing down all running containers from this machine${NC}"
    docker-compose down
    echo "${INFO}Removing all containers from this machine${NC}"
	docker rm -f $(docker ps -a -q)
	docker system prune -af --volumes
    
	if [ "$?" -ne "0" ];then
		echo "${RED}Failed to uninstall UWC contatiners.${NC}"
		exit 1
	else
		echo "${GREEN}Removed all the UWC containers.${NC}"
	fi
	return 0
}

cd ${Current_Dir}
check_root_user
uninstall_containers
uninstall_docker
uninstall_docker_compose
echo ""
echo -e "${GREEN}---------------------------------------Uninstallation Completed-----------------------------------${NC}"