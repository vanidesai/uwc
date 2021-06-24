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



