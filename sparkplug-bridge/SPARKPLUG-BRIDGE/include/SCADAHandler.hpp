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

	std::mutex m_mutexSparkPlugMsgPub; /** mutex to control publishing */

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

	bool publishSparkplugMsg(org_eclipse_tahu_protobuf_Payload& a_payload, string a_topic, bool a_bIsNBirth);

	void defaultPayload(org_eclipse_tahu_protobuf_Payload& a_payload);

	bool addModbusTemplateDefToNbirth(org_eclipse_tahu_protobuf_Payload& a_rTahuPayload);

	bool publishNewUDTs();

public:
	/** Destructor*/
	~CSCADAHandler();
	static CSCADAHandler& instance();

	bool pushMsgInQ(mqtt::const_message_ptr msg);
	bool prepareSparkPlugMsg(std::vector<stRefForSparkPlugAction>& a_stRefActionVec);
	bool processDCMDMsg(CMessageObject a_msg, std::vector<stRefForSparkPlugAction>& a_stRefActionVec);
	bool processNCMDMsg(CMessageObject a_msg, std::vector<stRefForSparkPlugAction>& a_stRefActionVec);
	bool processExtMsg(CMessageObject a_msg, std::vector<stRefForSparkPlugAction>& a_stRefActionVec);

	void signalIntMQTTConnLostThread();
	void signalIntMQTTConnEstablishThread();

	bool addModbusMetric(org_eclipse_tahu_protobuf_Payload_Metric &a_rMetric, const std::string &a_sName, 
		CValObj &a_oValObj, bool a_bIsBirth, uint32_t a_uiPollInterval, bool a_bIsRealTime, double a_iScale);

	bool addModbusPropForBirth(org_eclipse_tahu_protobuf_Payload_Template &a_rUdt, 
		const std::string &a_sProtocolVal);
#ifdef UNIT_TEST
	friend class SCADAHandler_ut;
	friend class SparkPlugUDTMgr_ut;
#endif
};

#endif
