/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/


#include "../Inc/SparkPlugUDTMgr_ut.hpp"

void SparkPlugUDTMgr_ut::SetUp()
{
	// Setup code
}

void SparkPlugUDTMgr_ut::TearDown()
{
	// TearDown code
}


TEST_F(SparkPlugUDTMgr_ut, ProcessTempDef)
{
	//recvdMsg.getStrMsg()
	//std::vector<stRefForSparkPlugAction> &stRefActionVec;
	std::vector<stRefForSparkPlugAction> stRefActionVec;
	//std::string PayLoad = m_mqttMsg->get_payload();
	CSparkPlugUDTManager::getInstance().processTemplateDef(recvdMsg.getStrMsg() ,stRefActionVec);
}


TEST_F(SparkPlugUDTMgr_ut, UDT_Present_check)
{
	CUDTDefRef = CSparkPlugUDTManager::getInstance().isPresent(UDTDefName, UDTDefVer);
	EXPECT_EQ(nullptr, CUDTDefRef);
}


TEST_F(SparkPlugUDTMgr_ut, UDT_PresentNonEmptyMap)
{
     _isPresent();
	//EXPECT_EQ(nullptr, CUDTDefRef);
}


//TEST_F(SparkPlugUDTMgr_ut, AddUDTdefsTonbirth)
//{
//	bool result = CSparkPlugUDTManager::getInstance().addUDTDefsToNbirth(nbirth_payload);
//	EXPECT_EQ(1, result);
//}

/* */


TEST_F(SparkPlugUDTMgr_ut, ProcessTempDefValidPayload)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;
	std::string PayLoad =  "{\"udt_name\": \"custom_udt\", \"version\": \"1.0\", \"metrics\": [{ \"name\": \"M1\", \"dataType\": \"String\", \"value\": \"\"}, {   \"name\": \"RTU_Time\", \"dataType\": \"Int32\", \"value\": 0 } ],  \"parameters\": [ {  \"name\": \"P1\", \"dataType\": \"String\",   \"value\": \"\"  }, { \"name\": \"P2\", \"dataType\": \"Int32\", \"value\": 0  } ]}";
	CSparkPlugUDTManager::getInstance().processTemplateDef(PayLoad ,stRefActionVec);
}



/**
 * Test case to check if addNewUDT() behaves as expected
 * @param :[in]
 * @param :[out] bool
 * @return bool
 */

TEST_F(SparkPlugUDTMgr_ut, addNewUDT)
{
	std::string sUDTName = "custom_udt";
	std::string sUDTVersion =  "1.0";
	auto cIfMetric = std::make_shared<CUDT>(sUDTName, METRIC_DATA_TYPE_TEMPLATE, true, sUDTVersion);
	if(nullptr == cIfMetric)
	{
		DO_LOG_ERROR("unable to create UDT definition.");
		return;
	}

	bool result = _addNewUDT(sUDTName, sUDTVersion, cIfMetric);
	EXPECT_EQ(true, result);
}


/**
 * Test case to check if addUDTDefToNbirth() behaves as expected
 * @param :[in]
 * @param :[out] bool
 * @return bool
 */

TEST_F(SparkPlugUDTMgr_ut, addUDTDefToNbirth)
{
	bool result = _addUDTDefToNbirth();
	EXPECT_EQ(true, result);
}


/**
 * Test case to check if addUDTDefToNbirth() behaves as expected
 * @param :[in]
 * @param :[out] bool
 * @return bool
 */

TEST_F(SparkPlugUDTMgr_ut, addUDTDefsToNbirthValidPayload)
{
	bool result = _addUDTDefsToNbirth();
	EXPECT_EQ(true, result);
}




TEST_F(SparkPlugUDTMgr_ut, ProcessTempDefTemplateNameInValid)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;
	std::string PayLoad =  "{\"udt_name\": \"custom%udt\", \"version\": \"1.0\", \"metrics\": [{ \"name\": \"M1\", \"dataType\": \"String\", \"value\": \"\"}, {   \"name\": \"RTU_Time\", \"dataType\": \"Int32\", \"value\": 0 } ],  \"parameters\": [ {  \"name\": \"P1\", \"dataType\": \"String\",   \"value\": \"\"  }, { \"name\": \"P2\", \"dataType\": \"Int32\", \"value\": 0  } ]}";
	CSparkPlugUDTManager::getInstance().processTemplateDef(PayLoad ,stRefActionVec);
}




