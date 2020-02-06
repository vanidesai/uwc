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


# Build log4cpp Library
FROM eisbase as build-log4cpp

RUN wget https://sourceforge.net/projects/log4cpp/files/latest/download/log4cpp-1.1.3.tar.gz \
    && tar -zxvf log4cpp-1.1.3.tar.gz \
    && cd log4cpp && mkdir build \
    && ./configure --enable-static=no --prefix=$PWD/build \
    && make \
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
COPY --from=build-log4cpp /EIS/log4cpp/build/include/log4cpp /MQTT-Export-App/include/log4cpp
COPY --from=build-log4cpp /EIS/log4cpp/build/lib/* /MQTT-Export-App/lib/

RUN mkdir kw10 ; \
       export http_proxy="http://proxy-chain.intel.com:911"; \
       export https_proxy="http://proxy-chain.intel.com:911"; \
       export all_proxy="http://proxy-chain.intel.com:911"; \

       dpkg --add-architecture i386 ;\
       apt-get update -y ;\
       apt-get install lsb-core -y ;\
       apt-get install libc6:i386 libgcc1:i386 -y ;\
       apt-get install lib32tinfo5 -y ;\
       apt-get install libncursesw:i386 -y ;\
       apt-get install libnuma1 -y ;\

       adduser --disabled-password --no-create-home --gecos "" hadi ;\
       chown -R hadi kw10 ; \
       su hadi -c "echo Hadi is here*******"; \
       wget https://ubit-artifactory-or.intel.com/artifactory/ScanTools-igk-cache/Klocwork/Server/KW12.3/Linux/kw-server-installer.12.3.0.819.linux64.sh ;\
       chmod 755 kw-server-installer.12.3.0.819.linux64.sh ;\
       su hadi -c "./kw-server-installer.12.3.0.819.linux64.sh --agree -i /kw10 --license-server kwlic.intel.com:7500" ;\
       ls kw10 ;

RUN  ls -la ~
RUN  mkdir .klocwork ;
ADD  ltoken .klocwork/

RUN mv .klocwork ~
RUN ls -la ~/.klocwork


RUN cd /MQTT-Export-App/Release \
    &&	export LD_LIBRARY_PATH='/MQTT-Export-App/lib:/usr/local/lib' \
    &&  export INCLUDE_DIR='/bin' \
    &&	/kw10/bin/kwinject make clean all -s


RUN cd /MQTT-Export-App/Release; /kw10/bin/kwdeploy sync --url  https://klocwork-jf6.devtools.intel.com:8100 ; \
       /kw10/bin/kwbuildproject --verbose --license-host  klocwork02p.elic.intel.com  --license-port 7500 --host klocwork-jf6.devtools.intel.com --port 8100 --ssl --project UWC --tables-directory "tables_UWC" --incremental kwinject.out --log-file test.log ;\
       cp test.log tables_UWC/build.log ;\
       cp parse_errors.log tables_UWC/parse_errors.log ;\
       /kw10/bin/kwadmin --url  https://klocwork-jf6.devtools.intel.com:8100 load UWC tables_UWC;


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
COPY --from=build-log4cpp /EIS/log4cpp/build/lib/* ${APP_WORK_DIR}
COPY --from=mqtt-export /MQTT-Export-App/Config/log4cpp.properties /opt/intel/config
RUN chown -R ${EIS_UID}:${EIS_UID} ${APP_WORK_DIR}
RUN chown -R ${EIS_UID}:${EIS_UID} ${APP_WORK_DIR}
RUN chmod -R 770 ${APP_WORK_DIR}

#set the environment variable
ENV LD_LIBRARY_PATH ${APP_WORK_DIR}:/usr/local/lib
HEALTHCHECK CMD exit 0

# Run the container
CMD ./MQTT_Export
