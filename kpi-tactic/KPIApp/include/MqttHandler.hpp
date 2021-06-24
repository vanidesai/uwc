/********************************************************************************
* Copyright (c) 2021 Intel Corporation.

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*********************************************************************************/

/*** MqttHandler.hpp is handler for MQTT operations*/

#ifndef MQTT_PUBLISH_HANDLER_HPP_
#define MQTT_PUBLISH_HANDLER_HPP_

#include <semaphore.h>
#include "MQTTPubSubClient.hpp"
#include "QueueMgr.hpp"

/** Handler class for mqtt*/
class CMqttHandler : public CMQTTBaseHandler
{
	sem_t m_semConnSuccess; /** semaphore for connection success*/

	CMqttHandler(const std::string &strPlBusUrl, int iQOS);//constructor

	/** delete copy and move constructors and assign operators*/
	CMqttHandler(const CMqttHandler&) = delete;	 			/** Copy construct*/
	CMqttHandler& operator=(const CMqttHandler&) = delete;	/** Copy assign*/

	void subscribeTopics();
	void connected(const std::string &a_sCause) override;
	void msgRcvd(mqtt::const_message_ptr a_pMsg) override;
	bool init();

	void handleConnSuccessThread();

	bool pushMsgInQ(mqtt::const_message_ptr& a_msgMQTT);

public:
	~CMqttHandler();/** destructor*/
	static CMqttHandler& instance(); /** function to get single instance of this class*/
};

#endif
