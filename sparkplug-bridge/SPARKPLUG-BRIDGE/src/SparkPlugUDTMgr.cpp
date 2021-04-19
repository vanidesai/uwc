/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/
#include <string.h>
#include "SparkPlugDevMgr.hpp"
#include "SparkPlugUDTMgr.hpp"
#include "SCADAHandler.hpp"

/**
 * Processes TEMPLATEDEF message from vendor app
 * @param a_sPayLoad :[in] payload of template definition message from vendor app
 * @param a_stRefActionVec :[out] structure containing device information to be marked dead
 */
void CSparkPlugUDTManager::processTemplateDef(std::string a_sPayLoad,
		std::vector<stRefForSparkPlugAction> &a_stRefActionVec)
{
	cJSON *cjRoot = NULL;
	do
	{
		try
		{
			cjRoot = cJSON_Parse(a_sPayLoad.c_str());
			if (NULL == cjRoot)
			{
				DO_LOG_ERROR("Message received from MQTT could not be parsed in json format");
				break;
			}

			auto readParam = [&](std::string &a_sOutput, const char*a_psKey) 
			{
				if(NULL == a_psKey)
				{ return false; }
				cJSON *cjParam = cJSON_GetObjectItem(cjRoot, a_psKey);
				if (cjParam == NULL)
				{
					return false;
				}
				if (NULL == cjParam->valuestring)
				{
					return false;
				}
				a_sOutput.assign(cjParam->valuestring);
				return true;
			};
			std::string sUDTName{""};
			if(false == readParam(sUDTName, "udt_name"))
			{
				DO_LOG_ERROR("Invalid payload: Name is not found.");
				return;
			}
			/*
			 * Check whether sUDTName is not left blank
			 */

			if (sUDTName.length() == 0)
			{
				DO_LOG_ERROR("Invalid PayLoad: Name cannot be blank");
				return;
			}

			auto has_special_char = [](char ch)-> bool
			{
				if(ch == '_')
					return false;
				else
					return !(isalnum(ch));
			};

			if (sUDTName.empty())
			{
				DO_LOG_ERROR("Invalid payload: Template Name is blank.");
				return;
			}

			/*
			 * Check whether templateDef has special characters
			 */

			bool result = std::find_if(sUDTName.begin(), sUDTName.end(),
					has_special_char) != sUDTName.end();

			if (result)
			{
				DO_LOG_ERROR("Invalid payload: Template Name has special characters.");
				return;
			}

			std::string sUDTVersion{""};
			if(false == readParam(sUDTVersion, "version"))
			{
				DO_LOG_ERROR("Invalid payload: Version is not found.");
				return;
			}

			if(nullptr != isPresent(sUDTName, sUDTVersion))
			{
				DO_LOG_ERROR("UDT with given name and version already present. Ignored now. Name: " + sUDTName
						+ ", Version: " + sUDTVersion);
				return;
			}

			// Create a UDT object
			auto cIfMetric = std::make_shared<CUDT>(sUDTName, METRIC_DATA_TYPE_TEMPLATE, true, sUDTVersion);
			if(nullptr == cIfMetric)
			{
				DO_LOG_ERROR("unable to create UDT definition.");
				return;
			}
			if(false == cIfMetric->processMetric(cjRoot))
			{
				DO_LOG_ERROR("unable to parse UDT definition.");
				return;
			}

			// This is new UDT. Add to list.
			if(false == addNewUDT(sUDTName, sUDTVersion, cIfMetric))
			{
				DO_LOG_ERROR("unable to add new UDT definition.");
				return;				
			}

			metricMapIf_t  mapChangedMetrics;
			stRefForSparkPlugAction stDummyAction
				{ std::ref(m_oDummyDev), enMSG_UDTDEF_TO_SCADA, mapChangedMetrics };
				a_stRefActionVec.push_back(stDummyAction);
		} catch (const std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
		}
	} while (0);

	if (NULL != cjRoot)
	{
		cJSON_Delete(cjRoot);
	}
}

/**
 * Checks whether given UDT is already present.
 * @param a_sName :[in] UDT name
 * @param a_sVersion :[in] UDT version
 * @return shared_ptr<CUDT>: referene (if Present), nullptr: NOT present
 */
std::shared_ptr<CUDT> CSparkPlugUDTManager::isPresent(const std::string &a_sName, const std::string &a_sVersion)
{
	std::lock_guard<std::mutex> lock(m_mutexUDTList);

	auto itr = m_mapUDT.find(a_sName);
	if(itr != m_mapUDT.end())
	{
		auto &elem = itr->second;
		auto itr2 = elem.find(a_sVersion);
		if(itr2 != elem.end())
		{
			return itr2->second;
		}
	}

	return nullptr;
}

/**
 * Adds UDT to list
 * @param a_sName :[in] UDT name
 * @param a_sVersion :[in] UDT version
 * @param a_pUDT :[in] UDT data
 * @return true: Present, false: NOT present
 */
bool CSparkPlugUDTManager::addNewUDT(const std::string &a_sName, const std::string &a_sVersion, 
			std::shared_ptr<CUDT> &a_pUDT)
{
	bool bRet = false;
	do
	{
		try
		{
			std::lock_guard<std::mutex> lock(m_mutexUDTList);
			auto itr = m_mapUDT.find(a_sName);
			if(itr != m_mapUDT.end())
			{
				auto &elem = itr->second;
				auto itr2 = elem.find(a_sVersion);
				if(itr2 == elem.end())
				{
					elem.emplace(a_sVersion, a_pUDT);
					bRet = true;
				}
			}
			else
			{
				udtMap_t tempMap;
				tempMap.emplace(a_sVersion, a_pUDT);
				m_mapUDT.emplace(a_sName, tempMap);
				bRet = true;
			}
		} catch (const std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
		}
	} while (0);

	return bRet;
}

/**
 * Prepare template definitions to be published on SCADA system
 * @param a_rTahuPayload :[out] reference of spark plug message payload 
 * @param a_pUDT :[in] UDT to add to NBIRTH 
 * @return true/false depending on the success/failure
 */
bool CSparkPlugUDTManager::addUDTDefToNbirth(org_eclipse_tahu_protobuf_Payload& a_rTahuPayload,
							std::shared_ptr<CUDT> &a_pUDT)
{
	try
	{
		// Create the root UDT definition and add the UDT definition value which includes the UDT members and parameters
		org_eclipse_tahu_protobuf_Payload_Metric metric = org_eclipse_tahu_protobuf_Payload_Metric_init_default;
		if(true == a_pUDT->addMetricForBirth(metric, true))
		{
			// Add the UDT to the payload
			add_metric_to_payload(&a_rTahuPayload, &metric);
		}		
	}
	catch(std::exception &ex)
	{
		DO_LOG_FATAL(ex.what());
		return false;
	}
	return true;
}

/**
 * Prepare template definitions to be published on SCADA system
 * @param a_rTahuPayload :[out] reference of spark plug message payload 
 * @return true/false depending on the success/failure
 */
bool CSparkPlugUDTManager::addUDTDefsToNbirth(org_eclipse_tahu_protobuf_Payload& a_rTahuPayload)
{
	try
	{
		std::lock_guard<std::mutex> lock(m_mutexUDTList);
		for (auto& itr : m_mapUDT)
		{
			auto &elem = itr.second;
			for (auto& itr1 : elem)
			{
				addUDTDefToNbirth(a_rTahuPayload, itr1.second);
			}
		}
	}
	catch(std::exception &ex)
	{
		DO_LOG_FATAL(ex.what());
		return false;
	}
	return true;
}
