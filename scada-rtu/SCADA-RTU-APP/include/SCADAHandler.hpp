/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

/** SCADAHandler.hpp is handler for scada operations*/

#ifndef SCADAHANDLER_HPP_
#define SCADAHANDLER_HPP_

#include <mqtt/async_client.h>
#include <vector>
#include <semaphore.h>
#include "MQTTPubSubClient.hpp"
#include "Common.hpp"
#include "Logger.hpp"
#include "NetworkInfo.hpp"
#include "SparkPlugDevMgr.hpp"
#include <tahu.pb.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <inttypes.h>

#include "QueueMgr.hpp"
extern "C"
{
#include <tahu.h>
}

/** namespace for network information*/
using namespace network_info;

/** SCADA handler class*/
class CSCADAHandler : public CMQTTBaseHandler
{
	uint64_t m_uiBDSeq = 0; /** sequence number for birth message */

	sem_t m_semSCADAConnSuccess; /** semaphore for connection success*/
	sem_t m_semIntMQTTConnLost; /** semaphore for internal mqtt connection lost*/
	sem_t m_semIntMQTTConnEstablished; /** semaphore for internal mqtt connection established*/

	std::atomic<bool> m_bIsInitDone = false; /** flag for initialization check */

	/** Default constructor*/
	CSCADAHandler(const std::string &strPlBusUrl, int iQOS);

	/** delete copy and move constructors and assign operators*/
	CSCADAHandler(const CSCADAHandler&) = delete;	 			/** Copy construct*/
	CSCADAHandler& operator=(const CSCADAHandler&) = delete;	/** Copy assign*/

	bool getInitStatus() {return m_bIsInitDone.load();}
	void setInitStatus(bool a_bStatus) {return m_bIsInitDone.store(a_bStatus);}

	bool init();
	void prepareNodeDeathMsg(bool a_bPublishMsg);
	void handleSCADAConnectionSuccessThread();
	void handleIntMQTTConnLostThread();
	void handleIntMQTTConnEstablishThread();
	void publish_node_birth();
	void publishAllDevBirths(bool a_bIsNBIRTHProcess);
	void publish_device_birth(string a_deviceName, bool a_bIsNBIRTHProcess);
	bool publishMsgDDEATH(const stRefForSparkPlugAction& a_stRefAction);
	bool publishMsgDDEATH(const std::string &a_sDevName);
	bool publishMsgDDATA(const stRefForSparkPlugAction& a_stRefAction);

	void subscribeTopics();
	void connected(const std::string &a_sCause) override;
	void disconnected(const std::string &a_sCause) override;
	void msgRcvd(mqtt::const_message_ptr a_pMsg) override;

	bool publishSparkplugMsg(org_eclipse_tahu_protobuf_Payload& a_payload, string a_topic);

public:
	/** Destructor*/
	~CSCADAHandler();
	static CSCADAHandler& instance();

	bool pushMsgInQ(mqtt::const_message_ptr msg);
	bool prepareSparkPlugMsg(std::vector<stRefForSparkPlugAction>& a_stRefActionVec);
	bool processDCMDMsg(CMessageObject a_msg, std::vector<stRefForSparkPlugAction>& a_stRefActionVec);

	void signalIntMQTTConnLostThread();
	void signalIntMQTTConnEstablishThread();
};

#endif
