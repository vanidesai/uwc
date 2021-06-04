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
FROM uwc_common:$EII_VERSION as uwc_common
LABEL description="KPI Tactic Application"

# Build KPI Tactic application
FROM ia_common:$EII_VERSION as kpitactic
WORKDIR /

# copy required dependencies
COPY KPIApp /KPIApp/. 
COPY --from=uwc_common /usr/yaml/include/yaml-cpp /usr/local/include/yaml-cpp
COPY --from=uwc_common /usr/yaml/lib/* /usr/local/lib/
COPY --from=uwc_common /usr/paho-cpp/lib/lib* /KPIApp/lib/
COPY --from=uwc_common /usr/lib/libpaho* /KPIApp/lib/
COPY --from=uwc_common /usr/ssl/lib/* /usr/local/lib/
COPY --from=uwc_common /usr/ssl/include/* /usr/local/include/
COPY --from=uwc_common /usr/paho-cpp/include/mqtt /usr/local/include/mqtt
COPY --from=uwc_common /usr/include/MQTT* /KPIApp/include/
COPY --from=uwc_common /EII/log4cpp/build/include/log4cpp /KPIApp/include/log4cpp
COPY --from=uwc_common /EII/log4cpp/build/lib/* /KPIApp/lib/
COPY --from=uwc_common /uwc_util/Release/libuwc-common.so /KPIApp/lib/
COPY --from=uwc_common /uwc_util/include/* /KPIApp/include/

# build sources
RUN cd /KPIApp/Release \
    &&	export LD_LIBRARY_PATH='/KPIApp/lib:/usr/local/lib' \
    &&	make clean all -s

# Build KPI App container service
FROM ia_common:$EII_VERSION as KPIApp-service

WORKDIR /
# creating base directories
ENV APP_WORK_DIR /opt/intel/app/
RUN mkdir -p /opt/intel/app \
	&& mkdir -p /opt/intel/app/logs \
	&& mkdir -p /opt/intel/config \
	&& mkdir -p /opt/intel/eii/uwc_data
WORKDIR ${APP_WORK_DIR}

# copy required dependencies
COPY --from=kpitactic /KPIApp/Release/KPIApp ${APP_WORK_DIR}
COPY --from=uwc_common /usr/yaml/lib/*.so.* ${APP_WORK_DIR}
COPY --from=uwc_common /usr/lib/libpaho* ${APP_WORK_DIR}
COPY --from=uwc_common /usr/ssl/lib/* /usr/local/lib/
COPY --from=uwc_common /usr/ssl/include/* /usr/local/include/
COPY --from=uwc_common /usr/paho-cpp/lib/lib* ${APP_WORK_DIR}
COPY --from=uwc_common /EII/log4cpp/build/lib/* ${APP_WORK_DIR}
COPY --from=kpitactic /KPIApp/Config/log4cpp.properties /opt/intel/config
COPY --from=uwc_common /uwc_util/Release/libuwc-common.so ${APP_WORK_DIR}

# set permissions to working dir for eiiuser
RUN chown -R ${EII_UID}:${EII_UID} /opt/intel/config \
   && chown -R ${EII_UID}:${EII_UID} ${APP_WORK_DIR} \
   && chmod -R 770 ${APP_WORK_DIR}

#set the environment variable
ENV LD_LIBRARY_PATH ${APP_WORK_DIR}:/usr/local/lib

# Run the container
CMD ./KPIApp
