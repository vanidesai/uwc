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


TEST_F(Common_ut, getMacro_test)
{
	struct timespec tsMsgReceived;
	bool result = true;
	std::string RetVal = std::to_string(commonUtilKPI::get_micros(tsMsgReceived));
	EXPECT_EQ(true, result);

}

TEST_F(Common_ut, timeStamp_in_string)
{
	std::string strCurTime = "8:56";
	bool result = true;
	commonUtilKPI::getCurrentTimestampsInString(strCurTime);
	EXPECT_EQ(true, result);
}


TEST_F(Common_ut, getKeyValue_FromJson_withMsg)
{
	//std::string RetVal = commonUtilKPI::getValueofKeyFromJSONMsg(recvdMsg.getMsg(), "app_seq");
	std::string RetVal = commonUtilKPI::getValueofKeyFromJSONMsg(strMsg, "app_seq");
	EXPECT_EQ("1234", RetVal);

}
/*

TEST_F(Common_ut, getKeyValue_FromJson_withoutMsg)
{
	std::string RetVal = commonUtilKPI::getValueofKeyFromJSONMsg(recvdMsg.getMsg(), "app_seq");
	EXPECT_EQ("", RetVal);

}
*/

TEST_F(Common_ut, FieldToMsg_allFields)
{
	bool result = true;
	std::string a_sAppSeq = "56666";
	commonUtilKPI::addFieldToMsg(strMsg, "app_seq", a_sAppSeq, false);
	EXPECT_EQ(true, result);
}

TEST_F(Common_ut, FieldToMsg_AbsentFields)
{
	bool result = false;
	std::string Msg;
	std::string a_sAppSeq ;
	commonUtilKPI::addFieldToMsg(Msg, "app_seq", a_sAppSeq, false);
	EXPECT_EQ(false, result);
}

TEST_F(Common_ut, createWriteReq_true)
{
	std::string a_sAppSeq = "56666";
	std::string sWrRT{"0"};
	bool RetVal = commonUtilKPI::createWriteRequest(strMsg, "data_topic", "well", "PL0", a_sAppSeq, sWrRT, "KPIAPP_WrReq_RT", true);
    EXPECT_EQ(1, RetVal);
}

TEST_F(Common_ut, createWriteReq_MQTTModeFalse)
{
	std::string a_sAppSeq = "56666";
	std::string sWrRT{"0"};
	bool RetVal = commonUtilKPI::createWriteRequest(strMsg, "data_topic", "well", "PL0", a_sAppSeq, sWrRT, "KPIAPP_WrReq_RT", false);
    EXPECT_EQ(1, RetVal);
}

TEST_F(Common_ut, createWriteReq_EmptyFiels_false)
{
	std::string a_sAppSeq;
	std::string sWrRT;
	std::string msg;
	bool RetVal = commonUtilKPI::createWriteRequest(msg, "", "", "", a_sAppSeq, sWrRT, "", false);
    EXPECT_EQ(0, RetVal);
}

/*Need to check
TEST_F(Common_ut, AnalysisMsg)
{
	struct stPollWrData a_stPollWrData;
	CMessageObject a_msgWrResp;
	commonUtilKPI::logAnalysisMsg(a_stPollWrData, a_msgWrResp);
}

TEST_F(Common_ut, AnalysisMsg_Empty)
{
	struct stPollWrData a_stPollWrData;
	CMessageObject a_msgWrResp;
	std::string RetVal;
	RetVal = commonUtilKPI::createAnalysisMsg(a_stPollWrData, a_msgWrResp);
	EXPECT_EQ("", RetVal);
}
*/




