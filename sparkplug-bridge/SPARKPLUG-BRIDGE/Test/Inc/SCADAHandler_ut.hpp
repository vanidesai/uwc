/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef TEST_INCLUDE_SCADAHANDLER_UT_H_
#define TEST_INCLUDE_SCADAHANDLER_UT_H_

#include "SCADAHandler.hpp"
#include "InternalMQTTSubscriber.hpp"
#include "Metric.hpp"

#ifdef UNIT_TEST
#include <gtest/gtest.h>
#endif


class SCADAHandler_ut : public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();
	bool _publishSparkplugMsg(org_eclipse_tahu_protobuf_Payload& a_payload, string spName, bool a_bIsNBirth)
	{
		return CSCADAHandler::instance().publishSparkplugMsg(a_payload, spName, a_bIsNBirth);
	}

	void _prepareNodeDeathMsg(bool a_bPublishMsg)
	{
		CSCADAHandler::instance().prepareNodeDeathMsg(a_bPublishMsg);
	}

	void _publish_node_birth()
	{
		CSCADAHandler::instance().publish_node_birth();
	}

	void _publishAllDevBirths(bool a_bIsNBIRTHProcess)
	{
		CSCADAHandler::instance().publishAllDevBirths(a_bIsNBIRTHProcess);
	}

	void _publish_device_birth(std::string a_deviceName, bool a_bIsNBIRTHProcess)
	{
		CSCADAHandler::instance().publish_device_birth(a_deviceName, a_bIsNBIRTHProcess);
	}

	bool _publishMsgDDEATH(const stRefForSparkPlugAction& a_stRefAction)
	{
		CSCADAHandler::instance().publishMsgDDEATH(a_stRefAction);
	}

	bool _publishMsgDDEATH(const std::string a_sDevName)
	{
		CSCADAHandler::instance().publishMsgDDEATH(a_sDevName);
	}

	bool _publishMsgDDATA(const stRefForSparkPlugAction& a_stRefAction)
	{
		CSCADAHandler::instance().publishMsgDDATA(a_stRefAction);
	}

	void _subscribeTopics()
	{
		CSCADAHandler::instance().subscribeTopics();
	}

	void _connected(const std::string &a_sCause)
	{
		CSCADAHandler::instance().connected(a_sCause);
	}

	void _disconnected(const std::string &a_sCause)
	{
		CSCADAHandler::instance().disconnected(a_sCause);
	}

	void _publishNewUDTs()
	{
		CSCADAHandler::instance().publishNewUDTs();
	}

public:
	bool Bool_Res = false;
	org_eclipse_tahu_protobuf_Payload_Metric a_metric = { NULL, false, 0, true, get_current_timestamp(), true,
							METRIC_DATA_TYPE_UNKNOWN, false, 0, false, 0, false,
							true, false,
				org_eclipse_tahu_protobuf_Payload_MetaData_init_default,
							false,
									org_eclipse_tahu_protobuf_Payload_PropertySet_init_default,
							0,
							{ 0 } };
	org_eclipse_tahu_protobuf_Payload_Template udt_template = org_eclipse_tahu_protobuf_Payload_Template_init_default;
bool success = true;
};

#endif /* TEST_INCLUDE_SCADAHANDLER_UT_H_ */
