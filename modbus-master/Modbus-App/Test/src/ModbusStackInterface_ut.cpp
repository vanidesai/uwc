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

extern uint8_t Modbus_Stack_API_Call(unsigned char u8FunCode, MbusAPI_t *pstMbusApiPram,
					void* vpCallBackFun);

void ModbusStackHandler_ut::SetUp()
{
	// Setup code
}

void ModbusStackHandler_ut::TearDown()
{
	// TearDown code
}


/***********************************ModbusMaster_AppCallback()*************************************/
TEST_F(ModbusStackHandler_ut, AppCallback01)
{
	uint8_t  u8FunCode = 1;
	//setenv("WRITE_RESPONSE_TOPIC", , 1);
	const char* pcWriteRespTopic = std::getenv("WRITE_RESPONSE_TOPIC");

	try
	{
		msg_envelope_t *msg = NULL;
		zmq_handler::insertAppSeq(2, "1111");
		ModbusMaster_AppCallback(u8UnitID,
				               u16TransacID,
							   NULL,
							   u8FunCode,
							   NULL,
							   u8numBytes,
							   NULL,
							   u16StartAdd,
							   u16Quantity);


	}
	catch(std::exception &e)
	{
		EXPECT_EQ("", e.what());
	}
}

TEST_F(ModbusStackHandler_ut, AppCallback1)
{
	uint8_t  u8FunCode = 1;
	setenv("WRITE_RESPONSE_TOPIC", "PL_0", 1);

	try
	{
		msg_envelope_t *msg = NULL;
		zmq_handler::insertAppSeq(2, "1111");
		ModbusMaster_AppCallback(u8UnitID,
				               u16TransacID,
							   NULL,
							   u8FunCode,
							   NULL,
							   u8numBytes,
							   NULL,
							   u16StartAdd,
							   u16Quantity);


	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ", e.what());
	}
}









TEST_F(ModbusStackHandler_ut, AppCallback2)
{
	uint8_t  u8FunCode = 1;
	setenv("WRITE_RESPONSE_TOPIC", "PL_0", 1);

	try
	{
		msg_envelope_t *msg = NULL;
		zmq_handler::insertAppSeq(2, "1111");
		ModbusMaster_AppCallback(u8UnitID,
				               u16TransacID,
							   *pu8IpAddr,
							   u8FunCode,
							   NULL,
							   u8numBytes,
							   NULL,
							   u16StartAdd,
							   u16Quantity);


	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ", e.what());
		cout<<e.what();
	}
}

TEST_F(ModbusStackHandler_ut, AppCallback3)
{
	uint8_t  u8FunCode = 1;
	setenv("WRITE_RESPONSE_TOPIC", "PL_0", 1);

	try
	{
		pstException->m_u8ExcCode = 0;
		pstException->m_u8ExcStatus = 0;


		zmq_handler::insertAppSeq(2, "1111");
		ModbusMaster_AppCallback(u8UnitID,
				               u16TransacID,
							   pu8IpAddr,
							   u8FunCode,
							   pstException,
							   u8numBytes,
							   NULL,
							   u16StartAdd,
							   u16Quantity);



	}
	catch(std::exception &e)
	{

		EXPECT_EQ(" ", e.what());
	}
}




/***************************************Modbus_Stack_API_Call()*****************************************/

TEST_F(ModbusStackHandler_ut, stack_API_call1)
{
	uint8_t  u8FunCode = 1;
	try
	{
		uint8_t val = Modbus_Stack_API_Call(u8FunCode, pstMbusApiPram, (void *)ModbusMaster_AppCallback);

	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ", e.what());

	}
}



TEST_F(ModbusStackHandler_ut, stack_API_call2)
{
	uint8_t  u8FunCode = 2;
	try
	{
		uint8_t val = Modbus_Stack_API_Call(u8FunCode, pstMbusApiPram, (void *)ModbusMaster_AppCallback);
		EXPECT_EQ(1,1);
	}
	catch(exception &ex) {
		EXPECT_EQ(0,1);
	}
}


TEST_F(ModbusStackHandler_ut, stack_API_call3)
{
	uint8_t  u8FunCode = 3;
	try
	{
		uint8_t val = Modbus_Stack_API_Call(u8FunCode, pstMbusApiPram, (void *)ModbusMaster_AppCallback);
		EXPECT_EQ(1,1);
	}
	catch(exception &ex) {
		EXPECT_EQ(0,1);
	}
}


TEST_F(ModbusStackHandler_ut, stack_API_call4)
{
	uint8_t  u8FunCode = 4;
	try
	{
		uint8_t val = Modbus_Stack_API_Call(u8FunCode, pstMbusApiPram, (void *)ModbusMaster_AppCallback);
		EXPECT_EQ(1,1);
	}
	catch(exception &ex) {
		EXPECT_EQ(0,1);
	}
}


//TEST_F(ModbusStackHandler_ut, stack_API_call5)
//{
//	uint8_t  u8FunCode = 5;
//	try
//	{
//		uint8_t val = Modbus_Stack_API_Call(u8FunCode, pstMbusApiPram, (void *)ModbusMaster_AppCallback);
//		EXPECT_EQ(1,1);
//	}
//	catch(exception &ex) {
//		EXPECT_EQ(0,1);
//	}
//}

//
//TEST_F(ModbusStackHandler_ut, stack_API_call6)
//{
//	uint8_t  u8FunCode = 6;
//	try
//	{
//		uint8_t val = Modbus_Stack_API_Call(u8FunCode, pstMbusApiPram, (void *)ModbusMaster_AppCallback);
//		EXPECT_EQ(1,1);
//	}
//	catch(exception &ex) {
//		EXPECT_EQ(0,1);
//	}
//}




TEST_F(ModbusStackHandler_ut, stack_API_call15)
{
	uint8_t  u8FunCode = 15;
	try
	{
		uint8_t val = Modbus_Stack_API_Call(u8FunCode, pstMbusApiPram, (void *)ModbusMaster_AppCallback);
		EXPECT_EQ(1,1);
	}
	catch(exception &e) {
		EXPECT_EQ(" ", e.what());
	}
}

TEST_F(ModbusStackHandler_ut, stack_API_call16)
{
	uint8_t  u8FunCode = 16;
	try
	{
		uint8_t val = Modbus_Stack_API_Call(u8FunCode, pstMbusApiPram, (void *)ModbusMaster_AppCallback);
		EXPECT_EQ(1,1);
	}
	catch(exception &e) {
		EXPECT_EQ(" ", e.what());
	}
}

TEST_F(ModbusStackHandler_ut, stack_API_call20)
{
	uint8_t  u8FunCode = 20;
	try
	{
		uint8_t val = Modbus_Stack_API_Call(u8FunCode, pstMbusApiPram, (void *)ModbusMaster_AppCallback);
		EXPECT_EQ(1,1);
	}
	catch(exception &e) {
		EXPECT_EQ(" ", e.what());
	}
}

TEST_F(ModbusStackHandler_ut, stack_API_call21)
{
	uint8_t  u8FunCode = 21;
	try
	{
		uint8_t val = Modbus_Stack_API_Call(u8FunCode, pstMbusApiPram, (void *)ModbusMaster_AppCallback);
		EXPECT_EQ(1,1);
	}
	catch(exception &e) {
		EXPECT_EQ(" ", e.what());
	}
}



TEST_F(ModbusStackHandler_ut, stack_API_call23)
{
	uint8_t  u8FunCode = 23;
	try
	{
		uint8_t val = Modbus_Stack_API_Call(u8FunCode, pstMbusApiPram, (void *)ModbusMaster_AppCallback);
		EXPECT_EQ(1,1);
	}
	catch(exception &ex) {
		EXPECT_EQ(0,1);
	}
}


TEST_F(ModbusStackHandler_ut, stack_API_call43)
{
	uint8_t  u8FunCode = 43;
	try
	{
		uint8_t val = Modbus_Stack_API_Call(u8FunCode, pstMbusApiPram, (void *)ModbusMaster_AppCallback);
		EXPECT_EQ(1,1);
	}
	catch(exception &ex) {
		EXPECT_EQ(0,1);
	}
}

TEST_F(ModbusStackHandler_ut, MbusApp_Process_Request1)
{
	if(MBUS_STACK_NO_ERROR == eFunRetType)

			{
				modbusInterface MbusInterface;// = new modbusInterface();

				eFunRetType = (eMbusStackErrorCode)MbusInterface.
						MbusApp_Process_Request(pstModbusRxPacket);
			}
}
















