/********************************************************************************
* Copyright (c) 2021 Intel Corporation.

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*********************************************************************************/
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

extern std::atomic<bool> g_stopTimer;

class PeriodicRead_ut : public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();

public:
	uint8_t	u8UnitID = 0;
	uint16_t u16TransacID = 0;
	uint8_t	u8IpAddr = 0;
	uint16_t u16Port = 0;
	uint8_t* pu8IpAddr = &u8IpAddr;
	uint8_t	u8FunCode = 0;

	stException_t stException_t_obj;
	stException_t* pstException = &stException_t_obj;

	uint8_t	u8numBytes = 0;
	uint8_t	pu8data[8] = {1,2,3,4,5,6,7,8};

	uint16_t u16StartAddress = 0;
	uint16_t u16Quantity = 0;

	string test_str = "";

	bool Temp_Bool = false;
	stTimeStamps a_objStackTimestamps;

	struct Some_Other_Str
	{
		int	a;
		int	b;
	};
	struct Some_Other_Str Some_Other_Str_obj;

	std::string YmlFile = "flowmeter_datapoints.yml";
	std::string DevName = "Device";
	network_info::CDataPointsYML CDataPointsYML_obj{YmlFile};
	network_info::CDeviceInfo CDeviceInfo_obj{YmlFile, DevName, CDataPointsYML_obj};
	network_info::CWellSiteInfo	CWellSiteInfo_obj;
	network_info::CWellSiteDevInfo CWellSiteDevInfo_obj{CDeviceInfo_obj};
	network_info::CDataPoint CDataPoint_obj;

	string str1;
	string str2;
	string str3;
	string Test_String = "Test_String";

//	const CUniqueDataPoint &

	network_info::CUniqueDataPoint CUniqueDataPoint_obj
	{
		Test_String,
		CWellSiteInfo_obj,
		CWellSiteDevInfo_obj,
		CDataPoint_obj
	};

//	zmq_handler::stZmqContext &busCTX = zmq_handler::getCTX("MQTT_Export_RdReq");
//	zmq_handler::stZmqPubContext &pubCTX = zmq_handler::getPubCTX("TCP1_PolledData");
//
//	uint8_t temp = 16;

//	CRefDataForPolling CRefDataForPolling_obj
//	{
//		(const CUniqueDataPoint &)CUniqueDataPoint_obj,
//		busCTX,
//		pubCTX,
//		temp
//	};

	stMbusAppCallbackParams_t MbusAppCallbackParams;
	stMbusAppCallbackParams_t *pstMbusAppCallbackParams = &MbusAppCallbackParams;
	stMbusAppCallbackParams_t *stMbusAppCallbackParams = NULL;
	std::vector<uint8_t> Vec;


	struct timespec tsPoll = {0};

};


#endif /*__PERIODICREAD_H */
