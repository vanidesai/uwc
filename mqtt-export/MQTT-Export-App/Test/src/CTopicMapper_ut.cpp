/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../include/CTopicMapper_ut.hpp"
#include <string>
#include <vector>


using namespace std;

void CTopicMapper_ut::SetUp()
{
	// Setup code
}

void CTopicMapper_ut::TearDown()
{
	// TearDown code
}

//test 01:: this test returns ZMQ topic which has been mapped with MQTT
TEST_F(CTopicMapper_ut, 0_get_zmq_topic)
{
	cout<<endl<<"TEST_0_manatory_param"<<endl;

	string returnVal = CTopicMapper::getInstance().GetZMQTopic("/iou/PL01/KeepAlive/write");

	std::cout << "returnVal : " << returnVal << "\n";

	EXPECT_EQ("PL01_iou_write", returnVal);
}

//test 02:: this test returns "" if ZMQ topic is not found in topic mapper
TEST_F(CTopicMapper_ut, 1_get_zmq_topic_negative)
{
	cout<<endl<<"TEST_1_manatory_param"<<endl;
	string returnVal = CTopicMapper::getInstance().GetZMQTopic("test_topic");

	std::cout << "returnVal : " << returnVal << "\n";

	EXPECT_EQ("", returnVal);
}

//test 03:: this test returns "" if respective MQTT topic is not found in topic mapper
TEST_F(CTopicMapper_ut, 2_manatory_param)
{
	cout<<endl<<"TEST_2_manatory_param"<<endl;
	string returnVal = CTopicMapper::getInstance().GetZMQTopic("No_ZMQ_for_Mqtt");

	std::cout << "returnVal : " << returnVal << "\n";

	EXPECT_EQ("", returnVal);
}

//test 04:: this test returns ZMQ topic which has been mapped with MQTT
TEST_F(CTopicMapper_ut, 3_manatory_param)
{
	cout<<endl<<"TEST_3_manatory_param"<<endl;
	string returnVal = CTopicMapper::getInstance().GetMQTTopic("PL0_flowmeter1_Point2");

	std::cout << "returnVal : " << returnVal << "\n";

	EXPECT_EQ("", returnVal);
}

//test 05:: this test returns "" if ZMQ topic is not found in topic mapper
TEST_F(CTopicMapper_ut, 4_manatory_param)
{
	cout<<endl<<"TEST_4_manatory_param"<<endl;
	string returnVal = CTopicMapper::getInstance().GetMQTTopic("test_topic");

	std::cout << "returnVal : " << returnVal << "\n";

	EXPECT_EQ("", returnVal);
}

//test 06:: this test returns "" if respective MQTT topic is not found in topic mapper
TEST_F(CTopicMapper_ut, 5_manatory_param)
{
	cout<<endl<<"TEST_5_manatory_param"<<endl;
	string returnVal = CTopicMapper::getInstance().GetMQTTopic("No_ZMQ_for_Mqtt");

	std::cout << "returnVal : " << returnVal << "\n";

	EXPECT_EQ("", returnVal);
}

//test 07:: this test returns "" if respective MQTT topic is not found in topic mapper
TEST_F(CTopicMapper_ut, 6_manatory_param)
{
	cout<<endl<<"TEST_6_manatory_param"<<endl;
	std::vector<std::string> allMqttTopic;
	allMqttTopic.push_back("/iou/PL01/KeepAlive/write");

	std::vector<std::string> returnVal = CTopicMapper::getInstance().GetMqttTopics();

	for(auto val : returnVal) {
		std::cout << "val : " << val << "\n";
	}

	EXPECT_EQ(allMqttTopic, returnVal);
}

//test 07:: this test returns blank if Mapping key is missing from topics json
TEST_F(CTopicMapper_ut, 7_mapping_key_missing)
{
	cout<<endl<<"TEST_7_mapping_key_missing"<<endl;

	string returnVal = CTopicMapper::getInstance().GetZMQTopic("/iou/PL01/KeepAlive/write");

	std::cout << "*************************************** returnVal : " << returnVal << "\n";

	EXPECT_EQ("", returnVal);
}
