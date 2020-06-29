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
LABEL description="MQTT Export Application"

# Build MQTT-Export application
FROM ia_common:$EIS_VERSION as mqtt-export
WORKDIR /

# copy required dependencies
COPY MQTT-Export-App /MQTT-Export-App/. 
COPY --from=uwc_common /usr/yaml/include/yaml-cpp /usr/local/include/yaml-cpp
COPY --from=uwc_common /usr/yaml/lib/* /usr/local/lib/
COPY --from=uwc_common /usr/paho-cpp/lib/lib* /MQTT-Export-App/lib/
COPY --from=uwc_common /usr/lib/libpaho* /MQTT-Export-App/lib/
COPY --from=uwc_common /usr/ssl/lib/* /usr/local/lib/
COPY --from=uwc_common /usr/ssl/include/* /usr/local/include/
COPY --from=uwc_common /usr/paho-cpp/include/mqtt /usr/local/include/mqtt
COPY --from=uwc_common /usr/include/MQTT* /MQTT-Export-App/include/
COPY --from=uwc_common /EIS/log4cpp/build/include/log4cpp /MQTT-Export-App/include/log4cpp
COPY --from=uwc_common /EIS/log4cpp/build/lib/* /MQTT-Export-App/lib/

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
# build sources
RUN cd /MQTT-Export-App/Release \
    &&	export LD_LIBRARY_PATH='/MQTT-Export-App/lib:/usr/local/lib' \
    &&  export INCLUDE_DIR='/bin' \
    &&	/kw10/bin/kwinject make clean all -s

# Build mqtt export container service
FROM ia_common:$EIS_VERSION as mqtt-export-service

WORKDIR /
# creating base directories
ENV APP_WORK_DIR /opt/intel/app/
RUN mkdir -p /opt/intel/app \
	&& mkdir -p /opt/intel/app/logs \
	&& mkdir -p /opt/intel/config \
	&& mkdir -p /opt/intel/eis/uwc_data

WORKDIR ${APP_WORK_DIR}

# copy required dependencies
COPY --from=mqtt-export /MQTT-Export-App/Release/MQTT_Export ${APP_WORK_DIR}
COPY --from=uwc_common /usr/yaml/lib/*.so.* ${APP_WORK_DIR}
COPY --from=uwc_common /usr/lib/libpaho* ${APP_WORK_DIR}
COPY --from=uwc_common /usr/paho-cpp/lib/lib* ${APP_WORK_DIR}
COPY --from=uwc_common /usr/ssl/lib/* /usr/local/lib/
COPY --from=uwc_common /usr/ssl/include/* /usr/local/include/
COPY --from=uwc_common /EIS/log4cpp/build/lib/* ${APP_WORK_DIR}
COPY --from=mqtt-export /MQTT-Export-App/Config/log4cpp.properties /opt/intel/config

# set permissions to working dir for eisuser
RUN chown -R ${EIS_UID}:${EIS_UID} /opt/intel/config \
   && chown -R ${EIS_UID}:${EIS_UID} ${APP_WORK_DIR} \
   && chmod -R 770 ${APP_WORK_DIR}

#set the environment variable
ENV LD_LIBRARY_PATH ${APP_WORK_DIR}:/usr/local/lib

# Run the container
CMD ./MQTT_Export
