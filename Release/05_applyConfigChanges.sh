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
	

verifyDirectory()
{  
    if [ ! -d ${Current_Dir}/docker_setup ]; then
    	echo "${RED}UWC installer files are not placed in right directory.${NC}"
	echo "${GREEN}Copy requirted files in EdgeInsightsSoftware-vX.X-Alpha/IEdgeInsights directory and re-run this script.${NC}"
	exit 1
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

applyConfigChanges()
{
	cd ${Current_Dir}/docker_setup/
	export PWD=$(pwd)
	echo "${GREEN}Stopping all the runninng containers to apply new changes.${NC}" 
	docker-compose down
	if [ "$?" -eq "0" ];then
		echo "${GREEN} Successfully stopped all running containers.${NC}"
	else
		echo "${RED}Failed to stop running containers.${NC}"
	fi
	docker-compose up -d
	if [ "$?" -eq "0" ];then
		echo "${GREEN}Successfully started all running containers with updated changes.${NC}"
	else
		echo "${RED}Failed to start running containers.${NC}"
	fi
}

verifyDirectory
checkrootUser
applyConfigChanges
