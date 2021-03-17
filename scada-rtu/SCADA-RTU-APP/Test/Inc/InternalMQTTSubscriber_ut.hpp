/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#ifndef TEST_INCLUDE_INTMQTTSUB_UT_HPP_
#define TEST_INCLUDE_INTMQTTSUB_UT_HPP_

#include "InternalMQTTSubscriber.hpp"
#include "Common.hpp"
#include "ConfigManager.hpp"
#include "SparkPlugDevices.hpp"
#include <gtest/gtest.h>

class InternalMQTTSubscriber_ut : public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();

public:

	std::string			Test_Str = "";
	std::string			Expected_output = "";
	std::vector<stRefForSparkPlugAction> stRefActionVec;
	std::vector<stRefForSparkPlugAction> stRefActionVec1;
	std::reference_wrapper<CSparkPlugDev> *a_ref;
		eMsgAction a_enAction = enMSG_NONE;
		metricMapIf_t m_mapChangedMetrics;

	stRefForSparkPlugAction stDummyAction{*a_ref, a_enAction, m_mapChangedMetrics};

	void _subscribeTopics()
	{
		CIntMqttHandler::instance().subscribeTopics();
	}

};

#endif /* TEST_INCLUDE_INTMQTTSUB_UT_HPP_  */
