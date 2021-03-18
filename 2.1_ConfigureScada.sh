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
source ./uwc_common_lib.sh
Current_Dir=$(pwd)
RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
MAGENTA=$(tput setaf 5)
NC=$(tput sgr0)

# vars
CA=""
CLIENT_CERT=""
CLIENT_KEY=""
IS_TLS=""
BROKER_HOST=""
BROKER_PORT=""
QOS=""

#------------------------------------------------------------------
# configure_external_certs
#
# Description:
#        This function is used to copy certificates in local directory 
# Return:
#        None
# Usage:
#        configure_external_certs
#------------------------------------------------------------------
configure_external_certs()
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
# edit_docker_compose_file
#
# Description:
#        This function is used to add docker secrets as per command line args
# Return:
#        None
# Usage:
#        edit_docker_compose_file
#------------------------------------------------------------------
edit_docker_compose_file()
{
    echo "${GREEN}Adding docker secrets for scada-rtu...${NC}"
    sed -i '/- etcd_SCADA-RTU_key/ a\    - scadahost_ca_cert' ${Current_Dir}/../build/docker-compose.yml
	sed -i '/scadahost_ca_cert/ a\    - scadahost_client_key' ${Current_Dir}/../build/docker-compose.yml
	sed -i '/scadahost_client_key/ a\    - scadahost_client_cert' ${Current_Dir}/../build/docker-compose.yml    
	echo "${GREEN}Appending docker secrets paths for scada-rtu...${NC}"
	ca_file_name=$(basename ${CA})
	client_cert_file_name=$(basename ${CLIENT_CERT})
	client_key_file_name=$(basename ${CLIENT_KEY})
	sed -i '$ a\  scadahost_ca_cert:' ${Current_Dir}/../build/docker-compose.yml
	sed -i "$ a\    file: provision\/Certificates\/scada_ext_certs\/ca\/$ca_file_name" ${Current_Dir}/../build/docker-compose.yml
	sed -i '$ a\  scadahost_client_cert:' ${Current_Dir}/../build/docker-compose.yml
	sed -i "$ a\    file: provision\/Certificates\/scada_ext_certs\/client_crt\/$client_cert_file_name" ${Current_Dir}/../build/docker-compose.yml
	sed -i '$ a\  scadahost_client_key:' ${Current_Dir}/../build/docker-compose.yml
	sed -i "$ a\    file: provision\/Certificates\/scada_ext_certs\/client_key\/$client_key_file_name" ${Current_Dir}/../build/docker-compose.yml
	echo "${GREEN}Done...${NC}"
}

#------------------------------------------------------------------
# print_all_args
#
# Description:
#        This function is used to print all given values on console 
# Return:
#        None
# Usage:
#        print_all_args
#------------------------------------------------------------------
print_all_args()
{
    echo "${GREEN}==========================================${NC}"
	echo "${GREEN}Given values..${NC}"
	if [ ! -z $IS_TLS ]; then echo "${INFO}--isTLS = $IS_TLS";fi
	if [ ! -z $CA ]; then echo "${INFO}--cafile = $CA";fi
	if [ ! -z $CLIENT_CERT ]; then echo "${INFO}--crtfile = $CLIENT_CERT";fi
	if [ ! -z $CLIENT_KEY ]; then echo "${INFO}--keyFile = $CLIENT_KEY";fi
	if [ ! -z $BROKER_HOST ]; then echo "${INFO}--brokerAddr = $BROKER_HOST";fi
	if [ ! -z $BROKER_PORT ]; then echo "${INFO}--brokerPort = $BROKER_PORT";fi
	if [ ! -z $QOS ]; then echo "${INFO}--qos = $QOS";fi
	echo "${GREEN}==========================================${NC}"
}

#------------------------------------------------------------------
# create_scada_config_file
#
# Description:
#        This function is used create scada_config.yml file required for scada-rtu container
# Return:
#        None
# Usage:
#        create_scada_config_file
#------------------------------------------------------------------
create_scada_config_file()
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
# get_user_inputs
#
# Description:
#        This function is used get required config from end user 
# Return:
#        None
# Usage:
#        get_user_inputs
#------------------------------------------------------------------
get_user_inputs()
{
	re='^[0-9]+$'

	echo "${INFO}Enter the follwing parameters required for scada-rtu container..${NC}"
	while :
	do 
		read -p "Is TLS required for scada-rtu (yes/no):" TLS
		if [ -z $TLS ];then
			echo "${RED}Empty value entered.${NC}"
		elif [ $TLS != "yes" ] && [ $TLS != "no" ] ;then
			echo "${RED}Invalid value entered. Allowed values (yes/no)${NC}"
		elif [ $TLS == "yes" ];then
			IS_TLS=true
			read -p "Enter the CA certificate full path including file name (e.g. /home/certs/root-ca.crt):" CA && if [ -z $CA ]; then echo "${RED}Error:: Empty value entered..${NC}"; echo "${RED}Kindly enter correct values and re-run the script..${NC}"; exit 1; fi 
			read -p "Enter the client certificate full path including file name (e.g. /home/certs/client.crt):" CLIENT_CERT && if [ -z $CLIENT_CERT ]; then echo "${RED}Error:: Empty value entered..${NC}"; echo "${RED}Kindly enter correct values and re-run the script..${NC}";exit 1; fi
			read -p "Enter the client key certificate full path including file name (e.g. /home/certs/client.key):" CLIENT_KEY && if [ -z $CLIENT_KEY ]; then echo "${RED}Error:: Empty value entered..${NC}"; echo "${RED}Kindly enter correct values and re-run the script..${NC}"; exit 1; fi
			break
		elif [ $TLS == "no" ];then
			IS_TLS=false
			break
		fi
	done

	while :
	do 
		read -p "Enter the external broker address/hostname (e.g. 192.168.0.5 or dummyhost.com):" BROKER_HOST
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
	print_all_args
}

echo "${GREEN}============================= Script 2.1 START ============================================${NC}"
get_user_inputs

if [ $IS_TLS == "1" ] || [ $IS_TLS == "true" ] ||  [ $IS_TLS == "yes" ] ;then
	configure_external_certs
	edit_docker_compose_file
else
	echo "Cert configuration is not required.."
fi

create_scada_config_file
cd "${Current_Dir}"
echo "${GREEN}============================= Script 2.1 END ============================================${NC}"
exit 0
