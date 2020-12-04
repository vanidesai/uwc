/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

/**
 * File contains the class CMQTTPublishHandler that manages instance that handles Publish message on MQTT broker
 */

#ifndef MQTT_PUBLISH_HANDLER_HPP_
#define MQTT_PUBLISH_HANDLER_HPP_

#include "MQTTPubSubClient.hpp"

/**
 * CMQTTPublishHandler class manages instance that handles Publish message on MQTT broker
 */
class CMQTTPublishHandler : public CMQTTBaseHandler
{
public:
	CMQTTPublishHandler(std::string strPlBusUrl, std::string strClientID, int iQOS);
	~CMQTTPublishHandler();

	bool createNPubMsg(std::string &a_sMsg, std::string &a_sTopic);
};

#endif
