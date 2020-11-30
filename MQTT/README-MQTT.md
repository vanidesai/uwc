```
********************************************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 **********************************************************************************************************************
```

This directory contains files to install MQTT container 

# Contents:

1. [Directory and file details](#All-internal-directory-file-details)

2. [Steps to set mosquitto version](#Steps-to-set-mosquitto-version-dev-or-prod)


# Directory and file details
Section to describe all directory contents and it's uses.

1. `Dockerfile` - Dockerfile to install mosquitto brocker inside MQTT container.
2. `mosquitto_dev.conf` - Mosquitto configuration file for eis IPC_DEV mode.
3. `mosquitto_prod.conf` - Mosquitto configuration file for eis IPC_PROD mode.