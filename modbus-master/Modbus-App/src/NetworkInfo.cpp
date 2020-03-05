/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#include "Logger.hpp"
#include <iostream>
#include <atomic>
#include <map>
#include <algorithm>
#include <arpa/inet.h>
#include "NetworkInfo.hpp"
#include "yaml-cpp/eventhandler.h"
#include "yaml-cpp/yaml.h"
#include "utils/YamlUtil.hpp"
#include "ConfigManager.hpp"
#include "PublishJson.hpp"

using namespace network_info;
using namespace CommonUtils;

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

	/**
	 * Populate unique point data
	 * @param a_oWellSite :[in] well site to populate info of
	 */
	void populateUniquePointData(const CWellSiteInfo &a_oWellSite)
	{
		CLogger::getInstance().log(DEBUG, LOGDETAILS("Start"));
		for(auto &objWellSiteDev : a_oWellSite.getDevices())
		{
			//unsigned int uiPoint = 0;
			for(auto &objPt : objWellSiteDev.getDevInfo().getDataPoints())
			{
				std::string sUniqueId(SEPARATOR_CHAR + objWellSiteDev.getID()
						+ SEPARATOR_CHAR + a_oWellSite.getID() + SEPARATOR_CHAR +
										objPt.getID());
				//CLogger::getInstance().log(INFO, LOGDETAILS(" " << sUniqueId;
				
				// Build unique data point
				CUniqueDataPoint oUniquePoint{sUniqueId, a_oWellSite, objWellSiteDev, objPt};
				g_mapUniqueDataPoint.emplace(sUniqueId, oUniquePoint);
				string temp = oUniquePoint.getID();
				temp.append("=");
				temp.append(std::to_string(oUniquePoint.getMyRollID()));
				CLogger::getInstance().log(INFO, LOGDETAILS(temp));
			}
		}
		CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));
	}

	
	#ifdef CONFIGFILES_IN_DOCKER_VOLUME
	std::vector<std::string> g_sWellSiteFileList;

	/**
	 * Get well site list
	 * @return 	true : on success,
	 * 			false : on error
	 */
	bool _getWellSiteList()
	{
		CLogger::getInstance().log(DEBUG, LOGDETAILS(" Start: Reading site_list.yaml"));
		try
		{
			YAML::Node Node = CommonUtils::loadYamlFile(PublishJsonHandler::instance().getSiteListFileName());
			CommonUtils::convertYamlToList(Node, g_sWellSiteFileList);
		}
		catch(YAML::Exception &e)
		{
			CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
			return false;
		}
		CLogger::getInstance().log(DEBUG, LOGDETAILS("End:"));
		return true;
	}
	#endif
}

/**
 * Add data point
 * @param a_oDataPoint :[in] data point to add
 * @return 	0 : on success,
 * 			-1 : on error
 */
int network_info::CDeviceInfo::addDataPoint(CDataPoint a_oDataPoint)
{
	// Search whether given point name is already present
	// If present, ignore the datapoint
	// If not present, add it
	string temp = "Start: To add DataPoint - ";
	temp.append(a_oDataPoint.getID());
	CLogger::getInstance().log(DEBUG, LOGDETAILS(temp));
	for(auto oDataPoint: m_DataPointList)
	{
		if(0 == oDataPoint.getID().compare(a_oDataPoint.getID()))
		{
			string temp = ":Already present DataPoint with id";
			temp.append(a_oDataPoint.getID());
			CLogger::getInstance().log(ERROR, LOGDETAILS(temp));
			// This point name is already present. Ignore this point
			return -1;
		}
	}
	m_DataPointList.push_back(a_oDataPoint);
	string temp0 = "End: Added DataPoint - " ;
	temp0.append(a_oDataPoint.getID());

	CLogger::getInstance().log(DEBUG, LOGDETAILS(temp0));
	return 0;
}

/**
 * Add device
 * @param a_oDevice :[in] device to add
 * @return 	0 : on success,
 * 			-1 : on error
 * 			-2 : if device does not match
 */
int network_info::CWellSiteInfo::addDevice(CWellSiteDevInfo a_oDevice)
{
	// 1. Check device type TCP or RTU and whether it matches with this network
	//    If not matched, ignore the device
	// 2. Search whether given device name is already present
	//    If present, ignore the device
	//    If not present, add it
	string temp = " Start: To add DataPoint - " ;
	temp.append(a_oDevice.getID());
	CLogger::getInstance().log(DEBUG, LOGDETAILS(temp));
	// Check network type
	if (g_eNetworkType != a_oDevice.getAddressInfo().a_NwType)
	{
		//CLogger::getInstance().log(ERROR, LOGDETAILS" Networktype mismatch. Expected - " << g_eNetworkType << ", but received " << a_oDevice.getAddressInfo().a_NwType;
		// Device type and network type are not matching.
		return -2;
	}
	
	// Search whether given device name is already present
	for(auto oDev: m_DevList)
	{
		if(0 == oDev.getID().compare(a_oDevice.getID()))
		{
			//CLogger::getInstance().log(ERROR, LOGDETAILS" : Already present device with id " << a_oDevice.getID();
			// This point name is already present. Ignore this point
			return -1;
		}
	}
	m_DevList.push_back(a_oDevice);
	string temp2 = " End: Added device - ";
	temp2.append(a_oDevice.getID());
	CLogger::getInstance().log(DEBUG, LOGDETAILS(temp2));
	return 0;
}

/**
 * build well site info
 * @param a_oData 		:[in] data
 * @param a_oWellSite	:[in] well site
 */
void network_info::CWellSiteInfo::build(const YAML::Node& a_oData, CWellSiteInfo &a_oWellSite)
{
	//CLogger::getInstance().log(DEBUG, LOGDETAILS(" Start";
	bool bIsIdPresent = false;
	try
	{
		for (auto test : a_oData)
		{
			if(test.first.as<std::string>() == "id")
			{
				a_oWellSite.m_sId = test.second.as<std::string>();
				string temp = " : Scanning site: ";
				temp.append(a_oWellSite.m_sId);
				CLogger::getInstance().log(INFO, LOGDETAILS(temp));
				bIsIdPresent = true;
				continue;
			}
			if(test.second.IsSequence() && test.first.as<std::string>() == "devicelist")
			{
				const YAML::Node& list = test.second;

				for (auto nodes : list)
				{
					CWellSiteDevInfo objWellsiteDev;
					int32_t i32RetVal = 0;
					CWellSiteDevInfo::build(nodes, objWellsiteDev);
					i32RetVal = a_oWellSite.addDevice(objWellsiteDev);
					if(0 == i32RetVal)
					{
						string temp = " : Added device with id: ";
						temp.append(objWellsiteDev.getID());
						CLogger::getInstance().log(INFO, LOGDETAILS(temp));
					}
					else if(-1 == i32RetVal)
					{
						string temp = "Ignoring device with id : " ;
						temp.append(objWellsiteDev.getID());
						CLogger::getInstance().log(ERROR, LOGDETAILS(temp + ", since this point name is already present. Ignore this point."));
						std::cout << temp << ", since this point name is already present. Ignore this point."<< endl;
					}
					else if(-2 == i32RetVal)
					{
						string temp = "Ignoring device with id : " ;
						temp.append(objWellsiteDev.getID());
						CLogger::getInstance().log(ERROR, LOGDETAILS(temp + ", since Device type and network type are not matching."));
						std::cout << temp << ", since Device type and network type are not matching."<< endl;
					}
				}
			}
		}
		if(false == bIsIdPresent)
		{
			CLogger::getInstance().log(FATAL, LOGDETAILS(" Site without id is found. Ignoring this site."));
			throw YAML::Exception(YAML::Mark::null_mark(), "Id key not found");
			cout << "Site without id is found. Ignoring this site."<<endl;
		}
	}
	catch(YAML::Exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS( e.what()));
		throw;
	}
	CLogger::getInstance().log(DEBUG, LOGDETAILS(" End"));
}

/**
 * function to validate IP address
 * @param ipAddress :[in] IP to validate
 * @return 	true : on success,
 * 			false : on error
 */
bool network_info::validateIpAddress(const string &ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress.c_str(), &(sa.sin_addr));
    return result != 0;
}

/**
 * build well site device info
 * @param a_oData			:[in] data
 * @param a_oWellSiteDevInfo:[in] well site
 */
void network_info::CWellSiteDevInfo::build(const YAML::Node& a_oData, CWellSiteDevInfo &a_oWellSiteDevInfo)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS(" Start"));
	bool bIsIdPresent = false;
	bool bIsProtocolPresent = false;
	try
	{
		for (auto it : a_oData)
		{
			if(it.first.as<std::string>() == "id")
			{
				a_oWellSiteDevInfo.m_sId = it.second.as<std::string>();
				string temp = " : Scanning site device: ";
				temp.append( a_oWellSiteDevInfo.m_sId);
				CLogger::getInstance().log(INFO, LOGDETAILS(temp));
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
						string temp = " : RTU protocol: ";
						temp.append(tempMap.at("port").c_str());
						CLogger::getInstance().log(INFO, LOGDETAILS(temp));
					}
					catch(exception &e)
					{
						CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
						std::cout << __func__ << "Required keys not found in PROTOCOL_RTU" << endl;
						throw YAML::Exception(YAML::Mark::null_mark(), "Required keys not found in PROTOCOL_RTU");
					}
				}
				else if(tempMap.at("protocol") == "PROTOCOL_TCP")
				{
					try
					{
						if(tempMap.at("ipaddress") == "" || tempMap.at("port") == "" || tempMap.at("unitid") == "")
						{
							std::cout << " ERROR:: ipaddress or port or unitid cannot be empty" <<endl;
							std::cout << " Given parameters:: " <<endl;
							std::cout << " ipaddress:: " << tempMap.at("ipaddress")<<endl;
							std::cout  << " port:: " << tempMap.at("port")<<endl;
							std::cout  << " unitid:: " <<tempMap.at("unitid")<<endl;
							CLogger::getInstance().log(ERROR, LOGDETAILS("ipaddress or port or unitid cannot be empty"));
							CLogger::getInstance().log(ERROR, LOGDETAILS("Given parameters are following ::"));
							CLogger::getInstance().log(ERROR, LOGDETAILS("ipaddress:: " + tempMap.at("ipaddress")));
							CLogger::getInstance().log(ERROR, LOGDETAILS("unitid:: " + tempMap.at("unitid")));
							CLogger::getInstance().log(ERROR, LOGDETAILS("port:: " + tempMap.at("port")));
							return;
						}

						if(validateIpAddress(tempMap.at("ipaddress")))
						{
							a_oWellSiteDevInfo.m_stAddress.m_stTCP.m_sIPAddress = tempMap.at("ipaddress");
						}
						else
						{
							std::cout << "ERROR : IP address is invalid!!" <<tempMap.at("ipaddress") <<endl;
							CLogger::getInstance().log(ERROR, LOGDETAILS("IP address is invalid " + tempMap.at("ipaddress")));
						}

						a_oWellSiteDevInfo.m_stAddress.m_stTCP.m_ui16PortNumber = atoi(tempMap.at("port").c_str());
						a_oWellSiteDevInfo.m_stAddress.a_NwType = network_info::eNetworkType::eTCP;
						a_oWellSiteDevInfo.m_stAddress.m_stTCP.m_uiUnitID = atoi(tempMap.at("unitid").c_str());
						bIsProtocolPresent = true;
						string tempVar = " : TCP protocol: ";
						tempVar.append(a_oWellSiteDevInfo.m_stAddress.m_stTCP.m_sIPAddress);
						tempVar.append(":" +std::to_string(a_oWellSiteDevInfo.m_stAddress.m_stTCP.m_ui16PortNumber));
						CLogger::getInstance().log(INFO, LOGDETAILS(tempVar));
					}
					catch(exception &e)
					{
						CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
						std::cout << "Required keys not found in PROTOCOL_TCP"<<endl;
						throw YAML::Exception(YAML::Mark::null_mark(), "Required keys not found in PROTOCOL_TCP");
					}
				}
				else
				{
					// error
					string temp = " : Unknown protocol: ";
					temp.append(tempMap.at("protocol"));
				//	CLogger::getInstance().log(FATAL, LOGDETAILS(" : Unknown protocol: " << tempMap.at("protocol");
					CLogger::getInstance().log(ERROR, LOGDETAILS(temp));
					std::cout << __func__<< temp << endl;
					throw YAML::Exception(YAML::Mark::null_mark(), "Unknown protocol found");
				}
			}

			if(it.first.as<std::string>() == "deviceinfo")
			{
				YAML::Node node = CommonUtils::loadYamlFile(it.second.as<std::string>());
				CDeviceInfo::build(node, a_oWellSiteDevInfo.getDevInfo1());
			}
		}
		if(false == bIsIdPresent)
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS(" Site device without id is found. Ignoring this well device."));
			std::cout << __func__ << " Site device without id is found. Ignoring this well device."<<endl;
			throw YAML::Exception(YAML::Mark::null_mark(), "Id key not found");

		}
		if(false == bIsProtocolPresent)
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS(" Site device without protocol is found. Ignoring it."));
			std::cout << __func__ << " Site device without protocol is found. Ignoring it.."<<endl;
			throw YAML::Exception(YAML::Mark::null_mark(), "Protocol key not found");
		}
	}
	catch(YAML::Exception &e)
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS(e.what()));
		std::cout << __func__<<" Exception :: " << e.what()<<endl;
		throw;
	}
	
	CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));
}

/**
 * build well site device info
 * @param a_oData			:[in] data
 * @param a_oCDeviceInfo:[in] well site device info
 */
void network_info::CDeviceInfo::build(const YAML::Node& a_oData, CDeviceInfo &a_oCDeviceInfo )
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS("Start"));
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
					CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
					throw YAML::Exception(YAML::Mark::null_mark(), "name key not found in device_info");
				}				
			}
			//a_oWellSite.m_sId = test["id"].as<std::string>();
			if(test.first.as<std::string>() == "pointlist")
			{
				YAML::Node node = CommonUtils::loadYamlFile(test.second.as<std::string>());
				string temp2 = " : pointlist found: ";
				temp2.append(test.second.as<std::string>());
				CLogger::getInstance().log(INFO, LOGDETAILS(temp2));

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
								string temp3 = "Added point with id: ";
								temp3.append(objCDataPoint.getID());
								CLogger::getInstance().log(INFO, LOGDETAILS(temp3));
							}
							else
							{
								CLogger::getInstance().log(ERROR, LOGDETAILS("Ignoring duplicate point ID from polling :"+ objCDataPoint.getID()));
								std::cout << "ERROR: Ignoring duplicate point ID from polling :"<< objCDataPoint.getID() <<endl;
							}
						}
					}
				}
			}
		}
		if(false == bIsNameFound)
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS(" Device without name is found. Ignoring this device."));
			throw YAML::Exception(YAML::Mark::null_mark(), "name key not found");
		}
	}
	catch(YAML::Exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		throw;
	}
	CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));
}

/**
 * Get well site list
 * @return well site map
 */
const std::map<std::string, CWellSiteInfo>& network_info::getWellSiteList()
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS(""));
	return g_mapYMLWellSite;	
}

/**
 * Get unique point list
 * @return map of unique points
 */
const std::map<std::string, CUniqueDataPoint>& network_info::getUniquePointList()
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS(""));
	return g_mapUniqueDataPoint;
}

/**
 * Get point type
 * @param a_type	:[in] point type
 * @return point type
 */
eEndPointType network_info::CDataPoint::getPointType(const std::string& a_type)
{
	string temp = " Start: Received type: ";
	temp.append(a_type);
	CLogger::getInstance().log(DEBUG, LOGDETAILS(temp));
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
		string temp4 = " : Unknown type: " ;
		temp4.append(a_type);
		CLogger::getInstance().log(INFO, LOGDETAILS(temp4));
		throw YAML::Exception(YAML::Mark::null_mark(), "Unknown Modbus point type");
	}

	CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));
	return type;
}

/**
 * check if input is number
 * @param s	:[in] string might be containing number
 * @return 	true : on success,
 * 			false : on error
 */
bool network_info:: isNumber(string s)
{
    for (uint32_t i32Count = 0; i32Count < s.length(); i32Count++)
        if (isdigit(s[i32Count]) == false)
            return false;

    return true;
}

/**
 * Build data points info
 * @param a_oData		:[in] data point
 * @param a_oCDataPoint	:[in] data points info
 */
void network_info::CDataPoint::build(const YAML::Node& a_oData, CDataPoint &a_oCDataPoint)
{
	CLogger::getInstance().log(DEBUG, LOGDETAILS(" Start"));
	// First check optional parameters
	try
	{
		// Set default values
		a_oCDataPoint.m_stPollingConfig.m_uiPollFreq = 0;
		a_oCDataPoint.m_stPollingConfig.m_bIsRealTime = false;
		a_oCDataPoint.m_stPollingConfig.m_usQOS = "0";

		if(a_oData["polling"]["realtime"])
		{
			if(a_oData["polling"]["realtime"].as<std::string>() != "")
			{
				a_oCDataPoint.m_stPollingConfig.m_bIsRealTime =  a_oData["polling"]["realtime"].as<bool>();
			}
			else
			{
				std::cout << "WARNING :: realtime parameter is empty !! setting it to default (i.e. false)\n";
				CLogger::getInstance().log(WARN, LOGDETAILS("realtime parameter is empty !! setting it to default (i.e. false)"));
				a_oCDataPoint.m_stPollingConfig.m_bIsRealTime = false;
			}
		}
		else
		{
			std::cout << "WARNING :: realtime parameter is not present !! setting it to default (i.e. false)\n";
			CLogger::getInstance().log(WARN, LOGDETAILS("realtime parameter is not present !! setting it to default (i.e. false)"));
			a_oCDataPoint.m_stPollingConfig.m_bIsRealTime = false;
		}
		if(a_oData["polling"]["pollinterval"])
		{
			if(isNumber(a_oData["polling"]["pollinterval"].as<std::string>()) == false
					|| a_oData["polling"]["pollinterval"].as<std::string>() == "")
			{
				std::cout << "WARNING :: pollinterval parameters is invalid !! setting it to default (i.e. 0 ms)\n";
				CLogger::getInstance().log(WARN, LOGDETAILS("pollinterval parameters invalid setting it to default (i.e. 0 ms)"));
				a_oCDataPoint.m_stPollingConfig.m_uiPollFreq = 0;
			}
			else
			{
				a_oCDataPoint.m_stPollingConfig.m_uiPollFreq = a_oData["polling"]["pollinterval"].as<std::uint32_t>();
			}
		}
		else
		{
			std::cout << "WARNING :: pollinterval parameter is not present "
					"!! setting it to default (i.e. 0 ms)\n";
			CLogger::getInstance().log(WARN, LOGDETAILS("pollinterval parameter is not present !! "
					"setting it to default (i.e.  ms)"));
			a_oCDataPoint.m_stPollingConfig.m_uiPollFreq = 0;
		}

		if(a_oData["polling"]["qos"])
		{
			if(isNumber(a_oData["polling"]["qos"].as<std::string>()) == false
					|| a_oData["polling"]["qos"].as<std::string>() == "")
			{
				std::cout << "WARNING :: qos parameters is either empty or invalid (i.e. expected value is between 0 to 2)!! setting it to default (i.e. zero)\n";
				CLogger::getInstance().log(WARN, LOGDETAILS("qos parameters is either empty or invalid  (i.e. expected value is between 0 to 2)!! setting it to default (i.e. zero)"));
				a_oCDataPoint.m_stPollingConfig.m_usQOS = "0";
			}
			else if(stoi(a_oData["polling"]["qos"].as<std::string>().c_str()) > 2)
			{
				std::cout << "WARNING :: qos parameters is not in range (i.e. expected value is between 0 to 2)!! setting it to default (i.e. zero)\n";
				std::cout << "Current qos value in yml is :: "<< stoi(a_oData["polling"]["qos"].as<std::string>().c_str()) <<endl;
				CLogger::getInstance().log(WARN, LOGDETAILS("Current qos value in yml is :: " + a_oData["polling"]["qos"].as<std::string>()));
				CLogger::getInstance().log(WARN, LOGDETAILS("qos parameters is not in range  (i.e. expected value is between 0 to 2)!! setting it to default (i.e. zero)"));
				a_oCDataPoint.m_stPollingConfig.m_usQOS = "0";
			}
			else
			{
				a_oCDataPoint.m_stPollingConfig.m_usQOS = a_oData["polling"]["qos"].as<std::string>();
			}
		}
		else
		{
			std::cout << "WARNING :: qos parameter is not present "
					"!! setting it to default (i.e. zero)\n";
			CLogger::getInstance().log(WARN, LOGDETAILS("qos parameter is not present !! "
					"setting it to default (i.e. zero)"));
			a_oCDataPoint.m_stPollingConfig.m_usQOS = "0";
		}

	}
	catch(exception &e)
	{
		CLogger::getInstance().log(WARN, LOGDETAILS("qos or pollinterval or realtime is either empty or not valid !!" + std::string(e.what())));
		std::cout << "WARNING :: qos or pollinterval or realtime is either empty or not valid !!" <<e.what()<<endl;
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
			try
			{
				a_oCDataPoint.m_stAddress.m_bIsByteSwap =  a_oData["attributes"]["byteswap"].as<bool>();
			}
			catch(YAML::Exception &e)
			{
				a_oCDataPoint.m_stAddress.m_bIsByteSwap = false;
				CLogger::getInstance().log(WARN, LOGDETAILS("ByteSwap value is incorrect. Set to default with exception ::" + std::string(e.what())));
				cout << "ByteSwap value is incorrect. Set to default with exception :: "<< e.what();
			}
		}
		if (a_oData["attributes"]["wordswap"])
		{
			try
			{
				a_oCDataPoint.m_stAddress.m_bIsWordSwap =  a_oData["attributes"]["wordswap"].as<bool>();
			}
			catch(YAML::Exception &e)
			{
				a_oCDataPoint.m_stAddress.m_bIsWordSwap = false;
				CLogger::getInstance().log(WARN, LOGDETAILS("WordSwap value is incorrect. Set to default." + std::string(e.what())));
				cout << "WordSwap value is incorrect. Set to default with exception :: "<< e.what();
			}
		}
	}
	catch(YAML::Exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		throw;
	}
	catch(exception &e)
	{
		CLogger::getInstance().log(FATAL, LOGDETAILS(e.what()));
		throw YAML::Exception(YAML::Mark::null_mark(), "key not found");
	}
	
	CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));
}

/**
 * Print well site info
 * @param a_oWellSite	:[in] well site
 */
void printWellSite(CWellSiteInfo a_oWellSite)
{
	string temp6 = " Start: wellsite: ";
	temp6.append(a_oWellSite.getID());
	CLogger::getInstance().log(DEBUG, LOGDETAILS(temp6));
	for(auto objWellSiteDev : a_oWellSite.getDevices())
	{
		string temp7 = a_oWellSite.getID();
		temp7.append("\\");
		temp7.append(objWellSiteDev.getID());
		CLogger::getInstance().log(DEBUG, LOGDETAILS(temp7));
		for(auto objPt : objWellSiteDev.getDevInfo().getDataPoints())
		{
			string temp7 =a_oWellSite.getID();
			temp7.append("\\");
			temp7.append(objWellSiteDev.getID());
			temp7.append("\\");
			temp7.append(objPt.getID());

			CLogger::getInstance().log(DEBUG, LOGDETAILS(temp7));
		}
	}
	CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));
}

/**
 * Build network info
 * @param a_bIsTCP	:[in] is network info for TCP
 */
void network_info::buildNetworkInfo(bool a_bIsTCP)
{
	string temp8 = " Start: is it TCP ? ";
	temp8.append(std::to_string(a_bIsTCP));
	CLogger::getInstance().log(DEBUG, LOGDETAILS(temp8));
	// Check if this function is already called once. If yes, then exit
	if(true == g_bIsStarted)
	{
		std::cout << "already started... return with no action \n";
		CLogger::getInstance().log(INFO, LOGDETAILS(" This function is already called once. Ignoring this call"));
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
	string temp9 = " Network set as: ";
	temp9.append(std::to_string((int)g_eNetworkType));
	CLogger::getInstance().log(INFO, LOGDETAILS(temp9));
	
	// Following stage is needed only when configuration files are placed in a docker volume
	#ifdef CONFIGFILES_IN_DOCKER_VOLUME
	//std::cout << "Config files are kept in a docker volume\n";
	//CLogger::getInstance().log(INFO, LOGDETAILS(" Config files are kept in a docker volume";
	
	// get list of well sites
	if(false == _getWellSiteList())
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS(" Site-list could not be obtained"));
		return;
	}
	std::vector<CWellSiteInfo> oWellSiteList;
	for(auto sWellSiteFile: g_sWellSiteFileList)
	{
		if(true == sWellSiteFile.empty())
		{
			std::cout << __func__ <<" : Encountered empty file name. Ignoring";
			CLogger::getInstance().log(INFO, LOGDETAILS(" : Encountered empty file name. Ignoring"));
			continue;
		}
		// Check if the file is already scanned
		std::map<std::string, CWellSiteInfo>::iterator it = g_mapYMLWellSite.find(sWellSiteFile);

		if(g_mapYMLWellSite.end() != it)
		{
			// It means record exists
			std::cout << __func__ << " " << sWellSiteFile <<" : is already scanned. Ignoring";
			string temp01 = "sWellSiteFile";
			temp01.append("Already scanned YML file: Ignoring it.");

			CLogger::getInstance().log(INFO, LOGDETAILS(temp01));
			continue;
		}
		string temp02 = " New YML file: ";
		temp02.append(sWellSiteFile);
		
		CLogger::getInstance().log(INFO, LOGDETAILS(temp02));

		try
		{
			YAML::Node baseNode = CommonUtils::loadYamlFile(sWellSiteFile);

			CWellSiteInfo objWellSite;
			CWellSiteInfo::build(baseNode, objWellSite);
			oWellSiteList.push_back(objWellSite);
			g_mapYMLWellSite.emplace(sWellSiteFile, objWellSite);
			string temp03 = " Successfully scanned: ";
			temp03.append(sWellSiteFile);
			temp03.append(": Id = ");
			temp03.append(objWellSite.getID());
			CLogger::getInstance().log(INFO, LOGDETAILS(temp03));
		}
		catch(YAML::Exception &e)
		{
			string temp04 = " Ignoring YML:";
			temp04.append(sWellSiteFile);
			temp04.append("Error: " );
			temp04.append(e.what());
			CLogger::getInstance().log(FATAL, LOGDETAILS(temp04));
			// Add this file to error YML files
			g_sErrorYMLs.push_back(sWellSiteFile);
		}
	}

	// Once network information is read, prepare a list of unique points
	// Set variables for unique point listing
	if(const char* env_p = std::getenv("MY_APP_ID"))
	{
		string temp05 = ": MY_APP_ID value = ";
		temp05.append(std::to_string(*env_p));
		CLogger::getInstance().log(INFO, LOGDETAILS(temp05));
		auto a = (unsigned short) atoi(env_p);
		a = a & 0x000F;
		g_usTotalCnt = (a << 12);
	}
	else
	{
		CLogger::getInstance().log(INFO, LOGDETAILS("MY_APP_ID value is not set. Expected values 0 to 16"));
		CLogger::getInstance().log(INFO, LOGDETAILS("Assuming value as 0"));
		g_usTotalCnt = 0;
	}
	CLogger::getInstance().log(INFO, LOGDETAILS(": Count start from = " +g_usTotalCnt));
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
	CLogger::getInstance().log(DEBUG, LOGDETAILS("End"));
}

/**
 * Constructor
 * @param a_sId 		:[in] site id
 * @param a_rWellSite	:[in] well site
 * @param a_rWellSiteDev:[in] well site device
 * @param a_rPoint		:[in] data points
 */
CUniqueDataPoint::CUniqueDataPoint(std::string a_sId, const CWellSiteInfo &a_rWellSite,
				const CWellSiteDevInfo &a_rWellSiteDev, const CDataPoint &a_rPoint) :
				m_uiMyRollID{((unsigned int)g_usTotalCnt)+1}, m_sId{a_sId},
				m_rWellSite{a_rWellSite}, m_rWellSiteDev{a_rWellSiteDev}, m_rPoint{a_rPoint}, m_bIsAwaitResp{false}, m_bIsRT{a_rPoint.getPollingConfig().m_bIsRealTime}
{
	++g_usTotalCnt;
}

/**
 * Constructor
 * @param a_objPt 		:[in] reference CUniqueDataPoint object for copy constructor
 */
CUniqueDataPoint::CUniqueDataPoint(const CUniqueDataPoint &a_objPt) :
				m_uiMyRollID{a_objPt.m_uiMyRollID}, m_sId{a_objPt.m_sId},
				m_rWellSite{a_objPt.m_rWellSite}, m_rWellSiteDev{a_objPt.m_rWellSiteDev}, m_rPoint{a_objPt.m_rPoint}, m_bIsAwaitResp{false}
{
		m_bIsRT.store(a_objPt.m_bIsRT);
}

/**
 * Check if response should await
 * @return 	true : on success,
 * 			false : on error
 */
bool CUniqueDataPoint::isIsAwaitResp() const {
	return m_bIsAwaitResp.load();
}

/**
 * Set if response should await
 * @param isAwaitResp	:[out] is response await true
 */
void CUniqueDataPoint::setIsAwaitResp(bool isAwaitResp) const {
	m_bIsAwaitResp.store(isAwaitResp);
}
