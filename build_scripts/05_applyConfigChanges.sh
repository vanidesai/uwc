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

Current_Dir=$(pwd)/../../build/
RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
MAGENTA=$(tput setaf 5)
NC=$(tput sgr0)
	
source uwc_common_lib.sh	

apply_config_changes()
{
	cd ${Current_Dir}
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

function harden()
{
	docker container update --pids-limit=100 --restart=on-failure:5 --cpu-shares 512 -m 1G --memory-swap -1 mqtt-bridge
	docker container update --pids-limit=100 --restart=on-failure:5 --cpu-shares 512 -m 1G --memory-swap -1 modbus-tcp-master
	docker container update --pids-limit=100 --restart=on-failure:5 --cpu-shares 512 -m 1G --memory-swap -1 mqtt_container
	docker container update --pids-limit=100 --restart=on-failure:5 --cpu-shares 512 -m 1G --memory-swap -1 modbus-rtu-master
	docker container update --pids-limit=100 --restart=on-failure:5 --cpu-shares 512 -m 1G --memory-swap -1 ia_etcd
	docker container update --pids-limit=100 --restart=on-failure:5 --cpu-shares 512 -m 1G --memory-swap -1 ia_etcd_provision
	docker ps -q --filter "name=sparkplug-bridge" | grep -q . && docker container update --pids-limit=100 --restart=on-failure:5 --cpu-shares 512 -m 1G --memory-swap -1 sparkplug-bridge
	docker ps -q --filter "name=kpi-tactic-app" | grep -q . && docker container update --pids-limit=100 --restart=on-failure:5 --cpu-shares 512 -m 1G --memory-swap -1 kpi-tactic-app
}

export DOCKER_CONTENT_TRUST=1
export DOCKER_BUILDKIT=1
check_root_user
apply_config_changes
harden
