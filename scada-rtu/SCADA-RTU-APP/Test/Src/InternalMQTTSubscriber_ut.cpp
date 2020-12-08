/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/


#include "../Inc/InternalMQTTSubscriber_ut.hpp"

#include <iostream>
using namespace std;


void InternalMQTTSubscriber_ut::SetUp()
{
	// Setup code
}

void InternalMQTTSubscriber_ut::TearDown()
{
	// TearDown code
}

/**
 * Test case to check if prepareCJSONMsg() behaves as expected
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(InternalMQTTSubscriber_ut, prepareCJSONMsg_EmptyVector)
{
	bool result = CIntMqttHandler::instance().prepareCJSONMsg(stRefActionVec);
	EXPECT_EQ(true, result);
}


TEST_F(InternalMQTTSubscriber_ut, writeRequest)
{
	/*std::reference_wrapper<CSparkPlugDev>& a_refSparkPlugDev;
	metricMap_t m_mapChangedMetrics;
	bool result = CIntMqttHandler.instance().prepareWriteMsg(a_refSparkPlugDev, m_mapChangedMetrics);*/

	/*bool result;
	for (auto &itr : stRefActionVec)
	{
		metricMap_t m_metrics = itr.m_mapChangedMetrics;
		result = CIntMqttHandler::instance().prepareWriteMsg(itr.m_refSparkPlugDev, itr.m_mapChangedMetrics);
	}*/
	/*std::cout<<"########################################"<<std::endl;
		std::cout<<result<<std::endl;
		std::cout<<"########################################"<<std::endl;*/

}
