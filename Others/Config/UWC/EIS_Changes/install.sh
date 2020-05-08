#!/bin/bash
#
# Copyright (C) 2018-2019 Intel Corporation.
#

iei_install_dir="$Current_Dir/../../IEdgeInsights"
iei_working_dir="$Current_Dir/../../IEdgeInsights/docker_setup"
iei_install_tmp_file=".iei_install_continue"
openvino_version="2020.1"
iei_install_fail=".iei_install_fail"
iei_version_check=".iei_version"
DOWNLOAD_LINK=""

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

    if [[ "${return_code}" -ne "0" ]];then
        echo "${RED}ERROR : (Error Code: ${return_code}) ${error_msg}${NC}"
        cleanup
        touch "${HOME}/${iei_install_fail}"
        exit 1
    else
        if [ "$#" -ge "3" ];then
            success_msg=${3}
            echo ${success_msg}
        fi
    fi
    return 0
}

#------------------------------------------------------------------------------
# cleanup_openvino
#
# Description:
#       Cleans up openvino temporary files.
# Args:
#       None
# Return:
#       None
# Usage:
#       cleanup
#------------------------------------------------------------------------------
cleanup_openvino()
{
    echo "Cleaning up temporary files"
    if [ -f "${Current_Dir}/src/${module}/${OPENVINO_VERSION}.tgz" ];then
        echo "Removing file ${Current_Dir}/src/${module}/${OPENVINO_VERSION}.tgz"
        rm -f "${Current_Dir}/src/${module}/${OPENVINO_VERSION}".tgz*
        if [ "$?" -ne "0" ]; then
            echo "${MAGENTA}WARNING: Unable to remove ${Current_Dir}/src/${module}/${OPENVINO_VERSION}.tgz*.${NC}"
        fi
    fi
    echo "Cleaned up temporary files in the system."
    return 0
}

#------------------------------------------------------------------------------
# cleanup
#
# Description:
#       Cleans up temporary files.
# Args:
#       None
# Return:
#       None
# Usage:
#       cleanup
#------------------------------------------------------------------------------
cleanup()
{
    echo "Removing version in ${HOME}/${iei_version_check}"
    export iei_version=$(sudo grep -r "EIS_VERSION=" "$(pwd)/../../IEdgeInsights/docker_setup" | cut -d "=" -f2 | awk 'NR==1{print $1}')
    if [ -f "${HOME}/${iei_version_check}" ];then
        sed -i '/^$/d' "${HOME}/${iei_version_check}"
        sed -i '/^[[:space:]]*$/d' "${HOME}/${iei_version_check}"
        sed -i '/^ *$/d' "${HOME}/${iei_version_check}"
        grep "${iei_version}" "${HOME}/${iei_version_check}" > /dev/null
        if [ "$?" -ne "0" ]; then
            echo "${MAGENTA}WARNING: Unable to remove version ${HOME}/${iei_version_check}.${NC}"
        else
            [[ ! -z "${iei_version}" ]] && sed -i -r "/${iei_version}/d" "${HOME}/${iei_version_check}"
        fi
    fi
    return 0
}
#------------------------------------------------------------------
# Description:
# This is used for cleanup when received Ctrl + C signal.
#------------------------------------------------------------------
if [ ! -z "$1" ]; then
    cleanup_str=$(echo $1 | tr '[:upper:]' '[:lower:]')
    if [ "${cleanup_str}" == "cleanup" ]; then
        type cleanup &>/dev/null && cleanup ||  echo "No temporary files to cleanup"
    fi
    exit 0
fi

#------------------------------------------------------------------------------
# docker_check
#
# Description:
#         Check Docker engine installation
# Usage:
#        docker_check
#------------------------------------------------------------------------------
docker_check()
{
    # Check for docker packages installation
    sudo apt list --installed | grep 'docker-ce-cli\|containerd.io\|docker-ce' >& /dev/null
    if [ $? -eq 0 ];then
        # returns 0 if docker installed
        return 0
    else
        # returns 1 if docker not installed
        return 1
    fi
}

#------------------------------------------------------------------
# docker_verify
#
# Description:
#        Verify docker installation on the system with docker hello-world image
# Return:
#        None
# Usage:
#        docker_verify
#------------------------------------------------------------------
docker_verify()
{
    var=$(grep -w "insights:docker_verify" "${resume_insights}")
    if [ -z ${var} ]; then
        docker_check
        if [ $? -eq 0 ];then
            sudo docker run --rm hello-world
            if [ $? -eq 0 ];then
                echo "${GREEN}Docker verified successfully.${NC}"
            else
                echo "${RED}Docker is already installed but unable to pull the hello-world image.${NC}"
                echo "${RED}If your behind proxy network please check proxy settings for docker.${NC}"
                touch "${HOME}/${iei_install_fail}"
                exit 1
            fi
        else
            echo "${RED}Docker is not installed${NC}"
            echo "${MAGENTA}Docker can be installed with this script.Please go through README for more information${NC}"
            touch "${HOME}/${iei_install_fail}"
            exit 1
        fi
        sudo docker rmi -f hello-world
        echo "insights:docker_verify" >> "${resume_insights}"
    else
        echo "${MAGENTA}docker_verify already executed${NC}"
    fi
    return 0
}

#------------------------------------------------------------------
# docker_compose_verify
#
# Description:
#        Verify docker-compose installation on the system
# Return:
#        None
# Usage:
#        docker_compose_verify
#------------------------------------------------------------------
docker_compose_verify()
{
    var=$(grep -w "insights:docker_compose_verify" "${resume_insights}")
    if [ -z ${var} ]; then
        docker-compose --version
        check_for_errors "$?" "Docker-compose version is not showing. Please check docker-compose is installed on host machine." \
                        "${GREEN}Verified docker-compose successfully.${NC}"
        echo "insights:docker_compose_verify" >> "${resume_insights}"
    else
        echo "${MAGENTA}docker_compose_verify already executed${NC}"
    fi
    return 0
}

#------------------------------------------------------------------
# container_cleanup
#
# Description:
#        Delete/Removes the all the Edge Insights container and images
# Usage:
#        container_cleanup
#------------------------------------------------------------------
container_cleanup()
{
    eis_uninstall=${1}
    # Removing the containers
    container_list=$(docker ps -a | grep "${eis_uninstall}" | awk '{print $1}')
    docker stop ${container_list}
    docker rm ${container_list}
    if [ "$?" -ne "0" ]; then
        echo "${MAGENTA}Unable to remove the Edge Insights Software containers from the system.${NC}"
    else
        echo "${GREEN}Successfully removed Edge Insights Software containers from the system.${NC}"
    fi
    # Removing the dangling images
    dangling=$(docker images --filter "dangling=true" -q --no-trunc)
    if [ ! -z "${dangling}" ]; then
        docker rmi "${dangling}"
    fi
    return 0
}

#------------------------------------------------------------------
# uninstall_eis
#
# Description:
#        Checking for existing Edge Insights Software version on the system
#        and prompt the user for uninstallation.
# Return:
#        None
# Usage:
#        uninstall_eis
#------------------------------------------------------------------
uninstall_eis()
{
    eis_uninstall=${1}
    # User prompt to remove the  Edge Insights Software containers.
    while :
    do
        echo "Do you wish to remove Edge Insights Software ${eis_uninstall} containers ?"
        echo "Please choose 1 for 'Yes' and 2 for 'No'"
        echo "1) Yes"
        echo "2) No"

        read yn

        case ${yn} in
            1 )
                echo "${MAGENTA}WARNING: Removing Edge Insights Software ${iei_version} containers and dangling images from the system.${NC}"
                container_cleanup ${eis_uninstall}
                break;;
            2 )
                echo "${MAGENTA}Continuing with Edge Insights Software setup.${NC}"
                break;;
            * )
                echo "Entered incorrect input : ${yn}"
        esac
    done
    return 0
}

#------------------------------------------------------------------
# check_eis_version
#
# Description:
#        Checking for existing Edge Insights Software version on the system and
#        Edge Insights Software version from recipe, based on user choice the setup
#        will be done.
# Return:
#        None
# Usage:
#        check_eis_version
#------------------------------------------------------------------
check_eis_version()
{
    var=$(grep -w "insights:check_eis_version" "${resume_insights}")
    if [ -z ${var} ]; then
        #Checking existing Edge Insights Software version
        export iei_version=$(sudo grep -r "EIS_VERSION=" "${iei_working_dir}" | cut -d "=" -f2 | awk 'NR==1{print $1}')
        if [ -f "${HOME}/${iei_version_check}" ];then
            declare -a list version_array
            version_array=($(cat ${HOME}/${iei_version_check} | tr '\n' ' '))
            list_count=0
            list[$list_count]=${version_array[0]}
            for (( i=0; i<${#version_array[@]}; i++ ))
            do
                if [ $i -ne 0 ];then
                    j=`expr $i - 1`
                    if [ ${version_array[$j]} != ${version_array[$i]} ] && [ list[0] != ${version_array[$i]} ];then
                        list[$list_count]=${version_array[$i]}
                        ((list_count++))
                    fi
                else
                    list[$list_count]=${version_array[$i]}
                    ((list_count++))
                fi
            done
            if [[ ! -z ${#list[@]} && -s "${HOME}/${iei_version_check}" ]];then
                echo "${GREEN}Existing Edge Insights Software version(s): ${list[*]}.${NC}"
                for (( j=0; j<${#list[@]}; j++ ))
                do
                    uninstall_eis "${list[j]}"
                done
            fi
        fi

        # User prompt to install Edge Insights Software
        while :
        do
            echo "Do you wish to install Edge Insights Software ${iei_version} ?"
            echo "Please choose 1 for 'Yes' and 2 for 'No'"
            echo "1) Yes"
            echo "2) No"

            read yn

            case ${yn} in
                1 )
                    echo "${MAGENTA}WARNING: Installing Edge Insights Software ${iei_version}.${NC}"
                    break;;
                2 )
                    echo "${MAGENTA}Exiting Edge Insights Software ${iei_version} setup. Installation aborted by user.${NC}"
                    return 1;;
                * )
                    echo "Entered incorrect input : ${yn}"
            esac
        done
        echo "insights:check_eis_version" >> "${resume_insights}"
    else
        echo "${MAGENTA}check_eis_version already executed${NC}"
    fi
    return 0
}

#------------------------------------------------------------------
# eisPrerequisites
#
# Description:
#        Removing existing temporary files and downloading required packages
#        for Edge Insights Software setup
# Return:
#        None
# Usage:
#        eisPrerequisites
#------------------------------------------------------------------
eisPrerequisites()
{
    var=$(grep -w "insights:eisPrerequisites" "${resume_insights}")
    if [ -z ${var} ]; then
        # Installing dependent packages
        echo "Executing Pre-requisites for Edge Insights Software ${iei_version}"
        sudo apt-get update
        sudo apt-get -y install build-essential python3-pip
        check_for_errors "$?" "Failed to install Python3-pip package. Please check your logs." \
                        "${GREEN}Installation of Python3-pip package is success.${NC}"

        if [ -d "${HOME}/iei-images" ];then
            echo "Removing the existing Edge Insights Software ${iei_version} repository on host machine"
            sudo rm -rf "${HOME}/iei-images"
        fi
        # Updating permissions inside Insights folder
        find ${iei_working_dir} -name '*.sh' | xargs chmod 755
        find ${iei_working_dir} -name '*.py' | xargs chmod 755

        echo "insights:eisPrerequisites" >> "${resume_insights}"
        echo "${YELLOW}${BOLD}INFO: 15% of insights installation is done.${NC}"
    else
        echo "${MAGENTA}eisPrerequisites already executed${NC}"
        echo "${YELLOW}${BOLD}INFO: 15% of insights installation is done.${NC}"
    fi
    return 0
}

#------------------------------------------------------------------
# eis_openvino_url_check
#
# Description:
#        Verifies the given OpenVINO toolkit URL
# Return:
#        None
# Usage:
#        eis_openvino_url_check
#------------------------------------------------------------------
eis_openvino_url_check()
{
    DOWNLOAD_LINK="${1}"
    URL_REGEX='(https?|ftp|file)://[-A-Za-z0-9\+&@#/%?=~_|!:,.;]*[-A-Za-z0-9\+&@#/%=~_|]'
    URL_CHECK=$(echo $DOWNLOAD_LINK | grep "l_openvino_toolkit_")
    if [[ "${DOWNLOAD_LINK}"  =~ ${URL_REGEX} ]] && [ ! -z ${URL_CHECK} ];then
        VERSION=$(echo $DOWNLOAD_LINK | sed 's/.*l_openvino_toolkit_//' | cut -d"_" -f2- | rev | cut -d"." -f2- | rev)
        echo ""
        echo "Downloading OpenVINO version ${VERSION} in ${Current_Dir}/src/${module} directory"
        OPENVINO_VERSION=$(echo $DOWNLOAD_LINK | cut -d"/" -f 7 | awk -F'.tgz' '{ print $1 }')
        wget -c -T 10 "${DOWNLOAD_LINK}"
        check_for_errors "$?" "OpenVINO toolkit tar file download Failed. Please check your internet connection." \
                         "${GREEN}OpenVINO toolkit tar file download completed.${NC}"
        return 0
    else
        echo "The OpenVINO toolkit download link: ${RED}${DOWNLOAD_LINK}${NC} is invalid/incorrect. Please check your link address."
        return 1
    fi
    return 0
}
#------------------------------------------------------------------
# eis_openvino_download
#
# Description:
#        Download OpenVINO toolkit and extracts it into required
#        path for Edge Insights Software setup
# Return:
#        None
# Usage:
#        eis_openvino_download
#------------------------------------------------------------------
eis_openvino_download()
{
    var=$(grep -w "insights:eis_openvino_download" "${resume_insights}")
    if [ -z ${var} ]; then
        # Downloading OpenVINO 2020.1
        if [ -d ${Current_Dir}/src/${module} ];then
            cd ${Current_Dir}/src/${module}
        else
            echo "${RED}Insights installation source directory is not found${NC}"
        fi
        printf "OpenVINO Toolkit:\n${YELLOW}You can copy the link from the Intel\AE Distribution of OpenVINO\99 toolkit \ndownload page https://software.seek.intel.com/openvino-toolkit after registration.\nPlease choose version ${openvino_version} \nRight click on Offline Installer button on the download page for Linux in your browser and select Copy link address.\n${NC}"
        echo "Enter the OpenVINO toolkit download link address:"
        read DOWNLOAD_LINK
        VALID_URL=1
        while [ ${VALID_URL} -eq 1 ]
        do
            if [ -z ${DOWNLOAD_LINK} ];then
                echo "${RED}Download Link is empty/invalid. OpenVINO toolkit link is required to setup Edge Insight Software.${NC}"
                echo "Do you wish to enter OpenVINO toolkit link (y/n)?"
                read INPUT
                INPUT=$(echo ${INPUT} | tr '[:upper:]' '[:lower:]')
                case ${INPUT} in
                    y|yes)
                            echo ""
                            echo "Enter the OpenVINO toolkit download link address:"
                            read DOWNLOAD_LINK
                            if [ ! -z ${DOWNLOAD_LINK} ];then
                                eis_openvino_url_check ${DOWNLOAD_LINK}
                                VALID_URL=$?
                                [ $VALID_URL -eq 1 ] && DOWNLOAD_LINK=""
                            else
                                echo "Download Link is empty. OpenVINO toolkit link is required to setup Edge Insight Software."
                                VALID_URL=1
                            fi
                            ;;
                    n|no)
                            echo "${GREEN}Continuing the Edge Insights Software setup without OpenVINO toolkit.${NC}"
                            break;;
                    * )
                            echo "Entered incorrect input : ${INPUT}"
                esac
            else
                eis_openvino_url_check ${DOWNLOAD_LINK}
                VALID_URL=$?
                [ $VALID_URL -eq 1 ] && DOWNLOAD_LINK=""
            fi
        done

        if [[ -d "${iei_install_dir}/common/openvino" && -f "${OPENVINO_VERSION}.tgz" ]]; then
            echo "Extracting ${OPENVINO_VERSION}.tgz in path ${iei_install_dir}/common/openvino"
            tar -xzf "${OPENVINO_VERSION}".tgz -C "${iei_install_dir}"/common/openvino/
            if [ "$?" -ne "0" ];then
                echo "${RED}Unable to extract the ${OPENVINO_VERSION}.tgz file. Exiting${NC}"
                sudo rm -f ${OPENVINO_VERSION}.tgz
                echo "${GREEN}Removed the existing ${OPENVINO_VERSION}.tgz, may be it is corrupted/unfinished.${NC}"
                touch "${HOME}/${iei_install_fail}"
                exit 1
            else
                echo "${GREEN}Extracted ${OPENVINO_VERSION}.tgz successfully${NC}"
            fi
        else
            echo "${RED}${iei_install_dir}/common/openvino or ${OPENVINO_VERSION}.tgz does't exist. Exiting${NC}"
            touch "${HOME}/${iei_install_fail}"
            exit 1
        fi

        #### [Requirement for video analytics container]
        count=$(ls "${iei_install_dir}/common/openvino/" | grep l_openvino_toolkit_ | wc -l)
        if [ "${count}" != "1" ];then
            if [ "${count}" > "1" ];then
                echo "${RED}ERROR: There should be only one l_openvino_toolkit_p_xxxxx folder inside ${iei_install_dir}/common/openvino/${NC}"
                touch "${HOME}/${iei_install_fail}"
                exit 1 # terminate and indicate error
            else
                echo "${RED}ERROR: There should be compulsory one l_openvino_toolkit_p_xxxxx folder inside ${iei_install_dir}/common/openvino/ ${NC}"
                touch "${HOME}/${iei_install_fail}"
                exit 1 # terminate and indicate error
            fi
        else
            echo "${GREEN}Verified that there is only one ${iei_install_dir}/common/openvino/l_openvino_toolkit_p_xxxxx successfully.${NC}"
        fi
        echo "insights:eis_openvino_download" >> "${resume_insights}"
        echo "${YELLOW}${BOLD}INFO: 20% of setup execution is completed.${NC}"
    else
        echo "${MAGENTA}eis_openvino_download already executed${NC}"
        echo "${YELLOW}${BOLD}INFO: 20% of setup execution is completede.${NC}"
    fi
    return 0
}
#------------------------------------------------------------------
# eis_use_cases
#
# Description:
#        Removing existing temporary files and downloading required packages
#        for Edge Insights Software setup
# Return:
#        None
# Usage:
#        eis_use_cases
#------------------------------------------------------------------
eis_use_cases()
{
    while :
        do
            echo ""
            echo "${YELLOW}INFO: By default Edge insights comes with Streaming setup configurations.${NC}"
            echo "Please choose use case for Edge Insights software ${iei_version} image build?"
            echo "1) Streaming  ${GREEN}(Requires OpenVINO toolkit download link)${NC}"
            echo "2) Historical ${GREEN}(Requires OpenVINO toolkit download link)${NC}"
            echo "3) Time series"
            echo "4) Streaming + Time series ${GREEN}(Requires OpenVINO toolkit download link)${NC}"
            echo "5) UWC"

            read yn

            case ${yn} in
                1)
                    if [ -f "${iei_working_dir}/docker-compose_prod.yml" ];then
                       cp -f "${iei_working_dir}/docker-compose_prod.yml" "${iei_working_dir}/docker-compose.yml"
                    fi
                    echo "${GREEN}Continuing with Streaming use case for Edge Insights Software ${iei_version}.${NC}"
                    export USE_CASE="streaming"
                    eis_openvino_download
                    break;;

                2)
                    if [ -f "${iei_working_dir}/samples/docker-compose-video-streaming-and-historical-usecase.yml" ];then
                        cp -f "${iei_working_dir}/samples/docker-compose-video-streaming-and-historical-usecase.yml" "${iei_working_dir}/docker-compose.yml"
                    else
                        echo "${iei_working_dir}/samples/docker-compose-video-streaming-and-historical-usecase.yml doesn't exist. Exiting"
                    fi
                    echo "${GREEN}Continuing with Historical use case for Edge Insights Software ${iei_version}.${NC}"
                    export USE_CASE="historical"
                    eis_openvino_download
                    break;;
                3)
                    if [ -f "${iei_working_dir}/samples/docker-compose-timeseries-usecase.yml" ];then
                        cp -f "${iei_working_dir}/samples/docker-compose-timeseries-usecase.yml" "${iei_working_dir}/docker-compose.yml"
                    else
                        echo "${iei_working_dir}/samples/docker-compose-timeseries-usecase.yml doesn't exist. Exiting"
                    fi
                    echo "${GREEN}Continuing with Timeseries use case for Edge Insights Software ${iei_version}.${NC}"
                    export USE_CASE="timeseries"
                    break;;
                4)
                    if [ -f "${iei_working_dir}/samples/docker-compose-video-streaming-timeseries-usecase.yml" ];then
                        cp -f "${iei_working_dir}/samples/docker-compose-video-streaming-timeseries-usecase.yml" "${iei_working_dir}/docker-compose.yml"
                    else
                        echo "${iei_working_dir}/samples/docker-compose-video-streaming-timeseries-usecase.yml doesn't exist. Exiting"
                    fi
                    echo "${GREEN}Continuing with Streaming + Timeseries use case for Edge Insights Software ${iei_version}.${NC}"
                    export USE_CASE="strandts"
                    eis_openvino_download
                    break;;
                5)
                    if [ -f "${iei_working_dir}/samples/docker-compose-uwc.yml" ];then
                        cp -f "${iei_working_dir}/samples/docker-compose-uwc.yml" "${iei_working_dir}/docker-compose.yml"
                    else
                        echo "${iei_working_dir}/samples/docker-compose-uwc.yml doesn't exist. Exiting"
                    fi
                    echo "${GREEN}Continuing with uwc use case for Edge Insights Software ${iei_version}.${NC}"
                    export USE_CASE="UWC"
                    break;;

                * )
                    echo "Entered incorrect input : ${yn}"
            esac
    done
    return 0
}

#------------------------------------------------------------------
# eis_mode
#
# Description:
#        Removing existing temporary files and downloading required packages
#        for Edge Insights Software setup
# Return:
#        None
# Usage:
#        eis_mode
#------------------------------------------------------------------
eis_mode()
{
    if [ ! -e "${iei_working_dir}/docker-compose_prod.yml" ];then
        cp -f ${iei_working_dir}/docker-compose.yml ${iei_working_dir}/docker-compose_prod.yml
    fi
    while :
        do
            echo ""
            echo "${YELLOW}INFO: Edge Insights Software can be installed in DEVELOPER or PRODUCTION mode.${NC}"
            echo "${YELLOW}INFO: By default Edge Insights Software is provisioned in PRODUCTION mode (Secure mode)${NC}"
            echo "Do you wish to continue with the production mode ?"
            echo "Please choose 'yes' for production mode and 'no' for developer mode"
            echo "yes|y|Yes|Y|YES) PRODUCTION  (Secure mode)"
            echo "no|N|No|NO|n)    DEVELOPER   (Security disabled)"

            read yn

            case ${yn} in
                yes|y|Yes|Y|YES)

                    if [ -f "${iei_working_dir}/.env" ];then
                        grep "DEV_MODE=true" "${iei_working_dir}/.env"
                        if [ "$?" -eq "0" ];then
                            sed -i 's/DEV_MODE=true/DEV_MODE=false/g' "${iei_working_dir}/.env"
                        fi
                    else
                        echo "${RED}Edge insights configuration file (${iei_working_dir}/.env) is not found.${NC}"
                        touch "${HOME}/${iei_install_fail}"
                        exit 1
                    fi
                    echo "${GREEN}Continuing Edge Insights Software ${iei_version} setup in production mode.${NC}"
                    eis_use_cases
                    break;;

                no|n|No|N|NO)

                    if [ -f "${iei_working_dir}/.env" ];then
                        grep "DEV_MODE=false" "${iei_working_dir}/.env"
                        if [ "$?" -eq "0" ];then
                            sed -i 's/DEV_MODE=false/DEV_MODE=true/g' "${iei_working_dir}/.env"
                        fi
                    else
                        echo "${RED}Edge insights configuration file (${iei_working_dir}/.env) is not found.${NC}"
                        touch "${HOME}/${iei_install_fail}"
                        exit 1
                    fi
                    echo "${GREEN}Continuing Edge Insights Software ${iei_version} setup in developer mode.${NC}"
                    eis_use_cases

                    if [ -f "${iei_working_dir}/docker-compose.yml" ];then
                        sed -e '/\ \ \ \ secrets\:/ s/^#*/#/g' -i "${iei_working_dir}/docker-compose.yml"
                        sed -e '/\-\ ca\_etcd/ s/^#*/#/g' -i "${iei_working_dir}/docker-compose.yml"
                        sed -e '/\-\ etcd\_/ s/^#*/#/g' -i "${iei_working_dir}/docker-compose.yml"
                    else
                        echo "${RED}${iei_working_dir}/docker-compose.yml doesn't exist. Exiting${NC}"
                        touch "${HOME}/${iei_install_fail}"
                        exit 1
                    fi
                    break;;

                * )
                    echo "Entered incorrect input : ${yn}"
            esac
    done
    return 0
}


### <u>Build & Installation</u>
### Building the Edge Insights Software containers from source

#------------------------------------------------------------------
# eisProvision
#
# Description:
#        Reads certificates and save it securely by storing it in the Hashicorp Vault
# Return:
#        None
# Usage:
#        eisProvision
#------------------------------------------------------------------
eisProvision()
{
    var=$(grep -w "insights-${USE_CASE}:eisProvision" "${resume_insights}")
    if [ -z ${var} ]; then
        if [ -d "${iei_working_dir}/provision/" ];then
            cd "${iei_working_dir}/provision/"
        else
            echo "${RED}ERROR: ${iei_working_dir}/provision/ is not present.${NC}"
            touch "${HOME}/${iei_install_fail}"
            exit 1 # terminate and indicate error
        fi

        sudo ./provision_eis.sh ../docker-compose.yml
        check_for_errors "$?" "Provision secrets to vault failed. Please check logs" \
                        "${GREEN}Provision the secrets to vault is completed successfully.${NC}"
    echo "${YELLOW}${BOLD}INFO: 40% of setup execution is completed.${NC}"
    echo "insights-${USE_CASE}:eisProvision" >> "${resume_insights}"
    else
        echo "${MAGENTA}eisProvision already executed${NC}"
        echo "${YELLOW}${BOLD}INFO: 40% of setup execution is completed.${NC}"
    fi
    return 0
}

#------------------------------------------------------------------
# eisImageBuild
#
# Description:
#        the Edge Insights Software is installed as a systemd service so that it comes up automatically
#        on machine boot and starts the analytics process.
# Return:
#        None
# Usage:
#        eisImageBuild
#-----------------------------------------------------------------
eisImageBuild()
{
    var=$(grep -w "insights:eisImageBuild" "${resume_insights}")
    if [ -z ${var} ]; then
        cd "${iei_working_dir}"
        docker-compose up --build -d
        if [ "$?" -eq "0" ];then
            cd "${Current_Dir}"
            iei_docker_images=$(cat src/insights/images_${USE_CASE}.list)
            for iei_docker_image in $iei_docker_images
            do
                docker_image_name=$(docker images | grep ${iei_docker_image} | grep "${iei_version}")
                check_for_errors "$?" "${iei_docker_image} image is not exist on host machine. Please check with docker images." \
                        "${GREEN}${iei_docker_image} image is exist on host machine.${NC}"
            done
            touch "${HOME}/$iei_install_tmp_file"
            if [ ! -e "${HOME}/${iei_version_check}" ];then
                touch "${HOME}/${iei_version_check}"
            fi
            echo ${iei_version} >> "${HOME}/${iei_version_check}"
            echo "${USE_CASE}" > "${HOME}/$iei_install_tmp_file"
            echo "${GREEN}Installed Edge Insights Software ${iei_version} successfully.${NC}"
            if [ -f "${HOME}/${iei_install_fail}" ];then
                rm -f "${HOME}/${iei_install_fail}"
            fi
            echo "${YELLOW}${BOLD}INFO: 90% of setup execution is completed.${NC}"
        else
            echo "${RED}Edge Insights Software ${iei_version} install failed. Please check logs.${NC}"
            touch "${HOME}/${iei_install_fail}"
            if [ -f "${HOME}/${iei_version_check}" ];then
                sed -i '/^$/d' "${HOME}/${iei_version_check}"
                sed -i '/^[[:space:]]*$/d' "${HOME}/${iei_version_check}"
                sed -i '/^ *$/d' "${HOME}/${iei_version_check}"
                grep "${iei_version}" "${HOME}/${iei_version_check}" > /dev/null
                if [ $? -eq 0 ];then
                    [[ ! -z "${iei_version}" ]] && sed -i -r "/${iei_version}/d" "${HOME}/${iei_version_check}"
                fi
            fi
            exit 1
        fi
        echo "insights:eisImageBuild" >> "${resume_insights}"
    else
        echo "${MAGENTA}eisImageBuild already executed${NC}"
        echo "${YELLOW}${BOLD}INFO: 90% of setup execution is completed.${NC}"
    fi
    return 0
}

#Internal function calls to set up Edge Insights Software
echo "Installation of insights is started"
docker_verify
docker_compose_verify
check_eis_version
if [ "$?" -eq "0" ];then
    eisPrerequisites
    eis_mode
    eisProvision
    eisImageBuild
    echo "Installation of insights is completed"
else
    # User doesn't want install Edge Insights Software. Exiting the setup.
    touch "${HOME}/${iei_install_fail}"
    cleanup_openvino
    exit 1
fi

#Redirect to working directory
cd "${Current_Dir}"
exit 0
