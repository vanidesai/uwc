#********************************************************************************
# The source code contained or described herein and all documents related to 
# the source code ("Material") are owned by Intel Corporation. Title to the 
# Material remains with Intel Corporation.
#
# No license under any patent, copyright, trade secret or other intellectual
# property right is granted to or conferred upon you by disclosure or delivery of
# the Materials, either expressly, by implication, inducement, estoppel or otherwise.
#*********************************************************************************

ARG EIS_VERSION
FROM uwc_common:$EIS_VERSION as uwc_common
LABEL description="MQTT Bridge Application"

# Build MQTT-Bridge application
FROM ia_common:$EIS_VERSION as mqtt-bridge
WORKDIR /

# copy required dependencies
COPY MQTT-Bridge /MQTT-Bridge/. 
COPY --from=uwc_common /usr/yaml/include/yaml-cpp /usr/local/include/yaml-cpp
COPY --from=uwc_common /usr/yaml/lib/* /usr/local/lib/
COPY --from=uwc_common /usr/paho-cpp/lib/lib* /MQTT-Bridge/lib/
COPY --from=uwc_common /usr/lib/libpaho* /MQTT-Bridge/lib/
COPY --from=uwc_common /usr/ssl/lib/* /usr/local/lib/
COPY --from=uwc_common /usr/ssl/include/* /usr/local/include/
COPY --from=uwc_common /usr/paho-cpp/include/mqtt /usr/local/include/mqtt
COPY --from=uwc_common /usr/include/MQTT* /MQTT-Bridge/include/
COPY --from=uwc_common /EIS/log4cpp/build/include/log4cpp /MQTT-Bridge/include/log4cpp
COPY --from=uwc_common /EIS/log4cpp/build/lib/* /MQTT-Bridge/lib/
COPY --from=uwc_common /uwc_util/Release/libuwc-common.so /MQTT-Bridge/lib/
COPY --from=uwc_common /uwc_util/include/* /MQTT-Bridge/include/

# build sources
RUN cd /MQTT-Bridge/Release \
    &&	export LD_LIBRARY_PATH='/MQTT-Bridge/lib:/usr/local/lib' \
    &&  export INCLUDE_DIR='/bin' \
    &&	make clean all -s

# Build mqtt bridge container service
FROM ia_common:$EIS_VERSION as mqtt-bridge-service

WORKDIR /
# creating base directories
ENV APP_WORK_DIR /opt/intel/app/
RUN mkdir -p /opt/intel/app \
	&& mkdir -p /opt/intel/app/logs \
	&& mkdir -p /opt/intel/config \
	&& mkdir -p /opt/intel/eis/uwc_data

WORKDIR ${APP_WORK_DIR}

# copy required dependencies
COPY --from=mqtt-bridge /MQTT-Bridge/Release/MQTT_Bridge ${APP_WORK_DIR}
COPY --from=uwc_common /usr/yaml/lib/*.so.* ${APP_WORK_DIR}
COPY --from=uwc_common /usr/lib/libpaho* ${APP_WORK_DIR}
COPY --from=uwc_common /usr/paho-cpp/lib/lib* ${APP_WORK_DIR}
COPY --from=uwc_common /usr/ssl/lib/* /usr/local/lib/
COPY --from=uwc_common /usr/ssl/include/* /usr/local/include/
COPY --from=uwc_common /EIS/log4cpp/build/lib/* ${APP_WORK_DIR}
COPY --from=mqtt-bridge /MQTT-Bridge/Config/log4cpp.properties /opt/intel/config
COPY --from=uwc_common /uwc_util/Release/libuwc-common.so ${APP_WORK_DIR}

# set permissions to working dir for eisuser
RUN chown -R ${EIS_UID}:${EIS_UID} /opt/intel/config \
   && chown -R ${EIS_UID}:${EIS_UID} ${APP_WORK_DIR} \
   && chmod -R 770 ${APP_WORK_DIR}

#set the environment variable
ENV LD_LIBRARY_PATH ${APP_WORK_DIR}:/usr/local/lib

# Run the container
CMD ./MQTT_Bridge
