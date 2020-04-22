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
LABEL description="Modbus TCP Master Application"

# Build Modbus Stack
FROM ia_common:$EIS_VERSION as modbus-lib
WORKDIR /

# compile modbus stack
COPY SoftMod_Stack /SoftMod_Stack/.
WORKDIR SoftMod_Stack
RUN cd Release \
    && make clean all -s

# Build Modbus application
FROM ia_common:$EIS_VERSION as modbus-app
WORKDIR /

# Copy built library from images
COPY --from=uwc_common /usr/yaml /bin/yaml-cpp/.

COPY Modbus-App /Modbus-App/. 
COPY --from=modbus-lib /SoftMod_Stack/Release/libModbusMasterStack.so /Modbus-App/lib/
COPY --from=modbus-lib /SoftMod_Stack/Inc/API.h /Modbus-App/include/
COPY --from=uwc_common /EIS/log4cpp/build/include/log4cpp /Modbus-App/include/log4cpp
COPY --from=uwc_common /EIS/log4cpp/build/lib/* /Modbus-App/lib/

# compile modbus app sources
RUN cd /Modbus-App/Release \
    &&	export LD_LIBRARY_PATH='/Modbus-App/lib:/usr/local/lib\
    :/bin/yaml-cpp/lib' \
    &&  export INCLUDE_DIR='/bin' \
    &&	make clean all -s

# Build Modbus container service
FROM ia_common:$EIS_VERSION as modbus-service
WORKDIR /

# creating base directories
ENV APP_WORK_DIR /opt/intel/app/
RUN mkdir -p /opt/intel/app \
	&& mkdir -p /opt/intel/config \
	&& mkdir -p /opt/intel/eis/uwc_data
WORKDIR ${APP_WORK_DIR}

# copy required libs and includes
COPY --from=modbus-lib /SoftMod_Stack/Release/libModbusMasterStack.so ${APP_WORK_DIR}
COPY --from=modbus-app /Modbus-App/Release/ModbusMaster ${APP_WORK_DIR}
COPY --from=uwc_common /usr/yaml/lib/*.so.* ${APP_WORK_DIR}
COPY --from=uwc_common /EIS/log4cpp/build/lib/* ${APP_WORK_DIR}
COPY --from=modbus-app /Modbus-App/Config/log4cpp.properties /opt/intel/config

# set permissions to working dir for eisuser
RUN chown -R ${EIS_UID}:${EIS_UID} /opt/intel/config \
   && chown -R ${EIS_UID}:${EIS_UID} ${APP_WORK_DIR} \
   && chmod -R 770 ${APP_WORK_DIR}

#set the environment variable
ENV LD_LIBRARY_PATH ${APP_WORK_DIR}:/usr/local/lib

# healthcheck for docker bench security
HEALTHCHECK CMD exit 0

# Run Modbus container
CMD ./ModbusMaster
