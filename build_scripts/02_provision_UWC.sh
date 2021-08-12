#!/bin/bash
# Copyright (c) 2021 Intel Corporation.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

Current_Dir=$(pwd)
RED=$(tput setaf 1)
GREEN=$(tput setaf 2)
MAGENTA=$(tput setaf 5)
NC=$(tput sgr0)

eii_build_dir="$Current_Dir/../../build"
Dev_Mode="false"
IS_SCADA=0
source uwc_common_lib.sh

deployMode=""
recipe=""
tls_req=""
cafile=""
crtFile=""
keyFile=""
brokerAddr=""
brokerPort=""
qos=""
ret=""

#------------------------------------------------------------------
# modifying_env
#
# Description:
#        Adding UWC env variables.
# Return:
#        None
# 
#------------------------------------------------------------------

modifying_env()
{
	echo "Addding UWC specific env variables"
	result=`grep -Fi 'env for UWC' ../../build/.env`
	if [ -z "${result}" ]; then
	    cat ../.env >> ../../build/.env
       
	fi
    result=`grep -F 'uwc/'  ../../.gitignore`
	if [ -z "${result}" ]; then
	    sed -i '$a uwc/' ../../.gitignore
	fi	
  
}
#------------------------------------------------------------------
# eii_provision
#
# Description:
#        Performs prvovisioning as per docker-compose.yml file.
# Return:
#        None
# Usage:
#       eii_provision
#------------------------------------------------------------------
eii_provision()
{
    docker stop $(docker ps -a -q)
    if [ -d "${eii_build_dir}/provision/" ];then
        cd "${eii_build_dir}/provision/"
    else
        echo "${RED}ERROR: ${eii_build_dir}/provision/ is not present.${NC}"
        exit 1 # terminate and indicate error
    fi

    ./provision.sh ../docker-compose.yml 
    check_for_errors "$?" "Provisioning is failed. Please check logs" \
                    "${GREEN}Provisioning is done successfully.${NC}"
    echo "${GREEN}>>>>>${NC}"
    echo "${GREEN}For building & running UWC services,run 03_Build_Run_UWC.sh script${NC}"

    return 0
}

mqtt_certs()
{
    echo "${GREEN} Genrating certs for mqtt${NC}"	
    mkdir ./temp ./temp/client ./temp/server

    openssl req -config ../../uwc/Others/mqtt_certs/mqtt_cert_openssl_config -new -newkey rsa:3072 -keyout  ./temp/client/key.pem -out ./temp/client/req.pem -days 3650 -outform PEM -subj /CN=mymqttcerts/O=client/L=$$$/ -nodes

    openssl req -config ../../uwc/Others/mqtt_certs/mqtt_cert_openssl_config -new -newkey rsa:3072 -keyout  ./temp/server/key.pem -out ./temp/server/req.pem -days 3650 -outform PEM -subj /CN=mymqttcerts/O=server/L=$$$/ -nodes

    openssl ca -days 3650 -cert ../../build/provision/rootca/cacert.pem -keyfile ../../build/provision/rootca/cakey.pem -in ./temp/server/req.pem -out ./temp/mymqttcerts_server_certificate.pem  -outdir ../../build/provision/rootca/certs -notext -batch -extensions server_extensions -config ../../uwc/Others/mqtt_certs/mqtt_cert_openssl_config

    openssl ca -days 3650 -cert ../../build/provision/rootca/cacert.pem -keyfile ../../build/provision/rootca/cakey.pem -in ./temp/client/req.pem -out ./temp/mymqttcerts_client_certificate.pem -outdir ../../build/provision/rootca/certs -notext -batch -extensions client_extensions -config ../../uwc/Others/mqtt_certs/mqtt_cert_openssl_config

    mkdir ../../build/provision/Certificates/mymqttcerts
    cp -rf ./temp/mymqttcerts_server_certificate.pem ../../build/provision/Certificates/mymqttcerts/mymqttcerts_server_certificate.pem
    cp -rf ./temp/mymqttcerts_client_certificate.pem ../../build/provision/Certificates/mymqttcerts/mymqttcerts_client_certificate.pem
    cp -rf ./temp/server/key.pem  ../../build/provision/Certificates/mymqttcerts/mymqttcerts_server_key.pem
    cp -rf ./temp/client/key.pem ../../build/provision/Certificates/mymqttcerts/mymqttcerts_client_key.pem
   
    sudo chown -R eiiuser:eiiuser ../../build/provision/Certificates/mymqttcerts/ 
    rm -rf ./temp
    return 0
}

harden()
{
    docker container update --pids-limit=100 --restart=on-failure:5 --cpu-shares 512 -m 1G --memory-swap -1 ia_etcd
    docker container update --pids-limit=100 --restart=on-failure:5 --cpu-shares 512 -m 128M --memory-swap -1 ia_etcd_provision
}

#------------------------------------------------------------------
# configure_usecase
#
# Description: 
#               Configures UWC modules. It takes a single parameter with recipe type in non interactive mode.
#       
# Return:
#        None
# Usage:
#       configure_usecase
#       configure_usecase  "3"
#------------------------------------------------------------------

configure_usecase()
{
    yn=""
    interactive=1
    if [ ! -z "$1" ]; then
        interactive=0       
        if [[ "$1" -ge 1 ]] && [[ "$1" -le 4 ]]; then
            yn="$1"
        else 
            echo "Invalid Option inputted for UWC recipes"
            usage
            exit 1
        fi
    else
        echo "Please choose one of the below options based on the use case (combination of UWC services) needed." 
        echo "1) Basic UWC micro-services without KPI-tactic Application & Sparkplug-Bridge - (Modbus-master TCP & RTU, mqtt-bridge, internal mqtt broker, ETCD server, ETCD UI & other base EII & UWC services)"
        echo "2) Basic UWC micro-services as in option 1 along with KPI-tactic Application (Without Sparkplug-Bridge)"
        echo "3) Basic UWC micro-services & KPI-tactic Application along with Sparkplug-Bridge"
        echo "4) Basic UWC micro-services with Sparkplug-Bridge and no KPI-tactic Application" 
        echo "5) Basic UWC micro-services with Time series micro-services (Telegraf & InfluxDBConnector)"
        read yn
    fi
    cd ${eii_build_dir}
    while :
    do
        case $yn in
            1)
                echo "Running Basic UWC micro-services without KPI-tactic Application & Sparkplug-Bridge"
                python3 builder.py -f ../uwc/uwc_recipes/uwc-pipeline-without-sparkplug-bridge.yml
                if [ "$?" != 0 ]; then
                    echo "${RED}Error running EII builder script. Check the recipe configuration file...!${NC}" 
                    exit 1
                fi
                echo "${GREEN}EII builder script successfully generated consolidated docker-compose & configuration files.${NC}"
                break
                ;;
            2)
                echo "Running Basic UWC micro-services with KPI-tactic Application & without Sparkplug-Bridge"
                python3 builder.py -f ../uwc/uwc_recipes/uwc-pipeline-with-kpi-no-sparkplug-bridge.yml
                if [ "$?" != 0 ]; then
                    echo "${RED}Error running EII builder script. Check the recipe configuration file...!${NC}" 
                    exit 1
                fi
                echo "${GREEN}EII builder script successfully generated consolidated docker-compose & configuration files.${NC}"
                break
                ;;
            3)
                echo "Running Basic UWC micro-services with KPI-tactic Application & with Sparkplug-Bridge"
                python3 builder.py -f ../uwc/uwc_recipes/uwc-pipeline-with-sparkplug-bridge_and_kpi.yml
                if [ "$?" != 0 ]; then
                    echo "${RED}Error running EII builder script. Check the recipe configuration file...!${NC}" 
                    exit 1
                fi
                echo "${GREEN}EII builder script successfully generated consolidated docker-compose & configuration files.${NC}"
                echo "${GREEN}Sparkplug-Bridge related configurations are being updated to the consolidated docker-compose.yml file.${NC}"

                IS_SCADA=1
                cd  ${Current_Dir}
                if [ $interactive == 1 ]; then
                    if [ "$deployMode" == "dev" ]; then
                        ./2.1_ConfigureScada.sh "--deployModeInteract=dev"
                        ret="$?"
                    else
                        ./2.1_ConfigureScada.sh
                        ret="$?"
                    fi
                fi
                break
                ;;                
            4)
                echo "Running Basic UWC micro-services with no KPI-tactic Application & with Sparkplug-Bridge"
                python3 builder.py -f ../uwc/uwc_recipes/uwc-pipeline-with-sparkplug-bridge-no-kpi.yml
                if [ "$?" != 0 ]; then
                    echo "${RED}Error running EII builder script. Check the recipe configuration file...!${NC}" 
                    exit 1
                fi
                echo "${GREEN}EII builder script successfully generated consolidated docker-compose & configuration files.${NC}"
                echo "${GREEN}Sparkplug-Bridge related configurations are being updated to the consolidated docker-compose.yml file.${NC}"

                IS_SCADA=1
                cd  ${Current_Dir}
                if [ $interactive == 1 ]; then                        
                    if [ "$deployMode" == "dev" ]; then
                        ./2.1_ConfigureScada.sh "--deployModeInteract=dev"
                        ret="$?"
                    else
                        ./2.1_ConfigureScada.sh
                        ret="$?"
                    fi
                fi
                break
                ;;
            5)
                echo "Running Basic UWC micro-services with time series services (Telegraf & InfluxDBCOnnector)"
                python3 builder.py -f ../uwc/uwc_recipes/uwc-pipeline-basic-timeseries.yml
                if [ "$?" != 0 ]; then
                    echo "${RED}Error running EII builder script. Check the recipe configuration file...!${NC}"
                    exit 1
                fi
                echo "${GREEN}EII builder script successfully generated consolidated docker-compose & configuration files.${NC}"
                break
                ;;
            6)
                echo "Running Basic UWC micro-services with time series services (Telegraf & InfluxDBCOnnector) along with KPI-tactic app"
                python3 builder.py -f ../uwc/uwc_recipes/uwc-pipeline-basic-kpiapp-timeseries.yml
                if [ "$?" != 0 ]; then
                    echo "${RED}Error running EII builder script. Check the recipe configuration file...!${NC}"
                    exit 1
                fi
                echo "${GREEN}EII builder script successfully generated consolidated docker-compose & configuration files.${NC}"
                break
                ;;
            7)
                echo "Running Basic UWC micro-services with time series services (Telegraf & InfluxDBCOnnector) along with KPI-tactic app & Sparkplug"
                python3 builder.py -f ../uwc/uwc_recipes/uwc-pipeline-basic-kpiapp-sparkplug-timeseries.yml
                if [ "$?" != 0 ]; then
                    echo "${RED}Error running EII builder script. Check the recipe configuration file...!${NC}"
                    exit 1
                fi
                echo "${GREEN}EII builder script successfully generated consolidated docker-compose & configuration files.${NC}"
                break
                ;;
            *)
                echo "Proper use-case option not selected. PLease select the right option as per help menu & re-provision UWC micro-services: ${yn}"
                usage
                exit 1
            esac
        done
}

#------------------------------------------------------------------
# set_mode
#
# Description:
#       Configures deployment mode to be used either dev or prod. It passes either "dev" or "prod"
#       in non-interactive mode.
# Return:
#        None
# Usage:
#       set_mode
#       set_mode "dev"
#       set_mode "prod"
#------------------------------------------------------------------
set_mode()
{
    mode=""
    if [ ! -z "$1" ]; then
        if [ "$1" == "dev" ] ; then
            mode=1
        elif [ "$1" == "prod" ]; then
            mode=2
        else
            echo "Invalid Option for deploy mode."
            usage
            exit 1
        fi
    else
        echo "Please choose one of the below options based on dev or prod mode. "
        echo "1) Dev"
        echo "2) Prod"
        read mode
    fi
    cd ${eii_build_dir}
    while :
        do
            case $mode in
                1)
                    deployMode="dev"
                    echo "User inputted dev mode"
                    echo "${INFO}Setting dev mode to true ${NC}"    
                    sed -i 's/DEV_MODE=false/DEV_MODE=true/g' $eii_build_dir/.env
                    sed -i 's/MQTT_PROTOCOL=ssl/MQTT_PROTOCOL=tcp/g' $eii_build_dir/.env
                    
                    if [ "$?" -ne "0" ]; then
                        echo "${RED}Failed to set dev mode."
                        echo "${GREEN}Kinldy set DEV_MODE to false manualy in .env file and then re--run this script"
                        exit 1
                    else
                        echo "${GREEN}Dev Mode is set to true ${NC}"
                    fi
                    break
                    ;;
                2)
                    deployMode="prod"
                    echo "User inputted prod mode"
                    echo "${INFO}Setting dev mode to false ${NC}"    
                    sed -i 's/DEV_MODE=true/DEV_MODE=false/g' $eii_build_dir/.env
                    sed -i 's/MQTT_PROTOCOL=tcp/MQTT_PROTOCOL=ssl/g' $eii_build_dir/.env
                    if [ "$?" -ne "0" ]; then
                        echo "${RED}Failed to set dev mode."
                        echo "${GREEN}Kinldy set DEV_MODE to false manually in .env file and then re--run this script"
                        exit 1
                    else
                        echo "${GREEN}Dev Mode is set to false ${NC}"
                    fi
                    break
                    ;;
                *)
                    echo "User entered wrong option, hence defaulting to prod mode: ${yn}"
                    exit 1
            esac
        done
}

#------------------------------------------------------------------
# parse_command_line_args
#
# Description:
#        This function is used to Parse command line arguments passed to this script
# Return:
#        None
# usage:
#        parse_command_line_args <list of arguments>
#------------------------------------------------------------------
parse_command_line_args()
{       
    echo "${INFO}Reading the command line args...${NC}"
    for ARGUMENT in "$@"
    do
        KEY=$(echo $ARGUMENT | cut -f1 -d=)
        VALUE=$(echo $ARGUMENT | cut -f2 -d=)

       echo ${GREEN}$KEY "=" $VALUE${NC}
       echo "${GREEN}==========================================${NC}"

        case "$KEY" in
            --deployMode)   deployMode=${VALUE} ;;  
            --recipe)       recipe=${VALUE} ;;
            --usage)        usage;;
            --help)         usage;;
        esac
    done

    if [ -z $deployMode ]; then 
        echo "Deploy Mode not provided. "
        read -p "Enter the Deploy Mode (e.g. prod or dev ):" deployMode 
        if [ -z $deployMode ]; then 
            echo "${RED}Error:: Empty value entered..${NC}"
            echo "${RED}Kindly enter correct values and re-run the script..${NC}" 
            exit 1 
        fi 
    fi

    if [ -z $recipe ]; then 
        echo "Recipe not provided. "
        echo "${INFO}--recipe  Recipe file to be used by EII builder.
                            1: All basic UWC modules. (no KPI-tactic Application, no SPARKPLUG-BRIDGE)
                            2: All basic UWC modules + KPI-tactic Application (no SPARKPLUG-BRIDGE)
                            3: All modules (with KPI-tactic Application and with SPARKPLUG-BRIDGE)
                            4: All basic UWC modules + SPARKPLUG-BRIDGE (no KPI-tactic Application)"
        read -p "Enter the recipe (e.g. --recipe=1 or 2 or 3 or 4 ):"  recipe 
        if [ -z $recipe ]; then 
            echo "${RED}Error:: Empty value entered..${NC}"
            echo "${RED}Kindly enter correct values and re-run the script..${NC}" 
            exit 1 
        fi 
    fi
}

#------------------------------------------------------------------
# usage
#
# Description:
#        Help function 
# Return:
#        None
# Usage:
#        Usage
#------------------------------------------------------------------
usage()
{
    echo 
    echo "${BOLD}${INFO}==================================================================================${NC}"
    echo
    echo "${BOLD}${GREEN}Note : If no options provided then script will run with the interactive mode${NC}"
    echo
    echo "${INFO}List of available options..."
    echo 
    echo "${INFO}--deployMode  dev or prod"
    echo
    echo "${INFO}--recipe  Recipe file to be referred for provisioning:
                            1: All basic UWC module. (no KPI-tactic Application, no SPARKPLUG-BRIDGE)
                            2: All basic UWC modules + KPI-tactic Application (no SPARKPLUG-BRIDGE)
                            3: All modules (with KPI-tactic Application and with SPARKPLUG-BRIDGE)
                            4: All basic UWC modules + SPARKPLUG-BRIDGE (no KPI-tactic Application)"
    echo
    echo "${INFO}--isTLS  yes/no to enable/disable TLS for sparkplug-bridge. Only applicable for recipes 3 and 4"
    echo 
    echo "${INFO}--cafile  Root CA file, required only if isTLS is true. This is applicable for SPARKPLUG-BRIDGE (ie recipes 3 and 4)"
    echo "${INFO}          This Root CA file is the file (e.g., .crt) issued for communication with sparkplug-bridge external MQTT broker for Sparkplug communication."
    echo
    echo "${INFO}--crtfile  client certificate file, required only if isTLS is true. This is applicable for SPARKPLUG-BRIDGE (ie recipes 3 and 4)"
    echo "${INFO}           client certificate file is the file (e.g., .crt) issued for communication with sparkplug-bridge external MQTT broker for Sparkplug communication."
    echo
    echo "${INFO}--keyFile  client key crt file, required only if isTLS is true. This is applicable for SPARKPLUG-BRIDGE (ie recipes 3 and 4)"
    echo "${INFO}           This is private key file is the file (e.g., .key) issued for communication with sparkplug-bridge external MQTT broker for Sparkplug communication."
    echo
    echo 
    echo "${INFO}  Below options namely brokerAddr, brokerPort and qos These are applicable for SPARKPLUG-BRIDGE. (i.e., value of --recipe is 3 or 4).
                   It tells QOS value to be used for external MQTT communication."
    echo
    echo "${INFO}--brokerAddr   sparkplug-bridge external broker IP address/Hostname"
    echo
    echo "${INFO}--brokerPort   sparkplug-bridge external broker port number"
    echo
    echo "${INFO}--qos  QOS used by sparkplug-bridge container to publish messages, can take values between 0 to 2 inclusive" 
    echo
    echo "Different use cases with --deployMode=dev. To switch to prod mode only replace --deployMode=prod" 
    echo 
    echo "${BOLD}${MAGENTA}
        1. sparkplug-bridge with TLS and no KPI-tactic Application 
         sudo ./02_provision_UWC.sh --deployMode=dev --recipe=4 --isTLS=yes  
         --caFile=\"scada_ext_certs/ca/root-ca.crt\" --crtFile=\"scada_ext_certs/client/client.crt\" 
         --keyFile=\"scada_ext_certs/client/client.key\" --brokerAddr=\"192.168.1.89\" --brokerPort=1883 --qos=1
        
        2. sparkplug-bridge without TLS  and no KPI-tactic Application
         sudo ./02_provision_UWC.sh --deployMode=dev --recipe=4 --isTLS=no --brokerAddr=\"192.168.1.89\" --brokerPort=1883 --qos=1


        3. All UWC basic modules (no KPI-tactic Application, no SPARKPLUG-BRIDGE) Note: TLS not required here.
         sudo ./02_provision_UWC.sh --deployMode=dev --recipe=1 

        4. All UWC basic modules (with KPI-tactic Application, no SPARKPLUG-BRIDGE). Note: TLS not required here.
         sudo ./02_provision_UWC.sh --deployMode=dev --recipe=2 

        5. All UWC modules (with KPI-tactic Application and with SPARKPLUG-BRIDGE). Note: This use case does not use TLS mode.
         sudo ./02_provision_UWC.sh --deployMode=dev --recipe=3 --isTLS=no  
         --brokerAddr=\"192.168.1.23\" --brokerPort=1883 --qos=1

        6. All UWC modules (with KPI-tactic Application and with SPARKPLUG-BRIDGE). Note: This use case uses TLS mode.
          sudo ./02_provision_UWC.sh --deployMode=dev --recipe=3 --isTLS=yes  
          --caFile=\"scada_ext_certs/ca/root-ca.crt\" --crtFile=\"scada_ext_certs/client/client.crt\" 
          --keyFile=\"scada_ext_certs/client/client.key\" --brokerAddr=\"192.168.1.11\" --brokerPort=1883 --qos=1

        7. Fully interactive
          sudo ./02_provision_UWC.sh
       "
    echo "${INFO}===================================================================================${NC}"
    exit 1
}

export DOCKER_CONTENT_TRUST=1
export DOCKER_BUILDKIT=1
check_root_user
check_internet_connection
modifying_env
docker_verify
docker_compose_verify
if [ -z "$1" ]; then
    set_mode
    configure_usecase 
else
    parse_command_line_args "$@"
    set_mode  "$deployMode"
    configure_usecase   "$recipe"
    if [[ "${IS_SCADA}" -eq "1" ]]; then
           ./2.1_ConfigureScada.sh "$@"
        ret="$?"
    fi    
fi
if [[ "${IS_SCADA}" -eq "1" ]]; then 
    if [[ "$ret" == 1 ]]; then 
        echo "ConfigureScada failed. Please see Usage"
        usage
        exit 1
    else
        echo "${GREEN}ConfigureScada successfully.${NC}"
    fi   
fi
eii_provision
if [[ "$deployMode" == "prod" ]]; then
    mqtt_certs
fi
if [[ "${IS_SCADA}" -eq "1" ]]; then 
   cd  ${Current_Dir}
  ./2.2_CopyScadaCertsToProvision.sh
fi
harden
