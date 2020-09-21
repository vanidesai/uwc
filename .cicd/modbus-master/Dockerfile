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
LABEL description="Modbus TCP Master Application"
ARG EIS_UID

FROM ia_common:$EIS_VERSION as common

# Build log4cpp Library
FROM eisbase as build-log4cpp

RUN wget https://sourceforge.net/projects/log4cpp/files/latest/download/log4cpp-1.1.3.tar.gz \
    && tar -zxvf log4cpp-1.1.3.tar.gz \
    && cd log4cpp && mkdir build \
    && ./configure --enable-static=no --prefix=$PWD/build \
    && make \
    && make install

# yaml build
FROM eisbase as build-yaml

RUN git clone --branch yaml-cpp-0.6.3 https://github.com/jbeder/yaml-cpp.git \
    && cd yaml-cpp \
    &&	mkdir build && cd build \
    &&	cmake -DYAML_BUILD_SHARED_LIBS=ON ../ \
    &&	make yaml-cpp \
    &&	mkdir -p /usr/yaml \
    &&  mkdir -p /usr/yaml/lib \
    &&  cp lib* /usr/yaml/lib \
    &&  cp -r ../include /usr/yaml \
    && cd ../../

# Build Modbus Stack
FROM eisbase as modbus-lib

WORKDIR /

COPY --from=common /usr/local/include/safe_* /usr/local/include/
COPY --from=common /usr/local/lib/libsafestring.so /usr/local/lib/

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
       wget https://ubit-artifactory-or.intel.com/artifactory/ScanTools-igk-cache/Klocwork/Server/KW20.1/Linux/kw-server-installer.20.1.0.97.linux64.sh ;\
       chmod 755 kw-server-installer.20.1.0.97.linux64.sh ;\
       su hadi -c "./kw-server-installer.20.1.0.97.linux64.sh --agree -i /kw10 --license-server klocwork03p.elic.intel.com:7500" ;\
       ls kw10 ;

RUN  ls -la ~
RUN  mkdir .klocwork ;
RUN  pwd ;
RUN  ls -la ;
ADD  ltoken .klocwork/

RUN mv .klocwork ~
RUN ls -la ~/.klocwork

COPY SoftMod_Stack /SoftMod_Stack/.
WORKDIR SoftMod_Stack
RUN cd Release ; make clean ;\
     /kw10/bin/kwinject --output kwinject.out make all -s;

RUN ls -la Release

       #/kw10/bin/kwauth --url https://klocwork-jf6.devtools.intel.com:8100 ;\
RUN cd Release; /kw10/bin/kwdeploy sync --url  https://klocwork-jf6.devtools.intel.com:8100 ; \
      /kw10/bin/kwdeploy install INTEL.BANNED.FUNCTION_MEMSET --url https://klocwork-jf6.devtools.intel.com:8100; \  
       /kw10/bin/kwbuildproject --verbose --license-host klocwork03p.elic.intel.com --license-port 7500 --host klocwork-jf6.devtools.intel.com --port 8100 --ssl --project UWC --tables-directory "tables_UWC" --incremental kwinject.out ;\
       /kw10/bin/kwadmin --url  https://klocwork-jf6.devtools.intel.com:8100 load UWC tables_UWC;

# Build Modbus application
FROM eisbase as modbus-app

WORKDIR /

COPY --from=common /usr/local/include /usr/local/include
COPY --from=common /usr/local/lib /usr/local/lib

# Copy built library from images
COPY --from=build-yaml /usr/yaml /bin/yaml-cpp/.

COPY Modbus-App /Modbus-App/. 
COPY --from=modbus-lib /SoftMod_Stack/Release/libModbusMasterStack.so /Modbus-App/lib/
COPY --from=modbus-lib /SoftMod_Stack/Inc/API.h /Modbus-App/include/
COPY --from=build-log4cpp /EIS/log4cpp/build/include/log4cpp /Modbus-App/include/log4cpp
COPY --from=build-log4cpp /EIS/log4cpp/build/lib/* /Modbus-App/lib/

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
       wget https://ubit-artifactory-or.intel.com/artifactory/ScanTools-igk-cache/Klocwork/Server/KW20.1/Linux/kw-server-installer.20.1.0.97.linux64.sh ;\
       chmod 755 kw-server-installer.20.1.0.97.linux64.sh ;\
       su hadi -c "./kw-server-installer.20.1.0.97.linux64.sh --agree -i /kw10 --license-server klocwork03p.elic.intel.com:7500" ;\
       ls kw10 ;

RUN  ls -la ~
RUN  mkdir .klocwork ;
ADD  ltoken .klocwork/

RUN mv .klocwork ~
RUN ls -la ~/.klocwork


RUN cd /Modbus-App/Release \
    &&	export LD_LIBRARY_PATH='/Modbus-App/lib:/usr/local/lib\
    :/bin/yaml-cpp/lib' \
    &&  export INCLUDE_DIR='/bin' \
    &&	/kw10/bin/kwinject make clean all -s

       #/kw10/bin/kwauth --url https://klocwork-jf6.devtools.intel.com:8100 ;\
RUN cd /Modbus-App/Release; /kw10/bin/kwdeploy sync --url  https://klocwork-jf6.devtools.intel.com:8100 ; \
	/kw10/bin/kwdeploy install INTEL.BANNED.FUNCTION_MEMSET --url https://klocwork-jf6.devtools.intel.com:8100; \
       /kw10/bin/kwbuildproject --verbose --license-host  klocwork03p.elic.intel.com  --license-port 7500 --host klocwork-jf6.devtools.intel.com --port 8100 --ssl --project UWC --tables-directory "tables_UWC" --incremental kwinject.out ;\
       /kw10/bin/kwadmin --url  https://klocwork-jf6.devtools.intel.com:8100 load UWC tables_UWC;

# Build Modbus container service
FROM eisbase as modbus-service

COPY --from=common /usr/local/include /usr/local/include
COPY --from=common /usr/local/lib /usr/local/lib

WORKDIR /

ENV APP_WORK_DIR /opt/intel/app/
RUN mkdir -p /opt/intel/app
RUN mkdir -p /opt/intel/config
RUN mkdir -p /opt/intel/eis/uwc_data

WORKDIR ${APP_WORK_DIR}

COPY --from=modbus-lib /SoftMod_Stack/Release/libModbusMasterStack.so ${APP_WORK_DIR}
COPY --from=modbus-app /Modbus-App/Release/ModbusMaster ${APP_WORK_DIR}
COPY --from=build-yaml /usr/yaml/lib/*.so.* ${APP_WORK_DIR}
COPY --from=build-log4cpp /EIS/log4cpp/build/lib/* ${APP_WORK_DIR}
COPY --from=modbus-app /Modbus-App/Config/log4cpp.properties /opt/intel/config

RUN chown -R ${EIS_UID}:${EIS_UID} /opt/intel/config
RUN chown -R ${EIS_UID}:${EIS_UID} ${APP_WORK_DIR}
RUN chmod -R 770 ${APP_WORK_DIR}

#set the environment variable
ENV LD_LIBRARY_PATH ${APP_WORK_DIR}:/usr/local/lib

ENV DOCKER_CONTENT_TRUST 1
HEALTHCHECK CMD exit 0

# Run Modbus container
CMD ./ModbusMaster
