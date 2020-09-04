/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef PUBLISHER_HPP_
#define PUBLISHER_HPP_

#include "mqtt/async_client.h"
#include "MQTTCallback.hpp"
#include "Logger.hpp"

//
#include <tahu.pb.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <inttypes.h>

extern "C"
{
#include <tahu.h>
}
//

using namespace std;

class CPublisher
{
	mqtt::async_client m_ExtPublisher;
	mqtt::async_client m_IntPublisher;
	bool m_bIsFirst;
	mqtt::connect_options m_connOpts;
	mqtt::connect_options m_SSLConnOpts;
	mqtt::token_ptr m_conntok;
	mqtt::delivery_token_ptr m_pubtok;
	int m_QOS;

	CPublisherCallback m_publisherCB;
	CMQTTActionListener m_listener;

	friend class CPublisherCallback;
	friend class CMQTTActionListener;

	CPublisher(std::string a_ExtMqttURL, std::string a_IntMqttURL, int a_QOS);
	bool connect(mqtt::async_client& a_mqttClient, mqtt::connect_options& a_connOpts);

	// delete copy and move constructors and assign operators
	CPublisher(const CPublisher&) = delete;	 			// Copy construct
	CPublisher& operator=(const CPublisher&) = delete;	// Copy assign

public:
	~CPublisher();
	static CPublisher& instance(); //function to get single instance of this class

	bool isPublisherConnected();

	bool publishIntMqttMsg(std::string &a_sMsg, std::string &a_sTopic);
	bool publishSparkplugMsg(org_eclipse_tahu_protobuf_Payload& a_ddata_payload, string a_topic);

 	void cleanup();
 };

#endif
