/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef MQTTHANDLER_HPP_
#define MQTTHANDLER_HPP_

#include <atomic>
#include "Common.hpp"
#include <semaphore.h>
#include <string>
#include "mqtt/async_client.h"
#include "MQTTPubSubClient.hpp"

class CMQTTHandler : public CMQTTBaseHandler
{
	sem_t m_semConnSuccess;

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
