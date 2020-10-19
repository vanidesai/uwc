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
   if [[ $EUID -eq 0 ]]; then
    	echo "${RED}Do not use sudo.${NC}"
	echo "${GREEN}E.g. ./<script_name>${NC}"
    	exit 1
   else
   	echo "${GREEN}Script is running with non-root user.${NC}"
   fi
   return 0
}

createUWCPackage()
{
	rm -rf ${Current_Dir}/Release/UWC.tar.gz
	tar  --exclude='modbus-master/bin'  --exclude='Old_scripts.zip' --exclude='create_uwc_package.sh' --exclude='README.md' modbus-master/ MQTT/  mqtt-export/ scada-rtu/ Others/ *.yml uwc_common/ DBS_Patches/ kpi-tactic/ -zcvf UWC.tar.gz 
	
	if [ "$?" -ne "0" ];then
		echo "${RED}Failed to create UWC package.${NC}"
		exit 1
	else
		echo "${GREEN}Successfully created UWC package.${NC}"
		mv UWC.tar.gz  ${Current_Dir}/Release
	fi
	return 0
}

checkrootUser
createUWCPackage
