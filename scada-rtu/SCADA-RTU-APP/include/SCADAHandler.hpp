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
#include <vector>
#include <semaphore.h>
#include "MQTTCallback.hpp"
#include "Common.hpp"
#include "Logger.hpp"
#include "NetworkInfo.hpp"
#include "SparkPlugDevMgr.hpp"
#include "QueueMgr.hpp"
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
using namespace network_info;

// Declarations used for MQTT
#define SUBSCRIBERID								"SCADA_SUBSCRIBER"

class CSCADAHandler
{
	uint64_t m_uiBDSeq = 0; 
	int m_QOS;

	CMQTTPubSubClient m_MQTTClient;

	sem_t m_semSCADAConnSuccess;
	sem_t m_semIntMQTTConnLost;

	std::atomic<bool> m_bIsInitDone = false;

	// Default constructor
	CSCADAHandler(const std::string &strPlBusUrl, int iQOS);

	// delete copy and move constructors and assign operators
	CSCADAHandler(const CSCADAHandler&) = delete;	 			// Copy construct
	CSCADAHandler& operator=(const CSCADAHandler&) = delete;	// Copy assign
	//bool connectSubscriber();

	bool getInitStatus() {return m_bIsInitDone.load();}
	void setInitStatus(bool a_bStatus) {return m_bIsInitDone.store(a_bStatus);}

	bool init();
	void prepareNodeDeathMsg(bool a_bPublishMsg);
	void startSCADAConnectionSuccessProcess();
	void handleSCADAConnectionSuccessThread();
	void handleIntMQTTConnLostThread();
	void publish_node_birth();
	void publishAllDevBirths();
	void publish_device_birth(string a_deviceName, bool a_bIsNBIRTHProcess);
	bool publishMsgDDEATH(const stRefForSparkPlugAction& a_stRefAction);
	bool publishMsgDDEATH(const std::string &a_sDevName);
	bool publishMsgDDATA(const stRefForSparkPlugAction& a_stRefAction);
	//bool initDataPoints();
	void populateDataPoints();

	void subscribeTopics();
	static void connected(const std::string &a_sCause);
	static void disconnected(const std::string &a_sCause);
	static void msgRcvd(mqtt::const_message_ptr a_pMsg);
	void vendor_app_birth_request();

	bool publishSparkplugMsg(org_eclipse_tahu_protobuf_Payload& a_payload, string a_topic);

public:
	~CSCADAHandler();
	static CSCADAHandler& instance();
	void disconnect();
	void connect();
	void cleanup();

	bool pushMsgInQ(mqtt::const_message_ptr msg);
	bool prepareSparkPlugMsg(std::vector<stRefForSparkPlugAction>& a_stRefActionVec);
	bool processDCMDMsg(mqtt::const_message_ptr a_msg, std::vector<stRefForSparkPlugAction>& a_stRefActionVec);

	void signalIntMQTTConnLostThread();
};

#endif
