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

COPY SoftMod_Stack /SoftMod_Stack/.
WORKDIR SoftMod_Stack
RUN cd Release \
    && make clean all -s

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

RUN cd /Modbus-App/Release \
    &&	export LD_LIBRARY_PATH='/Modbus-App/lib:/usr/local/lib\
    :/bin/yaml-cpp/lib' \
    &&  export INCLUDE_DIR='/bin' \
    &&	make clean all -s

# Build Modbus container service
FROM eisbase as modbus-service

COPY --from=common /usr/local/include /usr/local/include
COPY --from=common /usr/local/lib /usr/local/lib

WORKDIR /

ENV APP_WORK_DIR /opt/intel/app/
RUN mkdir -p /opt/intel/app
RUN mkdir -p /opt/intel/config

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
