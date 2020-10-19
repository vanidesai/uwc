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

eis_working_dir="$Current_Dir/docker_setup"
CA_DIR=${Current_Dir}/tmp_certs/ca
CLIENT_CERT_DIR=${Current_Dir}/tmp_certs/client_crt
CLIENT_KEY_DIR=${Current_Dir}/tmp_certs/client_key

verifyDirectory()
{
    echo "Verifying the directory..."    
    if [ ! -d ${Current_Dir}/docker_setup ]; then
    	echo "${RED}UWC installer files are not placed in right directory.${NC}"
	echo "${GREEN}Copy UWC installer files in EdgeInsightsSoftware-v2.1-Alpha/IEdgeInsights directory and re-run this script.${NC}"
	exit 1
    fi
    if [ ! -f UWC.tar.gz ] && [ ! f ${Current_Dir}/01_pre-requisites.sh ] && [ ! f ${Current_Dir}/02_provisionAndDeployEIS.sh ] &&  [ ! f ${Current_Dir}/03_uninstall_EIS.sh ]; then
    	echo "${RED}UWC.tar.gz, 01_pre-requisites.sh, 02_provisionAndDeployEIS.sh, 03_uninstall_EIS.sh files are not present in required directory.${NC}"
	echo "${GREEN}Copy all UWC installer files in EdgeInsightsSoftware-v2.1-Alpha/IEdgeInsights directory and re-run this script.${NC}"
	exit 1
    else
	echo "${GREEN}UWC files are present in current directory${NC}"
    fi

}

#------------------------------------------------------------------
# check_for_errors
#
# Description:
#        Check the return code of previous command and display either
#        success or error message accordingly. Exit if there is an error.
# Args:
#        string : return-code
#        string : failuer message to display if return-code is non-zero
#        string : success message to display if return-code is zero (optional)
# Return:
#       None
# Usage:
#        check_for_errors "return-code" "failure-message" <"success-message">
#------------------------------------------------------------------
check_for_errors()
{
    return_code=${1}
    error_msg=${2}
    if [ "${return_code}" -ne "0" ];then
        echo "${RED}ERROR : (Err Code: ${return_code}) ${error_msg}${NC}"
        exit 1
    else
        if [ "$#" -ge "3" ];then
            success_msg=${3}
            echo ${success_msg}
        fi
    fi
    return 0
}

# ----------------------------
# Checking for root user
# ----------------------------
checkrootUser()
{
   echo "Checking for root user..."    
   if [[ "$EUID" -ne "0" ]]; then
    	echo "${RED}This script must be run as root.${NC}"
	echo "${GREEN}E.g. sudo ./<script_name>${NC}"
    	exit 1
   else
   	echo "${GREEN}Script is running with root user.${NC}"
   fi
   return 0
}

# ----------------------------
# Checking the internet connection
# ----------------------------
checkInternetConnection()
{
    echo "Checking for Internet Connection..."    
    wget http://www.google.com > /dev/null 2>&1
    if [ "$?" != 0 ]; then
        echo "${RED}No Internet Connection. Please check your internet connection...!${NC}" 
        echo "${RED}Internet connection is required.${NC}"
        exit 1
    else
        echo "${GREEN}Internet is available.${NC}"
        #Removing temporary file
        if [ -e "index.html" ];then
            rm -rf index.html
        fi
    fi
    return 0
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
    docker --version
    result=$?
    if [ "$result" -ne "0" ];then
        echo -e "${RED}ERROR:May be docker is not installed.${NC}"
        echo "${MAGENTA}Please run the pre-rquiste script before deploying EIS.${NC}" 
        exit 1
    else
        echo "${GREEN}Docker is present.${NC}"
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
    docker-compose --version
    check_for_errors "$?" "Docker-compose version is not showing. Please check docker-compose is installed on host machine." \
                    "${GREEN}Verified docker-compose successfully.${NC}"
    return 0
}


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
    if [ -d "${eis_working_dir}/provision/" ];then
        cd "${eis_working_dir}/provision/"
    else
        echo "${RED}ERROR: ${eis_working_dir}/provision/ is not present.${NC}"
        exit 1 # terminate and indicate error
    fi

    ./provision_eis.sh ../docker-compose.yml
    check_for_errors "$?" "Provisioning is failed. Please check logs" \
                    "${GREEN}Provisioning is done successfully.${NC}"
    echo "${GREEN}>>>>>${NC}"
    echo "${GREEN}For container deployement,run 03_DeployEIS.sh script${NC}"
    return 0
}

harden()
{
    docker container update --pids-limit=100 --restart=on-failure:5 --cpu-shares 512 -m 1G --memory-swap -1 ia_etcd
    docker container update --pids-limit=100 --restart=on-failure:5 --cpu-shares 512 -m 128M --memory-swap -1 ia_etcd_provision
}

#------------------------------------------------------------------
# configureExternalCerts
#
# Description:
#        Copy external certificates to common directoty required for scada-rtu container
# Return:
#        None
# Usage:
#        configureExternalCerts
#------------------------------------------------------------------
configureExternalCerts()
{
	if [ -d ${Current_Dir}/tmp_certs ]; then
		echo "${GREEN}Copying required certs for scada-rtu.${NC}"
		cd ${Current_Dir}
		rm -rf ${Current_Dir}/docker_setup/provision/Certificates/scada_ext_certs
		mkdir -p ${Current_Dir}/docker_setup/provision/Certificates/scada_ext_certs
		mkdir -p ${Current_Dir}/docker_setup/provision/Certificates/scada_ext_certs/ca
		mkdir -p ${Current_Dir}/docker_setup/provision/Certificates/scada_ext_certs/client_crt
		mkdir -p ${Current_Dir}/docker_setup/provision/Certificates/scada_ext_certs/client_key
		cp $CA_DIR/*  ${Current_Dir}/docker_setup/provision/Certificates/scada_ext_certs/ca
		check_for_errors "$?" "Incorrect certificate path is given in command line...Please provide the required command line arguments and re-run the script" \
                    "${GREEN}"".${NC}"

		cp $CLIENT_CERT_DIR/*  ${Current_Dir}/docker_setup/provision/Certificates/scada_ext_certs/client_crt
		check_for_errors "$?" "Incorrect certificate path is given in command line...Please provide the required command line arguments and re-run the script" \
                    "${GREEN}"".${NC}"

		cp $CLIENT_KEY_DIR/*  ${Current_Dir}/docker_setup/provision/Certificates/scada_ext_certs/client_key
		check_for_errors "$?" "Incorrect certificate path is given in command line...Please provide the required command line arguments and re-run the script" \
                    "${GREEN}Certificates are successfully copied in required directory.${NC}"

		chown -R  eisuser:eisuser ${Current_Dir}/docker_setup/provision/Certificates/scada_ext_certs
		echo "${GREEN}Done.${NC}"
		
		rm -rf ${Current_Dir}/tmp_certs
		return 0
	else
		echo "${GREEN}No need to configure certificates for scada-rtu contyainer${NC}"
		echo "${INFO}******************* Script END *****************************${NC}"
    	fi
	
}

export DOCKER_CONTENT_TRUST=1
verifyDirectory
checkrootUser
checkInternetConnection
docker_verify
docker_compose_verify
eisProvision
configureExternalCerts
harden

cd "${Current_Dir}"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
	echo ""
else
	cd docker_setup/provision
	chcon -R -t container_file_t Certificates
	cd -
fi
exit 0
