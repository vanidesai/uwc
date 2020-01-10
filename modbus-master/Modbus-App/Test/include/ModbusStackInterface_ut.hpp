/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#ifndef TEST_INCLUDE_MODBUSSTACKINTERFACE_UT_HPP_
#define TEST_INCLUDE_MODBUSSTACKINTERFACE_UT_HPP_

#include <mutex>
#include <string.h>
#include <stdlib.h>
#include "PublishJson.hpp"
#include "ZmqHandler.hpp"
#include "Common.hpp"
#include "API.h"
#include "gtest/gtest.h"



class ModbusStackHandler_ut : public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();

public:
	uint8_t  u8UnitID = 0;
	uint8_t	 u8IpAddr = 0;
	uint16_t u16TransacID = 2;
	uint8_t* pu8IpAddr = &u8IpAddr;
	uint16_t u16Port = 0;

	stException_t stException_t_obj;
	stException_t *pstException =&stException_t_obj;
	uint8_t  u8numBytes =0;
	uint8_t* pu8data[8] = {1,2,3,4,5,6,7,8};
	uint16_t  u16StartAdd = 0;
	uint16_t  u16Quantity = 0;
	MbusAPI MbusAPI_obj;
	MbusAPI *pstMbusApiPram= &MbusAPI_obj;
	MbusAPI_t stMbusApiPram = {};
	RestMbusReqGeneric_t RestMbusReqGeneric_t_obj;
	RestMbusReqGeneric_t *pstMbusReqGen = &RestMbusReqGeneric_t_obj;
//	RestMbusReqGeneric_t *pstMbusReqGen = NULL;
//	RestMbusReqGeneric_t *pstModbusRxPacket = NULL;
	RestMbusReqGeneric_t *pstModbusRxPacket = &RestMbusReqGeneric_t_obj;
	eMbusStackErrorCode eFunRetType = MBUS_STACK_NO_ERROR;



};


#endif /* TEST_INCLUDE_MODBUSSTACKINTERFACE_UT_HPP_ */
