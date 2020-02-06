/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#ifndef MQTTCALLBACK_HPP_
#define MQTTCALLBACK_HPP_

#include "mqtt/async_client.h"
#include "mqtt/client.h"

/**
 * A callback class for use with the main MQTT client.
 */
class CMQTTCallback : public virtual mqtt::callback
{
public:
	void connection_lost(const std::string& cause) override;
	void connected(const std::string& cause) override;
	void message_arrived(mqtt::const_message_ptr msg) override;
};

/**
 * A base action listener.
 */
class CMQTTActionListener : public virtual mqtt::iaction_listener
{
protected:
	void on_failure(const mqtt::token& tok) override;
	void on_success(const mqtt::token& tok) override;
};

// Class to receive sync callbacks
class CSyncCallback : public virtual mqtt::callback
{
public:
	void connection_lost(const std::string& cause) override;
	void delivery_complete(mqtt::delivery_token_ptr tok) override;
	void connected(const std::string& cause) override;
};

#endif
