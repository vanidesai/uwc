/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef SCADAHANDLER_HPP_
#define SCADAHANDLER_HPP_

#include <mqtt/async_client.h>
#include "MQTTCallback.hpp"
#include "Common.hpp"
#include "QueueMgr.hpp"
#include "Logger.hpp"
#include "Publisher.hpp"
#include "NetworkInfo.hpp"
//
extern "C"
{
#include <tahu.h>
#include <tahu.pb.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <inttypes.h>
}
//

using namespace std;

// Declarations used for MQTT
#define SUBSCRIBERID								"SCADA_SUBSCRIBER"

class CSCADAHandler
{
	int m_QOS;

	mqtt::async_client m_subscriber;

	mqtt::connect_options m_subscriberConopts;
	mqtt::token_ptr m_conntok;

	CScadaCallback m_scadaSubscriberCB;
	CMQTTActionListener m_listener;

	// Default constructor
	CSCADAHandler(std::string strPlBusUrl, int iQOS);

	// delete copy and move constructors and assign operators
	CSCADAHandler(const CSCADAHandler&) = delete;	 			// Copy construct
	CSCADAHandler& operator=(const CSCADAHandler&) = delete;	// Copy assign
	bool connectSubscriber();

	friend class CScadaCallback;
	friend class CMQTTActionListener;

	void prepareNodeDeathMsg();
	void publish_births();
	void publish_node_birth();
public:

	~CSCADAHandler();
	static CSCADAHandler& instance(); //function to get single instance of this class
	void cleanup();
};

#endif
