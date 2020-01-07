/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#include "../include/CTimeRecord_ut.h"

void CTimeRecord_ut::SetUp()
{
	// Setup code
}

void CTimeRecord_ut::TearDown()
{
	// TearDown code
}

/**************************CTimeRecord::add()*************************************/

 /*test01 : this test for add function of class CTimeRecord is to check whether
      the timer record is available or not and throws exception accordingly  */

TEST_F(CTimeRecord_ut, time_record_available)
{

	CRefDataForPolling CRefDataForPolling_obj1{CUniqueDataPoint_obj,a_BusContext,a_uiFuncCode};
	CRefDataForPolling CRefDataForPolling_obj2{CUniqueDataPoint_obj,a_BusContext,a_uiFuncCode};
	CRefDataForPolling CRefDataForPolling_obj3{CUniqueDataPoint_obj,a_BusContext,a_uiFuncCode};
	CRefDataForPolling CRefDataForPolling_obj4{CUniqueDataPoint_obj,a_BusContext,a_uiFuncCode};
    CTimeRecord CTimeRecord_obj{U32_code, CRefDataForPolling_obj1};


	 m_PolledPoints.push_back(CRefDataForPolling_obj1);
	 m_PolledPoints.push_back(CRefDataForPolling_obj2);
	 m_PolledPoints.push_back(CRefDataForPolling_obj3);
	 m_PolledPoints.push_back(CRefDataForPolling_obj4);

	 cout << "elements of vector m_PolledPoints..." << endl;
	 std::string id = CUniqueDataPoint_obj.getID();

	    try
	    {
	    	bRet = CTimeRecord_obj.add(CRefDataForPolling_obj1);
	    	EXPECT_EQ(true, bRet);
	    }
	    catch(exception &e)
	    {
	    	bRet=false;
	    	EXPECT_EQ(false, bRet);
	    }



}
/*test02 : this test for add function of class CTimeRecord is to check whether
     the timer record is available or not and throws exception accordingly  (In this test the time record is not available)*/

TEST_F(CTimeRecord_ut, time_record_notavailable)
{


	CRefDataForPolling CRefDataForPolling_obj1{CUniqueDataPoint_obj,a_BusContext,a_uiFuncCode};
	CRefDataForPolling CRefDataForPolling_obj2{CUniqueDataPoint_obj,a_BusContext,a_uiFuncCode};
	CRefDataForPolling CRefDataForPolling_obj3{CUniqueDataPoint_obj,a_BusContext,a_uiFuncCode};
	CRefDataForPolling CRefDataForPolling_obj4{CUniqueDataPoint_obj,a_BusContext,a_uiFuncCode};
	CRefDataForPolling CRefDataForPolling_obj5{CUniqueDataPoint_obj,a_BusContext,a_uiFuncCode};
    CTimeRecord CTimeRecord_obj{U32_code, CRefDataForPolling_obj1};


	 m_PolledPoints.push_back(CRefDataForPolling_obj1);
	 m_PolledPoints.push_back(CRefDataForPolling_obj2);
	 m_PolledPoints.push_back(CRefDataForPolling_obj3);
	 m_PolledPoints.push_back(CRefDataForPolling_obj4);

	// cout << "elements of vector m_PolledPoints..." << endl;
	 std::string id = CUniqueDataPoint_obj.getID();

    	//CUniqueDataPoint_obj = CRefDataForPolling_obj1.getDataPoint();

  for(auto x: m_PolledPoints )
  {
	    try
	    {
	    	bRet = CTimeRecord_obj.add(CRefDataForPolling_obj5);
	    	EXPECT_EQ(true, bRet);

	    }
	    catch(exception &e)
	    {
	    	bRet=false;
	    	EXPECT_EQ(false, bRet);

	    }
  }

}





