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

uninstallEIS()
{
	docker rm -f $(docker ps -a -q)
	docker system prune -af --volumes
	rm -rf UWC/
	rm -rf modbus-master/ MQTT/ mqtt-export/
    	rm -rf docker_setup/provision/config/UWC/
	if [ "$?" -ne "0" ];then
		echo "${RED}Failed to uninstall EIS package.${NC}"
		exit 1
	else
		echo "${GREEN}Successfully uninstall.${NC}"
	fi
	return 0
}

cd ${Current_Dir}
verifyDirectory
checkrootUser
uninstallEIS
