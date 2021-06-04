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
FROM ia_eiibase:$EII_VERSION as eiibase
LABEL description="MQTT Export Application"
ARG EII_UID

FROM ia_common:$EII_VERSION as common

# Build Paho C Library
FROM eiibase as build-pahoc
RUN mkdir -p /usr/paho-c
RUN git clone --branch v1.3.1 https://github.com/eclipse/paho.mqtt.c.git \
  && cd paho.mqtt.c \
  && mkdir build && cd build \
  && cmake -DPAHO_WITH_SSL=FALSE -DPAHO_BUILD_STATIC:BOOL=ON \
  -DCMAKE_INSTALL_PREFIX=/usr/paho-c -DBUILD_TESTS=OFF .. \
  && make -s \
  && make install


# Build Paho CPP Library
FROM eiibase as build-pahocpp
COPY --from=build-pahoc /usr/paho-c /usr/.
RUN git clone --branch v1.1 https://github.com/eclipse/paho.mqtt.cpp.git \
  && cd paho.mqtt.cpp \
  && mkdir build && cd build \
  && cmake -DPAHO_WITH_SSL=FALSE -DPAHO_MQTT_C_PATH=/usr/paho-c \
  -DCMAKE_INSTALL_PREFIX=/usr/paho-cpp  -DPAHO_BUILD_SAMPLES:BOOL=OFF \
  -DBUILD_TESTS=OFF  -DPAHO_BUILD_SHARED:BOOL=ON  .. \
  && make -s \
  && make install


# Build log4cpp Library
FROM eiibase as build-log4cpp

RUN wget https://sourceforge.net/projects/log4cpp/files/latest/download/log4cpp-1.1.3.tar.gz \
  && tar -zxvf log4cpp-1.1.3.tar.gz \
  && cd log4cpp && mkdir build \
  && ./configure --enable-static=no --prefix=$PWD/build \
  && make \
  && make install


# Build MQTT-Export application
FROM eiibase as mqtt-export
WORKDIR /
COPY --from=common /usr/local/include /usr/local/include
COPY --from=common /usr/local/lib /usr/local/lib
COPY --from=common /usr/local/include/safe_* /usr/local/include/
COPY MQTT-Export-App /MQTT-Export-App/. 
COPY --from=build-pahoc /usr/paho-c/lib/lib* /MQTT-Export-App/lib/
COPY --from=build-pahocpp /usr/paho-cpp/lib/lib* /MQTT-Export-App/lib/
COPY --from=build-pahocpp /usr/paho-cpp/include/mqtt /usr/local/include/mqtt
COPY --from=build-pahoc /usr/paho-c/include/MQTT* /MQTT-Export-App/include/
COPY --from=build-log4cpp /EII/log4cpp/build/include/log4cpp /MQTT-Export-App/include/log4cpp
COPY --from=build-log4cpp /EII/log4cpp/build/lib/* /MQTT-Export-App/lib/

# begin kw ######################
RUN dpkg --add-architecture i386 \
  && apt-get update -y \
  && apt-get install -y lsb-core \
  libc6:i386 \
  libgcc1:i386 \
  lib32tinfo5 \
  libncursesw5:i386 \
  libnuma1

RUN mkdir /klocwork \
  && adduser --disabled-password --no-create-home --gecos "" klocwork \
  && chown -R klocwork /klocwork \
  && wget https://ubit-artifactory-or.intel.com/artifactory/ScanTools-igk-cache/Klocwork/Server/KW12.3/Linux/kw-server-installer.12.3.0.819.linux64.sh \
  && chmod 755 kw-server-installer.12.3.0.819.linux64.sh \
  && su klocwork -c "./kw-server-installer.12.3.0.819.linux64.sh --agree -i /klocwork --license-server kwlic.intel.com:7500"

RUN cd /MQTT-Export-App/Release \
  && export LD_LIBRARY_PATH='/MQTT-Export-App/lib:/usr/local/lib' \
  && export INCLUDE_DIR='/bin' \
  && /klocwork/bin/kwinject --output /tmp/kwinject.out make clean all -s

ARG KW_SERVER="https://klocwork-jf6.devtools.intel.com:8100"
ARG KW_TABLES="/tmp/tables_UWC"
ARG GIT_BRANCH=master
ARG BUILD_NUMBER=-1

# this file is written by Jenkins, do not commit credentials to Git
COPY kwcreds /tmp/kwcreds

RUN /klocwork/bin/kwauth --url "${KW_SERVER}" < /tmp/kwcreds \
  && /klocwork/bin/kwdeploy sync --url ${KW_SERVER} \
  && /klocwork/bin/kwbuildproject --verbose --url "${KW_SERVER}/UWC" --tables-directory "${KW_TABLES}" --incremental /tmp/kwinject.out --log-file "${KW_TABLES}/build.log" \
  && /klocwork/bin/kwadmin --url "${KW_SERVER}" load UWC "${KW_TABLES}" --name KWReport_MQTT-Export_${GIT_BRANCH}_${BUILD_NUMBER}
# end kw ######################

# Build mqtt export container service
FROM eiibase as mqtt-export-service
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
COPY --from=build-log4cpp /EII/log4cpp/build/lib/* ${APP_WORK_DIR}
COPY --from=mqtt-export /MQTT-Export-App/Config/log4cpp.properties /opt/intel/config
RUN chown -R ${EII_UID}:${EII_UID} ${APP_WORK_DIR}
RUN chown -R ${EII_UID}:${EII_UID} ${APP_WORK_DIR}
RUN chmod -R 770 ${APP_WORK_DIR}

#set the environment variable
ENV LD_LIBRARY_PATH ${APP_WORK_DIR}:/usr/local/lib
HEALTHCHECK CMD exit 0

# Run the container
CMD ./MQTT_Export
