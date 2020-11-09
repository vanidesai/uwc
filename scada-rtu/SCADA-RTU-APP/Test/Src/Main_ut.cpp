/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../Inc/Main_ut.hpp"

void Main_ut::SetUp()
{
	// Setup code
}

void Main_ut::TearDown()
{
	// TearDown code
}

/****************************************************************/
/* Test helper function:
 * Sets global variable g_shouldStop to true after 1 sec,
 * So that function timerThread doesnt run infinitely.
 */
static void Set_g_shouldStop()
{
	std::this_thread::sleep_for(std::chrono::seconds(2));
	g_shouldStop = true;
}

void Call_PushMsg(CQueueHandler& Qu, mqtt::const_message_ptr& msg)
{
	std::this_thread::sleep_for(std::chrono::seconds(1));
	Qu.pushMsg(msg);
	g_shouldStop = true;
}
/****************************************************************/

TEST_F(Main_ut, processInternalMQTTMsg_DeathMsg)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	CVendorAppList CVendorAppList_obj;
	CSparkPlugDev CSparkPlugDev_obj("A", "A", false);

	CVendorAppList_obj.addDevice("A", CSparkPlugDev_obj);

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"Death/A",
			"B",
			stRefActionVec);

	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_DeathMsgInvalid)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"A/A",
			"B",
			stRefActionVec);

	EXPECT_EQ(false, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_BirthMsg)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"Birth/A/B",
			"{\"metrics\": [{\"name\":\"UtData01\", \"dataType\":\"uint8\", \"value\": 100}],\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);

	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_DataMsg_Uint8)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"Data/A/B",
			"{\"metrics\": [{\"name\":\"UtData01\", \"dataType\":\"uint8\", \"value\": 100}],\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);

	CSCADAHandler::instance().prepareSparkPlugMsg(stRefActionVec);

	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_BirthMsg_repeated_sameDataType)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"Birth/A/B",
			"{\"metrics\": [{\"name\":\"UtData01\", \"dataType\":\"uint8\", \"value\": 100}],\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);

	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_BirthMsg_repeated_DifferentDataType)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"Birth/A/B",
			"{\"metrics\": [{\"name\":\"UtData01\", \"dataType\":\"int8\", \"value\": 9211}],\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);

	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_BirthMsg_repeated_DifferentValue)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"Birth/A/B",
			"{\"metrics\": [{\"name\":\"UtData01\", \"dataType\":\"int8\", \"value\": 420}],\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);

	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_DataMsg_Bool)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"Data/A/B",
			"{\"metrics\": [{\"name\":\"UtData01\", \"dataType\":\"boolean\", \"value\": true}],\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);

	CSCADAHandler::instance().prepareSparkPlugMsg(stRefActionVec);

	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_DataMsg_uint16)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"Data/A/B",
			"{\"metrics\": [{\"name\":\"UtData01\", \"dataType\":\"uint16\", \"value\": 111}],\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);

	CSCADAHandler::instance().prepareSparkPlugMsg(stRefActionVec);

	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_DataMsg_uint32)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"Data/A/B",
			"{\"metrics\": [{\"name\":\"UtData01\", \"dataType\":\"uint32\", \"value\": 111}],\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);

	CSCADAHandler::instance().prepareSparkPlugMsg(stRefActionVec);

	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_DataMsg_uint64)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"Data/A/B",
			"{\"metrics\": [{\"name\":\"UtData01\", \"dataType\":\"uint64\", \"value\": 111}],\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);

	CSCADAHandler::instance().prepareSparkPlugMsg(stRefActionVec);

	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_DataMsg_int8)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"Data/A/B",
			"{\"metrics\": [{\"name\":\"UtData01\", \"dataType\":\"int8\", \"value\": 111}],\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);

	CSCADAHandler::instance().prepareSparkPlugMsg(stRefActionVec);

	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_DataMsg_int16)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"Data/A/B",
			"{\"metrics\": [{\"name\":\"UtData01\", \"dataType\":\"int16\", \"value\": 111}],\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);

	CSCADAHandler::instance().prepareSparkPlugMsg(stRefActionVec);

	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_DataMsg_int32)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"Data/A/B",
			"{\"metrics\": [{\"name\":\"UtData01\", \"dataType\":\"int32\", \"value\": 111}],\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);

	CSCADAHandler::instance().prepareSparkPlugMsg(stRefActionVec);

	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_DataMsg_int64)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"Data/A/B",
			"{\"metrics\": [{\"name\":\"UtData01\", \"dataType\":\"int64\", \"value\": 111}],\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);

	CSCADAHandler::instance().prepareSparkPlugMsg(stRefActionVec);

	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_DataMsg_float)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"Data/A/B",
			"{\"metrics\": [{\"name\":\"UtData01\", \"dataType\":\"float\", \"value\": 111}],\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);

	CSCADAHandler::instance().prepareSparkPlugMsg(stRefActionVec);

	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_DataMsg_double)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"Data/A/B",
			"{\"metrics\": [{\"name\":\"UtData01\", \"dataType\":\"double\", \"value\": 111}],\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);

	CSCADAHandler::instance().prepareSparkPlugMsg(stRefActionVec);

	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_DataMsg_string)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"Data/A/B",
			"{\"metrics\": [{\"name\":\"UtData01\", \"dataType\":\"string\", \"value\": \"StringValue\"}],\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);

	CSCADAHandler::instance().prepareSparkPlugMsg(stRefActionVec);

	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_DataMsg_InvVal)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"Data/A/B",
			"{\"metrics\": [{\"name\":\"UtData01\", \"dataType\":\"double\", \"value\": \"111\"}],\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);

	CSCADAHandler::instance().prepareSparkPlugMsg(stRefActionVec);

	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_BirthDataMsgInvalid)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"A/B/C",
			"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);

	EXPECT_EQ(false, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_MessageData)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	/*Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"RealDev/A/B/D/Update",
			"{\"metric\": \"UtData02\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);*/

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
				"RealDev/A/B/D/Update",
				"{\"metric\": \"UtData01\", \"status\": \"bad\", \"value\": \"0x00\", \"usec\": \"1571887474111145\", \"lastGoodUsec\": \"val1\", \"error_code\": \"2002\"}",
				stRefActionVec);

	//Does nothing
	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_MessageData_DataMissingInPayload_01)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"A/B/C/D/Update",
			"{\"NoMetric\": \"UtData02\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);

	//Does nothing
	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_MessageData_DataMissingInPayload_02)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"A/B/C/D/Update",
			"{\"metric\": [{\"name\":\"UtData01\"}],\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);

	//Does nothing
	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_MessageData_DataMissingInPayload_03)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"A/B/C/D/Update",
			"{\"metric\": \"UtData02\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp_Invalid\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);

	//Does nothing
	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_MessageData_DataMissingInPayload_04)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"A/B/C/D/Update",
			"{\"metric\": \"UtData02\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);

	//Does nothing
	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_MessageData_DataMissingInPayload_05)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"A/B/C/D/Update",
			"{\"metric\": \"UtData02\",\"command\": \"D1\",\"value_Invalid\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);

	//Does nothing
	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_MessageDataArrayPayload)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"A/B/C/D/Update",
			"[{\"name\": \"UtData02\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\", \"dataType\": \"uint8\"}]",
			stRefActionVec);

	//Does nothing
	EXPECT_EQ(true, Bool_Res);
}


TEST_F(Main_ut, processInternalMQTTMsg_MessageDataArrayPayloadNameMissing)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"A/B/C/D/Update",
			"[{\"name_Invalid\": \"UtData02\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\", \"dataType\": \"uint8\"}]",
			stRefActionVec);

	//Does nothing
	EXPECT_EQ(true, Bool_Res);
}


TEST_F(Main_ut, processInternalMQTTMsg_MessageDataArrayPayloadNameValInv)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"A/B/C/D/Update",
			"[{\"name\": 100,\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\", \"dataType\": \"uint8\"}]",
			stRefActionVec);

	//Does nothing
	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_MessageDataArrayTimestampMissing)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"A/B/C/D/Update",
			"[{\"name\": \"UtData02\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp_Invalid\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\", \"dataType\": \"uint8\"}]",
			stRefActionVec);

	//Does nothing
	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_MessageDataArrayDataTypeMissing)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"A/B/C/D/Update",
			"[{\"name\": \"UtData02\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\", \"dataType_Invalid\": \"uint8\"}]",
			stRefActionVec);

	//Does nothing
	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_MessageDataArrayDataTypeValInv)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"A/B/C/D/Update",
			"[{\"name\": \"UtData02\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\", \"dataType\": 100}]",
			stRefActionVec);

	//Does nothing
	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_MessageDataArrayValueMissing)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"A/B/C/D/Update",
			"[{\"name\": \"UtData02\",\"command\": \"D1\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\", \"dataType\": \"uint8\"}]",
			stRefActionVec);

	//Does nothing
	EXPECT_EQ(true, Bool_Res);
}

TEST_F(Main_ut, processInternalMQTTMsg_InvalidTopic)
{
	std::vector<stRefForSparkPlugAction> stRefActionVec;

	Bool_Res = CSparkPlugDevManager::getInstance().processInternalMQTTMsg(
			"A",
			"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"1\"}",
			stRefActionVec);

	//Does nothing
	EXPECT_EQ(false, Bool_Res);
}

TEST_F(Main_ut, processInternalMqttMsgs_DeathMsg)
{
	mqtt::const_message_ptr msg = mqtt::make_message("Death/A", "Msg_UT", 0, false);
	CQueueHandler Qu;

	std::thread TestTarget( processInternalMqttMsgs, std::ref(Qu) );
	std::thread TestHelperPostSem( Call_PushMsg, std::ref(Qu), std::ref(msg) );

	TestTarget.join();
	TestHelperPostSem.join();

	// Setting the value of "g_shouldStop back" to default value
	g_shouldStop = false;
	Qu.cleanup();
}

TEST_F(Main_ut, processInternalMqttMsgs_BirthMsg)
{
	mqtt::const_message_ptr msg = mqtt::make_message("Birth/A", "Msg_UT", 0, false);
	CQueueHandler Qu;

	std::thread TestTarget( processInternalMqttMsgs, std::ref(Qu) );
	std::thread TestHelperPostSem( Call_PushMsg, std::ref(Qu), std::ref(msg) );

	TestTarget.join();
	TestHelperPostSem.join();

	// Setting the value of "g_shouldStop back" to default value
	g_shouldStop = false;
	Qu.cleanup();
}

TEST_F(Main_ut, processExternalMqttMsgs_venderapp)
{

	org_eclipse_tahu_protobuf_Payload ddata_payload;
	get_next_payload(&ddata_payload);

	size_t buffer_length = 128;
	uint8_t *binary_buffer = (uint8_t *)malloc(buffer_length * sizeof(uint8_t));
	size_t message_length = encode_payload(binary_buffer, buffer_length, &ddata_payload);

	mqtt::const_message_ptr msg = mqtt::make_message("spBv1.0/Sparkplug B Devices/DCMD/C Edge Node 1/A-B",
			(char*)binary_buffer,
			0,
			false);

	CQueueHandler Qu;
	Qu.pushMsg(msg);

	std::thread TestTarget( processExternalMqttMsgs, std::ref(Qu) );
	std::thread TestHelperPostSem( Call_PushMsg, std::ref(Qu), std::ref(msg) );

	TestTarget.join();
	TestHelperPostSem.join();

	// Setting the value of "g_shouldStop back" to default value
	g_shouldStop = false;
}


