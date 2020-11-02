/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../include/MQTTPublishHandler_ut.hpp"

void MQTTPublishHandler_ut::SetUp() {
	// Setup code
}

void MQTTPublishHandler_ut::TearDown() {
	// TearDown code
}

TEST_F(MQTTPublishHandler_ut, createNPubMsg_EmptyTopic)
{
	CMQTTPublishHandler mqttPublisher_ut("tcp://mqtt_test_container:1883", EmptyTopic, 1);
	EXPECT_EQ( false, mqttPublisher_ut.createNPubMsg(ValidMsg, EmptyTopic) );
}

TEST_F(MQTTPublishHandler_ut, createNPubMsg_EmptyMsg)
{
	CMQTTPublishHandler mqttPublisher_ut("tcp://mqtt_test_container:1883", EmptyTopic, 1);
	EXPECT_EQ( false, mqttPublisher_ut.createNPubMsg(EmptyMsg, ValidTopic) );

}

TEST_F(MQTTPublishHandler_ut, createNPubMsg_ValidTopic_ValidMsg)
{
	CMQTTPublishHandler mqttPublisher_ut("tcp://mqtt_test_container:1883", ValidTopic, 1);
	EXPECT_EQ( true, mqttPublisher_ut.createNPubMsg(ValidMsg, ValidTopic) );

}
