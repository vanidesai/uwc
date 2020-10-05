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
INFO=$(tput setaf 3)   # YELLOW (used for informative messages)

 # Variable to store user proxy provided by command line
USER_PROXY=""
DOCKER_REPO="deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"
DOCKER_GPG_KEY="0EBFCD88"

# vars
CA=""
CLIENT_CERT=""
CLIENT_KEY=""
IS_TLS="false"
BROKER_HOST=""
BROKER_PORT=""
QOS=""
SCADA_REQUIRED="yes"
PROXY_EXIST="no"
IS_INTERACTIVE=""
	

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

# function to set dev node based on deployment mode
setDevMode()
{
   echo "${INFO}Setting dev mode...${NC}"    
   sed -i "s/$1/$2/g" $working_dir/docker_setup/.env
   if [ "$?" -ne "0" ]; then
	echo "${RED}Failed to set dev mode."
	echo "${GREEN}Kinldy set DEV_MODE to false manualy in .env file and then re--run this script"
	exit 1
   else
	echo "${GREEN}Dev Mode is set to $2 ${NC}"
   fi
}

# ----------------------------
# Set host ip to localhost
# ----------------------------
setHostIP()
{
   echo "${INFO}Setting HOST_IP to 127.0.0.1 in .env file of EIS..${NC}"
   if grep -Fxq 'HOST_IP=127.0.0.1' $working_dir/docker_setup/.env
   then
   	echo "${GREEN}HOST_IP=127.0.0.1 variable already present in <>docker_setup/.env${NC}"
   else
   	echo 'HOST_IP=127.0.0.1' >> $working_dir/docker_setup/.env
   fi
   echo "${GREEN}HOST_IP=127.0.0.1 is set in .env file of EIS${NC}"

   return 0
}

# ----------------------------
# Set TTY ports for EIS user
# ----------------------------
configureRTUPorts()
{
	echo "${INFO}Checking TTY port setting..${NC}"
        rm -rf /etc/udev/rules.d/50-myusb.rules 
	if [ ! -f "/etc/udev/rules.d/50-myusb.rules" ];then
		cat >> /etc/udev/rules.d/50-myusb.rules <<EOF
KERNEL=="ttyUSB[0-9]*",MODE="0666"
KERNEL=="ttyS[0-31]*",MODE="0666"
KERNEL=="tty[0-63]*",MODE="0666"
EOF
                udevadm control --reload-rules && udevadm trigger
		#echo "KERNEL==\"ttyUSB[0-9]*\",MODE=\"0666\"" > /etc/udev/rules.d/50-myusb.rules
		#echo "KERNEL==\"ttyACM[0-9]*\",MODE=\"0666\"" > /etc/udev/rules.d/50-myusb.rules
		echo "${GREEN}TTY ports are configured successfully...${NC}"
		echo "${GREEN}Unplug the device and replug it, if connected...${NC}"
    	else
		# Files do not exist, display error message
		echo "${GREEN}TTY ports are already configured.${NC}" 
    	fi
}

# ----------------------------
# Checking for root user
# ----------------------------
checkrootUser()
{
   echo "${INFO}Checking for root user...${NC}"    
   if [[ $EUID -ne 0 ]]; then
    	echo "${RED}This script must be run as root.${NC}"
	echo "${GREEN}E.g. sudo ./<script_name>${NC}"
    	exit 1
   else
   	echo "${GREEN}Script is running with root user.${NC}"
	dpkg --configure -a
   fi
   return 0
}

# ----------------------------
# Checking the internet connection
# ----------------------------
checkInternetConnection()
{
	if [ ! -z $USER_PROXY ] && [ $PROXY_EXIST == "yes" ]
	then 
		echo "${GREEN}Script is running with --proxy mode ${NC}"
		
		sed '/httpProxy/d;/httpsProxy/d;/noProxy/d' -i /etc/environment
		echo "httpProxy=\"http://${USER_PROXY}\"
httpsProxy=\"http://${USER_PROXY}\"
noProxy=\"127.0.0.1,localhost\"" >> /etc/environment
	fi
	    echo "${INFO}Checking for Internet Connection...${NC}"    
	    wget http://www.google.com > /dev/null 2>&1
	    if [ "$?" != 0 ]; then
		echo "${RED}No Internet Connection. Please check your internet connection and proxy configuration...!${NC}"
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
    echo "${INFO}Executing Pre-requisites Edge Insights Software ${iei_version}${NC}"
    # Installing dependent packages
    apt-get update && apt-get -y install build-essential python3-pip wget curl patch diff
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
    echo "${INFO}Verifying the directory...${NC}"    
    if [ ! -d ${working_dir}/docker_setup ]; then
    	echo "${RED}UWC installer files are not placed in right directory.${NC}"
	    echo "${GREEN}Copy UWC installer files in <EIS>/IEdgeInsights directory and re-run this script.${NC}"
	    exit 1
    fi
    if [[ ! -f ${working_dir}/UWC.tar.gz ]]; then
    	echo "${RED}UWC.tar.gz file is not present in current directory.${NC}"
	    echo "${INFO}Kinldy copy all required UWC installer files in <EIS>/IEdgeInsights directory and re-run this script.${NC}"
	    exit 1
    else
	    echo "${GREEN}UWC files are present in current directory${NC}"
    fi
}

# ----------------------------
# Creating docker volume dir to store yaml files
# ----------------------------
createDockerVolumeDir()
{
    if [ ! -d /opt/intel/eis/uwc_data ]; then
    	echo "${GREEN}uwc_data directory is not present in /opt/intel/eis/ directory.${NC}"
    	echo "${GREEN}Creating /opt/intel/eis/uwc_data directory.${NC}"
    	mkdir -p /opt/intel/eis/uwc_data
	if [ "$?" -eq "0" ]; then
		echo "${GREEN}/opt/intel/eis/uwc_data is sucessfully created. ${NC}"
	else
        	echo "${RED}Failed to create docker volume directory${NC}"
		exit 1;
	fi
	
	rm -rf /opt/intel/eis/uwc_data/scada-rtu
    	mkdir -p /opt/intel/eis/uwc_data/scada-rtu
	if [ "$?" -eq "0" ]; then
		echo "${GREEN}/opt/intel/eis/uwc_data/scada-rtu is sucessfully created. ${NC}"
	else
        	echo "${RED}Failed to create docker volume directory${NC}"
		exit 1;
	fi
   fi
	echo "${GREEN}Deleting old /opt/intel/eis/container_logs directory.${NC}"
	rm -rf  /opt/intel/eis/container_logs
	echo "${GREEN}Done..${NC}"
	echo "${GREEN}Creating /opt/intel/eis/container_logs directory.${NC}"
	mkdir -p /opt/intel/eis/container_logs/modbus-tcp-master
	mkdir -p /opt/intel/eis/container_logs/modbus-rtu-master
	mkdir -p /opt/intel/eis/container_logs/mqtt-export
	mkdir -p /opt/intel/eis/container_logs/scada-rtu
	if [ "$?" -eq "0" ]; then
		echo "${GREEN}/opt/intel/eis/container_logs is sucessfully created. ${NC}"
	else
		echo "${RED}Failed to create docker volume directory${NC}"
		exit 1;
	fi
    if [ ! -d /opt/intel/eis/uwc_data/common_config ]; then
    	echo "${GREEN}common_config directory is not present in /opt/intel/eis/ directory.${NC}"
    	echo "${GREEN}Creating /opt/intel/eis/uwc_data/common_config directory.${NC}"
    	mkdir -p /opt/intel/eis/uwc_data/common_config
	if [ "$?" -eq "0" ]; then
		echo "${GREEN}/opt/intel/eis/uwc_data/common_config is sucessfully created. ${NC}"
	else
        	echo "${RED}Failed to create docker volume directory${NC}"
		exit 1;
	fi
    fi
}

# function to copy docker-compose.yml as per the deployment mode 
function copyDeployComposeFile()
{
    DEPLOY_MODE=$(grep UWC_DEPLOY_MODE ../docker_setup/.env | cut -d '=' -f 2-)
    if [ -z "$DEPLOY_MODE" ]; then
        echo "Deployment mode is set to default i.e. IPC_PROD"
    else
        echo "Deployment mode is set to = " $DEPLOY_MODE
    fi
    
    case $DEPLOY_MODE in
      IPC_PROD)
		if [ $SCADA_REQUIRED == "no" ];then
			cp docker-compose_without_scada.yml ../docker_setup/docker-compose.yml

		elif [ $IS_TLS == "no" ] || [ $IS_TLS == "false" ]; then
			cp docker-compose_IPC_PROD_nonTLSScada.yml ../docker_setup/docker-compose.yml
		else
            		cp docker-compose.yml ../docker_setup/docker-compose.yml
            		setDevMode "DEV_MODE=true" "DEV_MODE=false"
		fi
        ;;
      IPC_DEV)
            cp docker-compose_IPC_DEV.yml ../docker_setup/docker-compose.yml
            setDevMode "DEV_MODE=false" "DEV_MODE=true"
        ;;
      *)
            # set default mode to ipc prod
    		if [ $SCADA_REQUIRED == "no" ];then
			cp docker-compose_without_scada.yml ../docker_setup/docker-compose.yml

		elif [ $IS_TLS == "no" ] || [ $IS_TLS == "false" ]; then
			cp docker-compose_IPC_PROD_nonTLSScada.yml ../docker_setup/docker-compose.yml
		else
            		cp docker-compose.yml ../docker_setup/docker-compose.yml
            		setDevMode "DEV_MODE=true" "DEV_MODE=false"
		fi
        ;;
    esac 
}

# ---------------------------------------
# Function to apply EIS patch for DBS 
# --------------------------------------
applyEISPatch()
{
	echo "${GREEN}Applying EIS patch for DBS...${NC}"
	cd ${working_dir}
	rm -rf UWC/ && mkdir UWC
	tar -xzvf UWC.tar.gz -C UWC > /dev/null 2>&1
	cd UWC/DBS_Patches && cp 0001-U-ARCH-changes.patch ${working_dir}
	cd ${working_dir}
	
	# revert the patch if already applied
	#patch -d . -f --strip=1 -R  < 0001-U-ARCH-changes.patch || true >/dev/null

	patch -d . -f --strip=1  < 0001-U-ARCH-changes.patch
	check_for_errors "$?" "Failed to apply EIS patch for DBS. Please check logs" \
			"${GREEN}Successfuly applied EIS patch for DBS${NC}"
	cp ${working_dir}/UWC/DBS_Patches/provision_eis.sh ${working_dir}/docker_setup/provision
	cp ${working_dir}/UWC/DBS_Patches/docker-compose-provision.yml ${working_dir}/docker_setup/provision/dep/
	chmod +x ${working_dir}/docker_setup/provision/provision_eis.sh
	# cleanup
	rm -rf UWC/
}


# ----------------------------
# Copying UWC Containers in EIS
# ----------------------------
addUWCContainersInEIS()
{
    echo "${INFO}Copying UWC Containers in EIS...${NC}"   
    rm -rf UWC/ && mkdir UWC
    tar -xzvf UWC.tar.gz -C UWC > /dev/null 2>&1
    cd UWC
    cp -r modbus-master/ MQTT/ scada-rtu/ uwc_common/ mqtt-export/ ../
    cp Others/Config/UWC/x509_cert_config.json ../docker_setup/provision/config/
    cp -r Others/Config/UWC/Device_Config/* /opt/intel/eis/uwc_data
    cp -r Others/Config/UWC/Device_Config/* /opt/intel/eis/uwc_data
    cp Others/Config/UWC/Global_Config.yml /opt/intel/eis/uwc_data/common_config/Global_Config.yml
    #cp Others/Config/UWC/scada_config.yml /opt/intel/eis/uwc_data/scada-rtu/scada_config.yml
    copyDeployComposeFile
    copy_verification=$(echo $?)
    if [ "$copy_verification" -eq "0" ]; then
        echo "${GREEN}UWC containers are successfully copied ${NC}"
    else
        echo "${RED}failed to copy UWC containers.${NC}"
	    return 1
    fi
    cd ${working_dir}/ && rm -rf UWC/

    # set execute permissions to all shell scripts
    cd ../ && find . -type f -iname "*.sh" -exec chmod 777 {} \;

    return 0
}

endOfScript()
{
    echo "${GREEN}>>>>>${NC}"
	echo "${GREEN}************************* This script is sucessfully executed ***************************************************"
    echo "${INFO}Next script to be run for provisioning EIS is 02_provisionEIS.sh ${NC}"
    echo "${INFO}To execute unit test cases, run 06_UnitTestRun.sh script ${NC}"
}

validateUserInput()
{
	if [ $PROXY_EXIST == "no" ]
	then
		echo "${GREEN}Script is running without proxy${NC}"
		return;
	elif [ -z USER_PROXY ]
	then
		echo "${GREEN}Script is running in interactive mode to take proxy from user${NC}"
		proxy_settings
	elif [ $PROXY_EXIST == "yes" ] && [ ! -z USER_PROXY ]
	then 
		echo "${GREEN}Script is running with --proxy mode ${NC}"
		
		#Creating config.json
		if [ ! -d ~/.docker ];then
			mkdir ~/.docker
		fi
		if [ ! -f ~/.docker/config.json ];then
			touch ~/.docker/config.json
			chmod 766 ~/.docker/config.json
		fi
		
		echo "${GREEN}Configuring proxy setting in the system${NC}"
		echo "Docker services will be restarted after proxy settings configured"
		proxy_enabled_network
		check_for_errors "$?" "Failed to configure proxy settings on the system. Please check logs" \
			"${GREEN}Configured proxy settings in the system successfully.${NC}"
		dns_server_settings
		check_for_errors "$?" "Failed to update DNS server settings in the system. Please check logs" \
			"${GREEN}Updated DNS server settings in the system.${NC}"
		echo "${GREEN}proxy is set to :: ${NC}" $USER_PROXY
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

del_file() 
{
    if [[ -f $1 ]]; then
        rm -rf $1
        if [[ $? -ne 0 ]]; then
            clean_exit
        fi
    fi 
}

function uninstallDocker()
{
    # UNINSTALLING DOCKER 
    echo -e "${INFO}---------------------------------------Uninstalling Docker---------------------------------------${NC}"
    
    dpkg --purge --force-all docker-ce docker-ce-cli containerd.io
    apt-get purge -y docker docker.io 
    
    # Removing Docker GPG and removing the repository from sources
    apt-key del $DOCKER_GPG_KEY
    add-apt-repository --remove "$DOCKER_REPO"
    echo -e "${GREEN}-------------------------------Docker uninstalled successfully-----------------------------------${NC}" 
    
    #Delete UWC config
    echo -e "${INFO}---------------------------------------Removing UWC config-----------------------${NC}"
    del_file /opt/intel/eis/uwc_data
    del_file /opt/intel/eis/uwc_data/common_config
    echo -e "${GREEN}-------------------------------Removed all UWC config------------------------------------${NC}"
    
    #RESET THE PROXY SETTING 
    echo -e "${INFO}---------------------------------------Resetting proxy setting-----------------------------------${NC}"
    del_file /etc/docker/daemon.json
    del_file /etc/systemd/system/docker.service.d/http-proxy.conf
    del_file /etc/systemd/system/docker.service.d/https-proxy.conf      
    del_file $HOME/.docker/config.json
    del_file /etc/systemd/system/docker.service.d
    echo -e "${GREEN}-------------------------------Proxy setting reset to default------------------------------------${NC}"
    rm -rf  /opt/intel
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

# function to validate version
vercomp () {
    if [[ $1 == $2 ]]
    then
        return 0
    fi
    local IFS=.
    local i ver1=($1) ver2=($2)
    # fill empty fields in ver1 with zeros
    for ((i=${#ver1[@]}; i<${#ver2[@]}; i++))
    do
        ver1[i]=0
    done
    for ((i=0; i<${#ver1[@]}; i++))
    do
        if [[ -z ${ver2[i]} ]]
        then
            # fill empty fields in ver2 with zeros
            ver2[i]=0
        fi
        if ((10#${ver1[i]} > 10#${ver2[i]}))
        then
            return 1
        fi
        if ((10#${ver1[i]} < 10#${ver2[i]}))
        then
            return 2
        fi
    done
    return 0
}

# function to compare two versions
testvercomp () {
    vercomp $1 $2
    case $? in
        0) op='=';;
        1) op='>';;
        2) op='<';;
    esac
    if [[ $op != $3 ]]
    then
        #echo "VERSION MISMATCH: Expected '$3', Actual '$op', Arg1 '$1', Arg2 '$2'"
        echo "${RED}VERSION MISMATCH: Expected '$2', Actual '$1'${NC}"
	return 1
    else
        echo "${GREEN}Pass: '$1 $op $2'${NC}"
	return 0
    fi
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
    rm -rf $(which docker-compose)
    pip uninstall -y docker-compose
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
    requiredver="1.24.0"
    cur_v=$(echo $(docker-compose --version) | cut -d"n" -f 2 | cut -d"," -f 1) 
    testvercomp $cur_v $requiredver "="
    ret=$?
    if [ "$ret" -eq 0 ]; then
        echo "${GREEN}docker-compose is already installed, ${cur_v}${NC}"
    else
        echo "${INFO}docker-compose is either not installed or old one${NC}"
        echo "Required = "${requiredver}, "Present version = " ${cur_v}
        echo "${INFO}docker-compose needs to be Installed.${NC} "
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
    requiredver="19.03.0"
    cur_v=$(echo $(docker --version) | cut -d"n" -f 2 | cut -d"," -f 1) 
    testvercomp $cur_v $requiredver ">"
    ret=$?
    if [ "$ret" -eq 0 ]; then
        echo "${GREEN}docker is already installed, ${cur_v}${NC}"
    else
        echo "${INFO}docker version is either not installed or old one${NC}"
        echo "Required = "${requiredver}, "Present version = " ${cur_v}
        echo "${INFO}docker needs to be Installed.${NC} "
        uninstallDocker
        docker_install
    fi
	validateUserInput "$@"
    return 0
}

# fucntion used to change topic length in EIS
changeTopicLen()
{
	sed -i 's/#define SIZE 500/#define SIZE 1000/g' ${working_dir}/common/libs/ConfigManager/c/src/env_config.c
	if [ "$?" -ne "0" ]; then
		echo "${RED}Failed to set topic len in EIS."
		echo "${GREEN}Kinldy set topic leg to 1000 in EIS"
		return 1
	else
		echo "${GREEN}Topic length is set to 1000 characters.${NC}"
	fi
}

# function to create separate network for uwc containers
createDockerNw()
{
	docker network rm uwc_nw || true
	docker network create --driver=bridge --subnet=172.168.80.0/24 --gateway=172.168.80.1 --ip-range=172.168.80.128/25 -o "com.docker.network.bridge.enable_ip_masquerade"="1" uwc_nw
}

#------------------------------------------------------------------
# ParseCommandLineArgs
#
# Description:
#        This function is used to Parse command line arguments passed to this script
# Return:
#        None
# Usage:
#        ParseCommandLineArgs <list of arguments>
#------------------------------------------------------------------
ParseCommandLineArgs()
{
	if [ $# == "0" ];then
		IS_INTERACTIVE="yes"
		return;
	fi
	
	echo "${INFO}Reading the command line args...${NC}"
	for ARGUMENT in "$@"
	do
	    KEY=$(echo $ARGUMENT | cut -f1 -d=)
	    VALUE=$(echo $ARGUMENT | cut -f2 -d=)   

	   #echo ${GREEN}$KEY "=" $VALUE${NC}
	   #echo "${GREEN}==========================================${NC}"

	    case "$KEY" in
		    --caFile)    CA=${VALUE} ;;     
	            --crtFile)   CLIENT_CERT=${VALUE} ;; 
		    --keyFile)   CLIENT_KEY=${VALUE} ;; 
		    --isTLS)   	 IS_TLS=${VALUE} ;;
		    --brokerAddr) BROKER_HOST=${VALUE} ;; 
		    --brokerPort) BROKER_PORT=${VALUE} ;;
		    --qos) QOS=${VALUE} ;;  
		    --proxy) USER_PROXY=${VALUE} PROXY_EXIST="yes";;   
		    --withoutScada) SCADA_REQUIRED="no" ;;  
		    --help) Usage ;; 
		     *) echo "${RED}Invalid arguments passed..${NC}"; Usage; ;;  
	    esac    
	done
	
	if [ $SCADA_REQUIRED == "yes" ];then
		if [ $IS_TLS == "1" ] || [ $IS_TLS == "true" ] || [ $IS_TLS == "yes" ];then	
			if [ ! -z "$CA" ] && [ ! -z "$CLIENT_CERT" ] && [ ! -z "$CLIENT_KEY" ]; then
				echo ""
			else 
				echo "${RED}Missing required certificates needed for TLS..${NC}"
				echo "${RED}Please provide the required arguments and re-run the script..${NC}"
				Usage
			fi
		fi
		
		if [ ! -z "$BROKER_HOST" ] && [ ! -z "$BROKER_PORT" ] && [ ! -z "$QOS" ]; then
			IS_INTERACTIVE="no"
		else 
			echo "${RED}Missing broker addr or port or QOS..${NC}"
			IS_INTERACTIVE="yes"
			#Usage
		fi
		
	fi
	PrintAllArgs
	
	#echo "${GREEN}==========================================${NC}"
}

#------------------------------------------------------------------
# Usage
#
# Description:
#        Help function 
# Return:
#        None
# Usage:
#        Usage
#------------------------------------------------------------------
Usage()
{
	echo 
	echo "${BOLD}${INFO}==================================================================================${NC}"
	echo
	echo "${BOLD}${GREEN}Usage :: sudo ./01_pre-requisites.sh [OPTION...]  -- non-interative mode ${NC}"
	echo "${BOLD}${GREEN}Usage :: sudo ./01_pre-requisites.sh  -- interative mode ${NC}"
	echo 
	echo "${BOLD}${GREEN}Note : If no options provided then script will run with the interactive mode${NC}"
	echo
	echo "${INFO}List of available options..."
	echo
	echo "${INFO}--isTLS		yes/no to enable/disable TLS for scada-rtu"
	echo 
	echo "${INFO}--cafile		Root CA file, required only if isTLS is true"
	echo
	echo "${INFO}--crtfile		client certificate file, required only if isTLS is true"
	echo
	echo "${INFO}--keyFile		client key crt file, required only if isTLS is true"
	echo
	echo "${INFO}--brokerAddr	scada external broker IP address/Hostname"
	echo
	echo "${INFO}--brokerPort	scada external broker port number"
	echo
	echo "${INFO}--qos		QOS used by scada-rtu container to publish messages, can take values between 0 to 2 inclusive"
	echo
	echo "${INFO}--proxy		proxies, required when gateway is connected behind proxy"
	echo
	echo "${INFO}--withoutScada	Optional, this will skip the scada-rtu container installation from deployment, can contain value 1/true"
	echo
	echo "${INFO}--help		display this help and exit"${NC}
	echo
	echo "Different use cases..."
	echo "${BOLD}${MAGENTA}
		1. Scada with TLS 
		sudo ./01_pre-requisites.sh --isTLS=yes  --caFile=\"scada_ext_certs/ca/root-ca.crt\" --crtFile=\"scada_ext_certs/client/client.crt\" --keyFile=\"scada_ext_certs/client/client.key\" --brokerAddr=\"127.0.0.1\" --brokerPort=\"1883\" --qos=1

		2. Scada rtu without TLS
		sudo ./01_pre-requisites.sh --isTLS=no  --brokerAddr=\"192.168.0.5\" --brokerPort=\"8883\" --qos=1

		3. Without scada-rtu 
		sudo ./01_pre-requisites.sh --withoutScada=yes

		4. Full interactive
		sudo ./01_pre-requisites.sh

		5.With proxy and without scada-rtu 
		sudo ./01_pre-requisites.sh --withoutScada=yes --proxy=\"intel.proxy.com:811\"

		6.With proxy and scada-rtu 
		sudo ./01_pre-requisites.sh --proxy=\"intel.proxy.com:811\"

		7. With scada, TLS, and with proxy 
		sudo ./01_pre-requisites.sh --isTLS=yes  --caFile=\"scada_ext_certs/ca/root-ca.crt\" --crtFile=\"scada_ext_certs/client/client.crt\" --keyFile=\"scada_ext_certs/client/client.key\" --brokerAddr=\"127.0.0.1\" --brokerPort=\"1883\" --qos=1 --proxy=\"intel.proxy.com:811\"${NC}

	"
	echo "${INFO}===================================================================================${NC}"
	exit 1
}

#------------------------------------------------------------------
# PrintAllArgs
#
# Description:
#        This function is used to print all given values on console 
# Return:
#        None
# Usage:
#        PrintAllArgs
#------------------------------------------------------------------
PrintAllArgs()
{
	echo "${GREEN}==========================================${NC}"
	echo "${GREEN}Given values..${NC}"
	if [ ! -z $IS_TLS ]; then echo "${INFO}--isTLS = $IS_TLS";fi
	if [ ! -z $CA ]; then echo "${INFO}--cafile = $CA";fi
	if [ ! -z $CLIENT_CERT ]; then echo "${INFO}--crtfile = $CLIENT_CERT";fi
	if [ ! -z $CLIENT_KEY ]; then echo "${INFO}--keyFile = $CLIENT_KEY";fi
	if [ ! -z $BROKER_HOST ]; then echo "${INFO}--bokerAddr = $BROKER_HOST";fi
	if [ ! -z $BROKER_PORT ]; then echo "${INFO}--bokerPort = $BROKER_PORT";fi
	if [ ! -z $QOS ]; then echo "${INFO}--qos = $QOS";fi
	if [ ! -z $USER_PROXY ]; then echo "${INFO}--proxy = $USER_PROXY${NC}";fi
	echo "${GREEN}==========================================${NC}"
}


echo "${GREEN}============================= Script START ============================================${NC}"
# Internal function calls to set up Insights
ParseCommandLineArgs "$@"
verifyDirectory
checkrootUser
checkInternetConnection "$@"
installBasicPackages
docker_verification_installation	"$@"
docker_compose_verify_installation
#setHostIP
configureRTUPorts
createDockerVolumeDir
changeTopicLen
addUWCContainersInEIS

# only works with EIS 2.2 PV
applyEISPatch
changeFilePermissions
createDockerNw

if [ $SCADA_REQUIRED == "yes" ]; then
	if [ $IS_INTERACTIVE == "yes" ]; then
		echo "${INFO}Installing scada container with interactive mode${NC}"
		./1.1_ConfigureScada.sh --interactive=1 "$@"
	else
		echo "${INFO}Installing scada container with non-interactive mode${NC}"
		./1.1_ConfigureScada.sh --nonInteractive=1 "$@"
	fi
else
	echo "${INFO}scada-rtu container is skipped from the installation${NC}"
fi
endOfScript
echo "${GREEN}============================= Script END ============================================${NC}"

cd "${working_dir}"
exit 0
