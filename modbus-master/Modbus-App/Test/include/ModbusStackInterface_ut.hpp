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

class ModbusStackInterface_ut : public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();

public:

	uint16_t u16TransacID = 0;
	uint16_t u16Port = 0;

	MbusAPI_t MbusAPI_obj = {0, NULL, 0, 0, 0, 0, 0, NULL};
	MbusAPI_t *pstMbusApiPram = &MbusAPI_obj;
	MbusAPI_t stMbusApiPram = {};

	uint16_t m_u16StartAddr = 0;
	uint16_t m_u16Quantity = 0;
	uint16_t m_u16TxId = 0;
	long lPriority = 0;
		uint32_t u32mseTimeout = 0 ;
	unsigned char m_u8DevId = 0;
	unsigned char m_u8IpAddr = 0;
	//unsigned char* m_pu8IpAddr = NULL;
	unsigned short m_u16Port = 0;
	uint8_t  u8FunCode = READ_COIL_STATUS;
	stTimeStamps objStackTimestamps;
	//unsigned short  u8FunCode = 1;

	 uint8_t  u8numBytes =7;
	void* vpCallBackFun = NULL;
	uint16_t u16StartDI = 0;
	uint16_t u16NumOfDI = 0;
	uint8_t u8UnitId = 0;
	uint8_t *pu8SerIpAddr = NULL;
	void* pFunCallBack = NULL;
	uint16_t u16StartReg = 0;
	uint16_t u16NumberOfRegisters = 0;
	uint16_t u16StartCoil = 0;
	uint16_t u16OutputVal = 0;
	uint16_t u16RegOutputVal = 0;
	uint16_t u16Startcoil = 0;
	uint16_t u16NumOfCoil = 0;
	uint8_t  *pu8OutputVal = NULL;
	uint16_t u16NumOfReg = 0;
	 uint8_t pu8IpAddr[4];

	 //uint8_t pu8data[7] = {1,2,3,4,5,6,7};
	 uint8_t pu8data[7] = {};

	 stException_t stException_t_obj = {0};
	 stException_t* pstException = &stException_t_obj;
	 /*pstException->m_u8ExcCode = 0;
	 pstException->m_u8ExcStatus = 0;*/



	uint8_t result;

	stMbusAppCallbackParams_t stMbusAppCallbackParams;
	stMbusAppCallbackParams_t *pstMbusAppCallbackParams = &stMbusAppCallbackParams;
	stMbusAppCallbackParams_t *MbusAppCallbackParams = NULL;


};


#endif /* TEST_INCLUDE_MODBUSSTACKINTERFACE_UT_HPP_ */
