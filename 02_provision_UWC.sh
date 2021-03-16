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
Dev_Mode="false"
source uwc_common_lib.sh

#------------------------------------------------------------------
# eis_provision
#
# Description:
#        Reads certificates and save it securely by storing it in the Hashicorp Vault
# Return:
#        None
# Usage:
#       eis_provision
#------------------------------------------------------------------
eis_provision()
{
    docker stop $(docker ps -a -q)
    if [ -d "${eis_build_dir}/provision/" ];then
        cd "${eis_build_dir}/provision/"
    else
        echo "${RED}ERROR: ${eis_build_dir}/provision/ is not present.${NC}"
        exit 1 # terminate and indicate error
    fi

    ./provision_eis.sh ../docker-compose.yml
    check_for_errors "$?" "Provisioning is failed. Please check logs" \
                    "${GREEN}Provisioning is done successfully.${NC}"
    echo "${GREEN}>>>>>${NC}"
    echo "${GREEN}For building & running UWC services,run 03_Build_Run_UWC.sh script${NC}"
    return 0
}

harden()
{
    docker container update --pids-limit=100 --restart=on-failure:5 --cpu-shares 512 -m 1G --memory-swap -1 ia_etcd
    docker container update --pids-limit=100 --restart=on-failure:5 --cpu-shares 512 -m 128M --memory-swap -1 ia_etcd_provision
}

configure_usecase()
{
    echo "Please choose one of the below options based on the use case (combination of UWC services) needed." 
    echo "1) Basic UWC micro-services without KPI-tactic app & Scada - (Modbus-master TCP & RTU, mqtt-export, internal mqtt broker, ETCD server, ETCD UI & other base EIS & UWC services)"
    echo "2) Basic UWC micro-services as in option 1 along with KPI-tactic app (Without Scada-RTU)"
    echo "3) Basic UWC micro-services & KPI-tactic app alonh with Scada-RTU"
    cd ${eis_build_dir}
    read yn

    while :
    do
        # read yn
        case $yn in
	        1)
		        echo "Running Basic UWC micro-services without KPI-tactic app & Scada-RTU"
                python3.6 eis_builder.py -f uwc-pipeline-without-scada.yml
                if [ "$?" != 0 ]; then
                    echo "${RED}Error running EIS builder script. Check the recipe configuration file...!${NC}" 
                    exit 1
                else
                    echo "${GREEN}EIS builder script successfully generated consolidated docker-compose & configuration files.${NC}"
                fi
                break
                ;;
	        2)
		        echo "Running Basic UWC micro-services with KPI-tactic app & without Scada-RTU"
                python3.6 eis_builder.py -f uwc-pipeline-with-kpi-no-scada.yml
                if [ "$?" != 0 ]; then
                    echo "${RED}Error running EIS builder script. Check the recipe configuration file...!${NC}" 
                    exit 1
                else
                    echo "${GREEN}EIS builder script successfully generated consolidated docker-compose & configuration files.${NC}"
                fi
                break
                ;;
            3)
                echo "Running Basic UWC micro-services with KPI-tactic app & with Scada-RTU"
                python3.6 eis_builder.py -f uwc-pipeline-with-scada.yml
                if [ "$?" != 0 ]; then
                    echo "${RED}Error running EIS builder script. Check the recipe configuration file...!${NC}" 
                    exit 1
                else
                    echo "${GREEN}EIS builder script successfully generated consolidated docker-compose & configuration files.${NC}"
                fi
                break
                ;;
	        *)
		        echo "Proper use-case option not selected. PLease select the right option as per help menu & re-provision UWC micro-services: ${yn}"
                exit 1
            esac
        done
}

set_mode()
{
    echo "Please choose one of the below options based on dev or prod mode. " 
    echo "1) Dev"
    echo "2) Prod"
    cd ${eis_build_dir}
    read mode
    while :
        do
            # read mode
            case $mode in
	            1)
		            echo "User inputted dev mode"
                    echo "${INFO}Setting dev mode to true ${NC}"    
	                sed -i 's/DEV_MODE=false/DEV_MODE=true/g' $eis_build_dir/.env
                    sed -i 's/MQTT_PROTOCOL=ssl/MQTT_PROTOCOL=tcp/g' $eis_build_dir/.env
                    
	                if [ "$?" -ne "0" ]; then
			            echo "${RED}Failed to set dev mode."
			            echo "${GREEN}Kinldy set DEV_MODE to false manualy in .env file and then re--run this script"
			            exit 1
	                else
			            echo "${GREEN}Dev Mode is set to true ${NC}"
	                fi
                    break
                    ;;
	            2)
		            echo "User inputted prod mode"
                    echo "${INFO}Setting dev mode to false ${NC}"    
		            sed -i 's/DEV_MODE=true/DEV_MODE=false/g' $eis_build_dir/.env
                    sed -i 's/MQTT_PROTOCOL=tcp/MQTT_PROTOCOL=ssl/g' $eis_build_dir/.env
		            if [ "$?" -ne "0" ]; then
			            echo "${RED}Failed to set dev mode."
			            echo "${GREEN}Kinldy set DEV_MODE to false manualy in .env file and then re--run this script"
		    	        exit 1
	                else
			            echo "${GREEN}Dev Mode is set to false ${NC}"
	                fi
                    break
                    ;;
	            *)
		            echo "User entered wrong option , hence defaulting to prod mode: ${yn}"
                    exit 1
            esac
        done
}

export DOCKER_CONTENT_TRUST=1
check_root_user
check_internet_connection
docker_verify
docker_compose_verify
set_mode
configure_usecase
eis_provision
harden