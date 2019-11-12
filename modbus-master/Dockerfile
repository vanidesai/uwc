#********************************************************************************
# The source code contained or described herein and all documents related to 
# the source code ("Material") are owned by Intel Corporation. Title to the 
# Material remains with Intel Corporation.
#
# No license under any patent, copyright, trade secret or other intellectual
# property right is granted to or conferred upon you by disclosure or delivery of
# the Materials, either expressly, by implication, inducement, estoppel or otherwise.
#*********************************************************************************

# Build openssl Library
FROM ubuntu:18.04 as build-openssl

RUN apt-get update && apt-get install -y build-essential \
    curl \
    wget \
    perl \
    tar 

ARG OPENSSL_DIR=openssl
	
RUN mkdir -p ${OPENSSL_DIR} \
    && cd ${OPENSSL_DIR} \
    && wget https://www.openssl.org/source/openssl-1.1.1b.tar.gz \
    && tar -xzvf openssl-1.1.1b.tar.gz \
    && cd openssl-1.1.1b \
    && ./config --prefix=/usr/ssl shared -fPIC \
    && make depend \
    && make -s \
    && make install -s 

# Build Boost Library
FROM ubuntu:18.04 as build-boost
RUN apt-get update && apt-get install -y build-essential wget\
    curl \
    tar 

ENV CHECK_SUM f150be2231c13bb45206d79e0242372b

RUN wget http://download.icu-project.org/files/icu4c/64.1/icu4c-64_1-src.tgz \
	&& echo "${CHECK_SUM}  icu4c-64_1-src.tgz" | md5sum -c \
	&& tar -xzvf icu4c-64_1-src.tgz  \
	&& cd icu/source && ./configure --prefix=/usr  --x --without-test --enable-tests=no --enable-samples=no &&make -s\
	&& make install && rm -rf icu*

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
    && ./bootstrap.sh  \
    && ./b2 --without-python -sHAVE_ICU=1 -sICU_PATH=/icu/source --prefix=/usr/boost -j ${CONCURRENT_PROCESSES} link=shared runtime-link=shared install \
    && cd .. && rm -rf ${BOOST_DIR}


# Build Safe String Library
FROM ubuntu:18.04 as build-safestring

RUN apt-get update && apt-get install -y build-essential \
    git \
    cmake

RUN git clone https://github.com/intel/safestringlib.git \
    &&	cd safestringlib \
    &&	make clean all \
    &&	ar -x libsafestring.a \
    &&	gcc -shared *.o -o libsafestring.so \
    &&	mkdir /usr/safestring \
    &&  mkdir /usr/safestring/lib \
    &&  cp libsafestring.so /usr/safestring/lib \
    &&  cp -r include/ /usr/safestring

# Build Modbus Stack
FROM ubuntu:18.04 as modbus-lib

COPY --from=build-openssl /usr/ssl /usr/ssl/.
#COPY --from=build-pahoc /usr/paho-c /usr/paho-c/.
COPY --from=build-boost /usr/boost /usr/boost/.
#COPY --from=build-cpprest /usr/cpprestsdk /usr/cpprestsdk/.
#COPY --from=build-pahocpp /usr/paho-cpp /usr/paho-cpp/.
COPY --from=build-safestring /usr/safestring /usr/safestring/.
COPY --from=build-safestring /usr/safestring /bin/safestring/.

RUN apt-get update && apt-get install -y build-essential 

COPY Modbus_TCP_Stack /Modbus_TCP_Stack/.     
WORKDIR Modbus_TCP_Stack
RUN cd Release \
    && make clean all -s
	
# Build ModbusRestAPi
FROM ubuntu:18.04 as modbus-app

RUN apt-get update && apt-get install -y  build-essential 

# Copy built library from images
COPY --from=build-openssl /usr/ssl /bin/ssl/.
#COPY --from=build-pahoc /usr/paho-c /bin/paho-c/.
COPY --from=build-boost /usr/boost /bin/boost/.
#COPY --from=build-cpprest /usr/cpprestsdk /bin/cpprestsdk/.
#COPY --from=build-pahocpp /usr/paho-cpp /bin/paho-cpp/.
COPY --from=build-safestring /usr/safestring /bin/safestring/.

#RUN mv /bin/paho-c/lib64 /bin/paho-c/lib
#RUN mv /bin/cpprestsdk/lib64 /bin/cpprestsdk/lib

COPY Modbus-App /Modbus-App/. 
COPY --from=modbus-lib /Modbus_TCP_Stack/Release/libModbusTCPMasterStack.so /Modbus-App/lib
COPY --from=modbus-lib /Modbus_TCP_Stack/Inc/API.h /Modbus-App/include

#RUN cp  /bin/paho-c/lib/*.so /Modbus-App/lib
#RUN cp  /bin/paho-c/lib/*.so.* /Modbus-App/lib

#RUN cp  /bin/paho-cpp/lib/*.so /Modbus-App/lib
#RUN cp  /bin/paho-cpp/lib/*.so.* /Modbus-App/lib

RUN cp  /bin/boost/lib/*.so /Modbus-App/lib
RUN cp  /bin/boost/lib/*.so.* /Modbus-App/lib

#RUN cp  /bin/cpprestsdk/lib/*.so /Modbus-App/lib
#RUN cp  /bin/cpprestsdk/lib/*.so.* /Modbus-App/lib

RUN cp  /bin/ssl/lib/*.so /Modbus-App/lib
RUN cp  /bin/ssl/lib/*.so.* /Modbus-App/lib

RUN cp  /bin/safestring/lib/*.so /Modbus-App/lib

WORKDIR Modbus-App

COPY --from=build-boost /usr/lib/libicu* /Modbus-App/lib/

RUN cd Release \
# not effective as makefile overides
    &&	export LD_LIBRARY_PATH='/Modbus-App/lib' \
    &&  export INCLUDE_DIR='/bin' \
    &&	make clean all -s

ENV EIS_VERSION 2.0
FROM ia_eisbase:2.0 as eisbase
ARG EIS_UID
ARG EIS_USER_NAME
RUN echo $EIS_UID
RUN echo $EIS_USER_NAME
#RUN useradd -r -u ${EIS_UID} $EIS_USER_NAME

FROM ia_common:2.0 as common

# Build Modbus Server
FROM eisbase as modbus-service

#LABEL Description="This image is used to start the modbus service" Vendor="Intel" Version="0.8.6"

COPY --from=common /libs ${PY_WORK_DIR}/libs
copY --from=common /util ${PY_WORK_DIR}/util

RUN cd ./libs/EISMessageBus && \
    rm -rf build deps && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make && \
    make install && \
    cd ../

RUN mkdir -p /opt/app

#ENV RUN_USER            intelbmp
#ENV RUN_GROUP           intelbmp

#RUN addgroup -g 1111 -S ${RUN_GROUP} && adduser -u 1111 -S ${RUN_USER} -G ${RUN_GROUP} -h /opt/app -s /bin/sh ${RUN_USER}

#RUN apk --no-cache add libstdc++

COPY --from=modbus-app /Modbus-App/lib/libModbusTCPMasterStack.so /opt/app/
COPY --from=modbus-app /Modbus-App/Release/ModbusTCPMaster /opt/app/

#COPY --from=modbus-app /bin/paho-c/lib/*.so /opt/app/
#COPY --from=modbus-app /bin/paho-c/lib/*.so.* /opt/app/

#COPY --from=modbus-app /bin/paho-cpp/lib/*.so /opt/app/
#COPY --from=modbus-app /bin/paho-cpp/lib/*.so.* /opt/app/

COPY --from=modbus-app /bin/boost/lib/*.so /opt/app/
COPY --from=modbus-app /bin/boost/lib/*.so.* /opt/app/

#COPY --from=modbus-app /bin/cpprestsdk/lib/*.so /opt/app/
#COPY --from=modbus-app /bin/cpprestsdk/lib/*.so.* /opt/app/

COPY --from=modbus-app /bin/ssl/lib/*.so /opt/app/
COPY --from=modbus-app /bin/ssl/lib/*.so.* /opt/app/

COPY --from=modbus-app /bin/safestring/lib/*.so /opt/app/
COPY --from=build-boost /usr/lib/libicu*  /opt/app/

RUN chown -R $EIS_UID:$EIS_USER_NAME /opt/app
RUN chmod -R 770 /opt/app
#USER $EIS_UID

# Set the working directory to /
WORKDIR /opt/app

RUN export DOCKER_CONTENT_TRUST=1

#set the environment variable
ENV LD_LIBRARY_PATH ./

ENV DOCKER_CONTENT_TRUST 1

HEALTHCHECK CMD exit 0

# Run Modbus container
CMD ./ModbusTCPMaster

# Make port 9090 available to other container
EXPOSE 9090

