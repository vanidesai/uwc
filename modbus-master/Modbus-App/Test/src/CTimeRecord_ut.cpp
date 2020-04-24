/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../include/CTimeRecord_ut.hpp"

void CTimeRecord_ut::SetUp()
{
	// Setup code
}

void CTimeRecord_ut::TearDown()
{
	// TearDown code
}

/**************************CTimeRecord::add()*************************************/

/***CTimeRecord_ut::time_record_available***/
/*Test01 : This test for add function of class CTimeRecord is to check whether
      the timer record is available or not and throws exception accordingly  */

TEST_F(CTimeRecord_ut, time_record_available)
{

	try
	{
		CRefDataForPolling CRefDataForPolling_obj1{CUniqueDataPoint_obj,a_BusContext, a_objPubContext, a_uiFuncCode};
		CRefDataForPolling CRefDataForPolling_obj2{CUniqueDataPoint_obj,a_BusContext, a_objPubContext, a_uiFuncCode};
		CRefDataForPolling CRefDataForPolling_obj3{CUniqueDataPoint_obj,a_BusContext, a_objPubContext, a_uiFuncCode};
		CRefDataForPolling CRefDataForPolling_obj4{CUniqueDataPoint_obj,a_BusContext, a_objPubContext, a_uiFuncCode};
		CTimeRecord CTimeRecord_obj{U32_code, CRefDataForPolling_obj1};


		m_PolledPoints.push_back(CRefDataForPolling_obj1);
		m_PolledPoints.push_back(CRefDataForPolling_obj2);
		m_PolledPoints.push_back(CRefDataForPolling_obj3);
		m_PolledPoints.push_back(CRefDataForPolling_obj4);

		std::string id = CUniqueDataPoint_obj.getID();


		bRet = CTimeRecord_obj.add(CRefDataForPolling_obj1);
		EXPECT_EQ(true, bRet);
	}
	catch(exception &e)
	{
		bRet=false;
		EXPECT_EQ(false, bRet);
	}

}

/***CTimeRecord_ut::time_record_notavailable***/
/*Test02 : This test for add function of class CTimeRecord is to check whether
     the timer record is available or not and throws exception accordingly  (In this test the time record is not available)*/

TEST_F(CTimeRecord_ut, time_record_notavailable)
{

	try
	{
		CRefDataForPolling CRefDataForPolling_obj1{CUniqueDataPoint_obj,a_BusContext, a_objPubContext, a_uiFuncCode};
		CRefDataForPolling CRefDataForPolling_obj2{CUniqueDataPoint_obj,a_BusContext, a_objPubContext, a_uiFuncCode};
		CRefDataForPolling CRefDataForPolling_obj3{CUniqueDataPoint_obj,a_BusContext, a_objPubContext, a_uiFuncCode};
		CRefDataForPolling CRefDataForPolling_obj4{CUniqueDataPoint_obj,a_BusContext, a_objPubContext, a_uiFuncCode};
		CRefDataForPolling CRefDataForPolling_obj5{CUniqueDataPoint_obj,a_BusContext, a_objPubContext, a_uiFuncCode};
		CTimeRecord CTimeRecord_obj{U32_code, CRefDataForPolling_obj1};


		m_PolledPoints.push_back(CRefDataForPolling_obj1);
		m_PolledPoints.push_back(CRefDataForPolling_obj2);
		m_PolledPoints.push_back(CRefDataForPolling_obj3);
		m_PolledPoints.push_back(CRefDataForPolling_obj4);

		std::string id = CUniqueDataPoint_obj.getID();

		for(auto x: m_PolledPoints )
		{

			bRet = CTimeRecord_obj.add(CRefDataForPolling_obj5);
			EXPECT_EQ(true, bRet);

		}
	}
	catch(exception &e)
	{
		bRet=false;
		EXPECT_EQ(false, bRet);

	}


}

#if 0 //in progres
TEST_F(CTimeRecord_ut, IntervalTimer_counter_test)
{
	uint32_t Interval;
	CRefDataForPolling CRefDataForPolling_obj{CUniqueDataPoint_obj,a_BusContext, a_objPubContext, a_uiFuncCode};
	CTimeRecord CTimeRecord_obj{U32_code, CRefDataForPolling_obj};
	Interval =  CTimeRecord_obj.getIntervalTimerCounter();
}

TEST_F(CTimeRecord_ut, CutoffIntervalTimerCounter_ut)
{
	uint32_t cutoff;
	CRefDataForPolling CRefDataForPolling_obj{CUniqueDataPoint_obj,a_BusContext, a_objPubContext, a_uiFuncCode};
	CTimeRecord CTimeRecord_obj{U32_code, CRefDataForPolling_obj};
	cutoff = CTimeRecord_obj.getCutoffIntervalTimerCounter();

}
#endif

