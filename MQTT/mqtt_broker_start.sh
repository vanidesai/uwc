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

# Starting mosquitto service
echo "Dev Mode is set to :: "$DEV_MODE
echo ""

if [ "${DEV_MODE}" = "true" ]; then
    echo "========================================"
    echo "Running broker without TLS.."
    echo "========================================"
    mosquitto -c /etc/mosquitto/mosquitto_dev.conf
else
    echo "========================================"
    echo "Running broker with TLS.."
    echo "========================================"
    mosquitto -c /etc/mosquitto/mosquitto_prod.conf
fi
