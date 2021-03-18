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

eis_build_dir="$Current_Dir/../build"

# trap ctrl-c and call ctrl_c()
trap ctrl_c INT

source uwc_common_lib.sh

function ctrl_c()
{
        echo "** Trapped CTRL-C"
		echo "cleanup..."
		rm -rf $Current_Dir/temp1
		echo "Exiting the script..."
		exit 0
}

set_dev_mode()
{
   if [ "$1" == "true" ]; then
	   echo "${INFO}Setting dev mode to true ${NC}"    
	   sed -i 's/DEV_MODE=false/DEV_MODE=true/g' $eis_build_dir/.env
	   if [ "$?" -ne "0" ]; then
			echo "${RED}Failed to set dev mode."
			echo "${GREEN}Kinldy set DEV_MODE to false manualy in .env file and then re--run this script"
			exit 1
	   else
			echo "${GREEN}Dev Mode is set to true ${NC}"
	   fi
	else
		echo "${INFO}Setting dev mode to false ${NC}"    
		sed -i 's/DEV_MODE=true/DEV_MODE=false/g' $eis_build_dir/.env
		if [ "$?" -ne "0" ]; then
			echo "${RED}Failed to set dev mode."
			echo "${GREEN}Kinldy set DEV_MODE to false manualy in .env file and then re--run this script"
			exit 1
	   else
			echo "${GREEN}Dev Mode is set to false ${NC}"
	   fi
	fi
	sed -i 's/MQTT_PROTOCOL=ssl/MQTT_PROTOCOL=tcp/g' $eis_build_dir/.env
}

#------------------------------------------------------------------
# generate_unit_test_report
#
# Description:
#        the Edge Insights Software is installed as a systemd service so that it comes up automatically
#        on machine boot and starts the analytics process.
# Return:
#        None
# Usage:
#        generate_unit_test_report
#-----------------------------------------------------------------
generate_unit_test_report()
{
    cd "${eis_build_dir}"
    docker-compose up --build -d
    if [ "$?" -eq "0" ];then
	echo "*****************************************************************"
        echo "${GREEN}Successfully deployed unit test containers.${NC}"
    else
        echo "${RED}Installation is failed.${NC}"
	echo "*****************************************************************"
        exit 1
    fi
    return 0
}

create_test_dir()
{
	cd "${eis_build_dir}"
	rm -rf unit_test_reports
	mkdir -p unit_test_reports/modbus-tcp-master
	mkdir -p unit_test_reports/modbus-rtu-master
	mkdir -p unit_test_reports/mqtt-export
	mkdir -p unit_test_reports/scada-rtu
    mkdir -p unit_test_reports/kpi-tactic
	mkdir -p unit_test_reports/uwc-util
	chown -R $SUDO_USER:$SUDO_USER unit_test_reports
	chmod -R 777 unit_test_reports
	cd "${eis_build_dir}"
}

function cleanup()
{
	cd $Current_Dir
	chown -R $SUDO_USER:$SUDO_USER ${eis_build_dir}/unit_test_reports
	docker stop $(docker ps -a -q)
	sed -i 's/MQTT_PROTOCOL=tcp/MQTT_PROTOCOL=ssl/g' $eis_build_dir/.env
}

eis_provision()
{
    if [ -d "${eis_build_dir}/provision/" ];then
        cp -f ${eis_build_dir}/../uwc/docker-compose_UT.yml ${eis_build_dir}/docker-compose.yml
        cp -f ${eis_build_dir}/../uwc/eis_config_UT.json ${eis_build_dir}/provision/config/eis_config.json
        cd "${eis_build_dir}/provision/"
    else
        echo "${RED}ERROR: ${eis_build_dir}/provision/ is not present.${NC}"
        exit 1 # terminate and indicate error
    fi

    docker stop $(docker ps -a -q)
    ./provision_eis.sh ../docker-compose.yml
    check_for_errors "$?" "Provisioning is failed. Please check logs" \
                    "${GREEN}Provisioning is done successfully.${NC}"
    echo "${GREEN}>>>>>${NC}"
    echo "${GREEN}For building & running UWC services,run 03_Build_Run_UWC.sh script${NC}"
    return 0
}

#------------------------------------------------------------------
# create_scada_config_file_ut
#
# Description:
#        This function is used create scada_config.yml file required for scada-rtu container for unit testing
# Return:
#        None
# Usage:
#        create_scada_config_file_ut
#------------------------------------------------------------------
create_scada_config_file_ut()
{
rm -rf /opt/intel/eis/uwc_data/scada-rtu/scada_config.yml
cat > /opt/intel/eis/uwc_data/scada-rtu/scada_config.yml << ENDOFFILE
---
# scada config parameter file

isTLS: false
mqttServerAddrSCADA: "127.0.0.1"
mqttServerPortSCADA: 11883 
qos: 1
ENDOFFILE
}

checkrootUser
checkInternetConnection
docker_verify
docker_compose_verify
create_test_dir
set_dev_mode "false"
eis_provision
create_scada_config_file_ut
generate_unit_test_report

echo "Generating code coverage report..."
echo "It may take few minutes ..."
### To sleep for 2 min: ##
sleep 90s
cleanup
exit 0
