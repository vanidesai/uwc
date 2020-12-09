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
FROM ia_eisbase:$EIS_VERSION as eisbase
LABEL description="SCADA RTU container"

# Build tahu library
FROM eisbase as build-tahu
RUN apt-get update && apt-get install -y libmosquitto-dev \
    &&  git clone --branch develop https://github.com/eclipse/tahu \
    && cd tahu \ 
    && git checkout 9bf91b69f5e9214e7ec765f2ad20acc7bc41236d \
    &&  cd client_libraries/c \
    && sed -i 's/#define SPARKPLUG_DEBUG 1/\/\/#define SPARKPLUG_DEBUG 0/g' include/tahu.h \
    &&	make -s 

# Build SCADA-RTU application
FROM ia_common:$EIS_VERSION as scada-rtu
WORKDIR /

# copy required dependencies
COPY SCADA-RTU-APP /SCADA-RTU-APP/. 
COPY --from=uwc_common /usr/yaml/include/yaml-cpp /usr/local/include/yaml-cpp
COPY --from=uwc_common /usr/yaml/lib/* /usr/local/lib/
COPY --from=uwc_common /usr/paho-cpp/lib/lib* /SCADA-RTU-APP/lib/
COPY --from=uwc_common /usr/lib/libpaho* /SCADA-RTU-APP/lib/
COPY --from=uwc_common /usr/ssl/lib/* /usr/local/lib/
COPY --from=uwc_common /usr/ssl/include/* /usr/local/include/
COPY --from=uwc_common /usr/paho-cpp/include/mqtt /usr/local/include/mqtt
COPY --from=uwc_common /usr/include/MQTT* /SCADA-RTU-APP/include/
COPY --from=uwc_common /EIS/log4cpp/build/include/log4cpp /SCADA-RTU-APP/include/log4cpp
COPY --from=uwc_common /EIS/log4cpp/build/lib/* /SCADA-RTU-APP/lib/
COPY --from=build-tahu /EIS/tahu/client_libraries/c/lib/libtahu.so /SCADA-RTU-APP/lib/
COPY --from=build-tahu /EIS/tahu/client_libraries/c/include/* /SCADA-RTU-APP/include/tahu/
COPY --from=uwc_common /uwc_util/Release/libuwc-common.so /SCADA-RTU-APP/lib/
COPY --from=uwc_common /uwc_util/include/* /SCADA-RTU-APP/include/


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
       wget --no-check-certificate https://ubit-artifactory-or.intel.com/artifactory/ScanTools-igk-cache/Klocwork/Server/KW20.1/Linux/kw-server-installer.20.1.0.97.linux64.sh ;\
       sleep 60 ;\
       chmod 755 kw-server-installer.20.1.0.97.linux64.sh ;\
       su hadi -c "./kw-server-installer.20.1.0.97.linux64.sh --agree -i /kw10 --license-server klocwork03p.elic.intel.com:7500" ;\
       ls kw10 ;

RUN  ls -la ~
RUN  mkdir .klocwork ;
ADD  ltoken .klocwork/

RUN mv .klocwork ~
RUN ls -la ~/.klocwork

# build sources
RUN cd /SCADA-RTU-APP/Release \
    &&	export LD_LIBRARY_PATH='/SCADA-RTU-APP/lib:/usr/local/lib' \
    &&  export INCLUDE_DIR='/bin' \
    &&	/kw10/bin/kwinject make clean all -s

# Build scada-rtu container service
FROM ia_common:$EIS_VERSION as scada-rtu-service

WORKDIR /
# creating base directories
ENV APP_WORK_DIR /opt/intel/app/
RUN mkdir -p /opt/intel/app \
	&& mkdir -p /opt/intel/app/logs \
	&& mkdir -p /opt/intel/config \
	&& mkdir -p /opt/intel/eis/uwc_data

RUN cd /SCADA-RTU-APP/Release; /kw10/bin/kwdeploy sync --url  https://klocwork-jf6.devtools.intel.com:8100 ; \
       /kw10/bin/kwbuildproject --verbose --license-host  klocwork03p.elic.intel.com  --license-port 7500 --host klocwork-jf6.devtools.intel.com --port 8100 --ssl --project UWC --tables-directory "tables_UWC" --incremental kwinject.out ;\
       /kw10/bin/kwadmin --url  https://klocwork-jf6.devtools.intel.com:8100 load UWC tables_UWC;
       
WORKDIR ${APP_WORK_DIR}

# copy required dependencies
COPY --from=scada-rtu /SCADA-RTU-APP/Release/SCADARTU ${APP_WORK_DIR}
COPY --from=scada-rtu /SCADA-RTU-APP/lib/* ${APP_WORK_DIR}
COPY --from=uwc_common /usr/yaml/lib/*.so.* ${APP_WORK_DIR}
COPY --from=uwc_common /usr/lib/libpaho* ${APP_WORK_DIR}
COPY --from=uwc_common /usr/paho-cpp/lib/lib* ${APP_WORK_DIR}
COPY --from=uwc_common /EIS/log4cpp/build/lib/* ${APP_WORK_DIR}
COPY --from=uwc_common /usr/ssl/lib/* /usr/local/lib/
COPY --from=uwc_common /usr/ssl/include/* /usr/local/include/
COPY --from=build-tahu /EIS/tahu/client_libraries/c/lib/libtahu.so ${APP_WORK_DIR}/lib/
COPY --from=build-tahu /EIS/tahu/client_libraries/c/include/* ${APP_WORK_DIR}/include/tahu/
COPY --from=scada-rtu /SCADA-RTU-APP/Config/log4cpp.properties /opt/intel/config
COPY --from=uwc_common /uwc_util/Release/libuwc-common.so ${APP_WORK_DIR}

# set permissions to working dir for eisuser
RUN chown -R ${EIS_UID}:${EIS_UID} /opt/intel/config \
   && chown -R ${EIS_UID}:${EIS_UID} ${APP_WORK_DIR} \
   && chmod -R 770 ${APP_WORK_DIR}

#set the environment variable
ENV LD_LIBRARY_PATH ${APP_WORK_DIR}:/usr/local/lib

# Run the container
CMD ./SCADARTU
