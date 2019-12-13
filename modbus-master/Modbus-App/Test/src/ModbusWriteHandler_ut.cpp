/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/


#include "../include/ModbusWriteHandler_ut.h"


void ModbusWriteHandler_ut::SetUp()
{
	// Setup code
}

void ModbusWriteHandler_ut::TearDown()
{
	// TearDown code
}

//test 01:: Check the instance type returned by function

TEST_F(ModbusWriteHandler_ut, modWriteInfo_getInstance)
{
	EXPECT_EQ(typeid(modWriteInfo), typeid(modWriteInfo::Instance()));
}

//test 02:: Check the instance type returned by function

TEST_F(ModbusWriteHandler_ut, writeInfoHandler)
{
	EXPECT_EQ(typeid(MbusStackErrorCode), typeid(modWriteInfo::Instance().writeInfoHandler(topic, msg)));
}

//test 03:: Check the instance type returned by function

TEST_F(ModbusWriteHandler_ut, jsonParserForWrite)
{
	//	const string topic = "MQTT-EXPORT/PL0_flowmeter1_write,MQTT-EXPORT/PL0_flowmeter2_write";
	//	string msg = "{ 	\"value\": \"0xFF00\", 	\"command\": \"Pointname\", 	\"app_seq\": \"1234\" }";

	RestMbusReqGeneric_t *pstModbusRxPacket = NULL;
	pstModbusRxPacket = new RestMbusReqGeneric_t();
	pstModbusRxPacket->m_stReqData.m_pu8Data = NULL;
	pstModbusRxPacket->m_u16ReffId = 1;

	EXPECT_EQ(typeid(MbusStackErrorCode), typeid(modWriteInfo::Instance().jsonParserForWrite(topic, msg, pstModbusRxPacket)));
}


//test 01:: Check the instance type returned by function

TEST_F(ModbusWriteHandler_ut, modWriteHandler_getInstance)
{
	EXPECT_EQ(typeid(modWriteHandler), typeid(modWriteHandler::Instance()));
}

//test 02:: Check the instance type returned by function

TEST_F(ModbusWriteHandler_ut, zmqReadDeviceMessage)
{
	modWriteHandler::Instance().zmqReadDeviceMessage();
}

//test 02:: Check the instance type returned by function

TEST_F(ModbusWriteHandler_ut, writeToDevicwe)
{
	modWriteHandler::Instance().writeToDevicwe(topic, msg);
}

//test 02:: Check the instance type returned by function

TEST_F(ModbusWriteHandler_ut, subscribeDeviceListener)
{
	modWriteHandler::Instance().subscribeDeviceListener(topic);
}

//test 02:: Check the instance type returned by function

TEST_F(ModbusWriteHandler_ut, ltrim)
{
	modWriteHandler::Instance().ltrim(str);
}

//test 02:: Check the instance type returned by function

TEST_F(ModbusWriteHandler_ut, rtrim)
{
	modWriteHandler::Instance().rtrim(str);
}

//test 02:: Check the instance type returned by function

TEST_F(ModbusWriteHandler_ut, trim)
{
	modWriteHandler::Instance().trim(str);
}

//test 02:: Check the instance type returned by function

TEST_F(ModbusWriteHandler_ut, tokenize)
{
	std::vector<std::string> vec;
	const char del;
	modWriteHandler::Instance().tokenize(str, vec, del);
}
















