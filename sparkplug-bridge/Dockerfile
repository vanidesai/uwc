#********************************************************************************
# The source code contained or described herein and all documents related to 
# the source code ("Material") are owned by Intel Corporation. Title to the 
# Material remains with Intel Corporation.
#
# No license under any patent, copyright, trade secret or other intellectual
# property right is granted to or conferred upon you by disclosure or delivery of
# the Materials, either expressly, by implication, inducement, estoppel or otherwise.
#*********************************************************************************

ARG EII_VERSION
FROM uwc_common:$EII_VERSION as uwc_common
FROM ia_eiibase:$EII_VERSION as eiibase
LABEL description="SPARKPLUG BRIDGE container"

# Build tahu library
FROM eiibase as build-tahu
RUN apt-get update && apt-get install -y libmosquitto-dev \
    &&  git clone --branch develop https://github.com/eclipse/tahu \
    && cd tahu \ 
    && git checkout 9bf91b69f5e9214e7ec765f2ad20acc7bc41236d \
    &&  cd client_libraries/c \
    && sed -i 's/#define SPARKPLUG_DEBUG 1/\/\/#define SPARKPLUG_DEBUG 0/g' include/tahu.h \
    &&	make -s 

# Build SPARKPLUG-BRIDGE application
FROM ia_common:$EII_VERSION as sparkplug-bridge
WORKDIR /

# copy required dependencies
COPY SPARKPLUG-BRIDGE /SPARKPLUG-BRIDGE/. 
COPY --from=uwc_common /usr/yaml/include/yaml-cpp /usr/local/include/yaml-cpp
COPY --from=uwc_common /usr/yaml/lib/* /usr/local/lib/
COPY --from=uwc_common /usr/paho-cpp/lib/lib* /SPARKPLUG-BRIDGE/lib/
COPY --from=uwc_common /usr/lib/libpaho* /SPARKPLUG-BRIDGE/lib/
COPY --from=uwc_common /usr/ssl/lib/* /usr/local/lib/
COPY --from=uwc_common /usr/ssl/include/* /usr/local/include/
COPY --from=uwc_common /usr/paho-cpp/include/mqtt /usr/local/include/mqtt
COPY --from=uwc_common /usr/include/MQTT* /SPARKPLUG-BRIDGE/include/
COPY --from=uwc_common /EII/log4cpp/build/include/log4cpp /SPARKPLUG-BRIDGE/include/log4cpp
COPY --from=uwc_common /EII/log4cpp/build/lib/* /SPARKPLUG-BRIDGE/lib/
COPY --from=build-tahu /EII/tahu/client_libraries/c/lib/libtahu.so /SPARKPLUG-BRIDGE/lib/
COPY --from=build-tahu /EII/tahu/client_libraries/c/include/* /SPARKPLUG-BRIDGE/include/tahu/
COPY --from=uwc_common /uwc_util/Release/libuwc-common.so /SPARKPLUG-BRIDGE/lib/
COPY --from=uwc_common /uwc_util/include/* /SPARKPLUG-BRIDGE/include/

# build sources
RUN cd /SPARKPLUG-BRIDGE/Release \
    &&	export LD_LIBRARY_PATH='/SPARKPLUG-BRIDGE/lib:/usr/local/lib' \
    &&  export INCLUDE_DIR='/bin' \
    &&	make clean all -s

# Build sparkplug-bridge container service
FROM ia_common:$EII_VERSION as sparkplug-bridge-service

WORKDIR /
# creating base directories
ENV APP_WORK_DIR /opt/intel/app/
RUN mkdir -p /opt/intel/app \
	&& mkdir -p /opt/intel/app/logs \
	&& mkdir -p /opt/intel/config \
	&& mkdir -p /opt/intel/eii/uwc_data

WORKDIR ${APP_WORK_DIR}

# copy required dependencies
COPY --from=sparkplug-bridge /SPARKPLUG-BRIDGE/Release/SPARKPLUGBRIDGE ${APP_WORK_DIR}
COPY --from=sparkplug-bridge /SPARKPLUG-BRIDGE/lib/* ${APP_WORK_DIR}
COPY --from=uwc_common /usr/yaml/lib/*.so.* ${APP_WORK_DIR}
COPY --from=uwc_common /usr/lib/libpaho* ${APP_WORK_DIR}
COPY --from=uwc_common /usr/paho-cpp/lib/lib* ${APP_WORK_DIR}
COPY --from=uwc_common /EII/log4cpp/build/lib/* ${APP_WORK_DIR}
COPY --from=uwc_common /usr/ssl/lib/* /usr/local/lib/
COPY --from=uwc_common /usr/ssl/include/* /usr/local/include/
COPY --from=build-tahu /EII/tahu/client_libraries/c/lib/libtahu.so ${APP_WORK_DIR}/lib/
COPY --from=build-tahu /EII/tahu/client_libraries/c/include/* ${APP_WORK_DIR}/include/tahu/
COPY --from=sparkplug-bridge /SPARKPLUG-BRIDGE/Config/log4cpp.properties /opt/intel/config
COPY --from=uwc_common /uwc_util/Release/libuwc-common.so ${APP_WORK_DIR}

# set permissions to working dir for eiiuser
RUN chown -R ${EII_UID}:${EII_UID} /opt/intel/config \
   && chown -R ${EII_UID}:${EII_UID} ${APP_WORK_DIR} \
   && chmod -R 770 ${APP_WORK_DIR}

#set the environment variable
ENV LD_LIBRARY_PATH ${APP_WORK_DIR}:/usr/local/lib

# Run the container
CMD ./SPARKPLUGBRIDGE
