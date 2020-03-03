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


#if 1
extern void getTimeBasedParams(const CRefDataForPolling& a_objReqData, std::string &a_sTimeStamp, std::string &a_sUsec, std::string &a_sTxID);



void PeriodicRead_ut::SetUp()
{
	// Setup code
}

void PeriodicRead_ut::TearDown()
{
	// TearDown code
}
//#ifdef MODBUS_STACK_TCPIP_ENABLED
extern eMbusStackErrorCode readPeriodicCallBack(stMbusAppCallbackParams_t *pstMbusAppCallbackParams);
//#else
/*
extern eMbusStackErrorCode readPeriodicCallBack(uint8_t  u8UnitID,
		uint16_t u16TransacID,
		uint8_t* pu8IpAddr,
		uint8_t  u8FunCode,
		stException_t  *pstException,
		uint8_t  u8numBytes,
		uint8_t* pu8data,
		uint16_t  u16StartAddress,
		uint16_t  u16Quantity,
		stTimeStamps a_objStackTimestamps);
#endif
*/

/******************************PeriodicRead::handleResponse*****************************************/

/***Test:PeriodicRead_ut::handleResponse_NULLArgument_Exception()
  Execution should not hang when NULL pointer is passed as an argument.*/

TEST_F(PeriodicRead_ut, handleResponse_NULLArguments) {

	pstException->m_u8ExcCode = 0;
	pstException->m_u8ExcStatus = 0;

	try
	{
		/*CPeriodicReponseProcessor::Instance().handleResponse(u8UnitID, u16TransacID, pu8IpAddr, u8FunCode, NULL,
			u8numBytes, pu8data, u16StartAddress, u16Quantity,a_objStackTimestamps);*/
		CPeriodicReponseProcessor::Instance().handleResponse(pstMbusAppCallbackParams);

		EXPECT_EQ("", test_str);
	}
	catch(exception &e)
	{
		test_str = e.what();
		EXPECT_EQ("", test_str);

	}

}


/*** Test:PeriodicRead_ut::handleResponse_NULLArgument_Data()
  Execution should not hang when NULL pointer is passed as an argument.*/

TEST_F(PeriodicRead_ut, handleResponse_NULLArgument_Data) {

	pstException->m_u8ExcCode = 0;
	pstException->m_u8ExcStatus = 0;

	MbusAppCallbackParams.m_au8MbusRXDataDataFields[255];
	MbusAppCallbackParams.m_lPriority = 1;
	MbusAppCallbackParams.m_objTimeStamps.tsReqRcvd.tv_nsec = 10;
	MbusAppCallbackParams.m_objTimeStamps.tsReqRcvd.tv_sec = 20;
	MbusAppCallbackParams.m_objTimeStamps.tsReqSent.tv_nsec = 15;
	MbusAppCallbackParams.m_objTimeStamps.tsRespRcvd.tv_nsec = 18;
	MbusAppCallbackParams.m_objTimeStamps.tsRespSent.tv_sec = 5;
	MbusAppCallbackParams.m_u16Quantity = 1;
	MbusAppCallbackParams.m_u16StartAdd = 22344;
	MbusAppCallbackParams.m_u16TransactionID = 22;
	MbusAppCallbackParams.m_u8ExceptionExcCode = 0;
	MbusAppCallbackParams.m_u8ExceptionExcStatus = 0;
	MbusAppCallbackParams.m_u8FunctionCode = 1;
	MbusAppCallbackParams.m_u8IpAddr[4];
	MbusAppCallbackParams.m_u8MbusRXDataLength = 15;
	MbusAppCallbackParams.m_u8UnitID = 0;
	MbusAppCallbackParams.u16Port = 13;

	try
	{
		/*CPeriodicReponseProcessor::Instance().handleResponse(u8UnitID, u16TransacID, pu8IpAddr, u8FunCode, pstException,
				u8numBytes, NULL, u16StartAddress, u16Quantity, a_objStackTimestamps);*/
		CPeriodicReponseProcessor::Instance().handleResponse(pstMbusAppCallbackParams);

		EXPECT_EQ("", test_str);
	}
	catch(exception &e)
	{
		test_str = e.what();
		EXPECT_EQ("", test_str);

	}

}

#if 0 // TESTS are commented for now only ....Need to modify again

/* TC003
Test: Behaviour of handleResponse() when pstException->m_u8ExcCode and pstException->m_u8ExcStatus both are 0
 */

TEST_F(PeriodicRead_ut, handleResponse_ExcStatus_ExcCode_0) {


	pstException->m_u8ExcCode = 0;
	pstException->m_u8ExcStatus = 0;

	try
	{
		CPeriodicReponseProcessor::Instance().handleResponse(u8UnitID, u16TransacID, pu8IpAddr, u8FunCode, pstException,
				8, pu8data, u16StartAddress, u16Quantity, a_objStackTimestamps);

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
TEST_F(PeriodicRead_ut, handleResponse_ExcStatus_ExcCode_Non0) {


	try
	{
		pstException->m_u8ExcCode = 1;
		pstException->m_u8ExcStatus = 0;

		CPeriodicReponseProcessor::Instance().handleResponse(u8UnitID, u16TransacID, pu8IpAddr, u8FunCode, pstException,
				8, pu8data, u16StartAddress, u16Quantity, a_objStackTimestamps);

		pstException->m_u8ExcCode = 0;
		pstException->m_u8ExcStatus = 1;

		CPeriodicReponseProcessor::Instance().handleResponse(u8UnitID, u16TransacID, pu8IpAddr, u8FunCode, pstException,
				8, pu8data, u16StartAddress, u16Quantity, a_objStackTimestamps);

		EXPECT_EQ("", test_str);
	}
	catch(exception &e)
	{
		test_str = e.what();
		EXPECT_EQ("", test_str);

	}

}

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
				u8numBytes, pu8data, u16StartAddress, u16Quantity, a_objStackTimestamps);

		EXPECT_LT(pu8data[u8numBytes], 9);

		//EXPECT_GT(pu8data[u8numBytes], 0);

	}
	catch(exception &e)
	{

		test_str = e.what();
		EXPECT_EQ("", test_str);


	}

}
#endif
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
TEST_F(PeriodicRead_ut, getTimeBasedParams_return)
{
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
#ifdef MODBUS_STACK_TCPIP_ENABLED

		//readPeriodicCallBack(u8UnitID, u16TransacID, NULL, u16Port, u8FunCode, pstException, u8numBytes, pu8data, u16StartAddress, u16Quantity, a_objStackTimestamps);
		readPeriodicCallBack(pstMbusAppCallbackParams);

#else

		//readPeriodicCallBack(u8UnitID, u16TransacID, NULL, u8FunCode, pstException, u8numBytes, pu8data, u16StartAddress, u16Quantity, a_objStackTimestamps);
		readPeriodicCallBack(pstMbusAppCallbackParams);

#endif
	}
	catch(exception &e)
	{
		EXPECT_EQ("", e.what());
	}

}

#if 0
/* TC010
Test: Behaviour of readPeriodicCallBack()
 */
TEST_F(PeriodicRead_ut, readPeriodicCallBack_NULLArg2)
{
#ifdef MODBUS_STACK_TCPIP_ENABLED
	try
	{
		readPeriodicCallBack(u8UnitID, u16TransacID, pu8IpAddr, u16Port, u8FunCode, NULL, u8numBytes, pu8data, u16StartAddress, u16Quantity);
	}
	catch(exception &e)
	{
		EXPECT_EQ("", e.what());
	}


#else
	try
	{
		readPeriodicCallBack(u8UnitID, u16TransacID, pu8IpAddr, u8FunCode, NULL, u8numBytes, pu8data, u16StartAddress, u16Quantity);
	}
	catch(exception &e)
	{
		EXPECT_EQ("", e.what());
	}
}
#endif

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
#endif

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


/*
TEST_F(PeriodicRead_ut, Awaitresp)
{
	uint32_t uiRef;
	CRequestInitiator CRequestInitiator_obj;
	if(false == CRequestInitiator_obj.getFreqRefForPollCycle(uiRef))
	{
		break;
	}
	std::vector<CRefDataForPolling>& a_vReqData = CTimeMapper::instance().getPolledPointList(uiRef);

	for(auto a_oReqData: a_vReqData)
	{

		CRefDataForPolling objReqData = a_oReqData ;
		try
		{

			if(true == objReqData.getDataPoint().isIsAwaitResp())
			{
				CPeriodicReponseProcessor::Instance().postDummyBADResponse(objReqData);
				continue;
			}
		}

		catch(std::exception &e)
		{
			cout<<"#################################################################################"<<endl;
			cout<<e.what()<<endl;
			cout<<"#################################################################################"<<endl;
			EXPECT_EQ("map::at", (string)e.what());
		}

	}




}*/
/*

TEST_F(PeriodicRead_ut, set_Await)
{
	//stStackResponse& a_stResp;
	stStackResponse res;
	//CPeriodicReponseProcessor CPeriodicReponseProcessor_obj;
	try
	{
		const CRefDataForPolling& objReqData = CRequestInitiator::instance().getTxIDReqData(res.u16TransacID);
		// Response is received. Reset response awaited status
		objReqData.getDataPoint().setIsAwaitResp(false);

		//CPeriodicReponseProcessor_obj.postResponseJSON(a_stResp, objReqData);
	}
	catch(std::exception &e)
	{
		cout<<"#################################################################################"<<endl;
		cout<<e.what()<<endl;
		cout<<"#################################################################################"<<endl;
		EXPECT_EQ("map::at", (string)e.what());

	}
}
 */




/******************************CTimeMapper::ioPeriodicReadTimer****************************************
TC0016
Test: Behaviour of ioPeriodicReadTimer()
 */
#if 0
TEST_F(PeriodicRead_ut, ioPeriodicReadTimer_return) {

	try
	{
		CTimeMapper::instance().ioPeriodicReadTimer(1);
		EXPECT_EQ(1,1);

	}
	catch(exception &e)
	{
		EXPECT_EQ("", e.what()); //Test fails.s
	}

}


TEST_F(PeriodicRead_ut, Start_timer)
{
	try{

		LinuxTimer::start_timer(1);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("",(string)e.what());
	}
}
#endif
#endif
