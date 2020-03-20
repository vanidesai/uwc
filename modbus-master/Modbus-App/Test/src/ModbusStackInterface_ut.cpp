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
		msg_envelope_t *msg = NULL;
		stOnDemandRequest reqData;
		stOnDemandRequest onDemandReqData ;

		uint16_t u16TxID = 52;
		reqData.m_isByteSwap = true;
		reqData.m_isWordSwap = false;
		reqData.m_obtReqRcvdTS.tv_nsec = 21132323;
		reqData.m_obtReqRcvdTS.tv_sec = 1;
		reqData.m_strAppSeq = "455";
		reqData.m_strMetric = "Test";
		reqData.m_strQOS = "0";
		reqData.m_strTopic = "kzdjfhdszh";
		reqData.m_strVersion = "2.1";
		reqData.m_strWellhead = "test";

		stMbusAppCallbackParams.m_au8MbusRXDataDataFields[260];
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
		stMbusAppCallbackParams.m_u8IpAddr[4];
		stMbusAppCallbackParams.m_u8MbusRXDataLength = 100;
		stMbusAppCallbackParams.m_u8UnitID = 0;
		stMbusAppCallbackParams.u16Port = 0;

		common_Handler::insertOnDemandReqData(u16TxID, reqData);

		//zmq_handler::prepareCommonContext("pub");
		//zmq_handler::getOnDemandReqData(0, onDemandReqData);

		OnDemandRead_AppCallback(pstMbusAppCallbackParams);

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
	stOnDemandRequest onDemandReqData;

	zmq_handler::insertOnDemandReqData(0, reqData);
	zmq_handler::getOnDemandReqData(u16TransacID, onDemandReqData);
	OnDemandRead_AppCallback(pstMbusAppCallbackParams);

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
		msg_envelope_t *msg = NULL;
		stOnDemandRequest reqData;
		stOnDemandRequest onDemandReqData ;

		OnDemandRead_AppCallback(MbusAppCallbackParams);

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

	zmq_handler::insertOnDemandReqData(0, reqData);

	//zmq_handler::getOnDemandReqData(u16TransacID, onDemandReqData);
	OnDemandRead_AppCallback(MbusAppCallbackParams);


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
	try
	{
		OnDemandReadRT_AppCallback(pstMbusAppCallbackParams);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ", (string)e.what());
	}
}



TEST_F(ModbusStackInterface_ut, OnDemandRead_RT_NULL)
{
	try
	{
		OnDemandReadRT_AppCallback(MbusAppCallbackParams);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ",(string)e.what());
	}
}

TEST_F(ModbusStackInterface_ut, OnDemandWrite)
{
	try
	{
		OnDemandWrite_AppCallback(pstMbusAppCallbackParams);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ", (string)e.what());
	}
}


TEST_F(ModbusStackInterface_ut, OnDemandWrite_NULL)
{
	try
	{
		OnDemandWrite_AppCallback(MbusAppCallbackParams);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ", (string)e.what());
	}
}

TEST_F(ModbusStackInterface_ut, OnDemandWrite_RT)
{
	try
	{
		OnDemandWriteRT_AppCallback(pstMbusAppCallbackParams);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ", (string)e.what());
	}
}

TEST_F(ModbusStackInterface_ut, OnDemandWrite_RT_NULL)
{
	try
	{
		OnDemandWriteRT_AppCallback(MbusAppCallbackParams);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ", (string)e.what());
	}
}


#if 1

TEST_F(ModbusStackInterface_ut, ReadCoil)
{

	Modbus_Read_Coils(m_u16StartAddr,
			m_u16Quantity,
			m_u16TxId,
			m_u8DevId,
			pstMbusApiPram->m_u8IpAddr,
			m_u16Port,
			lPriority,
			u32mseTimeout,
			vpCallBackFun);

	int retval = Modbus_Stack_API_Call(READ_COIL_STATUS,
			pstMbusApiPram,
			NULL);

	//EXPECT_EQ(1, retval);
	EXPECT_EQ(11, retval);
}


/***********************************ReadDiscreteInputs()*************************************/

TEST_F(ModbusStackInterface_ut, ReadDiscreteInputs)
{


	Modbus_Read_Discrete_Inputs(u16StartDI,
			u16NumOfDI,
			u16TransacID,
			u8UnitId,
			pstMbusApiPram->m_u8IpAddr,
			u16Port,
			lPriority,
			u32mseTimeout,
			pFunCallBack);

	uint8_t retval = Modbus_Stack_API_Call(READ_INPUT_STATUS,
			pstMbusApiPram,
			NULL);

	//EXPECT_EQ(1, retval);
	EXPECT_EQ(11, retval);
}


/***********************************ReadHoldingReg()*************************************/

TEST_F(ModbusStackInterface_ut, ReadHoldingReg)
{

	Modbus_Read_Holding_Registers(u16StartReg,
			u16NumberOfRegisters,
			u16TransacID,
			u8UnitId,
			pstMbusApiPram->m_u8IpAddr,
			u16Port,
			lPriority,
			u32mseTimeout,
			pFunCallBack);

	uint8_t retval = Modbus_Stack_API_Call(READ_HOLDING_REG,
			pstMbusApiPram,
			NULL);

	//EXPECT_EQ(1, retval);
	EXPECT_EQ(11, retval);
}


/***********************************ReadInputReg()*************************************/

TEST_F(ModbusStackInterface_ut, ReadInputReg)
{
	Modbus_Read_Input_Registers(u16StartReg,
			u16NumberOfRegisters,
			u16TransacID,
			u8UnitId,
			pstMbusApiPram->m_u8IpAddr,
			u16Port,
			lPriority,
			u32mseTimeout,
			pFunCallBack);

	uint8_t retval = Modbus_Stack_API_Call(READ_INPUT_REG,
			pstMbusApiPram,
			NULL);

	//EXPECT_EQ(1, retval);
	EXPECT_EQ(11, retval);
}


/***********************************WriteSingleCoil()*************************************/

TEST_F(ModbusStackInterface_ut, WriteSingleCoil)
{

	uint8_t temp = 0;
	pstMbusApiPram->m_pu8Data = &temp;
	pstMbusApiPram->m_u16StartAddr = 0;

	Modbus_Write_Single_Coil(u16StartCoil,
			*(uint16_t *)pstMbusApiPram->m_pu8Data,
			u16TransacID,
			u8UnitId,
			pstMbusApiPram->m_u8IpAddr,
			u16Port,
			lPriority,
			u32mseTimeout,
			pFunCallBack);

	uint8_t retval = Modbus_Stack_API_Call(WRITE_SINGLE_COIL,
			pstMbusApiPram,
			NULL);

	//EXPECT_EQ(1, retval);
	EXPECT_EQ(11, retval);
}


/***********************************WriteSingleReg()*************************************/
/* Hangs */
TEST_F(ModbusStackInterface_ut, WriteSingleReg)
{

	uint8_t temp = 0;
	pstMbusApiPram->m_pu8Data = &temp;
	pstMbusApiPram->m_u16StartAddr = 0;


	Modbus_Write_Single_Register(u16StartReg,
			*(uint16_t *)pstMbusApiPram->m_pu8Data,
			u16TransacID,
			u8UnitId,
			pstMbusApiPram->m_u8IpAddr,
			u16Port,
			lPriority,
			u32mseTimeout,
			pFunCallBack);


	uint8_t retval = Modbus_Stack_API_Call(WRITE_SINGLE_REG,
			pstMbusApiPram,
			NULL);

	//EXPECT_EQ(1, retval);
	EXPECT_EQ(11, retval);
}


/***********************************WriteMulCoil()*************************************/

TEST_F(ModbusStackInterface_ut, WriteMulCoil)
{


	Modbus_Write_Multiple_Coils(u16Startcoil,
			u16NumOfCoil,
			u16TransacID,
			pu8OutputVal,
			u8UnitId,
			pstMbusApiPram->m_u8IpAddr,
			u16Port,
			lPriority,
			u32mseTimeout,
			pFunCallBack);

	uint8_t retval = Modbus_Stack_API_Call(WRITE_MULTIPLE_COILS,
			pstMbusApiPram,
			NULL);

	//EXPECT_EQ(1, retval);
	EXPECT_EQ(11, retval);
}



/***********************************WriteMulReg()************************************/

TEST_F(ModbusStackInterface_ut, WriteMulReg)
{

	Modbus_Write_Multiple_Register(u16StartReg,
			u16NumOfReg,
			u16TransacID,
			pu8OutputVal,
			u8UnitId,
			pstMbusApiPram->m_u8IpAddr,
			u16Port,
			lPriority,
			u32mseTimeout,
			pFunCallBack);

	uint8_t retval = Modbus_Stack_API_Call(WRITE_MULTIPLE_REG,
			pstMbusApiPram,
			NULL);

	//EXPECT_EQ(1, retval);
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


/*******************************************************************************************/
#endif
