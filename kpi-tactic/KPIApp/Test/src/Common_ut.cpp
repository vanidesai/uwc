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

TEST_F(Common_ut, AnalysisMsg_withMsg)
{
	std::map<std::string, std::vector<CControlLoopOp>> m_oControlLoopMap;
	//struct stPollWrData a_stPollWrData;
	//CMessageObject a_msgWrResp;
	struct stPollWrData oTempData{};
	std::string a_sAppSeq = "1234";
	CPollNWriteReqMapper::getInstace().getForProcessing(a_sAppSeq, oTempData);
	std::string sDummyErrorRep;
	std::string m_sWritePointFullPath = "";
	std::string Error = "WrRespNotRcvd";
	std::string AppSeq {""};
	sDummyErrorRep = "{" + sDummyErrorRep + "}";
	CMessageObject oDummyMsg{"errorDummyTopic", sDummyErrorRep};
	commonUtilKPI::addFieldToMsg(sDummyErrorRep, "app_seq", AppSeq, false);
	commonUtilKPI::addFieldToMsg(sDummyErrorRep, "data_topic", m_sWritePointFullPath + "/writeResponse", false);
	commonUtilKPI::addFieldToMsg(sDummyErrorRep, "error_code", Error + "/writeResponse", true);
	std::string RetVal;
	/*string a_sPolledTopic = "TCP_PolledData_RT";
	std::string sPollKey{a_sPolledTopic + "/update"};
	std::vector<CControlLoopOp> oTemp;
	m_oControlLoopMap.emplace(sPollKey, oTemp);*/

	RetVal = commonUtilKPI::createAnalysisMsg(oTempData, oDummyMsg);
	EXPECT_EQ("", RetVal);
}




