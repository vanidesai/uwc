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
working_dir=$(pwd)
RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
MAGENTA=$(tput setaf 5)
NC=$(tput sgr0)

eis_working_dir="$Current_Dir/docker_setup"

# vars
USER_PROXY=""
DOCKER_REPO="deb [arch=amd64] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable"
DOCKER_GPG_KEY="0EBFCD88"

# vars
CA=""
CLIENT_CERT=""
CLIENT_KEY=""
IS_TLS=""
BROKER_HOST=""
BROKER_PORT=""
QOS=""
SCADA_REQUIRED="yes"
KPI_APP_REQUIRED="no"
PROXY_EXIST="no"
DEPLOY_MODE=""


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
		rm -rf $Current_Dir/tmp_certs
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

# function to copy docker-compose.yml as per the deployment mode 
function copyDeployComposeFile()
{
    cd $working_dir
    
    rm -rf UWC/ && mkdir UWC
    tar -xzvf UWC.tar.gz -C UWC > /dev/null 2>&1
    cd UWC

	# check if kpi app needed or not 
    if [ $KPI_APP_REQUIRED == "yes" ] || [ $KPI_APP_REQUIRED == "Yes" ];then
		setDevMode "DEV_MODE=true" "DEV_MODE=false"
		cp docker-compose_with_kpi.yml ../docker_setup/docker-compose.yml
		return 0
    fi
    
    if [ -z $DEPLOY_MODE ]; then 
	echo ${INFO}"Deployment mode is not provided hence setting it to = " IPC_PROD ${NC}
	setDevMode "DEV_MODE=true" "DEV_MODE=false"
	if [ $SCADA_REQUIRED == "no" ];then
		cp docker-compose_without_scada.yml ../docker_setup/docker-compose.yml

	elif [ $IS_TLS == "no" ] || [ $IS_TLS == "false" ]; then
		cp docker-compose_IPC_PROD_nonTLSScada.yml ../docker_setup/docker-compose.yml
	else
    		cp docker-compose.yml ../docker_setup/docker-compose.yml
	fi
	rm -rf UWC/
	return 0
    fi

    case $DEPLOY_MODE in
      IPC_PROD)
		echo ${INFO}"Deployment mode is set to = " $DEPLOY_MODE  ${NC}
		setDevMode "DEV_MODE=true" "DEV_MODE=false"
		if [ $SCADA_REQUIRED == "no" ];then
			cp docker-compose_without_scada.yml ../docker_setup/docker-compose.yml

		elif [ $IS_TLS == "no" ] || [ $IS_TLS == "false" ]; then
			cp docker-compose_IPC_PROD_nonTLSScada.yml ../docker_setup/docker-compose.yml
		else
            		cp docker-compose.yml ../docker_setup/docker-compose.yml
		fi
        ;;
      IPC_DEV)
		echo ${INFO}"Deployment mode is set to = " $DEPLOY_MODE  ${NC}
		setDevMode "DEV_MODE=false" "DEV_MODE=true"
		if [ $SCADA_REQUIRED == "no" ];then
			cp docker-compose_IPC_DEV_without_scada.yml ../docker_setup/docker-compose.yml

		elif [ $IS_TLS == "no" ] || [ $IS_TLS == "false" ]; then
			cp docker-compose_IPC_DEV.yml ../docker_setup/docker-compose.yml
		else
            		cp docker-compose_IPC_DEV_scada_TLS.yml ../docker_setup/docker-compose.yml
		fi
        ;;
      *)
             echo "${RED}ERROR:: Invalid deployment mode is provided. supported mode are IPC_PROD and IPC_DEV ${NC}"
	     rm -rf UWC/
	     exit 1
        ;;
    esac 

    rm -rf UWC/
}

#------------------------------------------------------------------
# configureExternalCerts
#
# Description:
#        This function is used to copy certificates in local directory 
# Return:
#        None
# Usage:
#        configureExternalCerts
#------------------------------------------------------------------
configureExternalCerts()
{
		echo "${GREEN}Copying required certs for scada-rtu.${NC}"
		cd ${Current_Dir} && rm -rf tmp_certs && mkdir tmp_certs
		mkdir -p ${Current_Dir}/tmp_certs/ca
		mkdir -p ${Current_Dir}/tmp_certs/client_crt
		mkdir -p ${Current_Dir}/tmp_certs/client_key
		cp $CA  ${Current_Dir}/tmp_certs/ca/
		check_for_errors "$?" "Incorrect certificate path is given in CA path...Please provide the correct path and re-run the script" \
                    "${GREEN}"".${NC}"

		cp $CLIENT_CERT  ${Current_Dir}/tmp_certs/client_crt/
		check_for_errors "$?" "Incorrect certificate path is given in Client certificate  path...Please provide the correct path and re-run the script" \
                    "${GREEN}"".${NC}"

		cp $CLIENT_KEY  ${Current_Dir}/tmp_certs/client_key/
		check_for_errors "$?" "Incorrect certificate path is given in Client key certificate  path...Please provide the correct path and re-run the script" \
                    "${GREEN}Certificates are successfully copied in required directory.${NC}"

		echo "${GREEN}Done.${NC}"
		return
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
	echo "${INFO}Reading the command line args...${NC}"
	for ARGUMENT in "$@"
	do
	    KEY=$(echo $ARGUMENT | cut -f1 -d=)
	    VALUE=$(echo $ARGUMENT | cut -f2 -d=)   

	   #echo ${GREEN}$KEY "=" $VALUE${NC}
	   #echo "${GREEN}==========================================${NC}"

	    case "$KEY" in
		    --caFile)    	CA=${VALUE} ;;     
	            --crtFile)   	CLIENT_CERT=${VALUE} ;; 
		    --keyFile)   	CLIENT_KEY=${VALUE} ;; 
		    --isTLS)   	 	IS_TLS=${VALUE} ;;
		    --brokerAddr) 	BROKER_HOST=${VALUE} ;; 
		    --brokerPort) 	BROKER_PORT=${VALUE} ;;
		    --qos) 		QOS=${VALUE} ;;  
		    --proxy) 		USER_PROXY=${VALUE} PROXY_EXIST="yes";;   
		    --withoutScada) 	SCADA_REQUIRED="no" ;; 
		    --withKpiApp) KPI_APP_REQUIRED=${VALUE} ;;
                    --deployMode) 	DEPLOY_MODE=${VALUE} ;; 
                    --interactive) ;;
                    --nonInteractive) ;;
		     *) echo "${RED}Invalid arguments passed, ${NC}" $KEY; Usage; ;;  
	    esac    
	done
	
	if [ $SCADA_REQUIRED == "yes" ];then
		if [ $IS_TLS == "1" ] || [ $IS_TLS == "true" ] || [ $IS_TLS == "yes" ];then	
			if [ ! -z "$CA" ] && [ ! -z "$CLIENT_CERT" ] && [ ! -z "$CLIENT_KEY" ]; then
				echo "${INFO}Required certificates are provided..${NC}"
			else 
				echo "${RED}Missing required certificates needed for TLS..${NC}"
				echo "${RED}Please provide the required arguments and re-run the script..${NC}"
				Usage
			fi
		fi
		
		if [ ! -z "$BROKER_HOST" ] && [ ! -z "$BROKER_PORT" ] && [ ! -z "$QOS" ]; then
			echo "${INFO}Broker addr, port and QOS are provided${NC}"
		else 
			echo "${RED}Missing broker addr or port or QOS..${NC}"
			echo "${RED}Please provide the required arguments and re-run the script..${NC}"
			Usage
		fi
	fi
	#echo "${GREEN}==========================================${NC}"
}

#------------------------------------------------------------------
# ModifyDockerSecrets
#
# Description:
#        This function is used to modify docker secrets as per command line args
# Return:
#        None
# Usage:
#        ModifyDockerSecrets
#------------------------------------------------------------------
ModifyDockerSecrets()
{
	echo "${GREEN}Modifying docker secrets for scada-certs...${NC}"
	ca_file_name=$(basename ${CA})
	client_cert_file_name=$(basename ${CLIENT_CERT})
	client_key_file_name=$(basename ${CLIENT_KEY})
	sed -i "s/.*scada_ext_certs\/ca.*/    file: provision\/Certificates\/scada_ext_certs\/ca\/$ca_file_name/"#g  ${Current_Dir}/docker_setup/docker-compose.yml
	sed -i "s/.*scada_ext_certs\/client_crt.*/    file: provision\/Certificates\/scada_ext_certs\/client_crt\/$client_cert_file_name/"#g  ${Current_Dir}/docker_setup/docker-compose.yml
	sed -i "s/.*scada_ext_certs\/client_key.*/    file: provision\/Certificates\/scada_ext_certs\/client_key\/$client_key_file_name/"#g  ${Current_Dir}/docker_setup/docker-compose.yml
	echo "${GREEN}Done...${NC}"

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
	echo "${INFO}--withKpiApp	Optional, this will install kpi-app container, can contain value yes/no.." 
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
		sudo ./01_pre-requisites.sh --isTLS=true  --caFile=\"scada_ext_certs/ca/root-ca.crt\" --crtFile=\"scada_ext_certs/client/client.crt\" --keyFile=\"scada_ext_certs/client/client.key\" --brokerAddr=\"127.0.0.1\" --brokerPort=\"1883\" --qos=1 --proxy=\"intel.proxy.com:811\"${NC}

		8. With KPI app 
		sudo ./01_pre-requisites.sh --withKpiApp=yes

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
	if [ ! -z $KPI_APP_REQUIRED ]; then echo "${INFO}--withKpiApp = $KPI_APP_REQUIRED${NC}";fi
	echo "${GREEN}==========================================${NC}"
}


#------------------------------------------------------------------
# createScadaConfigFile
#
# Description:
#        This function is used create scada_config.yml file required for scada-rtu container
# Return:
#        None
# Usage:
#        createScadaConfigFile
#------------------------------------------------------------------
createScadaConfigFile()
{
rm -rf /opt/intel/eis/uwc_data/scada-rtu/scada_config.yml
cat > /opt/intel/eis/uwc_data/scada-rtu/scada_config.yml << ENDOFFILE
---
# scada config parameter file

isTLS: $IS_TLS
mqttServerAddrSCADA: "$BROKER_HOST"
mqttServerPortSCADA: $BROKER_PORT 
qos: $QOS
ENDOFFILE
}

#------------------------------------------------------------------
# getUserInputs
#
# Description:
#        This function is used get required config from end user 
# Return:
#        None
# Usage:
#        getUserInputs
#------------------------------------------------------------------
getUserInputs()
{
	re='^[0-9]+$'

	echo "${INFO}Enter the follwing parameters required for scada-rtu container..${NC}"

 	if [ $KPI_APP_REQUIRED == "yes" ] || [ $KPI_APP_REQUIRED == "Yes" ];then
 		echo "${INFO}Proceding with kpi app installation${NC}"
		IS_TLS=true
		read -p "Enter the CA certificate full path including file name:" CA && if [ -z $CA ]; then echo "${RED}Error:: Empty value entered..${NC}"; echo "${RED}Kindly entered correct values and re-run the script..${NC}"; exit 1; fi 
		read -p "Enter the client certificate full path including file name:" CLIENT_CERT && if [ -z $CLIENT_CERT ]; then echo "${RED}Error:: Empty value entered..${NC}"; echo "${RED}Kindly entered correct values and re-run the script..${NC}";exit 1; fi
		read -p "Enter the client key certificate full path including file name:" CLIENT_KEY && if [ -z $CLIENT_KEY ]; then echo "${RED}Error:: Empty value entered..${NC}"; echo "${RED}Kindly entered correct values and re-run the script..${NC}"; exit 1; fi 		
 	else
		while :
		do 
			read -p "Is TLS required for scada-rtu (yes/no):" TLS
			if [ -z $TLS ];then
				echo "${RED}Empty value entered.${NC}"
			elif [ $TLS != "yes" ] && [ $TLS != "no" ] ;then
				echo "${RED}Invalid value entered. Allowed values (yes/no)${NC}"
			elif [ $TLS == "yes" ];then
				IS_TLS=true
				read -p "Enter the CA certificate full path including file name:" CA && if [ -z $CA ]; then echo "${RED}Error:: Empty value entered..${NC}"; echo "${RED}Kindly entered correct values and re-run the script..${NC}"; exit 1; fi 
				read -p "Enter the client certificate full path including file name:" CLIENT_CERT && if [ -z $CLIENT_CERT ]; then echo "${RED}Error:: Empty value entered..${NC}"; echo "${RED}Kindly entered correct values and re-run the script..${NC}";exit 1; fi
				read -p "Enter the client key certificate full path including file name:" CLIENT_KEY && if [ -z $CLIENT_KEY ]; then echo "${RED}Error:: Empty value entered..${NC}"; echo "${RED}Kindly entered correct values and re-run the script..${NC}"; exit 1; fi
				break
			elif [ $TLS == "no" ];then
				IS_TLS=false
				break
			fi
		done
	fi

	while :
	do 
		read -p "Enter the external broker address/hostname:" BROKER_HOST
		if [ -z $BROKER_HOST ];then
			echo "${RED}Empty value entered for broker address.${NC}"
		else
			break
		fi
	done

	while :
	do 
		read -p "Enter the external broker port number:" BROKER_PORT
		if [ -z $BROKER_PORT ];then
			echo "${RED}Empty value entered for broker port number.${NC}"
		elif ! [[ $BROKER_PORT =~ $re ]];then
			echo "${RED}Not a number.${NC}"
		else
			break
		fi
	done


	while :
	do 
		read -p "Enter the QOS for scada (between 0 to 2):" QOS
		if [ -z $QOS ];then
			echo "${RED}Empty value entered for qos.${NC}"
		elif ! [[ $QOS =~ $re ]];then
			echo "${RED}Not a number.${NC}"
		elif [ $QOS -lt 0 ] || [ $QOS -gt 2 ];then
			echo "${RED}Invalid value entered for QOS. Allowed values 0 - 2${NC}"
		else
			break
		fi
	done
	PrintAllArgs
}

echo "${GREEN}============================= Script 1.1 START ============================================${NC}"
# get the deployment mode
echo "arg1 :: "$1
echo "arg2 :: "$2
echo "arg1 :: "$3

if [ $3 == "--deployMode=IPC_DEV" ] ; then
	DEPLOY_MODE=IPC_DEV
fi

if [ $3 == "--deployMode=IPC_PROD" ] ; then
	DEPLOY_MODE=IPC_PROD
fi

if [ $2 == "--withKpiApp=yes" ] || [ $2 == "--withKpiApp=Yes" ] ;then
	echo "KPI app is required.."
	KPI_APP_REQUIRED=yes
fi

if [ $1 == "--interactive=1" ] ; then 
	getUserInputs
else
	ParseCommandLineArgs "$@"
fi

#PrintAllArgs

if [ $IS_TLS == "1" ] || [ $IS_TLS == "true" ] ||  [ $IS_TLS == "yes" ] ;then
	configureExternalCerts
	ModifyDockerSecrets
else
	echo "Cert configuration is not required.."
fi

createScadaConfigFile
copyDeployComposeFile

cd "${Current_Dir}"
echo "${GREEN}============================= Script 1.1 END ============================================${NC}"
exit 0
