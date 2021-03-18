/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/


#include "../Inc/InternalMQTTSubscriber_ut.hpp"

#include <iostream>
using namespace std;


void InternalMQTTSubscriber_ut::SetUp()
{
	// Setup code
}

void InternalMQTTSubscriber_ut::TearDown()
{
	// TearDown code
}

/**
 * Test case to check if prepareCJSONMsg() behaves as expected
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(InternalMQTTSubscriber_ut, prepareCJSONMsg_EmptyVector)
{
	bool result = CIntMqttHandler::instance().prepareCJSONMsg(stRefActionVec);
	EXPECT_EQ(true, result);
}

/**
 * Test case to check if prepareWriteMsg() behaves as expected
 * @param :[in] None
 * @param :[out] None
 * @return: bool
 */
TEST_F(InternalMQTTSubscriber_ut, writeRequest)
{
	std::string value = "2.0.0.2";
	std::string a_Name = "Properties/Version";
	uint32_t a_uiDataType = METRIC_DATA_TYPE_STRING;
	//CValObj(uint32_t a_uiDataType, var_t a_objVal)
	CValObj ocval(METRIC_DATA_TYPE_STRING, value);
	uint64_t timestamp = 1486144502122;
	//CMetric(std::string a_sName, const CValObj &a_objVal, const uint64_t a_timestamp)
	std::shared_ptr<CIfMetric> ptrCIfMetric = std::make_shared<CMetric>(a_Name, ocval, timestamp);
	metricMapIf_t mapChangedMetrics;
	mapChangedMetrics.emplace(ptrCIfMetric->getName(), std::move(ptrCIfMetric));
	std::string a_sSubDev = "RBOX510";
	std::string a_sSparkPluName = "flowmeter-PL0";
	bool a_bIsVendorApp = false;
	CSparkPlugDev oDev(a_sSubDev, a_sSparkPluName, a_bIsVendorApp);
	auto refDev = std::ref(oDev);
	bool result = CIntMqttHandler::instance().prepareWriteMsg(refDev, mapChangedMetrics);
	EXPECT_EQ(true, result);
}


/**
 * Test case to check if prepareCMDMsg() behaves as expected
 * @param :[in] None
 * @param :[out] None
 * @return: bool
 */
TEST_F(InternalMQTTSubscriber_ut, prepareCMDMsg)
{
	std::string a_Name = "Properties/Version";
	uint32_t a_uiDataType = METRIC_DATA_TYPE_STRING;
	std::shared_ptr<CIfMetric> ptrCIfMetric = std::make_shared<CMetric>(a_Name, a_uiDataType);
	metricMapIf_t mapChangedMetrics;
	mapChangedMetrics.emplace(ptrCIfMetric->getName(), std::move(ptrCIfMetric));

	std::string a_sSubDev = "flowmeter";
	std::string a_sSparkPluName = "spBv-Test";
	bool a_bIsVendorApp = true;

	CSparkPlugDev oDev(a_sSubDev, a_sSparkPluName, a_bIsVendorApp);
	auto refDev = std::ref(oDev);
	bool result = CIntMqttHandler::instance().prepareCMDMsg(refDev, m_mapChangedMetrics);
	EXPECT_EQ(true, result);
}

/*
 * Test Case to check if prepareCJSONMsg
 *
 */
TEST_F(InternalMQTTSubscriber_ut, prepareCJSONMsg)
{

	std::string value = "2.0.0.2";
	std::string a_Name = "Properties/Version";
	uint32_t a_uiDataType = METRIC_DATA_TYPE_STRING;
	CValObj ocval(a_uiDataType, value);
	uint64_t timestamp = 1486144502122;
	std::shared_ptr<CIfMetric> ptrCIfMetric = std::make_shared<CMetric>(a_Name, ocval, timestamp);
	metricMapIf_t mapChangedMetrics;
	mapChangedMetrics.emplace(ptrCIfMetric->getName(), std::move(ptrCIfMetric));
	std::string a_sSubDev = "flowmeter";
	std::string a_sSparkPluName = "spBv-Test";
	bool a_bIsVendorApp = true;
	CSparkPlugDev oDev(a_sSubDev, a_sSparkPluName, a_bIsVendorApp);
	auto refDev = std::ref(oDev);
	stRefForSparkPlugAction stRefVec(refDev, enMSG_BIRTH, mapChangedMetrics);
	std::vector<stRefForSparkPlugAction> v1;
	v1.push_back(stRefVec);
	bool result = CIntMqttHandler::instance().prepareCJSONMsg(v1);
	EXPECT_EQ(true, result);
}


TEST_F(InternalMQTTSubscriber_ut, subscribeTopics)
{
	_subscribeTopics();
}












