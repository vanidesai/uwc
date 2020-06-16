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
LABEL description="SCADA RTU container"

# Build SCADA-RTU application
FROM ia_common:$EIS_VERSION as scada-rtu
WORKDIR /

# copy required dependencies
COPY SCADA-RTU-APP /SCADA-RTU-APP/. 
COPY --from=uwc_common /usr/yaml/include/yaml-cpp /usr/local/include/yaml-cpp
COPY --from=uwc_common /usr/yaml/lib/* /usr/local/lib/
COPY --from=uwc_common /usr/paho-cpp/lib/lib* /SCADA-RTU-APP/lib/
COPY --from=uwc_common /usr/lib/libpaho* /SCADA-RTU-APP/lib/
COPY --from=uwc_common /usr/paho-cpp/include/mqtt /usr/local/include/mqtt
COPY --from=uwc_common /usr/include/MQTT* /SCADA-RTU-APP/include/
COPY --from=uwc_common /EIS/log4cpp/build/include/log4cpp /SCADA-RTU-APP/include/log4cpp
COPY --from=uwc_common /EIS/log4cpp/build/lib/* /SCADA-RTU-APP/lib/
COPY --from=uwc_common /EIS/tahu/client_libraries/c/lib/libtahu.so /SCADA-RTU-APP/lib/
COPY --from=uwc_common /EIS/tahu/client_libraries/c/include/* /SCADA-RTU-APP/include/tahu/

# build sources
RUN cd /SCADA-RTU-APP/Release \
    &&	export LD_LIBRARY_PATH='/SCADA-RTU-APP/lib:/usr/local/lib' \
    &&  export INCLUDE_DIR='/bin' \
    &&	make clean all -s

# Build scada-rtu container service
FROM ia_common:$EIS_VERSION as scada-rtu-service

WORKDIR /
# creating base directories
ENV APP_WORK_DIR /opt/intel/app/
RUN mkdir -p /opt/intel/app \
	&& mkdir -p /opt/intel/config \
	&& mkdir -p /opt/intel/eis/uwc_data

WORKDIR ${APP_WORK_DIR}

# copy required dependencies
COPY --from=scada-rtu /SCADA-RTU-APP/Release/SCADARTU ${APP_WORK_DIR}
COPY --from=scada-rtu /SCADA-RTU-APP/lib/* ${APP_WORK_DIR}
COPY --from=uwc_common /usr/yaml/lib/*.so.* ${APP_WORK_DIR}
COPY --from=uwc_common /usr/lib/libpaho* ${APP_WORK_DIR}
COPY --from=uwc_common /usr/paho-cpp/lib/lib* ${APP_WORK_DIR}
COPY --from=uwc_common /EIS/log4cpp/build/lib/* ${APP_WORK_DIR}
COPY --from=uwc_common /EIS/tahu/client_libraries/c/lib/libtahu.so ${APP_WORK_DIR}/lib/
COPY --from=uwc_common /EIS/tahu/client_libraries/c/include/* ${APP_WORK_DIR}/include/tahu/
COPY --from=scada-rtu /SCADA-RTU-APP/Config/log4cpp.properties /opt/intel/config

# set permissions to working dir for eisuser
RUN chown -R ${EIS_UID}:${EIS_UID} /opt/intel/config \
   && chown -R ${EIS_UID}:${EIS_UID} ${APP_WORK_DIR} \
   && chmod -R 770 ${APP_WORK_DIR}

#set the environment variable
ENV LD_LIBRARY_PATH ${APP_WORK_DIR}:/usr/local/lib

# Run the container
CMD ./SCADARTU
