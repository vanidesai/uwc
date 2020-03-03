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



void ModbusMaster_AppCallback(stMbusAppCallbackParams_t *pstMbusAppCallbackParams);
void ModbusStackInterface_ut::SetUp()
{
	// Setup code
}

void ModbusStackInterface_ut::TearDown()
{
	// TearDown code
}






// This test is commented because this test is not running through the script





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

		zmq_handler::insertOnDemandReqData(u16TxID, reqData);

		zmq_handler::prepareCommonContext("pub");
		zmq_handler::getOnDemandReqData(0, onDemandReqData);

		ModbusMaster_AppCallback(pstMbusAppCallbackParams);

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
	/*ModbusMaster_AppCallback(u8UnitId,
						u16TransacID,
						pu8IpAddr,
						u8FunCode,
						pstException,
						u8numBytes,
						pu8data,
						m_u16StartAddr,
						m_u16Quantity,
						objStackTimestamps);*/
	ModbusMaster_AppCallback(pstMbusAppCallbackParams);

}
catch(std::exception &e)
{
	EXPECT_EQ("", e.what());
}
}

#endif








/************************************MOdbusMAster_AppCallback()*******************************/


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

		zmq_handler::insertOnDemandReqData(52, reqData);

		/*setenv("PubTopics", "Modbus-ReadRequest", 1);
		setenv("Modbus-ReadRequest_cfg", "zmq_tcp, 127.0.0.1:1234", 1);
		zmqhandler.prepareCommonContext("pub");*/
		//std::string sRespTopic = PublishJsonHandler::instance(msgbusMgr, msgbusEnvelope).getSReadResponseTopic();
		/*zmq_handler::stZmqContext objTempCtx;
		zmq_handler::stZmqPubContext objTempPubCtx;

		zmqhandler.insertCTX(sRespTopic, objTempCtx);
		zmq_handler::stZmqContext MsgbusCtx = zmqhandler.getCTX(sRespTopic);


		zmqhandler.insertPubCTX(sRespTopic, objTempPubCtx);
		zmq_handler::stZmqPubContext pubCtx = zmqhandler.getPubCTX(sRespTopic);*/


		ModbusMaster_AppCallback(pstMbusAppCallbackParams);

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

	//zmq_handler::getOnDemandReqData(u16TransacID, onDemandReqData);
	ModbusMaster_AppCallback(pstMbusAppCallbackParams);


}
catch(std::exception &e)
{
	EXPECT_EQ("", e.what());
}
}

#endif





/************************************ModbusMaster_AppCallback()*******************************/

/*
TEST_F(ModbusStackInterface_ut, app_callBack)
{

	try
	{
		ModbusMaster_AppCallback(u8UnitId,
				 	 	 	 	 	u16TransacID,
									 pu8IpAddr,
									u16Port,
									u8FunCode,
									NULL,
									u8numBytes,
									pu8data,
									m_u16StartAddr,
									m_u16Quantity,
									 objStackTimestamps);
	}
	catch(std::exception &e)
	{
		cout<<"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$"<<endl;
		cout<<e.what()<<endl;
		cout<<"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$"<<endl;
	}
}

 */


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


/*******************************************************************************************/
#endif



#if 0
/***********************************ModbusMaster_AppCallback()*************************************/
TEST_F(ModbusStackInterface_ut, AppCallback01)
{
	uint8_t  u8FunCode = 1;


	try
	{
		//msg_envelope_t *msg = NULL;
		stOnDemandRequest reqData;
		zmq_handler::insertOnDemandReqData(2, reqData);
		ModbusMaster_AppCallback(u8UnitId,
				u16TransacID,
				NULL,
				u16Port,
				u8FunCode,
				NULL,
				u8numBytes,
				NULL,
				m_u16StartAddr,
				m_u16Quantity,
				objStackTimestamps);


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
		//zmq_handler::insertAppSeq(2, "1111");
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
#endif

