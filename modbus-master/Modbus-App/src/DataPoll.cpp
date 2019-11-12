/*
 * DataPoll.cpp
 *
 *  Created on: 05-Nov-2019
 *      Author: user
 */
#include "DataPoll.hpp"

bool PeriodicDataPoll::initDataPoll()
{
	bool retVal = false;
	eMbusStackErrorCode eStackRetType = MBUS_STACK_NO_ERROR;

	RestRdPeriodicTagPart_t MbusRdPeriodic = {};
	RestRdPeriodicTagPart_t *pstModbusRxPacket = &MbusRdPeriodic;

	eStackRetType = filljsontoReq(pstModbusRxPacket);

	if(MBUS_STACK_NO_ERROR == eStackRetType)
	{
		eStackRetType = SubscibeOrUnSubPeriodicRead(MbusRdPeriodic);
	}

	return retVal;
}

eMbusStackErrorCode PeriodicDataPoll::filljsontoReq(RestRdPeriodicTagPart_t *pstModbusRxPacket)
{
	eMbusStackErrorCode retVal = MBUS_JSON_APP_ERROR_INVALID_INPUT_PARAMETER;

	pstModbusRxPacket->m_u8IpAddr[0] = 192;
	pstModbusRxPacket->m_u8IpAddr[1] = 168;
	pstModbusRxPacket->m_u8IpAddr[2] = 80;
	pstModbusRxPacket->m_u8IpAddr[3] = 101;

	pstModbusRxPacket->m_u8UnitId = 85;
	pstModbusRxPacket->m_u16StartAddr = 1;
	pstModbusRxPacket->m_u32Interval = 10;
	pstModbusRxPacket->m_stHaystackInfo.m_stHaystackId = "123123";
	pstModbusRxPacket->m_stHaystackInfo.m_stKind = "Number";
	pstModbusRxPacket->m_stHaystackInfo.m_stDataType =MBUS_DT_INVALID;
	pstModbusRxPacket->m_stHaystackInfo.m_stEnum = "2222";
	pstModbusRxPacket->m_stHaystackInfo.m_sMqttTopic = "periodicResponseModbusNumber";
	pstModbusRxPacket->IsSubscription = (uint8_t)1;
	pstModbusRxPacket->m_u8FunCode = (uint8_t)1;
	pstModbusRxPacket->m_u16Quantity = (uint16_t)1;
	pstModbusRxPacket->m_u16ReqDataLen = (uint8_t)1;

	retVal = MBUS_STACK_NO_ERROR;

	return retVal;
}



