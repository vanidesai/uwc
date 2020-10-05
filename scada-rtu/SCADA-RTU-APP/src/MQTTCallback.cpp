/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/
#include "MQTTCallback.hpp"

#include "SCADAHandler.hpp"
#include "InternalMQTTSubscriber.hpp"

#if 0
/**
 * This is a callback function which gets called when subscriber is disconnected from MQTT broker
 * @param cause :[in] cause of connection lost
 * @return None
 */
void CSubscriberCallback::connection_lost(const std::string& cause)
{
	DO_LOG_ERROR("MQTT Connection lost");

	if (!cause.empty())
	{
		DO_LOG_ERROR("MQTT Connection lost cause:  " + cause);
	}
}

/**
 * This is a callback function which gets called when subscriber is connected with MQTT broker
 * @param cause :[in]
 * @return None
 */
void CSubscriberCallback::connected(const std::string& cause)
{
	//CIntMqttHandler::instance().subscribeToTopics();
}

/**
 * This is a callback function which gets called when a message is arrived on subscriber
 * @param msg :[in] pointer to message arriving at subscriber
 * @return None
 */
void CSubscriberCallback::message_arrived(mqtt::const_message_ptr msg)
{
	//CIntMqttHandler::instance().pushMsgInQ(msg);
}

/**
 * This is a callback function which gets called when subscriber fails to send/receive
 * @param tok :[in] failed message token
 * @return None
 */
void CMQTTActionListener::on_failure(const mqtt::token& tok)
{
	DO_LOG_ERROR("MQTT action (connect/message sending) failed");
}

/**
 * This is a callback function which gets called when subscriber succeeds in send/receive
 * @param tok :[in] message token
 * @return None
 */
void CMQTTActionListener::on_success(const mqtt::token& tok)
{
}

/**
 * This is a callback function which gets called when publisher is disconnected from MQTT broker
 * @param cause :[in] cause of connection lost
 * @return None
 */
void CPublisherCallback::connection_lost(const std::string& cause)
{
	DO_LOG_ERROR("MQTT publisher lost MQTT Connection");

	if (!cause.empty())
	{
		DO_LOG_ERROR("MQTT publisher lost MQTT Connection : cause:  " + cause);
	}
}

/**
 * This is a callback function which gets called when publisher is connected with MQTT broker
 * @param cause :[in]
 * @return None
 */
void CPublisherCallback::connected(const std::string& cause)
{
}

/**
 * This is a callback function which gets called when publisher publishes message successfully
 * @param tok :[in] message token
 * @return None
 */
void CPublisherCallback::delivery_complete(mqtt::delivery_token_ptr tok)
{
}

/**
 * This is a callback function which gets called when subscriber is disconnected from MQTT broker
 * @param cause :[in] cause of connection lost
 * @return None
 */
void CScadaCallback::connection_lost(const std::string& cause)
{
	DO_LOG_ERROR("SCADA Connection lost");

	if (!cause.empty())
	{
		DO_LOG_ERROR("SCADA Connection lost cause:  " + cause);
	}
}

/**
 * This is a callback function which gets called when subscriber is connected with MQTT broker
 * @param cause :[in]
 * @return None
 */
void CScadaCallback::connected(const std::string& cause)
{
	DO_LOG_DEBUG("SCADA-RTU Connected with the MQTT broker");
	//CSCADAHandler::instance().connected();
}

/**
 * This is a callback function which gets called when a message is arrived on subscriber
 * @param msg :[in] pointer to message arriving at subscriber
 * @return None
 */
void CScadaCallback::message_arrived(mqtt::const_message_ptr msg)
{
	//CSCADAHandler::instance().pushMsgInQ(msg);
}
#endif

// ---------------------------------------------------

void action_listener::on_failure(const mqtt::token& tok)  
{
	auto top = tok.get_topics();
	if (top && !top->empty())
	{
		DO_LOG_ERROR(name_ + ": On failure - Type: " + std::to_string(tok.get_type()) + 
					", Topic- " + (*top)[0]);
	}
	else
	{
		DO_LOG_ERROR(name_ + ": On failure - Type: " + std::to_string(tok.get_type()));
	}
	/*std::cout << name_ << " failure";
	if (tok.get_message_id() != 0)
		std::cout << " for token: [" << tok.get_message_id() << "]" 
			<< ", type:" << tok.get_type() << std::endl;
	std::cout << std::endl;*/
}

void action_listener::on_success(const mqtt::token& tok)  
{
	auto top = tok.get_topics();
	if (top && !top->empty())
	{
		DO_LOG_DEBUG(name_ + ": Success - Type: " + std::to_string(tok.get_type()) + 
					", Topic- " + (*top)[0]);
	}
	else
	{
		DO_LOG_DEBUG(name_ + ": Success - Type: " + std::to_string(tok.get_type()));
	}
	/*std::cout << name_ << " success";
	if (tok.get_message_id() != 0)
		std::cout << " for token: [" << tok.get_message_id() << "]" << std::endl;
	auto top = tok.get_topics();
	if (top && !top->empty())
		std::cout << "\ttoken topic: '" << (*top)[0] << "', ..." 
		<< ", type:" << tok.get_type() << std::endl;
	std::cout << std::endl;*/
}

/**
 * Constructor: Sets all parameters needed to set a connection with MQTT broker
 * @param a_sBrokerURL :[in] MQTT broker URL
 * @param a_sClientID: [in] Client id to be used to establish a connection
 * @param a_iQOS :[in] QOS level to be used for communication with broker
 * @param a_bIsTLS :[in] Tells whether TLS connection is needed
 * @param a_sCATrustStoreSecret :[in] MQTT CA certificate, needed when TLS = true
 * @param a_sClientCertSecret :[in] MQTT client certificate, needed when TLS = true
 * @param a_sClientPvtKeySecret :[in] MQTT client private key, needed when TLS = true
 * @param a_sListener :[in] Action listener name to be used
 */
CMQTTPubSubClient::CMQTTPubSubClient(const std::string &a_sBrokerURL, std::string a_sClientID, 
	int a_iQOS, //mqtt::message_ptr &a_willMsg,
	bool a_bIsTLS, std::string a_sCATrustStoreSecret, 
	std::string a_sClientCertSecret, std::string a_sClientPvtKeySecret, 
	std::string a_sListener)
	: m_iQOS{a_iQOS}, m_sClientID{a_sClientID}, m_Client{a_sBrokerURL, a_sClientID}, m_Listener{a_sListener}
{
	try
	{
		// Check QOS value
		if(  (0 != m_iQOS) && (1 != m_iQOS) && (2 != m_iQOS) )
		{
			// Setting defalt value in case of incorrect configuration
			m_iQOS = 1;
		}
		//
		//connect options for sync publisher/client
		m_ConOptions.set_keep_alive_interval(20);
		m_ConOptions.set_clean_session(true);
		//m_ConOptions.set_automatic_reconnect(1, 10);
		//m_ConOptions.set_will_message(a_willMsg);

		// set the certificates if dev mode is false
		if(true == a_bIsTLS)
		{
			if(a_sCATrustStoreSecret.empty() || a_sClientPvtKeySecret.empty()
				|| a_sClientCertSecret.empty())
			{
				throw;
			}
			mqtt::ssl_options sslopts;
			sslopts.set_trust_store(a_sCATrustStoreSecret);
			sslopts.set_key_store(a_sClientCertSecret);
			sslopts.set_private_key(a_sClientPvtKeySecret);
			sslopts.set_enable_server_cert_auth(true);
			m_ConOptions.set_ssl(sslopts);
		}

		m_Client.set_callback(*this);
	}
	catch (const std::exception &e)
	{
		DO_LOG_ERROR(e.what());
		throw;
	}
}

/**
 * This function tries to establish a connection with MQTT broker
 * @return true/false status based on success/failure
 */
bool CMQTTPubSubClient::connect()
{
	try
	{
		auto oToken = m_Client.connect(m_ConOptions, nullptr, *this);
		/*if (false == oToken->wait_for(2000))
		{
			std::cout << "Error::Failed to connect to the server\n";
			return false;
		}*/
	}
	catch (const std::exception &e)
	{
		DO_LOG_ERROR(e.what());
		return false;
	}
	return true;
}

/**
 * This function tries to stop a connection with MQTT broker
 * @return true/false status based on success/failure
 */
bool CMQTTPubSubClient::disconnect()
{
	try
	{
		auto oToken = m_Client.disconnect();
		/*if (false == oToken->wait_for(2000))
		{
			std::cout << "Error::Failed to disconnect from the server\n";
			return false;
		}*/
	}
	catch (const std::exception &e)
	{
		DO_LOG_ERROR(e.what());
		return false;
	}
	return true;
}

/**
 * This function tries to establish a connection when it is lost with MQTT broker
 * @return none
 */
void CMQTTPubSubClient::reconnect() 
{
	try 
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		m_Client.connect(m_ConOptions, nullptr, *this);
	}
	catch (const mqtt::exception& exc) 
	{
		DO_LOG_ERROR(exc.what())
		std::cerr << "Error: " << exc.what() << std::endl;
	}
}

/**
 * This function subscribes to a topic on MQTT broker
 * @param a_sTopic :[in] topic to be published
 * @return none
 */
void CMQTTPubSubClient::subscribe(const std::string &a_sTopic)
{
	try
	{
		//if( std::find(m_sTopicList.begin(), m_sTopicList.end(), a_sTopic) == m_sTopicList.end() )
		{
			m_Client.subscribe(a_sTopic, m_iQOS, nullptr, m_Listener);
			//m_sTopicList.push_back(a_sTopic);
		}
	}
	catch (const std::exception &e)
	{
		DO_LOG_ERROR(e.what());
	}
}

/**
 * This function publishes a message on MQTT broker
 * @param a_pubMsg :[in] pointer to message to be published
 * @return true/false status based on success/failure 
 */
bool CMQTTPubSubClient::publishMsg(mqtt::message_ptr &a_pubMsg)
{
	try
	{
		if(true == m_Client.is_connected())
		{
			a_pubMsg->set_qos(m_iQOS);
			m_Client.publish(a_pubMsg, nullptr, m_Listener);
		}
	}
	catch (const std::exception &e)
	{
		DO_LOG_ERROR(e.what());
		return false;
	}
	return true;
}

/**
 * This is a callback function which gets called when subscriber fails to send/receive/connect
 * @param tok :[in] failed message token
 * @return None
 */
void CMQTTPubSubClient::on_failure(const mqtt::token& tok)
{
	try
	{
		if(mqtt::token::Type::CONNECT == tok.get_type())
		{
			DO_LOG_ERROR("Connection attempt failed: " + m_sClientID);
			if(m_bNotifyDisConnection)
			{
				m_fcbDisconnected("CONNECT_FAILED");
			}
			reconnect();
		}
	}
	catch (const std::exception &e)
	{
		DO_LOG_ERROR(e.what());
	}
}

/**
 * This is a callback function which gets called when subscriber succeeds in send/receive/(re)connection
 * Either this or connected() can be used for callbacks. 
 * @param tok :[in] message token
 * @return None
 */
void CMQTTPubSubClient::on_success(const mqtt::token& tok)
{
	try
	{
		if(mqtt::token::Type::CONNECT == tok.get_type())
		{
			DO_LOG_INFO("Connection attempt successful: " + m_sClientID);
			if(m_bNotifyConnection)
			{
				m_fcbConnected("CONNECTED");
			}
		}
		if(mqtt::token::Type::DISCONNECT == tok.get_type())
		{
			DO_LOG_INFO("Disconnect attempt successful");
			if(m_bNotifyDisConnection)
			{
				m_fcbDisconnected("DISCONNECTED");
			}
		}
	}
	catch (const std::exception &e)
	{
		DO_LOG_ERROR(e.what());
	}
}

/**
 * This is a callback function which gets called when subscriber is connected with MQTT broker
 * @param cause :[in] reason for connect
 * @return None
 */
void CMQTTPubSubClient::connected(const std::string& a_sCause)
{
	try
	{
		std::cout << "CMQTTPubSubClient::connected " << m_sClientID << std::endl;
		DO_LOG_INFO(m_sClientID + " Connected: " + a_sCause);
		/*if(m_bNotifyConnection)
		{
			m_fcbConnected(cause);
		}*/
	}
	catch (const std::exception &e)
	{
		DO_LOG_ERROR(e.what());
	}
}

/**
 * This is a callback function which gets called when subscriber is disconnected from MQTT broker
 * @param cause :[in] cause of connection lost
 * @return None
 */
void CMQTTPubSubClient::connection_lost(const std::string& a_sCause)
{
	try
	{
		DO_LOG_ERROR(m_sClientID + ": Connection lost: " + a_sCause);

		if(m_bNotifyDisConnection)
		{
			m_fcbDisconnected("CONNECT_LOST");
		}
	}
	catch (const std::exception &e)
	{
		DO_LOG_ERROR(e.what());
	}
}

/**
 * This is a callback function which gets called when a message is arrived on subscriber
 * @param msg :[in] pointer to message arriving at subscriber
 * @return None
 */
void CMQTTPubSubClient::message_arrived(mqtt::const_message_ptr msg)
{
	try
	{
		DO_LOG_DEBUG(m_sClientID + ": Message arrived: " + msg->get_topic());
		if(m_bNotifyMsgRcvd)
		{
			m_fcbMsgRcvd(msg);
		}
	}
	catch (const std::exception &e)
	{
		DO_LOG_ERROR(e.what());
	}
}
