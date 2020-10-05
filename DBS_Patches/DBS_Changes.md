# Changes for DBS issue fixes and integration 

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

## Changes done on EIS patch for UWC
1. Removed root user from all UWC containers. (Used EIS_UID)
2. Sample HEALTHCHECK is added for scada-rtu container 
3. HEALTHCHECK logic is modified to use Polling socket instead of On-Demand read.
4. Removed 0.0.0.0:11883:11883 port mapping from mqtt_conatiner
5. OS check is added before applying chcon command. (This command is used for SELinux) in following files
	a. 02_provisionEIS.sh script.
	b. provision_eis.sh script
6. Common Custom network is created for all UWC containers.
7. Commented harden function from 02, 03 and 05 script. (This change is done to disable memory limits and cpu assignments)
