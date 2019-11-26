/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#ifndef INCLUDE_INC_NETWORKINFO_HPP_
#define INCLUDE_INC_NETWORKINFO_HPP_

#include <string>
#include <vector>
#include "YamlUtil.h"

#define SEPARATOR_CHAR "_"

using std::string;
using std::vector;

namespace network_info 
{
	enum class eNetworkType
	{
		eTCP,
		eRTU,
	};
	enum class eEndPointType
	{
		eCoil,
		eHolding_Register,
		eInput_Register,
		eDiscrete_Input
	};
	struct stDataPointAddress
	{
		int m_iAddress;
		int m_iWidth;
		eEndPointType m_eType;
	};
	struct stPollingData
	{
		unsigned int m_uiPollFreq;
		bool m_bIsRealTime;
	};
	class CDataPoint
	{
		std::string m_sId;
		struct stDataPointAddress m_stAddress;
		struct stPollingData m_stPollingConfig;
		static eEndPointType getPointType(const std::string&);
		
		public:
		const std::string& getID() const {return m_sId;}
		const struct stDataPointAddress& getAddress() { return m_stAddress;}
		const struct stPollingData& getPollingConfig() { return m_stPollingConfig;}

		static void build(const YAML::Node& a_oData, CDataPoint &a_oCDataPoint);
	};
	
	class CDeviceInfo
	{
		std::string m_sName;
		std::vector<CDataPoint> m_DataPointList;
		
		public:
		int addDataPoint(CDataPoint a_oDataPoint); 
		const std::vector<CDataPoint>& getDataPoints() const {return m_DataPointList;}

		static void build(const YAML::Node& a_oData, CDeviceInfo &a_oWellSiteDevInfo);
	};
	
	struct stTCPAddrInfo
	{
		std::string m_sIPAddress;
		unsigned int m_uiPortNumber;
	};
	struct stRTUAddrInfo
	{
		std::string m_sPortAddress;
		unsigned int m_uiBaudRate;
		unsigned int m_uiDataBits;
		unsigned int m_uiParity;
		unsigned int m_uiStart;
		unsigned int m_uiStop;
	};
	struct stModbusAddrInfo
	{
		eNetworkType a_NwType;
		struct stTCPAddrInfo m_stTCP;
		struct stRTUAddrInfo m_stRTU;
	};
	
	class CWellSiteDevInfo
	{
		std::string m_sId;
		struct stModbusAddrInfo m_stAddress;
		class CDeviceInfo m_oDev;
		
		struct CDeviceInfo& getDevInfo1() {return m_oDev;}

		public:
		std::string getID() const {return m_sId;}
		const struct stModbusAddrInfo& getAddressInfo() {return m_stAddress;}
		const struct CDeviceInfo& getDevInfo() {return m_oDev;}

		static void build(const YAML::Node& a_oData, CWellSiteDevInfo &a_oWellSiteDevInfo);

	};
	
	class CWellSiteInfo
	{
		std::string m_sId;
		std::vector<CWellSiteDevInfo> m_DevList;
		
		public:
		CWellSiteInfo()
		{
		}
		std::string getID() const {return m_sId;}
		int addDevice(CWellSiteDevInfo a_oDevice);
		const std::vector<CWellSiteDevInfo>& getDevices() {return m_DevList;}
		const std::vector<CWellSiteDevInfo>& getDevices() const {return m_DevList;}

		static void build(const YAML::Node& a_oData, CWellSiteInfo &a_oWellSite);
	};
	
	class CUniqueDataPoint
	{
		const std::string m_sId;
		const CWellSiteInfo &m_rWellSite;
		const CWellSiteDevInfo &m_rWellSiteDev;
		const CDataPoint &m_rPoint;
		
		public:
		CUniqueDataPoint(std::string a_sId, const CWellSiteInfo &a_rWellSite,
				const CWellSiteDevInfo &a_rWellSiteDev, const CDataPoint &a_rPoint) :
				m_sId{a_sId}, 
				m_rWellSite{a_rWellSite}, m_rWellSiteDev{a_rWellSiteDev}, m_rPoint{a_rPoint}
		{
		}
		
		std::string getID() {return m_sId;}
		const CWellSiteInfo& getWellSite() {return m_rWellSite;}
		const CWellSiteDevInfo& getWellSiteDev() {return m_rWellSiteDev;}
		const CDataPoint& getDataPoint() {return m_rPoint;}
	};

	void buildNetworkInfo(bool a_bIsTCP);
	const std::map<std::string, CWellSiteInfo>& getWellSiteList();
	const std::map<std::string, CUniqueDataPoint>& getUniquePointList();
}

#endif /* INCLUDE_INC_NETWORKINFO_HPP_ */
