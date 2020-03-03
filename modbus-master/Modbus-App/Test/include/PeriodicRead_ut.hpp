/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#ifndef __PERIODICREAD_H
#define __PERIODICREAD_H

#include <gtest/gtest.h>
#include <string.h>
#include "API.h"
#include "PeriodicRead.hpp"
#include "ZmqHandler.hpp"
#include "NetworkInfo.hpp"
#include "PeriodicReadFeature.hpp"
#include "Common.hpp"


class PeriodicRead_ut : public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();

public:
	uint8_t						u8UnitID = 0;
	uint16_t					u16TransacID = 0;
	uint8_t						u8IpAddr = 0;
	uint16_t u16Port = 0;
	uint8_t*					pu8IpAddr = &u8IpAddr;
	uint8_t						u8FunCode = 0;

	stException_t				stException_t_obj;
	stException_t*				pstException = &stException_t_obj;

	uint8_t						u8numBytes = 0;
	uint8_t						pu8data[8] = {1,2,3,4,5,6,7,8};

	uint16_t					u16StartAddress = 0;
	uint16_t					u16Quantity = 0;

	string						test_str = "";

	bool 						Temp_Bool = false;
	stTimeStamps a_objStackTimestamps;

	struct Some_Other_Str
	{
		int	a;
		int	b;
	};
	struct Some_Other_Str		Some_Other_Str_obj;

	network_info::CWellSiteInfo			CWellSiteInfo_obj;
	network_info::CWellSiteDevInfo		CWellSiteDevInfo_obj;
	network_info::CDataPoint			CDataPoint_obj;

	string		str1;
	string		str2;
	string		str3;

	network_info::CUniqueDataPoint CUniqueDataPoint_obj
	{
		"Test_String",
		CWellSiteInfo_obj,
		CWellSiteDevInfo_obj,
		CDataPoint_obj
	};

	zmq_handler::stZmqContext stZmqContext_obj;
	zmq_handler::stZmqPubContext stZmqPubContext_obj;

	CRefDataForPolling CRefDataForPolling_obj
	{
		CUniqueDataPoint_obj,
		stZmqContext_obj,
		stZmqPubContext_obj,
		16
	};

	stMbusAppCallbackParams_t MbusAppCallbackParams;
	stMbusAppCallbackParams_t *pstMbusAppCallbackParams = &MbusAppCallbackParams;

};





#endif /*__PERIODICREAD_H */
