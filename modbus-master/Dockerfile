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
ARG EIS_USER_NAME

# Build Boost Library
FROM eisbase as build-boost
RUN apt-get update && apt-get install -y \
    curl \
    tar 

ARG BOOST_VERSION=1.71.0
ARG BOOST_CHECKSUM=96b34f7468f26a141f6020efb813f1a2f3dfb9797ecf76a7d7cbd843cc95f5bd
ARG BOOST_DIR=boost
ARG CONCURRENT_PROCESSES=1
ENV BOOST_VERSION ${BOOST_VERSION}

RUN mkdir -p ${BOOST_DIR} \
    && cd ${BOOST_DIR} \
    && curl -L --retry 3 "https://dl.bintray.com/boostorg/release/${BOOST_VERSION}/source/boost_1_71_0.tar.gz" -o boost.tar.gz \
    && echo "${BOOST_CHECKSUM}  boost.tar.gz" | sha256sum -c \
    && tar -xzf boost.tar.gz --strip 1 \
    && ./bootstrap.sh  --prefix=/usr/boost --without-icu --with-libraries=filesystem,log,regex,system,thread,date_time  \
    && ./b2 link=shared runtime-link=shared --no-cmake-config install \
    && cd .. && rm -rf ${BOOST_DIR}


# Build Safe String Library
FROM eisbase as build-safestring

RUN git clone https://github.com/intel/safestringlib.git \
    &&	cd safestringlib \
    &&	make clean all \
    &&	ar -x libsafestring.a \
    &&	gcc -shared *.o -o libsafestring.so \
    &&	mkdir /usr/safestring \
    &&  mkdir /usr/safestring/lib \
    &&  cp libsafestring.so /usr/safestring/lib \
    &&  cp -r include/ /usr/safestring

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

COPY --from=build-safestring /usr/safestring /usr/safestring/.
COPY --from=build-safestring /usr/safestring /bin/safestring/.

COPY SoftMod_Stack /SoftMod_Stack/.     
WORKDIR SoftMod_Stack
RUN cd Release \
    && make clean all -s

FROM ia_common:$EIS_VERSION as common

# Build Modbus application
FROM eisbase as modbus-app

WORKDIR /

COPY --from=common /libs /libs
COPY --from=common /util /util

RUN cd ./libs/EISMessageBus && \
    rm -rf build deps && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make && \
    make install && \
    cd /

# Copy built library from images
COPY --from=build-boost /usr/boost /bin/boost/.
COPY --from=build-safestring /usr/safestring /bin/safestring/.
COPY --from=build-yaml /usr/yaml /bin/yaml-cpp/.

COPY Modbus-App /Modbus-App/. 
COPY --from=modbus-lib /SoftMod_Stack/Release/libModbusTCPMasterStack.so /Modbus-App/lib/
COPY --from=modbus-lib /SoftMod_Stack/Inc/API.h /Modbus-App/include/

RUN cd /Modbus-App/Release \
# not effective as makefile overides
    &&	export LD_LIBRARY_PATH='/Modbus-App/lib:\
    /bin/boost/lib:/bin/safestring/lib':/bin/yaml-cpp/lib \
    &&  export INCLUDE_DIR='/bin' \
    &&	make clean all -s

# Build Modbus container service
FROM eisbase as modbus-service

WORKDIR /
ENV APP_WORK_DIR /opt/app/
RUN mkdir -p /opt/app
RUN mkdir -p /opt/config

COPY --from=common /libs /libs
COPY --from=common /util /util

RUN cd ./libs/EISMessageBus && \
    rm -rf build deps && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make && \
    make install && \
    cd ${APP_WORK_DIR}

ARG EIS_USER_NAME
RUN adduser --quiet --disabled-password ${EIS_USER_NAME}

WORKDIR ${APP_WORK_DIR}

COPY --from=modbus-lib /SoftMod_Stack/Release/libModbusTCPMasterStack.so ${APP_WORK_DIR}
COPY --from=modbus-app /Modbus-App/Release/ModbusTCPMaster ${APP_WORK_DIR}
COPY --from=modbus-app /bin/boost/lib/*.so.*  ${APP_WORK_DIR}
COPY --from=build-yaml /usr/yaml/lib/*.so.* ${APP_WORK_DIR}
COPY --from=build-safestring /usr/safestring/lib/* ${APP_WORK_DIR}

RUN chown -R ${EIS_USER_NAME}:${EIS_USER_NAME} /opt/config
RUN chown -R ${EIS_USER_NAME}:${EIS_USER_NAME} ${APP_WORK_DIR}
RUN chmod -R 770 ${APP_WORK_DIR}

#set the environment variable
ENV LD_LIBRARY_PATH ${APP_WORK_DIR}:/usr/local/lib

ENV DOCKER_CONTENT_TRUST 1
HEALTHCHECK CMD exit 0

# Run Modbus container
CMD ./ModbusTCPMaster
