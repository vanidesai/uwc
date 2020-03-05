/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#include "../include/PeriodicRead_ut.hpp"

#include <typeinfo>

extern void getTimeBasedParams(const CRefDataForPolling& a_objReqData, std::string &a_sTimeStamp, std::string &a_sUsec, std::string &a_sTxID);

void PeriodicRead_ut::SetUp()
{
	// Setup code
}

void PeriodicRead_ut::TearDown()
{
	// TearDown code
}



void getTimeStamps(std::string &a_sTimeStamp, std::string &a_sUsec);
eMbusStackErrorCode readPeriodicCallBack(uint8_t  u8UnitID,
										 uint16_t u16TransacID,
										 uint8_t* pu8IpAddr,
										 uint8_t  u8FunCode,
										 stException_t  *pstException,
										 uint8_t  u8numBytes,
										 uint8_t* pu8data,
										 uint16_t  u16StartAddress,
										 uint16_t  u16Quantity);

/******************************PeriodicRead::handleResponse*****************************************/

/***Test:PeriodicRead_ut::handleResponse_NULLAgument_Exception()
  Execution should not hang when NULL pointer is passed as an argument.*/

TEST_F(PeriodicRead_ut, handleResponse_NULLAgument_Exception) {

	pstException->m_u8ExcCode = 0;
	pstException->m_u8ExcStatus = 0;

	try
	{
		CPeriodicReponseProcessor::Instance().handleResponse(u8UnitID, u16TransacID, pu8IpAddr, u8FunCode, NULL,
				u8numBytes, pu8data, u16StartAddress, u16Quantity);

		EXPECT_EQ("", test_str);
	}
	catch(exception &e)
	{
		test_str = e.what();
		EXPECT_EQ("", test_str);

	}

}


/*** Test:PeriodicRead_ut::handleResponse_NULLAgument_Data()
  Execution should not hang when NULL pointer is passed as an argument.*/

TEST_F(PeriodicRead_ut, handleResponse_NULLAgument_Data) {

	pstException->m_u8ExcCode = 0;
	pstException->m_u8ExcStatus = 0;

	try
	{
		CPeriodicReponseProcessor::Instance().handleResponse(u8UnitID, u16TransacID, pu8IpAddr, u8FunCode, pstException,
				u8numBytes, NULL, u16StartAddress, u16Quantity);

		EXPECT_EQ("", test_str);
	}
	catch(exception &e)
	{
		test_str = e.what();
		EXPECT_EQ("", test_str);

	}

}

#if 0
/* TC003
Test: Behaviour of handleResponse() when pstException->m_u8ExcCode and pstException->m_u8ExcStatus both are 0
 */
TEST_F(PeriodicRead_ut, handleResponse_ExcStatus_ExcCode_0) {


	pstException->m_u8ExcCode = 0;
	pstException->m_u8ExcStatus = 0;

	try
	{
		CPeriodicReponseProcessor::Instance().handleResponse(u8UnitID, u16TransacID, pu8IpAddr, u8FunCode, pstException,
				8, pu8data, u16StartAddress, u16Quantity);

		EXPECT_EQ("", test_str);

	}
	catch(exception &e)
	{
		test_str = e.what();
		EXPECT_EQ("", test_str);

	}

}

///* TC004
//Test: Execution should not hang when pstException->m_u8ExcStatus or pstException->m_u8ExcCode is not equal to 0
// */
//TEST_F(PeriodicRead_ut, handleResponse_ExcStatus_ExcCode_Non0) {
//
//
//	try
//	{
//		pstException->m_u8ExcCode = 1;
//		pstException->m_u8ExcStatus = 0;
//
//		CPeriodicReponseProcessor::Instance().handleResponse(u8UnitID, u16TransacID, pu8IpAddr, u8FunCode, pstException,
//				8, pu8data, u16StartAddress, u16Quantity);
//
//		pstException->m_u8ExcCode = 0;
//		pstException->m_u8ExcStatus = 1;
//
//		CPeriodicReponseProcessor::Instance().handleResponse(u8UnitID, u16TransacID, pu8IpAddr, u8FunCode, pstException,
//				8, pu8data, u16StartAddress, u16Quantity);
//
//		EXPECT_EQ("", test_str);
//	}
//	catch(exception &e)
//	{
//		test_str = e.what();
//		EXPECT_EQ("", test_str);
//
//	}
//
//}
//#if 0
/* TC005
Test: Behaviour of handleResponse() when pstException->m_u8ExcCode
      and pstException->m_u8ExcStatus both are 0
 */
TEST_F(PeriodicRead_ut, handleResponse_dataSizeLessThanNumOfBytes) {

	try
	{



		pstException->m_u8ExcCode = 0;
		pstException->m_u8ExcStatus = 0;
		u8numBytes = 10;

		CPeriodicReponseProcessor::Instance().handleResponse(u8UnitID, u16TransacID, pu8IpAddr, u8FunCode, pstException,
				u8numBytes, pu8data, u16StartAddress, u16Quantity);

		EXPECT_LT(pu8data[u8numBytes], 9);

		//EXPECT_GT(pu8data[u8numBytes], 0);

	}
	catch(exception &e)
	{

		test_str = e.what();
		EXPECT_EQ("", test_str);


	}

}

/******************************PeriodicRead::isInitialized*****************************************/

/* TC006
Test: Behaviour of isInitialized(): Function should return correct value.
 */
TEST_F(PeriodicRead_ut, isInitialized_RetTrue) {

	try
	{

		pstException->m_u8ExcCode = 0;
		pstException->m_u8ExcStatus = 0;
		u8numBytes = 10;

		Temp_Bool = CPeriodicReponseProcessor::Instance().isInitialized();
		EXPECT_EQ(true, Temp_Bool);
	}
	catch(exception &e)
	{
		test_str = e.what();
		EXPECT_EQ("", test_str);

	}

	/*
	Below test case is not possible:
	isInitialized() returns false.
	*/

}
//#if 0

/******************************PeriodicRead::InitRespHandlerThreads****************************************
TC007
Test: Behaviour of initRespHandlerThreads(): Function executed 1st time
 */
TEST_F(PeriodicRead_ut, initRespHandlerThreads_CalledNTime) {

	try
	{
		/* Called 1st time */
		CPeriodicReponseProcessor::Instance().initRespHandlerThreads();
		EXPECT_EQ("", test_str);
		/* second time: if(false == bSpawned) should be false */
		CPeriodicReponseProcessor::Instance().initRespHandlerThreads();

		EXPECT_EQ("", test_str);
	}
	catch(exception &e)
	{
		test_str = e.what();
		EXPECT_EQ("", test_str);
	}

}

/******************************PeriodicRead::getTimeBasedParams****************************************
TC008
Test: Behaviour of getTimeBasedParams()
 */
TEST_F(PeriodicRead_ut, getTimeBasedParams_return) {

//	network_info::CWellSiteInfo			CWellSiteInfo_obj;
//	network_info::CWellSiteDevInfo		CWellSiteDevInfo_obj;
//	network_info::CDataPoint			CDataPoint_obj;
//
//	string		str1;
//	string		str2;
//	string		str3;
//
//	network_info::CUniqueDataPoint 		CUniqueDataPoint_obj
//	{
//		"Test_String",
//		CWellSiteInfo_obj,
//		CWellSiteDevInfo_obj,
//		CDataPoint_obj
//	};
//	using namespace zmq_handler;
//	struct stZmqContext				stZmqContext_obj;
//
//	CRefDataForPolling CRefDataForPolling_obj{CUniqueDataPoint_obj, stZmqContext_obj, 16};



	try
	{
		getTimeBasedParams(CRefDataForPolling_obj, str1, str2, str3);
		EXPECT_NE("", str1);
		EXPECT_NE("", str2);
		EXPECT_NE("", str3);
	}
	catch(exception &e)
	{
		EXPECT_EQ("", e.what());
	}

}

/******************************PeriodicRead::readPeriodicCallBack****************************************
TC009
Test: Behaviour of readPeriodicCallBack()
*/

TEST_F(PeriodicRead_ut, readPeriodicCallBack_NULLArg1) {

	try
	{
		readPeriodicCallBack(u8UnitID, u16TransacID, NULL, u8FunCode, pstException, u8numBytes, pu8data, u16StartAddress, u16Quantity);
	}
	catch(exception &e)
	{
		EXPECT_EQ("", e.what());
	}

}

/* TC010
Test: Behaviour of readPeriodicCallBack()
*/
TEST_F(PeriodicRead_ut, readPeriodicCallBack_NULLArg2) {

	try
	{
		readPeriodicCallBack(u8UnitID, u16TransacID, pu8IpAddr, u8FunCode, NULL, u8numBytes, pu8data, u16StartAddress, u16Quantity);
	}
	catch(exception &e)
	{
		EXPECT_EQ("", e.what());
	}

}

/* TC0011
Test: Behaviour of readPeriodicCallBack()
*/

TEST_F(PeriodicRead_ut, readPeriodicCallBack_NULLArg3) {

	try
	{
		readPeriodicCallBack(u8UnitID, u16TransacID, pu8IpAddr, u8FunCode, pstException, u8numBytes, NULL, u16StartAddress, u16Quantity);
	}
	catch(exception &e)
	{
		EXPECT_EQ("", e.what());
	}

}

/******************************CRequestInitiator::insertTxIDReqData****************************************
TC0012
Test: Behaviour of readPeriodicCallBack()
*/

TEST_F(PeriodicRead_ut, insertTxIDReqData_return) {

	try
	{
		CRequestInitiator::instance().insertTxIDReqData(3, CRefDataForPolling_obj);
		EXPECT_EQ(1, 1); //Programme doesnt hang

	}
	catch(exception &e)
	{
		EXPECT_EQ("", e.what()); //Test fails.s
	}

}

/******************************CRequestInitiator::removeTxIDReqData****************************************
TC0013
Test: Behaviour of removeTxIDReqData()
*/

TEST_F(PeriodicRead_ut, removeTxIDReqData_return) {

	try
	{
		CRequestInitiator::instance().removeTxIDReqData(3);
		EXPECT_EQ(1, 1); //Programme doesnt hang

	}
	catch(exception &e)
	{
		EXPECT_EQ("", e.what()); //Test fails.s
	}

}

/******************************CTimeMapper::initTimerFunction****************************************
TC0014
Test: Behaviour of initTimerFunction()
*/

TEST_F(PeriodicRead_ut, initTimerFunction_return) {

	try
	{
		CTimeMapper::instance().initTimerFunction();
		EXPECT_EQ(1,1);

	}
	catch(exception &e)
	{
		EXPECT_EQ("", e.what()); //Test fails.s
	}

}

/******************************CTimeMapper::checkTimer****************************************
TC0015
Test: Behaviour of checkTimer()
*/

TEST_F(PeriodicRead_ut, checkTimer_return) {

	try
	{
		CTimeMapper::instance().checkTimer();
		EXPECT_EQ(1,1);

	}
	catch(exception &e)
	{
		EXPECT_EQ("", e.what()); //Test fails.s
	}

}

/******************************CTimeMapper::ioPeriodicReadTimer****************************************
TC0016
Test: Behaviour of ioPeriodicReadTimer()
*/

//TEST_F(PeriodicRead_ut, ioPeriodicReadTimer_return) {
//
//	try
//	{
//		CTimeMapper::instance().ioPeriodicReadTimer(1);
//		EXPECT_EQ(1,1);
//
//	}
//	catch(exception &e)
//	{
//		EXPECT_EQ("", e.what()); //Test fails.s
//	}
//
//}
#endif


