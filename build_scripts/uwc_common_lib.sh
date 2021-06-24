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

# This script contains common functions which can be used by other scripts

# ----------------------------
# Checking for root user
# ----------------------------
check_root_user()
{
   echo "Checking for root user ..."    
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
check_internet_connection()
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
        echo "${MAGENTA}Please run the pre-rquiste script before deploying EII.${NC}" 
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

