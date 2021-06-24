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

/**
 * File contains the class CMQTTHandler to manage instance of MQTT connection
 */

#ifndef MQTTHANDLER_HPP_
#define MQTTHANDLER_HPP_

#include <atomic>
#include "Common.hpp"
#include <semaphore.h>
#include <string>
#include "mqtt/async_client.h"
#include "MQTTPubSubClient.hpp"

/**
 * MQTT Handler class to manage instance of MQTT connection
 */
class CMQTTHandler : public CMQTTBaseHandler
{
	sem_t m_semConnSuccess; /*!< an instance of semaphore */

	CMQTTHandler(const std::string &strPlBusUrl, int iQOS);

	// delete copy and move constructors and assign operators
	CMQTTHandler(const CMQTTHandler&) = delete;	 			// Copy construct
	CMQTTHandler& operator=(const CMQTTHandler&) = delete;	// Copy assign

	void subscribeTopics();
	bool init();

	void handleConnSuccessThread();
	void signalIntMQTTConnDoneThread();

	bool parseMQTTMsg(const std::string &sJson, bool &isRealtime, const bool bIsDefault);

	bool pushMsgInQ(mqtt::const_message_ptr& msg);

public:
	~CMQTTHandler();
	static CMQTTHandler& instance(); //function to get single instance of this class

	void connected(const std::string &a_sCause) override;
	void msgRcvd(mqtt::const_message_ptr a_pMsg) override;

	void cleanup();
};

#endif
