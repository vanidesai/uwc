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

working_dir=$(pwd)
RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
MAGENTA=$(tput setaf 5)
NC=$(tput sgr0)
BOLD=$(tput bold)

 # Variable to store user proxy provided by command line
USER_PROXY=""
	

eis_working_dir="$Current_Dir/docker_setup"

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
# Set dev mode to true
# ----------------------------
setdevmodetrue()
{
   echo "Setting dev mode to true..."    
   sed -i 's/DEV_MODE=false/DEV_MODE=true/g' $working_dir/docker_setup/.env
   if [ "$?" -ne "0" ]; then
	echo "${RED}Failed to set dev mode to true."
	echo "${GREEN}Kinldy set DEV_MODE to true manualy in .env file and then re--run this script"
	exit 1
   else
	echo "${GREEN}Dev Mode is set to true.${NC}"
   fi
   return 0
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

installBasicPackages()
{
    echo "Executing Pre-requisites Edge Insights Software ${iei_version}"
    # Installing dependent packages
    sudo apt-get -y install build-essential python3-pip wget curl > /dev/null 2>&1
    if [ "$?" -ne "0" ]; then
	echo "${RED}failed to download basic packages.${NC}"
    fi
    return 0
}

# ----------------------------
# Changing file permissions
# ----------------------------
changeFilePermissions()
{
    chown -R ${SUDO_USER}:${SUDO_USER} *
}

verifyDirectory()
{
    echo "Verifying the directory..."    
    if [ ! -d ${working_dir}/docker_setup ]; then
    	echo "${RED}UWC installer files file are not placed in right directory.${NC}"
	echo "${GREEN}Copy UWC installer files in EdgeInsightsSoftware-v2.1-Alpha/IEdgeInsights directory and re-run this script.${NC}"
	exit 1
    fi
    if [ ! -f UWC.tar.gz ] && [ ! f ${working_dir}/01_pre-requisites.sh ] && [ ! f ${working_dir}/02_provisionAndDeployEIS.sh ] &&  [ ! f ${working_dir}/03_uninstall_EIS.sh ]; then
    	echo "${RED}UWC.tar.gz, 01_pre-requisites.sh, 02_provisionAndDeployEIS.sh, 03_uninstall_EIS.sh files are not present in curent directory.${NC}"
	echo "${GREEN}Copy all UWC installer files in EdgeInsightsSoftware-v2.1-Alpha/IEdgeInsights directory and re-run this script.${NC}"
	exit 1
    else
	echo "${GREEN}UWC files are present in current directory${NC}"
    fi

}
# ----------------------------
# Copying UWC Containers in EIS
# ----------------------------
addUWCContainersInEIS()
{
    echo "Copying UWC Containers in EIS..."   
    rm -rf UWC/ && mkdir UWC
    tar -xzvf UWC.tar.gz -C UWC > /dev/null 2>&1
    cd UWC
    cp -r modbus-master/ MQTT/ mqtt-export/ ../
    cp -r docker-compose_DEV.yml ../docker_setup/docker-compose.yml
    cp -r Others/ETCD_Config/UWC/ Others/ETCD_Config/etcd_pre_load.json ../docker_setup/provision/config/
    cp Others/ETCD_Config/etcd_provision.py ../docker_setup/provision/dep/
    echo "${GREEN}UWC containers are successfully copied.${NC}"
    echo "${BOLD}${GREEN}>>>>>${NC}"
	echo "${BOLD}${GREEN}************************* This script is sucessfully executed ***************************************************"
    echo "${BOLD}${GREEN}Next script to be run for provisioning EIS is 02_provisionEIS.sh ${NC}"
    cd ${working_dir}/ && rm -rf UWC/
    return 0
}

Usage () {
    echo "${BOLD}${RED}">&2 "$@${NC}" 
	echo
	echo "${BOLD}${MAGENTA}***********************************************************************************************************"
	echo "${GREEN}Usage : Script can be run with following modes"
	echo
	echo "	${GREEN}1. For interactive mode - sudo ./01_pre-requisites.sh "
	echo "	${GREEN}2. For non-interactive & no-proxy mode - sudo ./01_pre-requisites.sh --no-proxy"
	echo "	${GREEN}3. For non-interactive & proxy mode - sudo ./01_pre-requisites.sh --proxy <proxy_address>:<proxy_port>"
	echo
	echo "${MAGENTA}**********************************************************************************************************************${NC}"
    exit 1
}

validateUserInput()
{
	if [ "$#" -eq 0 ]
	then
		echo "${GREEN}Script is running with interactive mode${NC}"
		proxy_settings
	elif [ "$#" -eq 1 ]
	then 
		echo "${GREEN}Script is running with interactive and no-proxy mode${NC}"
		if [ "$1" != "--no-proxy" ]
		then
			Usage "Invalid argument $1 provided"
		fi
		return 0
	elif [ "$#" -eq 2 ]
	then 
		echo "${GREEN}Script is running with interactive and --proxy mode ${NC}"
		if [ "$1" != "--proxy" ]
		then
			Usage "Invalid argument $1 provided"
		fi
		
		#Creating config.json
		if [ ! -d ~/.docker ];then
			mkdir ~/.docker
		fi
		if [ ! -f ~/.docker/config.json ];then
			touch ~/.docker/config.json
			chmod 766 ~/.docker/config.json
		fi
		
		# set proxy through command line
		USER_PROXY=$2
		
		echo "${GREEN}Configuring proxy setting in the system${NC}"
		echo "Docker services will be restarted after proxy settings configured"
		proxy_enabled_network
		check_for_errors "$?" "Failed to configure proxy settings on the system. Please check logs" \
			"${GREEN}Configured proxy settings in the system successfully.${NC}"
		dns_server_settings
		check_for_errors "$?" "Failed to update DNS server settings in the system. Please check logs" \
			"${GREEN}Updated DNS server settings in the system.${NC}"
		echo "${GREEN}Given arguments are :: ${NC}" $1 $2
	else
		Usage "Invalid argument, $1 provided"
	fi
	return 0
}
#------------------------------------------------------------------------------
# proxy_enabled_network
#
# Description:
#        Configure proxy settings for docker client and docker daemon to connect 
#        to internet and also for containers to access internet
# Usage:
#        proxy_enabled_network
#------------------------------------------------------------------------------
proxy_enabled_network()
{
    # 1. Configure the Docker client
    #USER_PROXY=""
		
echo "{
 \"proxies\":
  {
  \"default\":
   {
   \"httpProxy\": \"http://${USER_PROXY}\",
   \"httpsProxy\": \"http://${USER_PROXY}\",
   \"noProxy\": \"127.0.0.1,localhost\"
   }
  }
}" > ~/.docker/config.json

    # 2. HTTP/HTTPS proxy
    if [ -d "/etc/systemd/system/docker.service.d" ];then
        rm -rf /etc/systemd/system/docker.service.d
    fi
    mkdir -p /etc/systemd/system/docker.service.d
    touch /etc/systemd/system/docker.service.d/http-proxy.conf
    touch /etc/systemd/system/docker.service.d/https-proxy.conf

    if [ -f "/etc/systemd/system/docker.service.d/http-proxy.conf" ] && [ -f "/etc/systemd/system/docker.service.d/https-proxy.conf" ];then
        echo "[Service]" > /etc/systemd/system/docker.service.d/http-proxy.conf
        echo "Environment=\"HTTP_PROXY=http://${USER_PROXY}/\" \"NO_PROXY=localhost,127.0.0.1\"" >> /etc/systemd/system/docker.service.d/http-proxy.conf
        check_for_errors "$?" "Failed to update http-proxy.conf files. Please check logs"
        echo "[Service]" > /etc/systemd/system/docker.service.d/https-proxy.conf
        echo "Environment=\"HTTPS_PROXY=http://${USER_PROXY}/\" \"NO_PROXY=localhost,127.0.0.1\"" >> /etc/systemd/system/docker.service.d/https-proxy.conf
        check_for_errors "$?" "Failed to update https-proxy.conf files. Please check logs"
    else
        # Files do not exist, display error message
        check_for_errors "-1" "Files http-proxy.conf and https-proxy.conf not found. Please check logs"
    fi

    # Flush the changes
    systemctl daemon-reload
    check_for_errors "$?" "Failed to flush the changes. Please check logs"
    # Restart docker
    systemctl restart docker
    check_for_errors "$?" "Failed to restart docker service. Please check logs"

    return 0
}


#------------------------------------------------------------------------------
# dns_server_setting
#
# Description:
#        Updating correct DNS server details in /etc/resolv.conf
# Usage:
#        dns_server_setting
#------------------------------------------------------------------------------
dns_server_settings()
{
    UBUNTU_VERSION=$(grep "DISTRIB_RELEASE" /etc/lsb-release | cut -d "=" -f2)

    echo "${GREEN}Updating correct DNS server details in /etc/resolv.conf${NC}"
    # DNS server settings for Ubuntu 16.04 or earlier
    VERSION_COMPARE=$(echo "${UBUNTU_VERSION} <= 16.04" | bc)
    if [  "${VERSION_COMPARE}" -eq "1" ];then
        if [ -f "/etc/NetworkManager/NetworkManager.conf" ];then
            grep "#dns=dnsmasq" /etc/NetworkManager/NetworkManager.conf
            if [ "$?" -ne "0" ];then
                sed -i 's/dns=dnsmasq/#dns=dnsmasq/g' /etc/NetworkManager/NetworkManager.conf
                systemctl restart network-manager.service
                #Verify on the host
                echo "${GREEN}Udpated DNS server details on host machine${NC}"
                cat /etc/resolv.conf
            fi
        fi
    fi

    # DNS server settings for Ubuntu 18.04 or later
    VERSION_COMPARE=$(echo "${UBUNTU_VERSION} >= 18.04" | bc)
    if [ ${VERSION_COMPARE} -eq "1" ];then
        if [ -f "/run/systemd/resolve/resolv.conf" ];then
            ln -sf /run/systemd/resolve/resolv.conf /etc/resolv.conf
            #Verify on the host
            echo "${GREEN}Udpated DNS server details on host machine${NC}"
            cat /etc/resolv.conf
        fi
    fi

    return 0
}

#------------------------------------------------------------------------------
# proxy_settings
#
# Description:
#        Configuring proxy if user in proxy enabled network else
#        the setup will be done with no-proxy settings
# Usage:
#        proxy_settings
#------------------------------------------------------------------------------
proxy_settings()
{
    # Prompt the user for proxy address
    while :
    do
        echo "Is this system in Proxy enabled network?"
        echo "Please choose 1 for 'Yes' and 2 for 'No'"
        echo "1) Yes"
        echo "2) No"

        read yn

        case ${yn} in
            1)
                #Creating config.json
                if [ ! -d ~/.docker ];then
                    mkdir ~/.docker
                fi
                if [ ! -f ~/.docker/config.json ];then
                    touch ~/.docker/config.json
                    chmod 766 ~/.docker/config.json
                fi
				echo "Please enter your proxy address ${GREEN}(Ex: <proxy.example.com>:<port-number>):${NC}"
				read USER_PROXY
				while [ -z "${USER_PROXY}" ]
					do
						echo "${RED}Proxy is empty, please enter again${NC}"
						read USER_PROXY
				done
			
                echo "${GREEN}Configuring proxy setting in the system${NC}"
                echo "Docker services will be restarted after proxy settings configured"
                proxy_enabled_network
                check_for_errors "$?" "Failed to configure proxy settings on the system. Please check logs" \
                    "${GREEN}Configured proxy settings in the system successfully.${NC}"
                dns_server_settings
                check_for_errors "$?" "Failed to update DNS server settings in the system. Please check logs" \
                    "${GREEN}Updated DNS server settings in the system.${NC}"
                break;;
            2)
                echo "${GREEN}Continuing the setup with system network settings.${NC}"
                break;;
            *)
                echo "Entered incorrect input : ${yn}"
        esac
    done

    return 0
}

#------------------------------------------------------------------------------
# docker_install
#
# Description:
#        Install docker-ce and verify it with hello-world image
# Usage:
#        docker_install
#------------------------------------------------------------------------------
docker_install()
{
    # Update the apt package index:
    apt-get -y update

    # Install packages to allow apt to use a repository over HTTPS:
    apt-get -y install \
        apt-transport-https \
        ca-certificates \
        curl \
        gnupg-agent \
        software-properties-common
    check_for_errors "$?" "Installing packages to allow apt to use a repository over HTTPS failed. Please check logs" \
                    "${GREEN}Installed packages to allow apt to use a repository over HTTPS successfully.${NC}"


    #Add Docker’s official GPG key:
    curl -fsSL https://download.docker.com/linux/ubuntu/gpg | apt-key add -
    check_for_errors "$?" "Failed to add Docker’s official GPG key. Please check logs" \
                    "${GREEN}Added Docker’s official GPG key successfully.${NC}"

    # Verify that you now have the key with the fingerprint
    # 9DC8 5822 9FC7 DD38 854A E2D8 8D81 803C 0EBF CD88, by searching for
    # the last 8 characters of the fingerprint
    non_empty_key=$( apt-key fingerprint 0EBFCD88 | grep '9DC8 5822 9FC7 DD38 854A  E2D8 8D81 803C 0EBF CD88')
    check_for_errors "$?" "Docker's key verification failed. Please check logs" \
                    "${GREEN}Verified the key fingerprint 0EBFCD88 successfully.${NC}"

    # Set up the stable repository
    # Find Hardware-architecture
    # It should be one of amd64 (x86_64/amd64), armhf, arm64, ppc64le (IBM Power), s390x (IBM Z)
    # If there is any issue try one of the commands 'uname -m' or 'uname -p'
    hw_arch=$(uname --hardware-platform)    # uname -i
    if [ ${hw_arch}='amd64' ] || [ ${hw_arch}='x86_64' ]
    then 
        hw_arch='amd64'
    fi

    add-apt-repository \
       "deb [arch=${hw_arch}] https://download.docker.com/linux/ubuntu \
       $(lsb_release -cs) \
       stable"
    echo "${GREEN}Setting up the stable repository is done successfully.${NC}"
    # Stable repositoty set up done successfully
   
    apt-get -y update
    apt-get install -y docker-ce docker-ce-cli containerd.io
    check_for_errors "$?" "Docker CE installation failed. Please check logs" \
                    "${GREEN}Installed Docker CE successfully.${NC}"
    groupadd docker
    usermod -aG docker ${SUDO_USER}
    return 0
}

#------------------------------------------------------------------
# docker_compose_install
#
# Description:
#        Installing docker-compose on the system
# Return:
#        None
# Usage:
#        docker_compose_install
#------------------------------------------------------------------
docker_compose_install()
{
    rm -rf /usr/local/bin/docker-compose
    # Downloading docker-compose using curl utility.
    curl -L "https://github.com/docker/compose/releases/download/1.24.0/docker-compose-$(uname -s)-$(uname -m)" -o /usr/local/bin/docker-compose
    if [ "$?" -eq "0" ];then
        # Making the docker-compose executable. 
        chmod +x /usr/local/bin/docker-compose
	echo "${GREEN}Installed docker-compose successfully.${NC}"
    else
        echo "${RED}ERROR: Docker-compose Downloading Failed.Please Check Manually.${NC}"
        exit 1
    fi
    return 0  
}

#Verifying docker-compose
docker_compose_verify_installation()
{

    echo "Verifying if docker_compose already exists."
    docker-compose --version
    installation_verification=$(echo $?)
    if [ "$installation_verification" -eq "0" ]; then
        echo "${GREEN}docker-compose is already installed ${NC}"
    else
        echo "${RED}docker-compose needs to be Installed.${NC} "
        echo "${GREEN}Installing docker-compose${NC}"
        docker_compose_install
    fi
    return 0
}

#------------------------------------------------------------------------------ 
# docker_verification_installation
#
# Description: 
#       verifies the docker installation.
# Args:
#       None
# Return:
#       None
# Usage:
#       docker_verification_installation
#------------------------------------------------------------------------------
docker_verification_installation()
{
    echo "Verifying if docker already exists."
    docker --version
    installation_verification=$(echo $?)
    if [ "$installation_verification" -eq "0" ]; then
        echo "${GREEN}Docker is already installed ${NC}"
    else
        echo "${RED}Docker needs to be Installed.${NC} "
        echo "${GREEN}Installing Docker${NC}"
        docker_install
    fi
	validateUserInput "$@"
    return 0
}

# Internal function calls to set up Insights
verifyDirectory
checkrootUser
checkInternetConnection
installBasicPackages
docker_verification_installation	"$@"
docker_compose_verify_installation
setdevmodetrue
addUWCContainersInEIS
changeFilePermissions

cd "${working_dir}"
exit 0
