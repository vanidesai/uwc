# Universal Wellpad Controller (UWC)

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
## Directory details
UWC contains number of modules. These modules are deployed as containers. This directory contains list of libraries which are used in UWC modules. These libraries are built under uwc-common and used in all modules. 

Latest tagged version is taken for all the libraries. However, there are some critical defects fixed in these libraries under development branch. These fixes are needed for UWC. For these fixes, corrected files are maintained separately.
Listing of these files is provided below:

1. async_client.h
2. async_client.cpp
3. token.cpp

	Above files are maintained for following issues:
	https://github.com/eclipse/paho.mqtt.cpp/issues/211
	https://github.com/eclipse/paho.mqtt.cpp/issues/223
	https://github.com/eclipse/paho.mqtt.cpp/issues/235

4. tahu.c

	Above files are maintained for following issues:
	https://github.com/eclipse/tahu/pull/66

