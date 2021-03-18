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


#------------------------------------------------------------------
# copy_external_certs_to_provision
#
# Description:
#        Copy external certificates to common directoty required for scada-rtu container
# Return:
#        None
# Usage:
#        copy_external_certs_to_provision
#------------------------------------------------------------------
copy_external_certs_to_provision()
{
	if [ -d ${Current_Dir}/tmp_certs ]; then
        echo "${GREEN}Copying required certs for scada-rtu.${NC}"
		cd ${Current_Dir}
		rm -rf ${Current_Dir}/../build/provision/Certificates/scada_ext_certs
		mkdir -p ${Current_Dir}/../build/provision/Certificates/scada_ext_certs
		mkdir -p ${Current_Dir}/../build/provision/Certificates/scada_ext_certs/ca
		mkdir -p ${Current_Dir}/../build/provision/Certificates/scada_ext_certs/client_crt
		mkdir -p ${Current_Dir}/../build/provision/Certificates/scada_ext_certs/client_key
		cp ${Current_Dir}/tmp_certs/ca/*  ${Current_Dir}/../build/provision/Certificates/scada_ext_certs/ca
		check_for_errors "$?" "Incorrect certificate path is given in command line...Please provide the required command line arguments and re-run the script" \
		            "${GREEN}"".${NC}"
		cp ${Current_Dir}/tmp_certs/client_crt/*  ${Current_Dir}/../build/provision/Certificates/scada_ext_certs/client_crt
		check_for_errors "$?" "Incorrect certificate path is given in command line...Please provide the required command line arguments and re-run the script" \
		            "${GREEN}"".${NC}"
		cp ${Current_Dir}/tmp_certs/client_key/*  ${Current_Dir}/../build/provision/Certificates/scada_ext_certs/client_key
		check_for_errors "$?" "Incorrect certificate path is given in command line...Please provide the required command line arguments and re-run the script" \
		            "${GREEN}Certificates are successfully copied in required directory.${NC}"
		chown -R  eisuser:eisuser ${Current_Dir}/../build/provision/Certificates/scada_ext_certs
		echo "${GREEN}Done.${NC}"		
		rm -rf ${Current_Dir}/tmp_certs
		return 0
	else
		echo "${GREEN}No need to configure certificates for scada-rtu container${NC}"
		echo "${INFO}******************* Script END *****************************${NC}"
    	fi	
}

echo "${GREEN}============================= Script 2.2 START ============================================${NC}"
copy_external_certs_to_provision
echo "${GREEN}============================= Script 2.2 END ============================================${NC}"
exit 0
