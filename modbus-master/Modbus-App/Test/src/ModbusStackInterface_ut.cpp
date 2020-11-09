/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#include "../include/ModbusStackInterface_ut.hpp"
#include "Common.hpp"

//#define MOCKING

//using ::testing::Return;



//void ModbusMaster_AppCallback(stMbusAppCallbackParams_t *pstMbusAppCallbackParams);
void ModbusStackInterface_ut::SetUp()
{
	// Setup code
}

void ModbusStackInterface_ut::TearDown()
{
	// TearDown code
}




TEST_F(ModbusStackInterface_ut, AppCallback01)
{
#ifdef MODBUS_STACK_TCPIP_ENABLED
	//setenv("WRITE_RESPONSE_TOPIC", , 1);
	//const char* pcWriteRespTopic = std::getenv("WRITE_RESPONSE_TOPIC");

	try
	{
		stOnDemandRequest reqData;
		stOnDemandRequest onDemandReqData ;
		MbusAPI_t stMbusApiPram = {};

		uint16_t u16TxID = 52;
		reqData.m_isByteSwap = true;
		reqData.m_isWordSwap = false;
		reqData.m_obtReqRcvdTS.tv_nsec = 21132323;
		reqData.m_obtReqRcvdTS.tv_sec = 1;
		reqData.m_strAppSeq = "455";
		reqData.m_strMetric = "Test";
		reqData.m_strTopic = "kzdjfhdszh";
		reqData.m_strVersion = "2.1";
		reqData.m_strWellhead = "test";
		uint16_t uTxID = 20;

		stMbusAppCallbackParams.m_lPriority = 1;
		stMbusAppCallbackParams.m_objTimeStamps.tsReqRcvd.tv_nsec = 10;
		stMbusAppCallbackParams.m_objTimeStamps.tsReqRcvd.tv_sec = 20;
		stMbusAppCallbackParams.m_objTimeStamps.tsReqSent.tv_nsec = 15;
		stMbusAppCallbackParams.m_objTimeStamps.tsRespRcvd.tv_nsec = 18;
		stMbusAppCallbackParams.m_objTimeStamps.tsRespSent.tv_sec = 5;
		stMbusAppCallbackParams.m_u16Quantity = 0;
		stMbusAppCallbackParams.m_u16StartAdd = 0;
		stMbusAppCallbackParams.m_u16TransactionID = 52;
		stMbusAppCallbackParams.m_u8ExceptionExcCode = 1;
		stMbusAppCallbackParams.m_u8ExceptionExcStatus = 1;
		stMbusAppCallbackParams.m_u8FunctionCode = READ_COIL_STATUS;
		stMbusAppCallbackParams.m_u8MbusRXDataLength = 100;
		stMbusAppCallbackParams.m_u8UnitID = 0;
		stMbusAppCallbackParams.u16Port = 0;

		//common_Handler::insertOnDemandReqData(u16TxID, reqData);
		common_Handler::insertReqData(u16TxID, stMbusApiPram);

		//zmq_handler::prepareCommonContext("pub");
		//zmq_handler::getOnDemandReqData(0, onDemandReqData);

		OnDemandRead_AppCallback(pstMbusAppCallbackParams, uTxID);

	}
	catch(std::exception &e)
	{
		EXPECT_EQ("", e.what());
	}
}

#else
const char* pcWriteRespTopic = std::getenv("WRITE_RESPONSE_TOPIC");

try
{
	msg_envelope_t *msg = NULL;
	stOnDemandRequest reqData;
	uint16_t uTxID = 20;
	stOnDemandRequest onDemandReqData;

	zmq_handler::insertOnDemandReqData(0, reqData);
	zmq_handler::getOnDemandReqData(u16TransacID, onDemandReqData);
	OnDemandRead_AppCallback(pstMbusAppCallbackParams, uTxID);

}
catch(std::exception &e)
{
	EXPECT_EQ(" ", e.what());
}
}

#endif




/************************************MOdbusMAster_AppCallback()*******************************/
#if 1
// This test is commented because ModbusMaster_AppCallback() function is not available in the source code
TEST_F(ModbusStackInterface_ut, AppCallback_null_ip)
{
#ifdef MODBUS_STACK_TCPIP_ENABLED
	//setenv("WRITE_RESPONSE_TOPIC", , 1);
	//const char* pcWriteRespTopic = std::getenv("WRITE_RESPONSE_TOPIC");

	try
	{
		stOnDemandRequest reqData;
		stOnDemandRequest onDemandReqData ;
		uint16_t uTxID = 20;

		OnDemandRead_AppCallback(MbusAppCallbackParams, uTxID);

	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ", e.what());
	}
}

#else
const char* pcWriteRespTopic = std::getenv("WRITE_RESPONSE_TOPIC");

try
{
	msg_envelope_t *msg = NULL;
	stOnDemandRequest reqData;
	stOnDemandRequest onDemandReqData;
	uint16_t uTxID = 20;

	zmq_handler::insertOnDemandReqData(0, reqData);

	//zmq_handler::getOnDemandReqData(u16TransacID, onDemandReqData);
	OnDemandRead_AppCallback(MbusAppCallbackParams, uTxID);


}
catch(std::exception &e)
{
	EXPECT_EQ(" ", e.what());
}
}

#endif
#endif



TEST_F(ModbusStackInterface_ut, OnDemandRead_RT)
{
	uint16_t uTxID = 20;

	try
	{
		OnDemandReadRT_AppCallback(pstMbusAppCallbackParams, uTxID);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ", (std::string)e.what());
	}
}



TEST_F(ModbusStackInterface_ut, OnDemandRead_RT_NULL)
{
	uint16_t uTxID = 20;
	try
	{
		OnDemandReadRT_AppCallback(MbusAppCallbackParams, uTxID);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ",(std::string)e.what());
	}
}

TEST_F(ModbusStackInterface_ut, OnDemandWrite)
{
	uint16_t uTxID = 20;
	try
	{
		OnDemandWrite_AppCallback(pstMbusAppCallbackParams, uTxID);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ", (std::string)e.what());
	}
}


TEST_F(ModbusStackInterface_ut, OnDemandWrite_NULL)
{
	uint16_t uTxID = 20;
	try
	{
		OnDemandWrite_AppCallback(MbusAppCallbackParams, uTxID);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ", (std::string)e.what());
	}
}

TEST_F(ModbusStackInterface_ut, OnDemandWrite_RT)
{
	uint16_t uTxID = 20;
	try
	{
		OnDemandWriteRT_AppCallback(pstMbusAppCallbackParams, uTxID);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ", (std::string)e.what());
	}
}

TEST_F(ModbusStackInterface_ut, OnDemandWrite_RT_NULL)
{
	uint16_t uTxID = 20;
	try
	{
		OnDemandWriteRT_AppCallback(MbusAppCallbackParams, uTxID);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ", (std::string)e.what());
	}
}


TEST_F(ModbusStackInterface_ut, Modbus_Stack_API_Call_Readcoil)
{

	int retval = Modbus_Stack_API_Call(READ_COIL_STATUS,
			pstMbusApiPram,
			NULL);

	EXPECT_EQ(11, retval);
}

TEST_F(ModbusStackInterface_ut, Modbus_Stack_API_Call_ReadInStatus)
{

	uint8_t retval = Modbus_Stack_API_Call(READ_INPUT_STATUS,
			pstMbusApiPram,
			NULL);

	EXPECT_EQ(11, retval);
}


TEST_F(ModbusStackInterface_ut, Modbus_Stack_API_Call_ReadHoldReg)
{

	uint8_t retval = Modbus_Stack_API_Call(READ_HOLDING_REG,
			pstMbusApiPram,
			NULL);

	EXPECT_EQ(11, retval);
}


TEST_F(ModbusStackInterface_ut, Modbus_Stack_API_Call_ReadInReg)
{

	uint8_t retval = Modbus_Stack_API_Call(READ_INPUT_REG,
			pstMbusApiPram,
			NULL);

	EXPECT_EQ(11, retval);
}


TEST_F(ModbusStackInterface_ut, Modbus_Stack_API_Call_WriteSingleCoils)
{


	uint8_t retval = Modbus_Stack_API_Call(WRITE_SINGLE_COIL,
			pstMbusApiPram,
			NULL);

	EXPECT_EQ(11, retval);
}


TEST_F(ModbusStackInterface_ut, Modbus_Stack_API_Call_WriteSingleReg)
{

	uint8_t retval = Modbus_Stack_API_Call(WRITE_SINGLE_REG,
			pstMbusApiPram,
			NULL);

	EXPECT_EQ(11, retval);
}


TEST_F(ModbusStackInterface_ut, Modbus_Stack_API_Call_WriteMulCoils)
{

	uint8_t retval = Modbus_Stack_API_Call(WRITE_MULTIPLE_COILS,
			pstMbusApiPram,
			NULL);

	EXPECT_EQ(11, retval);
}


TEST_F(ModbusStackInterface_ut, Modbus_Stack_API_Call_WriteMulReg)
{

	uint8_t retval = Modbus_Stack_API_Call(WRITE_MULTIPLE_REG,
			pstMbusApiPram,
			NULL);

	EXPECT_EQ(11, retval);
}


/*
This test case is commented because the MbusApp_Process_Requeset() function is no more longer
availabe.. it is commented in the source code also......check this test once  again

TEST_F(ModbusStackInterface_ut, Process_Req)
{
	modbusInterface modbusInterface_obj;
	try{
		result = modbusInterface_obj.MbusApp_Process_Request(pstMbusReqGen);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("", (string)e.what());

	}
}
*/
