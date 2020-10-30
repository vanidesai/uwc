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


# ---------------------------------------
# Function to apply EIS patch for DBS 
# --------------------------------------
applyEISPatch()
{
	echo "${GREEN}Applying EIS patch for DBS...${NC}"
	cd ${working_dir}
	rm -rf UWC/ && mkdir UWC
	tar -xzvf UWC.tar.gz -C UWC > /dev/null 2>&1
	

	if [ $1 == "false" ];then
		cd UWC/DBS_Patches && cp 0001-U-ARCH-changes.patch ${working_dir}
		cd ${working_dir}
		# revert the patch if already applied
		if [ -f EISPatched.txt ];then
			patch -d . -f --strip=0 -R  < 0001-U-ARCH-changes.patch || true >/dev/null
		fi

		patch -d . -f --strip=0  < 0001-U-ARCH-changes.patch
		check_for_errors "$?" "Failed to apply EIS patch for DBS. Please check logs" \
				"${GREEN}Successfuly applied EIS patch for DBS${NC}"
	else
		cd UWC/DBS_Patches && cp 0001-U-ARCH-changes_DEV.patch ${working_dir}
		cd ${working_dir}
		# revert the patch if already applied
		if [ -f EISPatched.txt ];then
			patch -d . -f --strip=0 -R  < 0001-U-ARCH-changes_DEV.patch || true >/dev/null
		fi

		patch -d . -f --strip=0  < 0001-U-ARCH-changes_DEV.patch
		check_for_errors "$?" "Failed to apply EIS patch for DBS. Please check logs" \
				"${GREEN}Successfuly applied EIS patch for DBS${NC}"
	fi
	touch EISPatched.txt && chown $SUDO_USER:$SUDO_USER EISPatched.txt
	rm -rf UWC/
}

#------------------------------------------------------------------
# ConfigureDockerDaemon
#
# Description:
#        This function is used to set paramaters required for docker daemon
# Return:
#        None
# Usage:
#        ConfigureDockerDaemon
#------------------------------------------------------------------
ConfigureDockerDaemon()
{
	echo "${INFO}Setting up docker daemon..${NC}"
	rm -f /etc/docker/daemon.json && touch /etc/docker/daemon.json
cat > /etc/docker/daemon.json << EOB
{
  "icc": false,
  "log-level": "info",
  "log-driver": "json-file",
  "live-restore": true,
  "userland-proxy": false,
  "no-new-privileges": true,
  "default-ulimits": {
    "nofile": {
      "Name": "nofile",
      "Hard": 1024,
      "Soft": 1024
    },
    "nproc" : {
      "Name": "nproc",
      "Hard": 1024,
      "Soft": 1024
    }
  }
}
EOB
	echo "${INFO}Restarting docker..."
	systemctl restart docker
	if [ "$?" -eq 0 ]; then
        	echo "${GREEN}Docker is restarted successfully${cur_v}${NC}"
    	else
		echo "${RED}Failed to start the docker..${NC}"
		exit 1
	fi
	echo "${GREEN}Daemon is successfully configured ${NC}"
}

#------------------------------------------------------------------
# setDockerContentTrust
#
# Description:
#        This function is used to set DOCKER_CONTENT_TRUST paramater
# Return:
#        None
# Usage:
#        setDockerContentTrust
#------------------------------------------------------------------
setDockerContentTrust()
{

	echo "${INFO}Setting docker content trust..${NC}"
	grep -qxF 'DOCKER_CONTENT_TRUST=1' /etc/environment || echo 'DOCKER_CONTENT_TRUST=1' | tee -a /etc/environment
	if [ "$?" -eq 0 ]; then
        	echo "${GREEN}DOCKER_CONTENT_TRUST is successfully configured ${NC}"
    	else
		echo "${RED}Failed to set DOCKER_CONTENT_TRUST..${NC}"
		exit 1
	fi
}

#------------------------------------------------------------------
# install_audit_and_setup
#
# Description:
#        This function is used to set auditing for docker directories
# Return:
#        None
# Usage:
#        install_audit_and_setup
#------------------------------------------------------------------
install_audit_and_setup()
{
	echo "${INFO}Setting auditing for docker directories..${NC}"
	AUDIT_FILE=/etc/audit/rules.d/audit.rules
	apt-get install -y auditd audispd-plugins > /dev/null
	if grep -Fxq '## Docker Audit Rules' $AUDIT_FILE
	then
		echo "${INFO}Audit is already exists..${NC}"
	else
		echo "## Docker Audit Rules" >> $AUDIT_FILE
		echo "-w /usr/bin/docker -p wa -k docker" >> $AUDIT_FILE
		echo "-w /var/lib/docker -p wa -k docker" >> $AUDIT_FILE
		echo "-w /etc/docker -p wa -k docker" >> $AUDIT_FILE
		echo "-w /usr/lib/systemd/system/docker.service -p wa -k docker" >> $AUDIT_FILE
		echo "-w /etc/systemd/system/multi-user.target.wants/docker.service -p wa -k docker" >> $AUDIT_FILE 
		echo "-w /usr/lib/systemd/system/docker.socket -p wa -k docker" >> $AUDIT_FILE
		echo "-w /etc/default/docker -p wa -k docker" >> $AUDIT_FILE
		echo "-w /etc/docker/daemon.json -p wa -k docker" >> $AUDIT_FILE
		echo "-w /usr/bin/docker-containerd -p wa -k docker" >> $AUDIT_FILE
		echo "-w /usr/bin/docker-runc -p wa -k docker" >> $AUDIT_FILE
		echo "-w /usr/bin/containerd -p wa -k docker" >> $AUDIT_FILE
		echo "-w /etc/sysconfig/docker -p wa -k docker" >> $AUDIT_FILE
		echo "-w /usr/bin/dockerd -p wa -k docker" >> $AUDIT_FILE
		echo "-w /usr/sbin/runc -p wa -k docker" >> $AUDIT_FILE

	fi
	systemctl restart auditd
	if [ "$?" -eq 0 ]; then
        	echo "${GREEN}auditing is done..${NC}"
    	else
		echo "${RED}auditing is failed....${NC}"
		exit 1
	fi
}


echo "${GREEN}============================= Script 1.2 START ============================================${NC}"
#setHostIP
#changeTopicLen

# only works with EIS 2.2 PV
applyEISPatch "$@"
ConfigureDockerDaemon
setDockerContentTrust
install_audit_and_setup
echo "${GREEN}============================= Script 1.2 END ============================================${NC}"

cd "${working_dir}"
exit 0
