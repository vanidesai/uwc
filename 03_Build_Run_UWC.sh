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
resume_insights="${HOME}/.resume_process"

eis_build_dir="$Current_Dir/../build"
source uwc_common_lib.sh

#------------------------------------------------------------------
# build_run_UWC
#
# Description:
#        This fun will install all UWC containers
# Return:
#        None
# Usage:
#        build_run_UWC
#-----------------------------------------------------------------
build_run_UWC()
{
    cd "${eis_build_dir}"
    docker-compose up --build -d
    if [ "$?" -eq "0" ];then
	echo "*****************************************************************"
        echo "${GREEN}Installed UWC containers successfully.${NC}"
    else
        echo "${RED}Failed to install UWC containers.${NC}"
	echo "*****************************************************************"
        exit 1
    fi
    return 0
}

verify_container()
{
    cd "${eis_build_dir}"
    echo "*****************************************************************"
    echo "${GREEN}Below is the containers deployed status.${NC}"
    echo "*****************************************************************"
    docker ps
    return 0
}

function harden()
{
	docker container update --pids-limit=100 --restart=on-failure:5 --cpu-shares 512 -m 1G --memory-swap -1 mqtt-export
	docker container update --pids-limit=100 --restart=on-failure:5 --cpu-shares 512 -m 1G --memory-swap -1 modbus-tcp-master
	docker container update --pids-limit=100 --restart=on-failure:5 --cpu-shares 512 -m 1G --memory-swap -1 mqtt_container
	docker container update --pids-limit=100 --restart=on-failure:5 --cpu-shares 512 -m 1G --memory-swap -1 modbus-rtu-master
	docker container update --pids-limit=100 --restart=on-failure:5 --cpu-shares 512 -m 1G --memory-swap -1 ia_etcd
	docker container update --pids-limit=100 --restart=on-failure:5 --cpu-shares 512 -m 1G --memory-swap -1 ia_etcd_provision
	docker ps -q --filter "name=scada-rtu" | grep -q . && docker container update --pids-limit=100 --restart=on-failure:5 --cpu-shares 512 -m 1G --memory-swap -1 scada-rtu
	docker ps -q --filter "name=kpi-tactic-app" | grep -q . && docker container update --pids-limit=100 --restart=on-failure:5 --cpu-shares 512 -m 1G --memory-swap -1 kpi-tactic-app
}

function main()
{
    echo "${INFO}Deployment started${NC}"
    STARTTIME=$(date +%s)
    check_root_user
    check_internet_connection
    docker_verify
    docker_compose_verify
    build_run_UWC
    verify_container
    harden
    echo "${INFO}Deployment Completed${NC}"
    ENDTIME=$(date +%s)
    ELAPSEDTIME=$(( ${ENDTIME} - ${STARTTIME} ))
    echo "${GREEN}Total Elapsed time is : $(( ${ELAPSEDTIME} / 60 )) minutes ${NC}"
}

export DOCKER_CONTENT_TRUST=0
main

cd "${Current_Dir}"
exit 0
