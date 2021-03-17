/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

/*** MQTTPubSubClient.hpp is used to handle operations like publishing, subscribing, connection etc to mqtt broker */

#ifndef MQTTPUBSUBCLIENT_HPP_
#define MQTTPUBSUBCLIENT_HPP_

#include "mqtt/async_client.h"
#include "mqtt/will_options.h"
#include <mutex>

/** class is for action failure or success related to mqtt*/
class action_listener : public virtual mqtt::iaction_listener
{
	std::string name_;/** name of listener*/

	void on_failure(const mqtt::token& tok) override;
	void on_success(const mqtt::token& tok) override;

public:
	action_listener(const std::string& name) : name_(name) {}
};

/** class holds information regarding mqtt connection on success and on connection failure, message received or not*/
class CMQTTPubSubClient : public virtual mqtt::callback,
					public virtual mqtt::iaction_listener

{
	int m_iQOS; /** QoS value to be used for publishing*/
	std::string m_sClientID; /** client id to be used for mqtt connection*/
	mqtt::async_client m_Client; /** mqtt async client*/
	mqtt::connect_options m_ConOptions; /** mqtt async client connection options*/
	/** An action listener to display the result of actions.*/
	action_listener m_Listener;

	/** Callback functions for various operations*/
	bool m_bNotifyConnection = false; 
	mqtt::async_client::connection_handler m_fcbConnected;

	bool m_bNotifyDisConnection = false;
	mqtt::async_client::connection_handler m_fcbDisconnected;

	bool m_bNotifyMsgRcvd = false;
	mqtt::async_client::message_handler m_fcbMsgRcvd;

	/** Re-connection failure */
	void on_failure(const mqtt::token& tok) override;

	/** (Re)connection success */
	/** Either this or connected() can be used for callbacks. */
	void on_success(const mqtt::token& tok) override; 
	
	/** (Re)connection success */
	void connected(const std::string& cause) override;

	/** Callback for when the connection is lost.*/
	/** This will initiate the attempt to manually reconnect. */
	void connection_lost(const std::string& cause) override;

	/** Callback for when a message arrives. */
	void message_arrived(mqtt::const_message_ptr msg) override;

	void delivery_complete(mqtt::delivery_token_ptr token) override {}

public:
	/** constructor*/
	CMQTTPubSubClient(const std::string &a_sBrokerURL, std::string a_sClientID, 
		int a_iQOS, 
		bool a_bIsTLS, std::string a_sCATrustStoreSecret, 
		std::string a_sClientPvtKeySecret, std::string a_sClientCertSecret, 
		std::string a_sListener = "Subscription");

	bool publishMsg(mqtt::message_ptr &a_pubMsg);

	bool setWillMsg(const mqtt::will_options & will)
	{
		m_ConOptions.set_will(will);

		return true;
	}

	bool connect();
	bool disconnect();

	bool isConnected()
	{
		return m_Client.is_connected();
	}

	void subscribe(const std::string &a_sTopic);

	/** Function to set notification for connection*/
	void setNotificationConnect(mqtt::async_client::connection_handler a_fcbConnected)
	{
		m_fcbConnected = a_fcbConnected;
		m_bNotifyConnection = true;
	}

	/** Function to set on desconnection*/
	void setNotificationDisConnect(mqtt::async_client::connection_handler a_fcbDisconnected)
	{
		m_fcbDisconnected = a_fcbDisconnected;
		m_bNotifyDisConnection = true;
	}
	/** Function to set notification for msg received*/
	void setNotificationMsgRcvd(mqtt::async_client::message_handler a_fcbMsgRcvd)
	{
		m_fcbMsgRcvd = a_fcbMsgRcvd;
		m_bNotifyMsgRcvd = true;
	}
};

/** handler class for mqtt operations*/
class CMQTTBaseHandler
{
protected: 
	CMQTTPubSubClient m_MQTTClient; /** mqtt client*/
	int m_QOS; /** qos value*/

	/** delete copy and move constructors and assign operators*/
	CMQTTBaseHandler(const CMQTTBaseHandler&) = delete;	 			/** Copy construct*/
	CMQTTBaseHandler& operator=(const CMQTTBaseHandler&) = delete;	/** Copy assign */

public:
	CMQTTBaseHandler(const std::string &a_sBrokerURL, const std::string &a_sClientID,
		int a_iQOS, bool a_bIsTLS, const std::string &a_sCaCert, const std::string &a_sClientCert,
		const std::string &a_sClientKey, const std::string &a_sListener);
	virtual ~CMQTTBaseHandler();

	virtual void connected(const std::string &a_sCause);
	virtual void disconnected(const std::string &a_sCause);
	virtual void msgRcvd(mqtt::const_message_ptr a_pMsg);
	
	bool isConnected();
	void connect();
	void disconnect();

	bool publishMsg(const std::string &a_sMsg, const std::string &a_sTopic);
};

#endif
