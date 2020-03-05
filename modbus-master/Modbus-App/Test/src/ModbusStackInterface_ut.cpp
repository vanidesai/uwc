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

/***Test:ModbusStackHandler_ut::AppCallback_allNULLPointerInput ***/
/**ModbusMaster_AppCallback() should throw exception as the *pu8IpAddr, *pstException, *pu8data these are given as NUll **/


TEST_F(ModbusStackHandler_ut, AppCallback_allNULLPointerInput)
{
	uint8_t  u8FunCode = 1;
	setenv("WRITE_RESPONSE_TOPIC", "PL_0", 1);

	try
	{
		msg_envelope_t *msg = NULL;
		//zmq_handler::insertAppSeq(2, "1111");
		ModbusMaster_AppCallback(u8UnitID,
				               u16TransacID,
							   NULL,
							   u16Port,
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







/*** Test:ModbusStackHandler_ut::AppCallback_twoNullInput***/

/*** ModbusMaster_AppCallback() should throw exception as *pstException, *pu8data these input pointers are NULL  */

TEST_F(ModbusStackHandler_ut, AppCallback_twoNullInput)
{
	uint8_t  u8FunCode = 1;
	setenv("WRITE_RESPONSE_TOPIC", "PL_0", 1);

	try
	{
		msg_envelope_t *msg = NULL;
	//	zmq_handler::insertAppSeq(2, "1111");
		ModbusMaster_AppCallback(u8UnitID,
				               u16TransacID,
							   *pu8IpAddr,
							   u16Port,
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

/*** Test:ModbusStackHandler_ut::AppCallbackOneNullInput***/
/** ModbusMaster_AppCallback() should throw exception as one input NULL pointer **/

TEST_F(ModbusStackHandler_ut, AppCallbackOneNullInput)
{
	uint8_t  u8FunCode = 1;
	setenv("WRITE_RESPONSE_TOPIC", "PL_0", 1);

	try
	{
		pstException->m_u8ExcCode = 0;
		pstException->m_u8ExcStatus = 0;


	//	zmq_handler::insertAppSeq(2, "1111");
		ModbusMaster_AppCallback(u8UnitID,
				               u16TransacID,
							   pu8IpAddr,
							   u16Port,
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

/***Test:ModbusStackHandler_ut::stack_API_call_stackError ***/


TEST_F(ModbusStackHandler_ut, stack_API_call_stackError)
{
	uint8_t  u8FunCode = 11;
	try
	{
		uint8_t val = Modbus_Stack_API_Call(u8FunCode, pstMbusApiPram, (void *)ModbusMaster_AppCallback);

	}
	catch(std::exception &e)
	{
		EXPECT_EQ(" ", e.what());

	}
}

/***Test:ModbusStackHandler_ut::stack_API_call_READ_INPUT_STATUS  ***/

TEST_F(ModbusStackHandler_ut, stack_API_call_READ_INPUT_STATUS)
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
/***Test:ModbusStackHandler_ut::stack_API_call_READ_HOLDING_REG  ***/

TEST_F(ModbusStackHandler_ut, stack_API_call_READ_HOLDING_REG)
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

/***Test:*ModbusStackHandler_ut::stack_API_call_READ_INPUT_REG ***/

TEST_F(ModbusStackHandler_ut, stack_API_call_READ_INPUT_REG)
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
/****Test:ModbusStackHandler_ut::stack_API_call_WRITE_MULTIPLE_COILS ***/

TEST_F(ModbusStackHandler_ut, stack_API_call1_WRITE_MULTIPLE_COILS)
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

/***Test:*ModbusStackHandler_ut::stack_API_call_WRITE_MULTIPLE_REG ***/

TEST_F(ModbusStackHandler_ut, stack_API_call_WRITE_MULTIPLE_REG)
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

/****Test:ModbusStackHandler_ut::stack_API_call_READ_FILE_RECORD ***/

TEST_F(ModbusStackHandler_ut, stack_API_call_READ_FILE_RECORD)
{
	uint8_t  u8FunCode = 20;
	try
	{
		uint8_t val = Modbus_Stack_API_Call(u8FunCode, pstMbusApiPram, (void *)ModbusMaster_AppCallback);
		EXPECT_EQ(11,(int)val);
	}
	catch(exception &e) {
		EXPECT_EQ(" ", e.what());
	}
}


/****Test:ModbusStackHandler_ut::stack_API_call_WRITE_FILE_RECORD ***/
TEST_F(ModbusStackHandler_ut, stack_API_call_WRITE_FILE_RECORD)
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


/****Test:ModbusStackHandler_ut::stack_API_call_READ_WRITE_MUL_REG ***/
TEST_F(ModbusStackHandler_ut, stack_API_call_READ_WRITE_MUL_REG)
{
	uint8_t  u8FunCode = 23;
	try
	{
		uint8_t val = Modbus_Stack_API_Call(u8FunCode, &stMbusApiPram, (void *)ModbusMaster_AppCallback);
		EXPECT_EQ(11,(int)val);
	}
	catch(exception &ex) {
		EXPECT_EQ(0,1);
	}
}

/****Test:ModbusStackHandler_ut::stack_API_call_READ_DEVICE_IDENTIFICATION ***/

TEST_F(ModbusStackHandler_ut, stack_API_call_READ_DEVICE_IDENTIFICATION)
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

/***Test:ModbusStackHandler_ut::MbusApp_Process_Request1***/

TEST_F(ModbusStackHandler_ut, MbusApp_Process_Request1)
{
	if(MBUS_STACK_NO_ERROR == eFunRetType)

			{
				modbusInterface MbusInterface;// = new modbusInterface();

				eFunRetType = (eMbusStackErrorCode)MbusInterface.
						MbusApp_Process_Request(pstModbusRxPacket);
			}
}








