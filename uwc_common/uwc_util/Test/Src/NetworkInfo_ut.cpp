/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../include/NetworkInfo_ut.hpp"

void NetworkInfo_ut::SetUp()
{
	// Setup code
}

void NetworkInfo_ut::TearDown()
{
	// TearDown code
}

// addDataPoint: datapoints not added previously
TEST_F(NetworkInfo_ut, addDataPoint_NotPresent)
{
	Int_Res = CDataPointsYML_obj.addDataPoint(CDataPoint_obj);

	EXPECT_EQ(0, Int_Res);
}

// addDataPoint: datapoints added previously
TEST_F(NetworkInfo_ut, addDataPoint_AlreadyPresent)
{
	Int_Res = CDataPointsYML_obj.addDataPoint(CDataPoint_obj);
	Int_Res = CDataPointsYML_obj.addDataPoint(CDataPoint_obj);

	EXPECT_EQ(-1, Int_Res);
}

// addDevice: Device not added previously
TEST_F(NetworkInfo_ut, addDevice_NotPresent)
{
	YAML::Node a_oData = YAML::LoadFile("UT_Yml/Device_group1.yml");

	for (auto test : a_oData)
	{
		if(test.second.IsSequence() && test.first.as<std::string>() == "devicelist")
		{
			const YAML::Node& list = test.second;
			for (auto nodes : list)
			{
				network_info::CWellSiteDevInfo::build(nodes, CWellSiteDevInfo_obj);

				Int_Res = CWellSiteInfo_obj.addDevice(CWellSiteDevInfo_obj);

				if( network_info::eNetworkType::eTCP == CWellSiteDevInfo_obj.getAddressInfo().m_NwType )
				{
					EXPECT_EQ(0, Int_Res);
				}
			}
		}
	}
}

// addDevice: Device already added previously
TEST_F(NetworkInfo_ut, addDevice_Present)
{
	YAML::Node a_oData = YAML::LoadFile("UT_Yml/Device_group1.yml");

	for (auto test : a_oData)
	{
		if(test.second.IsSequence() && test.first.as<std::string>() == "devicelist")
		{
			const YAML::Node& list = test.second;
			for (auto nodes : list)
			{
				network_info::CWellSiteDevInfo::build(nodes, CWellSiteDevInfo_obj);

				Int_Res = CWellSiteInfo_obj.addDevice(CWellSiteDevInfo_obj);
				Int_Res = CWellSiteInfo_obj.addDevice(CWellSiteDevInfo_obj);

				if( network_info::eNetworkType::eTCP == CWellSiteDevInfo_obj.getAddressInfo().m_NwType )
				{
					EXPECT_EQ(-1, Int_Res);
				}
			}
		}
	}
}

// CWellSiteInfo::build: Successfully gets ID from yml file
TEST_F(NetworkInfo_ut, CWellSiteInfo_build_GetsID)
{
	YAML::Node a_oData = YAML::LoadFile("UT_Yml/Device_group1.yml");

	network_info::CWellSiteInfo::build(a_oData, CWellSiteInfo_obj);

	EXPECT_EQ( "PL0", CWellSiteInfo_obj.getID() );

}

// CWellSiteInfo::build: When "id" keyword is not present in yml file
TEST_F(NetworkInfo_ut, CWellSiteInfo_build_NoID)
{
	YAML::Node WrongNode = YAML::LoadFile("UT_Yml/Devices_group_list.yml");

	try
	{
		network_info::CWellSiteInfo::build(WrongNode, CWellSiteInfo_obj);
		EXPECT_EQ(1, 2); //Fails; Throw shoud be executed
	}
	catch(YAML::Exception &e)
	{
		EXPECT_EQ("Id key not found", (string)e.what());
	}
}

// CWellSiteInfo::build: When duplicate devices are present
TEST_F(NetworkInfo_ut, CWellSiteInfo_build_DuplicateDevice)
{
	YAML::Node a_oData = YAML::LoadFile("UT_Yml/Device_group1.yml");

	network_info::CWellSiteInfo::build(a_oData, CWellSiteInfo_obj);
	network_info::CWellSiteInfo::build(a_oData, CWellSiteInfo_obj);

	EXPECT_EQ( "PL0", CWellSiteInfo_obj.getID() );
}

// CWellSiteDevInfo::build: When IP address/port/unitId is empty in yml(TCP) file
TEST_F(NetworkInfo_ut, CWellSiteDevInfo_build_NoIPaDDress)
{
	YAML::Node Node_IP_Blank = YAML::LoadFile("UT_Yml/Device_group1_UT.yml");

	for (auto test : Node_IP_Blank)
	{
		if(test.second.IsSequence() && test.first.as<std::string>() == "devicelist")
		{
			const YAML::Node& list = test.second;
			for (auto nodes : list)
			{
				network_info::CWellSiteDevInfo::build(nodes, CWellSiteDevInfo_obj);

				EXPECT_EQ( "", CWellSiteDevInfo_obj.getAddressInfo().m_stTCP.m_sIPAddress );
			}
		}
	}
}

// buildNetworkInfo: TCP Network
TEST_F(NetworkInfo_ut, buildNetworkInfo_TCP_Success)
{
	network_info::buildNetworkInfo("TCP", "Devices_group_list.yml", "TestApp");
}

// buildNetworkInfo: called twice; Should return without doing anything
TEST_F(NetworkInfo_ut, buildNetworkInfo_RTU)
{

	network_info::buildNetworkInfo("RTU", "Devices_group_list.yml", "TestApp");
}

