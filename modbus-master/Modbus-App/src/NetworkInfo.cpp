/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#include <iostream>
#include <atomic>
#include <map>
#include <algorithm>
#include "NetworkInfo.hpp"
#include "yaml-cpp/eventhandler.h"
#include "yaml-cpp/yaml.h"
#include "utils/YamlUtil.hpp"
#include "BoostLogger.hpp"
#include "ConfigManager.hpp"

using namespace network_info;
using namespace CommonUtils;

std::vector<std::string> g_sWellSiteFileList1;

// Set this if configuration YML files are kept in a docker volume
// This is true for SPRINT 1
#define CONFIGFILES_IN_DOCKER_VOLUME

// Unnamed namespace to define globals 
namespace  
{
	eNetworkType g_eNetworkType{eNetworkType::eTCP};
	std::atomic<bool> g_bIsStarted{false};
	std::map<std::string, CWellSiteInfo> g_mapYMLWellSite;
	std::map<std::string, CUniqueDataPoint> g_mapUniqueDataPoint;
	std::vector<std::string> g_sErrorYMLs;
	unsigned short g_usTotalCnt{0};

	void populateUniquePointData(const CWellSiteInfo &a_oWellSite)
	{
		BOOST_LOG_SEV(lg, debug) << __func__ << " Start";
		for(auto &objWellSiteDev : a_oWellSite.getDevices())
		{
			//unsigned int uiPoint = 0;
			for(auto &objPt : objWellSiteDev.getDevInfo().getDataPoints())
			{
				std::string sUniqueId(a_oWellSite.getID() + SEPARATOR_CHAR + 
										objWellSiteDev.getID() + SEPARATOR_CHAR +
										objPt.getID());
				//BOOST_LOG_SEV(lg, info) << __func__ << " " << sUniqueId;
				
				// Build unique data point
				CUniqueDataPoint oUniquePoint{sUniqueId, a_oWellSite, objWellSiteDev, objPt};
				g_mapUniqueDataPoint.emplace(sUniqueId, oUniquePoint);
				BOOST_LOG_SEV(lg, info) << __func__ << " " << oUniquePoint.getID() << "=" << oUniquePoint.getMyRollID();
			}
		}
		BOOST_LOG_SEV(lg, debug) << __func__ << " End";
	}

	
	#ifdef CONFIGFILES_IN_DOCKER_VOLUME
	std::vector<std::string> g_sWellSiteFileList;

	bool _getWellSiteList()
	{
		BOOST_LOG_SEV(lg, debug) << __func__ << " Start: Reading site_list.yaml";
		try
		{
			if(!CfgManager::Instance().IsClientCreated())
			{
				BOOST_LOG_SEV(lg, error) << __func__ << " ETCD client is not created ..";
				return false;
			}
			char *cEtcdValue  = CfgManager::Instance().getETCDValuebyKey("/Device_Config/site_list.yaml");
			if(NULL == cEtcdValue)
			{
				BOOST_LOG_SEV(lg, error) << __func__ << " No value received from ETCD  ..";
				return false;
			}
			std::string sYamlStr(cEtcdValue);
			YAML::Node Node = loadFromETCD(sYamlStr);
			//YAML::Node Node = CommonUtils::loadYamlFile("site_list.yaml");
			CommonUtils::convertYamlToList(Node, g_sWellSiteFileList);
		}
		catch(YAML::Exception &e)
		{
			BOOST_LOG_SEV(lg, error) << __func__ << " " << e.what();
			return false;
		}
		BOOST_LOG_SEV(lg, debug) << __func__ << " End: ";
		return true;
	}

	#endif
	

}

int network_info::CDeviceInfo::addDataPoint(CDataPoint a_oDataPoint)
{
	// Search whether given point name is already present
	// If present, ignore the datapoint
	// If not present, add it
	BOOST_LOG_SEV(lg, debug) << __func__ << " Start: To add DataPoint - " << a_oDataPoint.getID();
	for(auto oDataPoint: m_DataPointList)
	{
		if(0 == oDataPoint.getID().compare(a_oDataPoint.getID()))
		{
			BOOST_LOG_SEV(lg, error) << __func__ << ": Already present DataPoint with id " << a_oDataPoint.getID();
			// This point name is already present. Ignore this point
			return -1;
		}
	}
	m_DataPointList.push_back(a_oDataPoint);
	BOOST_LOG_SEV(lg, debug) << __func__ << " End: Added DataPoint - " << a_oDataPoint.getID();
	return 0;
}

int network_info::CWellSiteInfo::addDevice(CWellSiteDevInfo a_oDevice)
{
	// 1. Check device type TCP or RTU and whether it matches with this network
	//    If not matched, ignore the device
	// 2. Search whether given device name is already present
	//    If present, ignore the device
	//    If not present, add it
	
	BOOST_LOG_SEV(lg, debug) << __func__ << " Start: To add DataPoint - " << a_oDevice.getID();
	// Check network type
	if (g_eNetworkType != a_oDevice.getAddressInfo().a_NwType)
	{
		//BOOST_LOG_SEV(lg, error) << __func__ << " Networktype mismatch. Expected - " << g_eNetworkType << ", but received " << a_oDevice.getAddressInfo().a_NwType;
		// Device type and network type are not matching.
		return -2;
	}
	
	// Search whether given device name is already present
	for(auto oDev: m_DevList)
	{
		if(0 == oDev.getID().compare(a_oDevice.getID()))
		{
			//BOOST_LOG_SEV(lg, error) << __func__ << " : Already present device with id " << a_oDevice.getID();
			// This point name is already present. Ignore this point
			return -1;
		}
	}
	m_DevList.push_back(a_oDevice);
	BOOST_LOG_SEV(lg, debug) << __func__ << " End: Added device - " << a_oDevice.getID();
	return 0;
}

void network_info::CWellSiteInfo::build(const YAML::Node& a_oData, CWellSiteInfo &a_oWellSite)
{
	BOOST_LOG_SEV(lg, debug) << __func__ << " Start";
	bool bIsIdPresent = false;
	try
	{
		for (auto test : a_oData)
		{
			if(test.first.as<std::string>() == "id")
			{
				a_oWellSite.m_sId = test.second.as<std::string>();
				BOOST_LOG_SEV(lg, info) << __func__ << " : Scanning site: " << a_oWellSite.m_sId;
				bIsIdPresent = true;
				continue;
			}
			if(test.second.IsSequence() && test.first.as<std::string>() == "devicelist")
			{
				const YAML::Node& list = test.second;

				for (auto nodes : list)
				{
					CWellSiteDevInfo objWellsiteDev;
					CWellSiteDevInfo::build(nodes, objWellsiteDev);
					if(0 == a_oWellSite.addDevice(objWellsiteDev))
					{
						BOOST_LOG_SEV(lg, info) << __func__ << " : Added device with id: " << objWellsiteDev.getID();
					}
					else
					{
						BOOST_LOG_SEV(lg, error) << __func__ << " : Ignored device with id : " << objWellsiteDev.getID();
					}
				}
			}
		}
		if(false == bIsIdPresent)
		{
			BOOST_LOG_SEV(lg, error) << __func__ << " Site without id is found. Ignoring this site.";
			throw YAML::Exception(YAML::Mark::null_mark(), "Id key not found");
		}
	}
	catch(YAML::Exception &e)
	{
		BOOST_LOG_SEV(lg, error) << __func__ << " " << e.what();
		throw;
	}
	BOOST_LOG_SEV(lg, debug) << __func__ << " End";
}

void network_info::CWellSiteDevInfo::build(const YAML::Node& a_oData, CWellSiteDevInfo &a_oWellSiteDevInfo)
{
	BOOST_LOG_SEV(lg, debug) << __func__ << " Start";
	bool bIsIdPresent = false;
	bool bIsProtocolPresent = false;
	try
	{
		for (auto it : a_oData)
		{
			if(it.first.as<std::string>() == "id")
			{
				a_oWellSiteDevInfo.m_sId = it.second.as<std::string>();
				BOOST_LOG_SEV(lg, info) << __func__ << " : Scanning site device: " << a_oWellSiteDevInfo.m_sId;
				bIsIdPresent = true;
				continue;
			}
			if(it.first.as<std::string>() == "protocol" && it.second.IsMap())
			{
				std::map<std::string, std::string > tempMap;
				tempMap = it.second.as<std::map<string, string>>();

				//a_oWellSiteDevInfo.m_stAddress.a_NwType = tempMap.at("protocol");
				if(tempMap.at("protocol") == "PROTOCOL_RTU")
				{
					try
					{
						a_oWellSiteDevInfo.m_stAddress.m_stRTU.m_uiBaudRate = atoi(tempMap.at("baud").c_str());
						a_oWellSiteDevInfo.m_stAddress.m_stRTU.m_uiDataBits = atoi(tempMap.at("databits").c_str());
						a_oWellSiteDevInfo.m_stAddress.m_stRTU.m_uiParity = atoi(tempMap.at("parity").c_str());
						a_oWellSiteDevInfo.m_stAddress.m_stRTU.m_uiStart = atoi(tempMap.at("start").c_str());
						a_oWellSiteDevInfo.m_stAddress.m_stRTU.m_uiStop = atoi(tempMap.at("stop").c_str());
						a_oWellSiteDevInfo.m_stAddress.m_stRTU.m_sPortAddress = tempMap.at("port").c_str();
						a_oWellSiteDevInfo.m_stAddress.m_stRTU.m_uiSlaveId = atoi(tempMap.at("slaveid").c_str());

						a_oWellSiteDevInfo.m_stAddress.a_NwType = network_info::eNetworkType::eRTU;
						bIsProtocolPresent = true;
						BOOST_LOG_SEV(lg, info) << __func__ << " : RTU protocol: " << tempMap.at("port").c_str();
					}
					catch(exception &e)
					{
						BOOST_LOG_SEV(lg, error) << __func__ << " : key not found in PROTOCOL_RTU " << e.what();
						throw YAML::Exception(YAML::Mark::null_mark(), "Required keys not found in PROTOCOL_RTU");
					}
				}
				else if(tempMap.at("protocol") == "PROTOCOL_TCP")
				{
					try
					{
						a_oWellSiteDevInfo.m_stAddress.m_stTCP.m_sIPAddress = tempMap.at("ipaddress");
						a_oWellSiteDevInfo.m_stAddress.m_stTCP.m_ui16PortNumber = atoi(tempMap.at("port").c_str());
						a_oWellSiteDevInfo.m_stAddress.a_NwType = network_info::eNetworkType::eTCP;
						a_oWellSiteDevInfo.m_stAddress.m_stTCP.m_uiUnitID = atoi(tempMap.at("unitid").c_str());
						bIsProtocolPresent = true;
						BOOST_LOG_SEV(lg, info) << __func__ << " : TCP protocol: " << a_oWellSiteDevInfo.m_stAddress.m_stTCP.m_sIPAddress << ":" << a_oWellSiteDevInfo.m_stAddress.m_stTCP.m_ui16PortNumber;
					}
					catch(exception &e)
					{
						BOOST_LOG_SEV(lg, error) << __func__ << " : key not found in PROTOCOL_TCP " << e.what();
						throw YAML::Exception(YAML::Mark::null_mark(), "Required keys not found in PROTOCOL_TCP");
					}
				}
				else
				{
					// error
					BOOST_LOG_SEV(lg, error) << __func__ << " : Unknown protocol: " << tempMap.at("protocol");
					throw YAML::Exception(YAML::Mark::null_mark(), "Unknown protocol found");
				}
			}

			if(it.first.as<std::string>() == "deviceinfo")
			{
				if(!CfgManager::Instance().IsClientCreated())
				{
					BOOST_LOG_SEV(lg, error) << __func__ << " ETCD client is not created ..";
					return;
				}
				char *cEtcdValue  = CfgManager::Instance().getETCDValuebyKey(("/Device_Config/" +it.second.as<std::string>()).c_str());
				if(NULL == cEtcdValue)
				{
					BOOST_LOG_SEV(lg, error) << __func__ << " No value received from ETCD  ..";
					return;
				}
				std::string sYamlStr(cEtcdValue);

				YAML::Node node = CommonUtils::loadFromETCD(sYamlStr);
				CDeviceInfo::build(node, a_oWellSiteDevInfo.getDevInfo1());
			}
		}
		if(false == bIsIdPresent)
		{
			BOOST_LOG_SEV(lg, error) << __func__ << " Site device without id is found. Ignoring this well device.";
			throw YAML::Exception(YAML::Mark::null_mark(), "Id key not found");
		}
		if(false == bIsProtocolPresent)
		{
			BOOST_LOG_SEV(lg, error) << __func__ << " Site device without protocol is found. Ignoring it.";
			throw YAML::Exception(YAML::Mark::null_mark(), "Protocol key not found");
		}
	}
	catch(YAML::Exception &e)
	{
		BOOST_LOG_SEV(lg, error) << __func__ << " " << e.what();
		throw;
	}
	
	BOOST_LOG_SEV(lg, debug) << __func__ << " End";
}

void network_info::CDeviceInfo::build(const YAML::Node& a_oData, CDeviceInfo &a_oCDeviceInfo )
{
	BOOST_LOG_SEV(lg, debug) << __func__ << " Start";
	bool bIsNameFound = false;
	try
	{
		for (auto test : a_oData)
		{
			if(test.first.as<std::string>() == "device_info")
			{
				try
				{
					a_oCDeviceInfo.m_sName = test.second["name"].as<std::string>();
					bIsNameFound = true;
					continue;
				}
				catch(exception &e)
				{
					BOOST_LOG_SEV(lg, error) << __func__ << " : key 'name' not found in device_info " << e.what();
					throw YAML::Exception(YAML::Mark::null_mark(), "name key not found in device_info");
				}				
			}
			//a_oWellSite.m_sId = test["id"].as<std::string>();
			if(test.first.as<std::string>() == "pointlist")
			{
				if(!CfgManager::Instance().IsClientCreated())
				{
					BOOST_LOG_SEV(lg, error) << __func__ << " ETCD client is not created ..";
					return;
				}
				const char *cEtcdValue  = CfgManager::Instance().getETCDValuebyKey(( "/Device_Config/"+ test.second.as<std::string>()).c_str());
				if(NULL == cEtcdValue)
				{
					BOOST_LOG_SEV(lg, error) << __func__ << " No value received from ETCD  ..";
					return;
				}
				std::string sYamlStr(cEtcdValue);

				YAML::Node node = CommonUtils::loadFromETCD(sYamlStr);
				BOOST_LOG_SEV(lg, info) << __func__ << " : pointlist found: " << test.second.as<std::string>();

				for (auto it : node)
				{
					if(it.first.as<std::string>() == "file" && it.second.IsMap())
					{
						// store if required
					}
					if(it.second.IsSequence() && it.first.as<std::string>() == "datapoints")
					{
						const YAML::Node& points =  it.second;
						for (auto it1 : points)
						{
							CDataPoint objCDataPoint;
							CDataPoint::build(it1, objCDataPoint);
							if(0 == a_oCDeviceInfo.addDataPoint(objCDataPoint))
							{
								BOOST_LOG_SEV(lg, info) << __func__ << " : Added point with id: " << objCDataPoint.getID();
							}
							else
							{
								BOOST_LOG_SEV(lg, error) << __func__ << " : Ignored point with id : " << objCDataPoint.getID();
							}
						}
					}
				}
			}
		}
		if(false == bIsNameFound)
		{
			BOOST_LOG_SEV(lg, error) << __func__ << " Device without name is found. Ignoring this device.";
			throw YAML::Exception(YAML::Mark::null_mark(), "name key not found");
		}
	}
	catch(YAML::Exception &e)
	{
		BOOST_LOG_SEV(lg, error) << __func__ << " " << e.what();
		throw;
	}
	BOOST_LOG_SEV(lg, debug) << __func__ << " End";
}

const std::map<std::string, CWellSiteInfo>& network_info::getWellSiteList()
{
	BOOST_LOG_SEV(lg, debug) << __func__;
	return g_mapYMLWellSite;	
}

const std::map<std::string, CUniqueDataPoint>& network_info::getUniquePointList()
{
	BOOST_LOG_SEV(lg, debug) << __func__;
	return g_mapUniqueDataPoint;
}

eEndPointType network_info::CDataPoint::getPointType(const std::string& a_type)
{
	BOOST_LOG_SEV(lg, debug) << __func__ << " Start: Received type: " << a_type;
	eEndPointType type;
	if(a_type == "INPUT_REGISTER")
	{
		type = eEndPointType::eInput_Register;
	}
	else if(a_type == "HOLDING_REGISTER")
	{
		type = eEndPointType::eHolding_Register;
	}
	else if(a_type == "COIL")
	{
		type = eEndPointType::eCoil;
	}
	else if(a_type == "DISCRETE_INPUT")
	{
		type = eEndPointType::eDiscrete_Input;
	}
	else
	{
		BOOST_LOG_SEV(lg, error) << __func__ << " : Unknown type: " << a_type;
		throw YAML::Exception(YAML::Mark::null_mark(), "Unknown Modbus point type");
	}

	BOOST_LOG_SEV(lg, debug) << __func__ << " End";
	return type;
}

void network_info::CDataPoint::build(const YAML::Node& a_oData, CDataPoint &a_oCDataPoint)
{
	BOOST_LOG_SEV(lg, debug) << __func__ << " Start";
	// First check optional parameters
	try
	{
		// Set default values
		a_oCDataPoint.m_stPollingConfig.m_uiPollFreq = 0;
		a_oCDataPoint.m_stPollingConfig.m_bIsRealTime = false;
		
		// Assign values from YML, if available
		a_oCDataPoint.m_stPollingConfig.m_uiPollFreq = a_oData["polling"]["pollinterval"].as<std::uint32_t>();
		a_oCDataPoint.m_stPollingConfig.m_bIsRealTime =  a_oData["polling"]["realtime"].as<bool>();
	}
	catch(exception &e)
	{
		BOOST_LOG_SEV(lg, info) << __func__ << " Proceeding further, even though few of polling keys are not available: " << e.what();
	}
	
	// Check mandatory parameters
	try
	{
		a_oCDataPoint.m_stAddress.m_bIsByteSwap =  false;
		a_oCDataPoint.m_stAddress.m_bIsWordSwap =  false;

		a_oCDataPoint.m_sId = a_oData["id"].as<std::string>();
		a_oCDataPoint.m_stAddress.m_eType = getPointType(a_oData["attributes"]["type"].as<std::string>());
		a_oCDataPoint.m_stAddress.m_iAddress = a_oData["attributes"]["addr"].as<std::int32_t>();
		a_oCDataPoint.m_stAddress.m_iWidth =  a_oData["attributes"]["width"].as<std::int32_t>();
		if (a_oData["attributes"]["byteswap"])
		{
			a_oCDataPoint.m_stAddress.m_bIsByteSwap =  a_oData["attributes"]["byteswap"].as<bool>();
		}
		if (a_oData["attributes"]["wordswap"])
		{
			a_oCDataPoint.m_stAddress.m_bIsWordSwap =  a_oData["attributes"]["wordswap"].as<bool>();
		}
	}
	catch(YAML::Exception &e)
	{
		BOOST_LOG_SEV(lg, error) << __func__ << " " << e.what();
		throw;
	}
	catch(exception &e)
	{
		BOOST_LOG_SEV(lg, error) << __func__ << " " << e.what();
		throw YAML::Exception(YAML::Mark::null_mark(), "key not found");
	}
	
	BOOST_LOG_SEV(lg, debug) << __func__ << " End";
}

void printWellSite(CWellSiteInfo a_oWellSite)
{
	BOOST_LOG_SEV(lg, debug) << __func__ << " Start: wellsite: " << a_oWellSite.getID();
	for(auto objWellSiteDev : a_oWellSite.getDevices())
	{
		BOOST_LOG_SEV(lg, debug) << __func__ << " " << a_oWellSite.getID() << "\\" << objWellSiteDev.getID();
		for(auto objPt : objWellSiteDev.getDevInfo().getDataPoints())
		{
			BOOST_LOG_SEV(lg, debug) << __func__ << " " << a_oWellSite.getID() << "\\" << objWellSiteDev.getID() << "\\" << objPt.getID();
		}
	}
	BOOST_LOG_SEV(lg, debug) << __func__ << " End";
}

void network_info::buildNetworkInfo(bool a_bIsTCP)
{
	BOOST_LOG_SEV(lg, debug) << __func__ << " Start: is it TCP ? " << a_bIsTCP;
	// Check if this function is already called once. If yes, then exit
	if(true == g_bIsStarted)
	{
		std::cout << "already started... return with no action \n";
		BOOST_LOG_SEV(lg, info) << __func__ << " This function is already called once. Ignoring this call";
		return;
	}
	// Set the flag to avoid all future calls to this function. 
	// This is done to keep data structures in tact once network is built
	g_bIsStarted = true;
	
	// Set the network type TCP or RTU
	if(false == a_bIsTCP)
	{
		g_eNetworkType = eNetworkType::eRTU;
	}
	else
	{
		g_eNetworkType = eNetworkType::eTCP;
	}

	std::cout << "Network set as: " << (int)g_eNetworkType << std::endl;
	BOOST_LOG_SEV(lg, info) << __func__ << " Network set as: " << (int)g_eNetworkType;
	
	// Following stage is needed only when configuration files are placed in a docker volume
	#ifdef CONFIGFILES_IN_DOCKER_VOLUME
	//std::cout << "Config files are kept in a docker volume\n";
	//BOOST_LOG_SEV(lg, info) << __func__ << " Config files are kept in a docker volume";
	
	// get list of well sites
	if(false == _getWellSiteList())
	{
		BOOST_LOG_SEV(lg, error) << __func__ << " Site-list could not be obtained";
		return;
	}
	std::vector<CWellSiteInfo> oWellSiteList;
	for(auto sWellSiteFile: g_sWellSiteFileList)
	{
		if(true == sWellSiteFile.empty())
		{
			std::cout << __func__ <<" : Encountered empty file name. Ignoring";
			BOOST_LOG_SEV(lg, info) << __func__ << " : Encountered empty file name. Ignoring";
			continue;
		}
		// Check if the file is already scanned
		std::map<std::string, CWellSiteInfo>::iterator it = g_mapYMLWellSite.find(sWellSiteFile);

		if(g_mapYMLWellSite.end() != it)
		{
			// It means record exists
			std::cout << __func__ << " " << sWellSiteFile <<" : is already scanned. Ignoring";
			BOOST_LOG_SEV(lg, info) << __func__ << " " << sWellSiteFile << "Already scanned YML file: Ignoring it.";
			continue;
		}
		
		BOOST_LOG_SEV(lg, info) << __func__ << " New YML file: " << sWellSiteFile;

		try
		{
			/// get value from ETCD
			if(!CfgManager::Instance().IsClientCreated())
			{
				BOOST_LOG_SEV(lg, error) << __func__ << " ETCD client is not created ..";
				return;
			}
			const char *cEtcdValue  = CfgManager::Instance().getETCDValuebyKey(( "/Device_Config/" + sWellSiteFile).c_str());
			if(NULL == cEtcdValue)
			{
				BOOST_LOG_SEV(lg, error) << __func__ << " No value received from ETCD  ..";
				return;
			}
			std::string sYamlStr(cEtcdValue);

			YAML::Node baseNode = CommonUtils::loadFromETCD(sYamlStr);
			CWellSiteInfo objWellSite;
			CWellSiteInfo::build(baseNode, objWellSite);
			oWellSiteList.push_back(objWellSite);
			g_mapYMLWellSite.emplace(sWellSiteFile, objWellSite);
			BOOST_LOG_SEV(lg, info) << __func__ << " Successfully scanned: " << sWellSiteFile << ": Id = " << objWellSite.getID();
		}
		catch(YAML::Exception &e)
		{
			BOOST_LOG_SEV(lg, error) << __func__ << " Ignoring YML:" << sWellSiteFile << "Error: " << e.what();
			// Add this file to error YML files
			g_sErrorYMLs.push_back(sWellSiteFile);
		}
	}

	// Once network information is read, prepare a list of unique points
	// Set variables for unique point listing
	if(const char* env_p = std::getenv("MY_APP_ID"))
	{
		BOOST_LOG_SEV(lg, info) << __func__ << ": MY_APP_ID value = " << env_p;
		auto a = (unsigned short) atoi(env_p);
		a = a & 0x000F;
		g_usTotalCnt = (a << 12);
	}
	else
	{
		BOOST_LOG_SEV(lg, info) << __func__ << "MY_APP_ID value is not set. Expected values 0 to 16";
		BOOST_LOG_SEV(lg, info) << __func__ << "Assuming value as 0";
		g_usTotalCnt = 0;
	}
	BOOST_LOG_SEV(lg, info) << __func__ << ": Count start from = " << g_usTotalCnt;
	for(auto a: g_mapYMLWellSite)
	{
		populateUniquePointData(g_mapYMLWellSite.at(a.first));
	}

	for(auto a: oWellSiteList)
	{
		std::cout << "\nNew Well Site\n";
		printWellSite(a);
	}

	#endif
	BOOST_LOG_SEV(lg, debug) << __func__ << " End";
}

CUniqueDataPoint::CUniqueDataPoint(std::string a_sId, const CWellSiteInfo &a_rWellSite,
				const CWellSiteDevInfo &a_rWellSiteDev, const CDataPoint &a_rPoint) :
				m_uiMyRollID{((unsigned int)g_usTotalCnt)+1}, m_sId{a_sId},
				m_rWellSite{a_rWellSite}, m_rWellSiteDev{a_rWellSiteDev}, m_rPoint{a_rPoint}
{
	++g_usTotalCnt;
}
