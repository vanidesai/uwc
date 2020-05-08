#!/bin/bash
#
# Copyright (C) 2018-2019 Intel Corporation.
#

iei_install_fail=".iei_install_fail"
module_failed="${module}:failed"
module_success="${module}:success"
#------------------------------------------------------------------
# check_for_errors
#
# Description:
#        Check the return code of previous command and display either
#        success or error message accordingly.
# Args:
#        string : return-code of previous command
#        string : failuer message to display if return-code is non-zero
#        string : success message to display if return-code is zero (optional)
# Return:
#        None
# Usage:
#        check_for_errors "return-code" "failure-message" <"success-message">
#------------------------------------------------------------------
check_for_errors()
{
    return_code=${1}
    error_msg=${2}

    if [ "${return_code}" -ne "0" ];then
        echo "${RED}ERROR : (Error Code: ${return_code}) ${error_msg}${NC}"
        exit 1
    else
        if [ "$#" -ge "3" ];then
            success_msg=${3}
            echo ${success_msg}
        fi
    fi
    return 0
}

#------------------------------------------------------------------
# eisVisualize
#
# Description:
#        Installing visualizer for Edge Insights Software setup to display the output demo
# Return:
#        None
# Usage:
#        eisVisualize
#------------------------------------------------------------------
eisVisualize()
{
    # Counter to store Time for containers to be up.
    counter=0
    if [ -f "${HOME}/$iei_install_tmp_file" ];then
        USE_CASE=$(cat ${HOME}/$iei_install_tmp_file)
        if [ "${USE_CASE}" == "streaming" ];then
            eis_containers_status="ia_video_ingestion ia_video_analytics ia_etcd_ui"
            eis_containers="ia_visualizer ia_etcd"
        elif [ "${USE_CASE}" == "historical" ];then
            eis_containers_status="ia_video_ingestion ia_video_analytics ia_influxdbconnector ia_imagestore ia_etcd_ui"
            eis_containers="ia_etcd"
        elif [ "${USE_CASE}" == "timeseries" ];then
            eis_containers_status="ia_influxdbconnector ia_telegraf ia_dc ia_data_analytics ia_etcd_ui"
            eis_containers="ia_visualizer ia_etcd"
        elif [ "${USE_CASE}" == "strandts" ];then
            eis_containers_status="ia_video_ingestion ia_video_analytics ia_dc ia_influxdbconnector ia_telegraf ia_data_analytics ia_etcd_ui"
            eis_containers="ia_visualizer ia_etcd"
        elif [ "${USE_CASE}" == "UWC" ];then
            eis_containers_status="modbus-tcp-master modbus-rtu-master mqtt_container mqtt-export ia_etcd_ui"
            eis_containers="ia_etcd"
        else
            eis_containers_status="ia_video_ingestion ia_video_analytics ia_etcd_ui"
            eis_containers="ia_visualizer ia_etcd"
        fi
   else
       echo "${HOME}/$iei_install_tmp_file doesn't exist. Maybe Edge Insights setup not completed. Exiting"
       sed -i "s/${module_success}/${module_failed}/g" ${install_status_log}
       exit 1
   fi

   echo "${GREEN}Waiting for containers to be up and healthy, it'll take 5 to 6 minutes.${NC}"
   for eis_container_status in $eis_containers_status
   do
       container_health_check=$(docker ps | grep $eis_container_status | grep 'Up' | wc -l)
       while [[ "${container_health_check}" -lt "1" && "${counter}" -lt "15" ]]
       do
           sleep 1m
           counter=$(expr $counter + 1)
           container_health_check=$(docker ps | grep $eis_container_status | grep 'Up' | wc -l)
           if [ "$counter" -eq "15" ];then
               echo -e "${RED}ERROR: ${eis_container_status} Container is not Up even after 15 minutes,so please check 'docker ps'on the device, Exiting. \n**Note**: Use cmd: 'docker logs -f <<container-name>>' to find out the reason for container failure${NC}"
               sed -i "s/${module_success}/${module_failed}/g" ${install_status_log}
               exit 1
           fi
       done
       echo "${GREEN}$eis_container_status container is up and healthy.${NC}"
   done

   for eis_container in ${eis_containers}
   do
       container_check=$(docker ps | grep ${eis_container})
       if [ "$?" -eq "0" ];then
           echo "${GREEN}${eis_container} container is up${NC}"
       else
           echo "${RED}ERROR: ${eis_container} container doesn't exist. Please check logs${NC}"
           exit 1
       fi
   done
   echo "${GREEN}Edge Insights Software v2.2 setup is completed successfully and containers are up and healthy.${NC}"
   sudo xhost +
   echo "${GREEN}The demo will be displayed on the screen. Please wait.${NC}"
   if [ -f "${HOME}/${iei_install_tmp_file}" ];then
       rm -f "${HOME}/${iei_install_tmp_file}"
   fi
   return 0
}

if [ -f "${HOME}/${iei_install_fail}" ];then
   echo "${RED}Edge Insights installation failed or aborted by user. Skipping the post installation health check.${NC}"
   rm -f "${HOME}/${iei_install_fail}"
   sed -i "s/${module_success}/${module_failed}/g" ${install_status_log}
   exit 1
else
   eisVisualize
fi
exit 0
