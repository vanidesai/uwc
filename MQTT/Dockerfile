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
FROM ia_eiibase:$EII_VERSION as eiibase
LABEL description="MQTT container"
ARG EII_UID
ARG EII_USER_NAME

FROM eiibase
WORKDIR /

RUN apt-get update && apt-get install -y mosquitto mosquitto-clients
RUN mkdir -p /mosquitto/config /mosquitto/data /mosquitto/log
COPY mosquitto_prod.conf /etc/mosquitto/mosquitto_prod.conf
COPY mosquitto_dev.conf /etc/mosquitto/mosquitto_dev.conf

COPY mqtt_broker_start.sh /
RUN chmod +x mqtt_broker_start.sh

CMD ["./mqtt_broker_start.sh"]
