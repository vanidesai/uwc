/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../include/Common_ut.hpp"


void Common_ut::SetUp()
{
	// Setup code
}

void Common_ut::TearDown()
{
	// TearDown code
}

/**
 * Test case to check if get_micross() Gets current time in micro seconds successfully
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Common_ut, getMacro_test)
{
	struct timespec tsMsgReceived;
	bool result = true;
	std::string RetVal = std::to_string(commonUtilKPI::get_micros(tsMsgReceived));
	EXPECT_EQ(true, result);

}

/**
 * Test case to check if getCurrentTimestampsInString() Gets current epoch time in string successfully
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Common_ut, timeStamp_in_string)
{
	std::string strCurTime = "8:56";
	bool result = true;
	commonUtilKPI::getCurrentTimestampsInString(strCurTime);
	EXPECT_EQ(true, result);
}

/**
 * Test case to check if getValueofKeyFromJSONMsg() function extracts the value of a key from given JSON payload successfully
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Common_ut, getKeyValue_FromJson_withMsg)
{
	//std::string RetVal = commonUtilKPI::getValueofKeyFromJSONMsg(recvdMsg.getMsg(), "app_seq");
	std::string RetVal = commonUtilKPI::getValueofKeyFromJSONMsg(strMsg, "app_seq");
	EXPECT_EQ("1234", RetVal);

}


/**
 * Test case to check if addFieldToMsg() function adds field to JSON payload successfully
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Common_ut, FieldToMsg_allFields)
{
	bool result = true;
	std::string a_sAppSeq = "56666";
	commonUtilKPI::addFieldToMsg(strMsg, "app_seq", a_sAppSeq, false);
	EXPECT_EQ(true, result);
}

/**
 * Test case to check if addFieldToMsg() function do not adds field to JSON payload successfully if
 * one of the parameter is empty.
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Common_ut, FieldToMsg_AbsentFields)
{
	bool result = false;
	std::string Msg;
	std::string a_sAppSeq ;
	commonUtilKPI::addFieldToMsg(Msg, "app_seq", a_sAppSeq, false);
	EXPECT_EQ(false, result);
}

/**
 * Test case to check if createWriteRequest() Function creates a write message to be sent as a part of control loop successfully
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Common_ut, createWriteReq_true)
{
	std::string a_sAppSeq = "56666";
	std::string sWrRT{"0"};
	bool RetVal = commonUtilKPI::createWriteRequest(strMsg, "data_topic", "well", "PL0", a_sAppSeq, sWrRT, "KPIAPP_WrReq_RT", true);
    EXPECT_EQ(1, RetVal);
}

/**
 * Test case to check if createWriteRequest() Function creates a write message to be sent as a part of control loop successfully
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Common_ut, createWriteReq_MQTTModeFalse)
{
	std::string a_sAppSeq = "56666";
	std::string sWrRT{"0"};
	bool RetVal = commonUtilKPI::createWriteRequest(strMsg, "data_topic", "well", "PL0", a_sAppSeq, sWrRT, "KPIAPP_WrReq_RT", false);
    EXPECT_EQ(1, RetVal);
}

/**
 * Test case to check if createWriteRequest() Function do not create a write message to be sent as a part of control loop
 * when all parameters are not available
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Common_ut, createWriteReq_EmptyFields_false)
{
	std::string a_sAppSeq;
	std::string sWrRT;
	std::string msg;
	bool RetVal = commonUtilKPI::createWriteRequest(msg, "", "", "", a_sAppSeq, sWrRT, "", false);
    EXPECT_EQ(0, RetVal);
}

/**
 * Test case to check if logAnalysisMsg() function does creates teh analysis message and logs it into required logger successfully
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Common_ut, AnalysisMsg)
{
	struct stPollWrData a_stPollWrData;
	CMessageObject a_msgWrResp;
	commonUtilKPI::logAnalysisMsg(a_stPollWrData, a_msgWrResp);
}

/**
 * Test case to check if createAnalysisMsg() function do not create analysis message for a control loop
 * if m_mqttMsg is null
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Common_ut, AnalysisMsg_Empty)
{
	struct stPollWrData a_stPollWrData;
	CMessageObject a_msgWrResp;
	std::string RetVal;
	RetVal = commonUtilKPI::createAnalysisMsg(a_stPollWrData, a_msgWrResp);
	EXPECT_EQ("", RetVal);
}

/**
 * Test case to check if createAnalysisMsg() function returns with valid message string when
 * a valid polling and write data are provided as input
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Common_ut, AnalysisMsg_withMsg)
{
	CMessageObject a_msgWrResp(Topic_UT, msg);
	a_stPollWrData.m_oPollData = a_msgWrResp;

	RetVal = commonUtilKPI::createAnalysisMsg(a_stPollWrData, a_msgWrResp);
	EXPECT_NE("", RetVal);
}




