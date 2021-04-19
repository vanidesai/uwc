/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef SPARKPLUGUDTMGR_UT_HPP_
#define SPARKPLUGUDTMGR_UT_HPP_

#include "SparkPlugUDTMgr.hpp"
#include "QueueHandler.hpp"
#include "SCADAHandler.hpp"

#ifdef UNIT_TEST
#include <gtest/gtest.h>
#endif


class SparkPlugUDTMgr_ut : public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();
	bool _addNewUDT(std::string sUDTName, std::string sUDTVersion,  std::shared_ptr<CUDT> &cIfMetric)
	{
		std::string a_sName = "custom_udt";
		uint32_t a_uiDataType = METRIC_DATA_TYPE_STRING;
		std::string a_sVersion = "1.0";
		std::shared_ptr<CUDT> a_pUDT = std::make_shared<CUDT>(a_sName, a_uiDataType, false, a_sVersion);
		udtMap_t tempMap;
		tempMap.emplace(a_sName, a_pUDT);
		std::map<std::string, udtMap_t> tmpUDTMap;
		tmpUDTMap.emplace(a_sName, tempMap);
		CSparkPlugUDTManager::getInstance().testSetUDTMap(tmpUDTMap);
		if(false == CSparkPlugUDTManager::getInstance().addNewUDT(sUDTName, sUDTVersion, cIfMetric))
			{
				DO_LOG_ERROR("unable to add new UDT definition.");
				return false;
			}
		return true;
	}


	bool _addUDTDefToNbirth()
	{
	 org_eclipse_tahu_protobuf_Payload nbirth_payload;
	 memset(&nbirth_payload, 0, sizeof(org_eclipse_tahu_protobuf_Payload));
	 nbirth_payload.has_timestamp = true;
	 nbirth_payload.timestamp = get_current_timestamp();
	 nbirth_payload.has_seq = true;
	 nbirth_payload.seq = 0;
	 uint64_t a_uiBDSeq = 0;
		try
			{
				// Create the NBIRTH payload
				string strAppName = EnvironmentInfo::getInstance().getDataFromEnvMap("AppName");
				if(strAppName.empty())
				{
					DO_LOG_ERROR("App name is empty");
					return false;
				}

				nbirth_payload.uuid = (char*) strAppName.c_str();

				// Add getNBirthTopicsome device metrics
				add_simple_metric(&nbirth_payload, "Name", false, 0,
						METRIC_DATA_TYPE_STRING, false, false,
						CCommon::getInstance().getNodeName().c_str(), CCommon::getInstance().getNodeName().length()+1);

				add_simple_metric(&nbirth_payload, "bdSeq", false, 0, METRIC_DATA_TYPE_UINT64, false, false,
						&a_uiBDSeq, sizeof(a_uiBDSeq));
				CSCADAHandler::instance().addModbusTemplateDefToNbirth(nbirth_payload);

				// udtMap_t
				std::string a_sName = "custom_udt";
				uint32_t a_uiDataType = METRIC_DATA_TYPE_STRING;
				std::string a_sVersion = "1.0";
				std::shared_ptr<CUDT> a_pUDT = std::make_shared<CUDT>(a_sName, a_uiDataType, false, a_sVersion);
				bool result = CSparkPlugUDTManager::getInstance().addUDTDefToNbirth(nbirth_payload, a_pUDT);
				return result;
			}

		catch(std::exception& e){

			DO_LOG_ERROR(e.what());
			return false;
		}
	}

	bool _addUDTDefsToNbirth()
		{
		 org_eclipse_tahu_protobuf_Payload nbirth_payload;
		 memset(&nbirth_payload, 0, sizeof(org_eclipse_tahu_protobuf_Payload));
		 nbirth_payload.has_timestamp = true;
		 nbirth_payload.timestamp = get_current_timestamp();
		 nbirth_payload.has_seq = true;
		 nbirth_payload.seq = 0;
		 uint64_t a_uiBDSeq = 0;
		try
			{
				// Create the NBIRTH payload
				string strAppName = EnvironmentInfo::getInstance().getDataFromEnvMap("AppName");
				if(strAppName.empty())
				{
					DO_LOG_ERROR("App name is empty");
					return false;
				}

				nbirth_payload.uuid = (char*) strAppName.c_str();

				// Add getNBirthTopicsome device metrics
				add_simple_metric(&nbirth_payload, "Name", false, 0,
						METRIC_DATA_TYPE_STRING, false, false,
						CCommon::getInstance().getNodeName().c_str(), CCommon::getInstance().getNodeName().length()+1);

				add_simple_metric(&nbirth_payload, "bdSeq", false, 0, METRIC_DATA_TYPE_UINT64, false, false,
						&a_uiBDSeq, sizeof(a_uiBDSeq));
				CSCADAHandler::instance().addModbusTemplateDefToNbirth(nbirth_payload);

				std::string a_sName = "custom_udt";
				uint32_t a_uiDataType = METRIC_DATA_TYPE_STRING;
				std::string a_sVersion = "1.0";
				std::shared_ptr<CUDT> a_pUDT = std::make_shared<CUDT>(a_sName, a_uiDataType, false, a_sVersion);
				udtMap_t tempMap;
				tempMap.emplace(a_sVersion, a_pUDT);
				std::map<std::string, udtMap_t> tmpUDTMap;
				tmpUDTMap.emplace(a_sName, tempMap);
				CSparkPlugUDTManager::getInstance().testSetUDTMap(tmpUDTMap);
				bool result = CSparkPlugUDTManager::getInstance().addUDTDefsToNbirth(nbirth_payload);
				return result;
			}

			catch(std::exception& e){

				DO_LOG_ERROR(e.what());
				return false;
			}
		}


	void _isPresent()
	{
		std::string a_sName = "custom_udt";
		uint32_t a_uiDataType = METRIC_DATA_TYPE_STRING;
		std::string a_sVersion = "1.0";
		std::shared_ptr<CUDT> a_pUDT = std::make_shared<CUDT>(a_sName, a_uiDataType, false, a_sVersion);
		udtMap_t tempMap;
		tempMap.emplace(a_sVersion, a_pUDT);
		std::map<std::string, udtMap_t> tmpUDTMap;
		tmpUDTMap.emplace(a_sName, tempMap);
		CSparkPlugUDTManager::getInstance().testSetUDTMap(tmpUDTMap);
		std::string UDTDefName = a_sName;
		std::string UDTDefVer = "1.0";
		CSparkPlugUDTManager::getInstance().isPresent(UDTDefName, UDTDefVer);
	}



public:

	//std::vector<stRefForSparkPlugAction> &stRefActionVec;
	CMessageObject recvdMsg{};
	mqtt::const_message_ptr msg =  mqtt::make_message(
				"{\"topic\": \"UT_writeRequest\"}",
				"{\"wellhead\": \"PL0\",\"command\": \"D1\",\"value\": \"0x00\",\"timestamp\": \"2019-09-20 12:34:56\",\"usec\": \"1571887474111145\",\"version\": \"2.0\",\"app_seq\": \"1234\",\"realtime\":\"0	\"}"
		);
	std::string UDTDefName = "";
	std::string UDTDefVer;
	std::shared_ptr<CUDT> CUDTDefRef;
	org_eclipse_tahu_protobuf_Payload_Metric a_metric = { NULL, false, 0, true, get_current_timestamp(), true,
											METRIC_DATA_TYPE_UNKNOWN, false, 0, false, 0, false,
											true, false,
								org_eclipse_tahu_protobuf_Payload_MetaData_init_default,
											false,
													org_eclipse_tahu_protobuf_Payload_PropertySet_init_default,
											0,
											{ 0 } };
	org_eclipse_tahu_protobuf_Payload nbirth_payload = { true, get_current_timestamp(), 0, &a_metric,
					true, 0, NULL, NULL, NULL};

	std::string Payload = "True, 0, &a_metric,True, 0, 0, 0, 0";

	//m_mqttMsg = mqtt::make_message(a_sTopic, a_sMsg);

};






#endif /* SPARKPLUGUDTMGR_UT_HPP_ */
