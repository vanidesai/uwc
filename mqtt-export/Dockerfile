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
FROM ia_eisbase:$EIS_VERSION as eisbase
LABEL description="MQTT Export Application"
ARG EIS_UID

FROM ia_common:$EIS_VERSION as common

# Build Paho C Library
FROM eisbase as build-pahoc
RUN mkdir -p /usr/paho-c
RUN git clone --branch v1.3.1 https://github.com/eclipse/paho.mqtt.c.git \
    &&	cd paho.mqtt.c \
    &&	mkdir build && cd build \
    &&	cmake -DPAHO_WITH_SSL=FALSE -DPAHO_BUILD_STATIC:BOOL=ON \
	-DCMAKE_INSTALL_PREFIX=/usr/paho-c -DBUILD_TESTS=OFF .. \
    &&	make -s \
    &&	make install


# Build Paho CPP Library
FROM eisbase as build-pahocpp
COPY --from=build-pahoc /usr/paho-c /usr/.
RUN git clone --branch v1.1 https://github.com/eclipse/paho.mqtt.cpp.git \
    && cd paho.mqtt.cpp \
    && mkdir build && cd build \
    && cmake -DPAHO_WITH_SSL=FALSE -DPAHO_MQTT_C_PATH=/usr/paho-c \
	-DCMAKE_INSTALL_PREFIX=/usr/paho-cpp  -DPAHO_BUILD_SAMPLES:BOOL=OFF \
	-DBUILD_TESTS=OFF  -DPAHO_BUILD_SHARED:BOOL=ON  .. \
    && make -s \
    && make install


# Build MQTT-Export application
FROM eisbase as mqtt-export
WORKDIR /
COPY --from=common /usr/local/include /usr/local/include
COPY --from=common /usr/local/lib /usr/local/lib
COPY --from=common /usr/local/include/safe_* /usr/local/include/
COPY MQTT-Export-App /MQTT-Export-App/. 
COPY --from=build-pahoc /usr/paho-c/lib/lib* /MQTT-Export-App/lib/
COPY --from=build-pahocpp /usr/paho-cpp/lib/lib* /MQTT-Export-App/lib/
COPY --from=build-pahocpp /usr/paho-cpp/include/mqtt /usr/local/include/mqtt
COPY --from=build-pahoc /usr/paho-c/include/MQTT* /MQTT-Export-App/include/
RUN cd /MQTT-Export-App/Release \
    &&	export LD_LIBRARY_PATH='/MQTT-Export-App/lib:/usr/local/lib' \
    &&  export INCLUDE_DIR='/bin' \
    &&	make clean all -s


# Build mqtt export container service
FROM eisbase as mqtt-export-service
COPY --from=common /usr/local/include /usr/local/include
COPY --from=common /usr/local/lib /usr/local/lib
WORKDIR /
ENV APP_WORK_DIR /opt/intel/app/
RUN mkdir -p /opt/intel/app
RUN mkdir -p /opt/intel/config
WORKDIR ${APP_WORK_DIR}
COPY --from=mqtt-export /MQTT-Export-App/Release/MQTT_Export ${APP_WORK_DIR}
COPY --from=build-pahoc /usr/paho-c/lib/lib* ${APP_WORK_DIR}
COPY --from=build-pahocpp /usr/paho-cpp/lib/lib* ${APP_WORK_DIR}
RUN chown -R ${EIS_UID}:${EIS_UID} ${APP_WORK_DIR}
RUN chown -R ${EIS_UID}:${EIS_UID} ${APP_WORK_DIR}
RUN chmod -R 770 ${APP_WORK_DIR}

#set the environment variable
ENV LD_LIBRARY_PATH ${APP_WORK_DIR}:/usr/local/lib
HEALTHCHECK CMD exit 0

# Run the container
CMD ./MQTT_Export
