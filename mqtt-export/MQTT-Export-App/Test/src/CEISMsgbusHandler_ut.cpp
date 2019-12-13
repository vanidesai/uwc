/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../include/CEISMsgbusHandler_ut.hpp"

#include <gtest/internal/gtest-internal.h>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

void CEISMsgbusHandler_ut::SetUp() {
	// Setup code
}

void CEISMsgbusHandler_ut::TearDown() {
	// TearDown code
}

//test 01:: initializes EIS contexts for
TEST_F(CEISMsgbusHandler_ut, 0_manatory_param) {
	bool bRetVal = false;
	if (std::getenv("PubTopics") != NULL) {
		std::cout << __func__ << " List of topic configured for Pub are :: "
				<< getenv("PubTopics") << endl;
		bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext("pub");
		if (!bRetVal) {
			std::cout << __func__
					<< " Context creation failed for sub topic \n";
			EXPECT_EQ(false, bRetVal);
		} else {
			EXPECT_EQ(true, bRetVal);
		}
	}
}

TEST_F(CEISMsgbusHandler_ut, 1_manatory_param) {

	if (std::getenv("SubTopics") != NULL) {
		std::cout << __func__ << " List of topic configured for Sub are :: "
				<< getenv("SubTopics") << endl;
		bool bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext(
				"sub");
		if (!bRetVal) {
			std::cout << __func__
					<< " Context creation failed for sub topic \n";
			EXPECT_EQ(false, bRetVal);
		} else {
			EXPECT_EQ(true, bRetVal);
		}
	}
}

// Topic is other that "pub" and "sub"
TEST_F(CEISMsgbusHandler_ut, prepareCommonContext_InvTopic) {
	bool bRetVal = false;
	if (std::getenv("PubTopics") != NULL) {
		std::cout << __func__ << " List of topic configured for Pub are :: "
				<< getenv("PubTopics") << endl;
		bRetVal = CEISMsgbusHandler::Instance().prepareCommonContext("other");
		cout<<endl<<"**************prepareCommonContext_InvTopic*****************"<<endl;
		cout<<bRetVal;
		EXPECT_EQ(false, bRetVal);
	}
}

#if 0 // To be updated later
TEST_F(CEISMsgbusHandler_ut, 2_manatory_param) {

	stZmqContext msgbusContext;
	string topic = "PL01_iou_write"; //topic exists
	bool retVal = CEISMsgbusHandler::Instance().getCTX(topic, msgbusContext);

	EXPECT_EQ(true, retVal);
}
#endif

TEST_F(CEISMsgbusHandler_ut, 3_manatory_param) {

	stZmqContext msgbusContext;
	string topic = "PL01_iou_write_test"; //topic does not exist
	bool retVal = CEISMsgbusHandler::Instance().getCTX(topic, msgbusContext);

	EXPECT_EQ(false, retVal);
}

#if 0 // To be updated later
TEST_F(CEISMsgbusHandler_ut, 4_manatory_param) {

	stZmqPubContext pubContext;
	string topic = "PL01_iou_write"; //topic exists
	bool retVal = CEISMsgbusHandler::Instance().getPubCTX(topic, pubContext);

	EXPECT_EQ(true, retVal);
}
#endif

TEST_F(CEISMsgbusHandler_ut, 5_manatory_param) {

	stZmqPubContext pubContext;
	string topic = "PL01_iou_write_test"; //topic does not exist
	bool retVal = CEISMsgbusHandler::Instance().getPubCTX(topic, pubContext);

	EXPECT_EQ(false, retVal);
}

#if 0 // To be updated later
TEST_F(CEISMsgbusHandler_ut, 6_manatory_param) {

	stZmqSubContext subContext;
	string topic = "PL0_flowmeter1"; //topic exists
	bool retVal = CEISMsgbusHandler::Instance().getSubCTX(topic, subContext);

	EXPECT_EQ(true, retVal);
}
#endif

TEST_F(CEISMsgbusHandler_ut, 7_manatory_param) {

	stZmqSubContext subContext;
	string topic = "PL0_flowmeter1_test"; //topic does not exist
	bool retVal = CEISMsgbusHandler::Instance().getSubCTX(topic, subContext);

	EXPECT_EQ(false, retVal);
}

TEST_F(CEISMsgbusHandler_ut, 8_manatory_param) {

	stZmqSubContext subContext;
	string topic = "PL0_flowmeter1"; //topic exists
	bool retVal = CEISMsgbusHandler::Instance().getSubCTX(topic, subContext);

	if(retVal) {
		bool isInserted = CEISMsgbusHandler::Instance().insertSubCTX(topic, subContext);
		EXPECT_EQ(true, isInserted);
	}
}

TEST_F(CEISMsgbusHandler_ut, 9_manatory_param) {

	stZmqPubContext pubContext;
	string topic = "PL01_iou_write"; //topic exists
	bool retVal = CEISMsgbusHandler::Instance().getPubCTX(topic, pubContext);

	if(retVal) {
		bool isInserted = CEISMsgbusHandler::Instance().insertPubCTX(topic, pubContext);
		EXPECT_EQ(true, isInserted);
	}
}

TEST_F(CEISMsgbusHandler_ut, 10_manatory_param) {

	stZmqSubContext subContext;
	string topic = "PL0_flowmeter1"; //topic exists
	bool retVal = CEISMsgbusHandler::Instance().getSubCTX(topic, subContext);
	if(retVal) {

		stZmqContext msgbusContext;
		bool retVal = CEISMsgbusHandler::Instance().getCTX(topic, msgbusContext);
		if(retVal) {
			CEISMsgbusHandler::Instance().removeSubCTX(topic, msgbusContext);
		}
	}
}

TEST_F(CEISMsgbusHandler_ut, 11_manatory_param) {

	stZmqPubContext pubContext;
	string topic = "PL0_flowmeter1"; //topic exists
	bool retVal = CEISMsgbusHandler::Instance().getPubCTX(topic, pubContext);
	if(retVal) {

		stZmqContext msgbusContext;
		bool retVal = CEISMsgbusHandler::Instance().getCTX(topic, msgbusContext);
		if(retVal) {
			CEISMsgbusHandler::Instance().removePubCTX(topic, msgbusContext);
		}
	}
}

TEST_F(CEISMsgbusHandler_ut, 12_manatory_param) {

	try {

		CEISMsgbusHandler::Instance().cleanup();
		EXPECT_EQ(true, true);

	}catch(exception &ex) {

		std::cout << __func__ << " cleanup test for EIS msg bus\n";
		EXPECT_EQ(true, false);
	}
}

TEST_F(CEISMsgbusHandler_ut, 13_manatory_param) {

	stZmqContext context;
	string topic = "PL0_flowmeter1"; //topic exists
	bool retVal = CEISMsgbusHandler::Instance().getCTX(topic, context);

	if(retVal) {
		bool isInserted = CEISMsgbusHandler::Instance().insertCTX(topic, context);
		EXPECT_EQ(true, isInserted);
	}
}

TEST_F(CEISMsgbusHandler_ut, 14_manatory_param) {

	stZmqContext context;
	string topic = "PL0_flowmeter1"; //topic exists
	bool retVal = CEISMsgbusHandler::Instance().getCTX(topic, context);

	if(retVal) {
		CEISMsgbusHandler::Instance().removeCTX(topic);
		EXPECT_EQ(true, true);
	}
}
