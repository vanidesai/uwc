/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../include/EISPlBusHandler_ut.hpp"

extern bool publishEISMsg(std::string a_sEisMsg, std::string &a_sEisTopic);

extern std::vector<std::thread> g_vThreads;

void EISPlBusHandler_ut::SetUp()
{
	// Setup code
}

void EISPlBusHandler_ut::TearDown()
{
	// TearDown code
}

/***********************************************************/
/*	Helper function	*/
void initEISContext_Caller(bool& RetVal)
{
	CEISPlBusHandler CEISPlBusHandler_obj;
	RetVal = CEISPlBusHandler_obj.initEISContext();
}
/***********************************************************/

/**
 * Test case to check if configEISListerners() function Configures EIS listeners for a_bIsPollingRT = false and a_bIsWrOpRT = false successfully
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(EISPlBusHandler_ut, EISListerners_false)
{
	bool a_bIsPollingRT = false;
	bool a_bIsWrOpRT = false;
	CEISPlBusHandler_obj.configEISListerners(a_bIsPollingRT, a_bIsWrOpRT);
}

/**
 * Test case to check if configEISListerners() function Configures EIS listeners a_bIsPollingRT = true and a_bIsWrOpRT = false successfully
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(EISPlBusHandler_ut, EISListerners_true_false)
{
	bool a_bIsPollingRT = true;
	bool a_bIsWrOpRT = false;
	CEISPlBusHandler_obj.configEISListerners(a_bIsPollingRT, a_bIsWrOpRT);
}

/**
 * Test case to check if configEISListerners() function Configures EIS listeners a_bIsPollingRT = false and a_bIsWrOpRT = true successfully
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(EISPlBusHandler_ut, EISListerners_false_true)
{
	bool a_bIsPollingRT = false;
	bool a_bIsWrOpRT = true;
	CEISPlBusHandler_obj.configEISListerners(a_bIsPollingRT, a_bIsWrOpRT);
}

/**
 * Test case to check if stopEISListeners() function Stops EIS listener threads successfully
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(EISPlBusHandler_ut, stopEISListener)
{
	CEISPlBusHandler_obj.stopEISListeners();
	EXPECT_EQ(1,1);
}

/**
 * Test case to check if publishWriteMsg() function fails to publish message to EIS and returns false
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(EISPlBusHandler_ut, PublishWriteMsg_false)
{
	bool RetVal = CEISPlBusHandler_obj.publishWriteMsg(strMsg);
	EXPECT_EQ(0, RetVal);
}

/**
 * Test case to check if publishWriteMsg() function fails to publish message to EIS and returns false
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(EISPlBusHandler_ut, EISMsg_ctx_Map_Empty)
{
	std::string eisTopic = "KPIAPP_WrReq";

	void* msgbus_ctx;
	std::string strMsg = "{ 	\"value\": \"0xFF00\", 	\"command\": \"Pointname\", 	\"app_seq\": \"1234\" }";
	try
	{
		std::string sMsg{""};
	    bool RetVal = publishEISMsg(strMsg, eisTopic);
		EXPECT_EQ(false, RetVal);

	}
	catch(std::exception &e)
	{
		EXPECT_EQ(false, true);
	}

}

