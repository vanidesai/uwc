/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/


#include "../include/PublishJson_ut.hpp"

void PublishJson_ut::SetUp()
{
	// Setup code
}

void PublishJson_ut::TearDown()
{
	// TearDown code
}

/**
 * Test case to check the behaviour of isElementExistInJson() with NULL argument
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(PublishJson_ut, element_exist_inJSON)
{
	cJSON *root = NULL;
	std::string a_sKeyName = "datapoints" ;
	bool result = false;
	try
	{
		result = isElementExistInJson(root, a_sKeyName);
		EXPECT_EQ(false, result);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("", e.what());
	}
}

/**
 * Test case to check the behaviour of setTopicForOperation() with empty topic
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(PublishJson_ut, setTopicForOperation_emptyTopic)
{
	bool result;
	std::string topic;
	result = PublishJsonHandler::instance().setTopicForOperation(topic);
	EXPECT_EQ(false, result);
}

/**
 * Test case to check the behaviour of setTopicForOperation() with valid non rt topic
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(PublishJson_ut, setTopicForOperation_ValNonRTTopic)
{
	bool result;
	std::string topic = "TCP1_RdResp";
	result = PublishJsonHandler::instance().setTopicForOperation(topic);
	EXPECT_EQ(true, result);
}

/**
 * Test case to check the behaviour of setTopicForOperation() with valid rt topic
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(PublishJson_ut, setTopicForOperation_ValRTTopic)
{
	bool result;
	std::string topic = "TCP1_RdResp_RT";
	result = PublishJsonHandler::instance().setTopicForOperation(topic);
	EXPECT_EQ(true, result);
}

/**
 * Test case to check the behaviour of setTopicForOperation() with valid wrResp topic
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(PublishJson_ut, setTopicForOperation_WrRes)
{
	bool result;
	std::string topic = "TCP1_WrResp";
	result = PublishJsonHandler::instance().setTopicForOperation(topic);
	EXPECT_EQ(true, result);
}

/**
 * Test case to check the behaviour of setTopicForOperation() with valid wrResp rt topic
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(PublishJson_ut, setTopicForOperation_WRITE_RES_RT)
{
	bool result;
	std::string topic = "TCP1_WrResp_RT";
	result = PublishJsonHandler::instance().setTopicForOperation(topic);
	EXPECT_EQ(true, result);
}

/**
 * Test case to check the behaviour of setTopicForOperation() with polled data topic
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(PublishJson_ut, setTopicForOperation_PolledData)
{
	bool result;
	std::string topic = "TCP1_PolledData";
	result = PublishJsonHandler::instance().setTopicForOperation(topic);
	EXPECT_EQ(true, result);
}

/**
 * Test case to check the behaviour of setTopicForOperation() with polled data rt topic
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(PublishJson_ut, setTopicForOperation_PolledData_RT)
{
	bool result;
	std::string topic = "TCP1_PolledData_RT";
	result = PublishJsonHandler::instance().setTopicForOperation(topic);
	EXPECT_EQ(true, result);
}
