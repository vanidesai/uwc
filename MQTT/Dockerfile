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
LABEL description="MQTT container"
ARG EIS_UID
ARG EIS_USER_NAME

FROM eisbase
RUN apt-get update && apt-get install -y mosquitto
RUN mkdir -p /mosquitto/config /mosquitto/data /mosquitto/log
COPY mosquitto.conf /etc/mosquitto/mosquitto.conf

HEALTHCHECK CMD exit 0

EXPOSE 1883

CMD ["mosquitto", "-c", "/etc/mosquitto/mosquitto.conf"]
