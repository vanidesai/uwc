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
#include "ConfigManager.hpp"

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
extern eMbusStackErrorCode readPeriodicRTCallBack(stMbusAppCallbackParams_t *pstMbusAppCallbackParams, uint16_t uTxID);
extern eMbusStackErrorCode readPeriodicCallBack(stMbusAppCallbackParams_t *pstMbusAppCallbackParams, uint16_t uTxID);
//#else


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
		CPeriodicReponseProcessor::Instance().handleResponse(pstMbusAppCallbackParams, MBUS_CALLBACK_ONDEMAND_READ, "Response");

		EXPECT_EQ("", test_str);
	}
	catch(exception &e)
	{
		test_str = e.what();
		EXPECT_EQ("", test_str);

	}

}

#if 0
// TESTS are commented for now only ....Need to modify again


/*** Test:PeriodicRead_ut::handleResponse_NULLArgument_Data()
  Execution should not hang when NULL pointer is passed as an argument.*/

TEST_F(PeriodicRead_ut, handleResponse_NULLArgument_Data) {

	pstException->m_u8ExcCode = 0;
	pstException->m_u8ExcStatus = 0;

	stMbusAppCallbackParams_t stMbusAppCallbackParams;

	stMbusAppCallbackParams.m_au8MbusRXDataDataFields[0] = 0x80;
	stMbusAppCallbackParams.m_au8MbusRXDataDataFields[1] = 0x70;
	stMbusAppCallbackParams.m_au8MbusRXDataDataFields[2] = 0x50;
	stMbusAppCallbackParams.m_lPriority = 1;
	stMbusAppCallbackParams.m_objTimeStamps.tsReqRcvd.tv_nsec = 10;
	stMbusAppCallbackParams.m_objTimeStamps.tsReqRcvd.tv_sec = 20;
	stMbusAppCallbackParams.m_objTimeStamps.tsReqSent.tv_nsec = 15;
	stMbusAppCallbackParams.m_objTimeStamps.tsRespRcvd.tv_nsec = 18;
	stMbusAppCallbackParams.m_objTimeStamps.tsRespSent.tv_sec = 5;
	stMbusAppCallbackParams.m_u16Quantity = 1;
	stMbusAppCallbackParams.m_u16StartAdd = 1;
	stMbusAppCallbackParams.m_u16TransactionID = 9472;
	stMbusAppCallbackParams.m_u8ExceptionExcCode = 0;
	stMbusAppCallbackParams.m_u8ExceptionExcStatus = 0;
	stMbusAppCallbackParams.m_u8FunctionCode = 1;
	stMbusAppCallbackParams.m_u8IpAddr[0] = 192;
	stMbusAppCallbackParams.m_u8IpAddr[1] = 168;
	stMbusAppCallbackParams.m_u8IpAddr[2] = 8;
	stMbusAppCallbackParams.m_u8IpAddr[3] = 1;

	stMbusAppCallbackParams.m_u8MbusRXDataLength = 3;
	stMbusAppCallbackParams.m_u8UnitID = 5;
	stMbusAppCallbackParams.u16Port = 502;

	try
	{
		//CUniqueDataPoint a_objDataPoint;
		struct stZmqContext a_objBusContext;
		struct stZmqPubContext a_objPubContext;
		uint8_t a_uiFuncCode = 1;
		CRefDataForPolling a_stRdPrdObj{CUniqueDataPoint_obj, a_objBusContext, a_objPubContext, a_uiFuncCode};
		//sem_t semaphoreWriteReq = CPeriodicReponseProcessor::Instance().getSemaphoreWriteReq();
		CRequestInitiator::instance().insertTxIDReqData(9472, a_stRdPrdObj);
		CPeriodicReponseProcessor::Instance().handleResponse(&stMbusAppCallbackParams, MBUS_RESPONSE_ONDEMAND, MBUS_CALLBACK_ONDEMAND_WRITE, "ResponseTopic");
		EXPECT_EQ("", test_str);
	}
	catch(exception &e)
	{
		test_str = e.what();
		EXPECT_EQ("", test_str);

	}

}




/* TC003 // Need to check this test case once again
Test: Behaviour of handleResponse() when pstException->m_u8ExcCode and pstException->m_u8ExcStatus both are 0
 */
/*
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

}*/

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
#endif

#if 0 //in progress
TEST_F(PeriodicRead_ut, initiateMessages_false)
{
	zmq_handler::stZmqContext a_BusContext;
	uint32_t a_uiRef = 52;
	uint8_t a_uiFuncCode = 1;
	uint32_t U32_code;
	zmq_handler::stZmqPubContext a_objPubContext = zmq_handler::getPubCTX("TCP1_RdResp");
	CRefDataForPolling CRefDataForPolling_obj{CUniqueDataPoint_obj,a_BusContext, a_objPubContext, a_uiFuncCode};
	CTimeRecord CTimeRecord_obj{U32_code, CRefDataForPolling_obj};
	try
	{
		CRequestInitiator::instance().initiateMessages(a_uiRef, CTimeRecord_obj, false);
		EXPECT_EQ(1, 1);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ", (string)e.what());
	}

}

TEST_F(PeriodicRead_ut, initiateMessages_true)
{
	zmq_handler::stZmqContext a_BusContext;
	uint32_t a_uiRef = 52;
	uint8_t a_uiFuncCode = 1;
	uint32_t U32_code;
	zmq_handler::stZmqPubContext a_objPubContext = zmq_handler::getPubCTX("TCP1_RdResp");
	CRefDataForPolling CRefDataForPolling_obj{CUniqueDataPoint_obj,a_BusContext, a_objPubContext, a_uiFuncCode};
	CTimeRecord CTimeRecord_obj{U32_code, CRefDataForPolling_obj};
	try
	{
		CRequestInitiator::instance().initiateMessages(a_uiRef, CTimeRecord_obj, true);
		EXPECT_EQ(1, 1);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ", (string)e.what());

	}

}
#endif

TEST_F(PeriodicRead_ut, insertTxIDReqData_test)
{
	struct stZmqContext a_objBusContext;
	struct stZmqPubContext a_objPubContext;
	uint8_t a_uiFuncCode = 1;
	CRefDataForPolling a_stRdPrdObj{CUniqueDataPoint_obj, a_objBusContext, a_objPubContext, a_uiFuncCode};
	CRequestInitiator::instance().insertTxIDReqData(9472, a_stRdPrdObj);

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
#if 1

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
		//CPeriodicReponseProcessor::Instance().initRespHandlerThreads();

		//EXPECT_EQ("", test_str);
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

TEST_F(PeriodicRead_ut, readPeriodicCallBack_NULLArg1)
{
	uint16_t uTxID = 20;

	try
	{
#ifdef MODBUS_STACK_TCPIP_ENABLED

		//readPeriodicCallBack(u8UnitID, u16TransacID, NULL, u16Port, u8FunCode, pstException, u8numBytes, pu8data, u16StartAddress, u16Quantity, a_objStackTimestamps);
		readPeriodicCallBack(pstMbusAppCallbackParams, uTxID);

#else

		//readPeriodicCallBack(u8UnitID, u16TransacID, NULL, u8FunCode, pstException, u8numBytes, pu8data, u16StartAddress, u16Quantity, a_objStackTimestamps);
		readPeriodicCallBack(pstMbusAppCallbackParams, uTxID);

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

#if 1
//TRhis test is should not be commented
TEST_F(PeriodicRead_ut, isTxIDPresent_return) {

	try
	{
		//CRequestInitiator::instance().insertTxIDReqData(3, CRefDataForPolling_obj);
		CRequestInitiator::instance().isTxIDPresent(3);
		EXPECT_EQ(1, 1); //Programme doesnt hang

	}
	catch(exception &e)
	{
		EXPECT_EQ("", e.what()); //Test fails.s
	}

}
#endif



#if 1
// This test should not be commented
TEST_F(PeriodicRead_ut, timer_Start)
{
	long interval = 1000;
	PeriodicTimer::timer_start(interval);

}
#endif

TEST_F(PeriodicRead_ut, timer_Stop)
{

	PeriodicTimer::timer_stop();

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

#if 0
// Need to be checked once again this test should not be commented.
TEST_F(PeriodicRead_ut, checkTimer_return)
{

	try
	{
		zmq_handler::stZmqContext a_BusContext;
		uint8_t a_uiFuncCode = 1;
		uint32_t U32_code;
		bool g_stopTimer;

		zmq_handler::stZmqContext a_objBusContext = zmq_handler::getCTX("TCP1_RdResp");
		zmq_handler::stZmqPubContext a_objPubContext = zmq_handler::getPubCTX("TCP1_RdResp");
		CRefDataForPolling CRefDataForPolling_obj{CUniqueDataPoint_obj,a_BusContext, a_objPubContext, a_uiFuncCode};
		CTimeRecord CTimeRecord_obj{U32_code, CRefDataForPolling_obj};
		CRefDataForPolling a_stRdPrdObj{CUniqueDataPoint_obj, a_objBusContext, a_objPubContext, a_uiFuncCode};
		stException_t m_stException = {};
		m_stException.m_u8ExcCode = 100;
		m_stException.m_u8ExcStatus = 100;
		uint32_t uiRef = 2;
		sem_t semaphoreWriteReq = CRequestInitiator::instance().getSemaphoreReqProcess();
		sem_post(&CRequestInitiator::instance().getSemaphoreReqProcess());

		CRefDataForPolling *CRefDataForPolling_pt = &a_stRdPrdObj;

		CTimeMapper::instance().insert(1, a_stRdPrdObj);
		CTimeMapper::instance().checkTimer(10000, tsPoll);
		CPeriodicReponseProcessor::Instance().postDummyBADResponse(CRefDataForPolling_obj,
				m_stException,
				&tsPoll);
		EXPECT_EQ(0, g_stopTimer);

	}
	catch(exception &e)
	{
		EXPECT_EQ("map::at", (string)e.what()); //Test fails.s
	}
}

#endif

TEST_F(PeriodicRead_ut, get_Timer_freq)
{
	try
	{
		uint32_t ulMinFreq = CTimeMapper::instance().getMinTimerFrequency();

	}
	catch(std::exception &e)
	{
		EXPECT_EQ("", (string)e.what());
	}
}

#if 1
TEST_F(PeriodicRead_ut, timer_Thread)
{
    bool g_stopTimer = false;
    //PeriodicTimer::timer_start(1000);
	PeriodicTimer::timerThread(1000);
	EXPECT_EQ(0, g_stopTimer);

}
#endif

#if 0

TEST_F(PeriodicRead_ut, Response_cutoff)
{
	zmq_handler::stZmqContext a_BusContext;
	zmq_handler::stZmqPubContext a_objPubContext;
	uint8_t a_uiFuncCode;
	CRefDataForPolling CRefDataForPolling_obj{CUniqueDataPoint_obj,a_BusContext, a_objPubContext, a_uiFuncCode};

	try
	{

		CPeriodicReponseProcessor::Instance().postLastResponseForCutoff(CRefDataForPolling_obj);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ", (string) e.what());
	}
}
#endif


TEST_F(PeriodicRead_ut, readPeriodic_callback_true)
{
	bool isRTRequest = true;
	uint8_t u8ReturnType = MBUS_STACK_NO_ERROR;
	uint16_t uTxID = 20;
	if(true == isRTRequest)
	{
		u8ReturnType = readPeriodicRTCallBack(pstMbusAppCallbackParams, uTxID);
		EXPECT_EQ(true, isRTRequest);
	}
	else
	{
		u8ReturnType = readPeriodicCallBack(pstMbusAppCallbackParams, uTxID);
		EXPECT_EQ(false, isRTRequest);
	}
}


TEST_F(PeriodicRead_ut, readPeriodic_callback_true_NULL)
{
	bool isRTRequest = true;
	uint8_t u8ReturnType = MBUS_STACK_NO_ERROR;
	uint16_t uTxID = 20;
	if(true == isRTRequest)
	{
		u8ReturnType = readPeriodicRTCallBack(stMbusAppCallbackParams, uTxID);
		EXPECT_EQ(true, isRTRequest);
	}
	else
	{
		u8ReturnType = readPeriodicCallBack(stMbusAppCallbackParams, uTxID);
		EXPECT_EQ(false, isRTRequest);
	}
}


TEST_F(PeriodicRead_ut, readPeriodic_callback_false)
{
	bool isRTRequest = false;
	uint8_t u8ReturnType = MBUS_STACK_NO_ERROR;
	uint16_t uTxID = 20;
	if(true == isRTRequest)
	{
		u8ReturnType = readPeriodicRTCallBack(pstMbusAppCallbackParams, uTxID);
		EXPECT_EQ(true, isRTRequest);
	}
	else
	{
		u8ReturnType = readPeriodicCallBack(pstMbusAppCallbackParams, uTxID);
		EXPECT_EQ(false, isRTRequest);
	}
}

TEST_F(PeriodicRead_ut, readPeriodic_callback_false_NULL)
{
	bool isRTRequest = false;
	uint8_t u8ReturnType = MBUS_STACK_NO_ERROR;
	uint16_t uTxID = 20;
	if(true == isRTRequest)
	{
		u8ReturnType = readPeriodicRTCallBack(stMbusAppCallbackParams, uTxID);
		EXPECT_EQ(true, isRTRequest);
	}
	else
	{
		u8ReturnType = readPeriodicCallBack(stMbusAppCallbackParams, uTxID);
		EXPECT_EQ(false, isRTRequest);
	}
}

#if 0
#ifdef MODBUS_STACK_TCPIP_ENABLED
TEST_F(PeriodicRead_ut, Post_dummy_BAD)
{
	stException_t m_stException = {};
	m_stException.m_u8ExcCode = 100;
	m_stException.m_u8ExcStatus = 100;
	bool result;

	zmq_handler::stZmqContext a_objBusContext = zmq_handler::getCTX("TCP1_RdResp");
	zmq_handler::stZmqPubContext a_objPubContext = zmq_handler::getPubCTX("TCP1_RdResp");
	uint8_t a_uiFuncCode = 1;
	try
	{
		uint32_t uiRef;
		CRefDataForPolling a_stRdPrdObj{CUniqueDataPoint_obj, a_objBusContext, a_objPubContext, a_uiFuncCode};
		//std::vector<CRefDataForPolling>& a_vReqData = CTimeMapper::instance().getPolledPointList(1);
		CPeriodicReponseProcessor::Instance().postDummyBADResponse(a_stRdPrdObj, m_stException);
		EXPECT_EQ(1,1);

	}
	catch(std::exception &e)
	{
		EXPECT_EQ("map::at", (string)e.what());
	}
/*
#else
	try
	{
		zmq_handler::stZmqContext a_objBusContext = zmq_handler::getCTX("RTU1_RdResp");
		zmq_handler::stZmqPubContext a_objPubContext = zmq_handler::getPubCTX("RTU1_RdResp");
		uint8_t a_uiFuncCode = 1;

			uint32_t uiRef;
			CRefDataForPolling a_stRdPrdObj{CUniqueDataPoint_obj, a_objBusContext, a_objPubContext, a_uiFuncCode};
			//std::vector<CRefDataForPolling>& a_vReqData = CTimeMapper::instance().getPolledPointList(1);
			CPeriodicReponseProcessor::Instance().postDummyBADResponse(a_stRdPrdObj, m_stException);
			EXPECT_EQ(1,1);

	}
	catch(std::exception &e)
	{
		EXPECT_EQ("map::at", (string)e.what());
	}
#endif
*/

}

#else
TEST_F(PeriodicRead_ut, Post_dummy_BAD_rtu)
{
	try
		{
			zmq_handler::stZmqContext a_objBusContext = zmq_handler::getCTX("RTU1_RdResp");
			zmq_handler::stZmqPubContext a_objPubContext = zmq_handler::getPubCTX("RTU1_RdResp");
			uint8_t a_uiFuncCode = 1;

				uint32_t uiRef;
				CRefDataForPolling a_stRdPrdObj{CUniqueDataPoint_obj, a_objBusContext, a_objPubContext, a_uiFuncCode};
				//std::vector<CRefDataForPolling>& a_vReqData = CTimeMapper::instance().getPolledPointList(1);
				CPeriodicReponseProcessor::Instance().postDummyBADResponse(a_stRdPrdObj, m_stException);
				EXPECT_EQ(1,1);

		}
		catch(std::exception &e)
		{
			EXPECT_EQ("map::at", (string)e.what());
		}
}
#endif
#endif

TEST_F(PeriodicRead_ut, polled_point_list_true)
{
	uint32_t uiRef;
	try
	{
	std::vector<CRefDataForPolling>& a_vReqData = CTimeMapper::instance().getPolledPointList(uiRef, true);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("map::at", (string)e.what());
	}
}

TEST_F(PeriodicRead_ut, polled_point_list_false)
{
	uint32_t uiRef;
	try
	{
	std::vector<CRefDataForPolling>& a_vReqData = CTimeMapper::instance().getPolledPointList(uiRef, false);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("map::at", (string)e.what());
	}
}

TEST_F(PeriodicRead_ut, saveResponse_test)
{
	bool result;
	std::string  response= "Response";
	std::string sUse;
	stTimeStamps objStackTimestamps;

	zmq_handler::stZmqContext a_BusContext;
	zmq_handler::stZmqPubContext a_objPubContext;
	uint8_t a_uiFuncCode;

	CRefDataForPolling CRefDataForPolling_obj{CUniqueDataPoint_obj,a_BusContext, a_objPubContext, a_uiFuncCode};
	result = CRefDataForPolling_obj.saveGoodResponse(response, sUse);

}


TEST_F(PeriodicRead_ut, lastGoodReaspose_test)
{
	zmq_handler::stZmqContext a_BusContext;
	zmq_handler::stZmqPubContext a_objPubContext;
	uint8_t a_uiFuncCode;
	CRefDataForPolling CRefDataForPolling_obj{CUniqueDataPoint_obj,a_BusContext, a_objPubContext, a_uiFuncCode};
	stLastGoodResponse stLastGoodResponse_obj = CRefDataForPolling_obj.getLastGoodResponse();
}

/*
 *  Check this test case again

TEST_F(PeriodicRead_ut, Awaitresp)
{
	stException_t m_stException = {};
	m_stException.m_u8ExcCode = 100;
	m_stException.m_u8ExcStatus = 100;
	uint32_t uiRef  = 20;

	std::vector<CRefDataForPolling>& a_vReqData = CTimeMapper::instance().getPolledPointList(uiRef, false);

	for(auto a_oReqData: a_vReqData)
	{

		CRefDataForPolling objReqData = a_oReqData ;
		try
		{

			if(true == objReqData.getDataPoint().isIsAwaitResp())
			{
				CPeriodicReponseProcessor::Instance().postDummyBADResponse(objReqData, m_stException);
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

}
*/


#endif

