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
	docker system prune -a --volumes
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
checkrootUser
uninstallEIS
