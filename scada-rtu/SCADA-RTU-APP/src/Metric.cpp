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
#include <limits>
#include "Metric.hpp"

/**
 * Assign values to sparkplug metric data-structure according to the sparkplug specification
 * @param a_metric :[out] metric in which to assign value in sparkplug format
 * @return true/false based on success/failure
 */
bool CValObj::assignToSparkPlug(org_eclipse_tahu_protobuf_Payload_Metric &a_metric) const
{
	do
	{
		try
		{
			switch (m_uiDataType)
			{
			case METRIC_DATA_TYPE_BOOLEAN:
				a_metric.which_value = org_eclipse_tahu_protobuf_Payload_Metric_boolean_value_tag;
				a_metric.value.boolean_value = std::get<bool>(m_objVal);
				break;

			case METRIC_DATA_TYPE_UINT8:
				a_metric.which_value = org_eclipse_tahu_protobuf_Payload_Metric_int_value_tag;
				a_metric.value.int_value = std::get<uint8_t>(m_objVal);
				break;
			case METRIC_DATA_TYPE_UINT16:
				a_metric.which_value = org_eclipse_tahu_protobuf_Payload_Metric_int_value_tag;
				a_metric.value.int_value = std::get<uint16_t>(m_objVal);
				break;
			case METRIC_DATA_TYPE_INT8:
				a_metric.which_value = org_eclipse_tahu_protobuf_Payload_Metric_int_value_tag;
				a_metric.value.int_value = std::get<int8_t>(m_objVal);
				break;
			case METRIC_DATA_TYPE_INT16:
				a_metric.which_value = org_eclipse_tahu_protobuf_Payload_Metric_int_value_tag;
				a_metric.value.int_value = std::get<int16_t>(m_objVal);
				break;
			case METRIC_DATA_TYPE_INT32:
				a_metric.which_value = org_eclipse_tahu_protobuf_Payload_Metric_int_value_tag;
				a_metric.value.int_value = std::get<int32_t>(m_objVal);
				break;

			case METRIC_DATA_TYPE_UINT32:
				a_metric.which_value = org_eclipse_tahu_protobuf_Payload_Metric_long_value_tag;
				a_metric.value.long_value = std::get<uint32_t>(m_objVal);
				break;
			case METRIC_DATA_TYPE_UINT64:
				a_metric.which_value = org_eclipse_tahu_protobuf_Payload_Metric_long_value_tag;
				a_metric.value.long_value = std::get<uint64_t>(m_objVal);
				break;
			case METRIC_DATA_TYPE_INT64:
				a_metric.which_value = org_eclipse_tahu_protobuf_Payload_Metric_long_value_tag;
				a_metric.value.long_value = std::get<int64_t>(m_objVal);
				break;

			case METRIC_DATA_TYPE_FLOAT:
				a_metric.which_value = org_eclipse_tahu_protobuf_Payload_Metric_float_value_tag;
				a_metric.value.float_value = std::get<float>(m_objVal);
				break;

			case METRIC_DATA_TYPE_DOUBLE:
				a_metric.which_value = org_eclipse_tahu_protobuf_Payload_Metric_double_value_tag;
				a_metric.value.double_value = std::get<double>(m_objVal);
				break;

			case METRIC_DATA_TYPE_STRING:
				a_metric.which_value = org_eclipse_tahu_protobuf_Payload_Metric_string_value_tag;
				a_metric.value.string_value = strdup((std::get<std::string>(m_objVal)).c_str());
				break;

			default:
				DO_LOG_ERROR(
						"Not supported datatype encountered: "
								+ std::to_string(m_uiDataType))
				;
				return false;
				break;
			}
		} catch (std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
			if(METRIC_DATA_TYPE_STRING == m_uiDataType)
			{
				if(NULL != a_metric.value.string_value)
				{
					free(a_metric.value.string_value);
					a_metric.value.string_value = NULL;
				}
			}
			return false;
		}
	} while (0);
	return true;
}

/**
 * Assign values to sparkplug metric data-structure according to the sparkplug specification
 * @param a_metric :[out] metric in which to assign value in sparkplug format
 * @return true/false based on success/failure
 */
bool CValObj::assignToCJSON(cJSON *a_cjMetric) const
{
	do
	{
	try
	{
		switch (m_uiDataType)
		{
		case METRIC_DATA_TYPE_BOOLEAN:
			{
				cJSON_AddItemToObject(a_cjMetric, "dataType", cJSON_CreateString("Boolean"));
				bool val = std::get<bool>(m_objVal);
				if(val == true)
				{
					cJSON_AddItemToObject(a_cjMetric, "value", cJSON_CreateTrue());
				}
				else
				{
					cJSON_AddItemToObject(a_cjMetric, "value", cJSON_CreateFalse());
				}
			}
			break;

		case METRIC_DATA_TYPE_UINT8:
		{
			cJSON_AddItemToObject(a_cjMetric, "dataType", cJSON_CreateString("UInt8"));
			uint8_t val = std::get<uint8_t>(m_objVal);
			cJSON_AddItemToObject(a_cjMetric, "value", cJSON_CreateNumber(val));
		}
			break;
		case METRIC_DATA_TYPE_UINT16:
			{
				cJSON_AddItemToObject(a_cjMetric, "dataType", cJSON_CreateString("UInt16"));
				uint16_t val = std::get<uint16_t>(m_objVal);
				cJSON_AddItemToObject(a_cjMetric, "value", cJSON_CreateNumber(val));
			}
			break;
		case METRIC_DATA_TYPE_INT8:
			{
				cJSON_AddItemToObject(a_cjMetric, "dataType", cJSON_CreateString("Int8"));
				int8_t val = std::get<int8_t>(m_objVal);
				cJSON_AddItemToObject(a_cjMetric, "value", cJSON_CreateNumber(val));
			}
			break;
		case METRIC_DATA_TYPE_INT16:
			{
				cJSON_AddItemToObject(a_cjMetric, "dataType", cJSON_CreateString("Int16"));
				int16_t val = std::get<int16_t>(m_objVal);
				cJSON_AddItemToObject(a_cjMetric, "value", cJSON_CreateNumber(val));
			}
			break;
		case METRIC_DATA_TYPE_INT32:
			{
				cJSON_AddItemToObject(a_cjMetric, "dataType", cJSON_CreateString("Int32"));
				int32_t val = std::get<int32_t>(m_objVal);
				cJSON_AddItemToObject(a_cjMetric, "value", cJSON_CreateNumber(val));
			}
			break;

		case METRIC_DATA_TYPE_UINT32:
			{
				cJSON_AddItemToObject(a_cjMetric, "dataType", cJSON_CreateString("UInt32"));
				uint32_t val = std::get<uint32_t>(m_objVal);
				cJSON_AddItemToObject(a_cjMetric, "value", cJSON_CreateNumber(val));
			}
			break;
		case METRIC_DATA_TYPE_UINT64:
			{
				cJSON_AddItemToObject(a_cjMetric, "dataType", cJSON_CreateString("UInt64"));
				uint64_t val = std::get<uint64_t>(m_objVal);
				cJSON_AddItemToObject(a_cjMetric, "value", cJSON_CreateNumber(val));
			}
			break;
		case METRIC_DATA_TYPE_INT64:
			{
				cJSON_AddItemToObject(a_cjMetric, "dataType", cJSON_CreateString("Int64"));
				int64_t val = std::get<int64_t>(m_objVal);
				cJSON_AddItemToObject(a_cjMetric, "value", cJSON_CreateNumber(val));
			}
			break;

		case METRIC_DATA_TYPE_FLOAT:
			{
				cJSON_AddItemToObject(a_cjMetric, "dataType", cJSON_CreateString("Float"));
				float val = std::get<float>(m_objVal);
				cJSON_AddItemToObject(a_cjMetric, "value", cJSON_CreateNumber(val));
			}
			break;

		case METRIC_DATA_TYPE_DOUBLE:
			{
				cJSON_AddItemToObject(a_cjMetric, "dataType", cJSON_CreateString("Double"));
				double val = std::get<double>(m_objVal);
				cJSON_AddItemToObject(a_cjMetric, "value", cJSON_CreateNumber(val));
			}
			break;

		case METRIC_DATA_TYPE_STRING:
			{
				cJSON_AddItemToObject(a_cjMetric, "dataType", cJSON_CreateString("String"));
				string val = std::get<std::string>(m_objVal);
				cJSON_AddItemToObject(a_cjMetric, "value", cJSON_CreateString(val.c_str()));
			}
			break;

		default:
			DO_LOG_ERROR(
					"Not supported datatype encountered: "
							+ std::to_string(m_uiDataType))
			;
			return false;
		}
	}
	catch (std::exception &e)
	{
		DO_LOG_ERROR(std::string("Error:") + e.what());
		return false;
	}
	}while(0);
	return true;
}

/**
 * Set value and data-type of a metric
 * @param a_sDatatype :[in] data-type to set
 * @param a_cjValue :[in] value to set
 * @return true/false based on success/failure
 */
bool CValObj::setValObj(std::string a_sDatatype, cJSON *a_cjValue)
{
	do
	{
		try
		{
			if (NULL == a_cjValue)
			{
				DO_LOG_ERROR("Received NULL data-type");
				return false;
			}

			std::transform(a_sDatatype.begin(), a_sDatatype.end(),
					a_sDatatype.begin(), ::tolower);

			// bool, uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t, float, double, std::string
			if (("boolean" == a_sDatatype) && (1 == cJSON_IsBool(a_cjValue)))
			{
				m_uiDataType = METRIC_DATA_TYPE_BOOLEAN;
				if (cJSON_IsTrue(a_cjValue) == 1)
				{
					m_objVal = true;
				}
				else
				{
					m_objVal = false;
				}
			}
			else if (("uint8" == a_sDatatype)
					&& (1 == cJSON_IsNumber(a_cjValue)))
			{
				m_uiDataType = METRIC_DATA_TYPE_UINT8;
				//m_objVal = (uint8_t)cJSON_GetNumberValue(a_cjValue);
				m_objVal = (uint8_t) a_cjValue->valueint;
			}
			else if (("uint16" == a_sDatatype)
					&& (1 == cJSON_IsNumber(a_cjValue)))
			{
				m_uiDataType = METRIC_DATA_TYPE_UINT16;
				//m_objVal = (uint16_t)cJSON_GetNumberValue(a_cjValue);
				m_objVal = (uint16_t) a_cjValue->valueint;
			}
			else if (("uint32" == a_sDatatype)
					&& (1 == cJSON_IsNumber(a_cjValue)))
			{
				m_uiDataType = METRIC_DATA_TYPE_UINT32;
				//m_objVal = (uint32_t)cJSON_GetNumberValue(a_cjValue);
				m_objVal = (uint32_t) a_cjValue->valuedouble;
			}
			else if (("uint64" == a_sDatatype)
					&& (1 == cJSON_IsNumber(a_cjValue)))
			{
				m_uiDataType = METRIC_DATA_TYPE_UINT64;
				//m_objVal = (uint64_t)cJSON_GetNumberValue(a_cjValue);
				m_objVal = (uint64_t) a_cjValue->valuedouble;
			}
			else if (("int8" == a_sDatatype)
					&& (1 == cJSON_IsNumber(a_cjValue)))
			{
				m_uiDataType = METRIC_DATA_TYPE_INT8;
				//m_objVal = (int8_t)cJSON_GetNumberValue(a_cjValue);
				m_objVal = (int8_t) a_cjValue->valueint;
			}
			else if (("int16" == a_sDatatype)
					&& (1 == cJSON_IsNumber(a_cjValue)))
			{
				m_uiDataType = METRIC_DATA_TYPE_INT16;
				//m_objVal = (int16_t)cJSON_GetNumberValue(a_cjValue);
				m_objVal = (int16_t) a_cjValue->valueint;
			}
			else if (("int32" == a_sDatatype)
					&& (1 == cJSON_IsNumber(a_cjValue)))
			{
				m_uiDataType = METRIC_DATA_TYPE_INT32;
				//m_objVal = (int32_t)cJSON_GetNumberValue(a_cjValue);
				m_objVal = (int32_t) a_cjValue->valueint;
			}
			else if (("int64" == a_sDatatype)
					&& (1 == cJSON_IsNumber(a_cjValue)))
			{
				int64_t i64 = static_cast<std::int64_t>(a_cjValue->valuedouble);
				// Handle corner scenario of max value
				if((i64 < 0) && (a_cjValue->valuedouble > 0.0))
				{
					i64 = std::numeric_limits<int64_t>::max();
				}
				m_uiDataType = METRIC_DATA_TYPE_INT64;
				m_objVal = i64;
			}
			else if (("float" == a_sDatatype)
					&& (1 == cJSON_IsNumber(a_cjValue)))
			{
				m_uiDataType = METRIC_DATA_TYPE_FLOAT;
				//m_objVal = (float)cJSON_GetNumberValue(a_cjValue);
				m_objVal = (float) a_cjValue->valuedouble;
			}
			else if (("double" == a_sDatatype)
					&& (1 == cJSON_IsNumber(a_cjValue)))
			{
				m_uiDataType = METRIC_DATA_TYPE_DOUBLE;
				//m_objVal = (double)cJSON_GetNumberValue(a_cjValue);
				m_objVal = (double) a_cjValue->valuedouble;
			}
			else if (("string" == a_sDatatype)
					&& (1 == cJSON_IsString(a_cjValue)))
			{
				m_uiDataType = METRIC_DATA_TYPE_STRING;
				char *val = cJSON_GetStringValue(a_cjValue);
				if (val == NULL)
				{
					DO_LOG_ERROR("Cannot find value in payload");
					return false;
				}
				std::string sTemp(val);
				m_objVal = sTemp;
			}
			else
			{
				DO_LOG_ERROR(
						"Invalid data type or mismatch found. Mentioned type: "
								+ a_sDatatype + ", Value datatype:"
								+ std::to_string(a_cjValue->type));
				return false;
			}
		} catch (std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
			return false;
		}
	} while (0);
	return true;
}

/**
 * Set value and data-type of a metric from sparkplug DCMD message
 * @param a_sDatatype :[in] data-type to set
 * @param a_cjValue :[in] value to set
 * @return true/false based on success/failure
 */
bool CValObj::setValObj(org_eclipse_tahu_protobuf_Payload_Metric& a_metric)
{
	try
	{
		m_uiDataType = a_metric.datatype;

		switch (m_uiDataType)
		{
		case METRIC_DATA_TYPE_BOOLEAN:
			m_objVal = (bool)a_metric.value.boolean_value;
			break;

		case METRIC_DATA_TYPE_UINT8:
			m_objVal = (uint8_t)a_metric.value.int_value;
			break;
		case METRIC_DATA_TYPE_UINT16:
			m_objVal = (uint16_t)a_metric.value.int_value;
			break;
		case METRIC_DATA_TYPE_INT8:
			m_objVal = (int8_t)a_metric.value.int_value;
			break;
		case METRIC_DATA_TYPE_INT16:
			m_objVal = (int16_t)a_metric.value.int_value;
			break;
		case METRIC_DATA_TYPE_INT32:
			m_objVal = (int32_t) a_metric.value.int_value;
			break;

		case METRIC_DATA_TYPE_UINT32:
			m_objVal = (uint32_t)a_metric.value.int_value;
			break;
		case METRIC_DATA_TYPE_UINT64:
			m_objVal = (uint64_t)a_metric.value.long_value;
			break;
		case METRIC_DATA_TYPE_INT64:
			m_objVal = (int64_t)a_metric.value.long_value;
			//m_objVal = a_metric.value.long_value;
			break;

		case METRIC_DATA_TYPE_FLOAT:
			m_objVal = (float)a_metric.value.float_value;
			break;

		case METRIC_DATA_TYPE_DOUBLE:
			m_objVal = (double)a_metric.value.double_value;
			break;

		case METRIC_DATA_TYPE_STRING:
			m_objVal = (string)a_metric.value.string_value;
			break;

		default:
			DO_LOG_ERROR(
					"Not supported datatype encountered: "
							+ std::to_string(m_uiDataType))
			;
			return false;
		}
	}
	catch (std::exception &e)
	{
		DO_LOG_FATAL(e.what());
		return false;
	}
	return true;
}

/**
 * Add name value to a metric to be published on SCADA system
 * @param dbirth_payload :[out] reference of spark plug message payload in which to store birth messages
 * @return true/false depending on the success/failure
 */
bool CMetric::addMetricNameValue(org_eclipse_tahu_protobuf_Payload_Metric& a_rMetric)
{
	using namespace network_info;
	try
	{
		{
			a_rMetric.name = strdup(m_sSparkPlugName.c_str());
			if(a_rMetric.name == NULL)
			{
				DO_LOG_ERROR("Failed to allocate new memory");
				return false;
			};

			m_objVal.assignToSparkPlug(a_rMetric);
		}
	}
	catch(exception &ex)
	{
		DO_LOG_FATAL(ex.what());
		if(a_rMetric.name != NULL)
		{
			free(a_rMetric.name);
			a_rMetric.name = NULL;
		}
		return false;
	}
	return true;
}

/**
 * Prepare device birth messages to be published on SCADA system
 * @param dbirth_payload :[out] reference of spark plug message payload in which to store birth messages
 * @return true/false depending on the success/failure
 */
bool CMetric::addMetricForBirth(org_eclipse_tahu_protobuf_Payload_Metric& a_rMetric)
{
	using namespace network_info;
	try
	{
		{
			if(addMetricNameValue(a_rMetric) == false)
			{
				DO_LOG_ERROR(m_sSparkPlugName + ":Failed to add metric name and value");
				return false;
			};

			if (true == std::holds_alternative<std::reference_wrapper<const network_info::CUniqueDataPoint>>(m_rDirectProp))
			{
				org_eclipse_tahu_protobuf_Payload_PropertySet prop = org_eclipse_tahu_protobuf_Payload_PropertySet_init_default;
				auto &orUniqueDataPoint = std::get<std::reference_wrapper<const network_info::CUniqueDataPoint>>(m_rDirectProp);
				
				int iAddr = orUniqueDataPoint.get().getDataPoint().getAddress().m_iAddress;
				add_property_to_set(&prop, "Addr", PROPERTY_DATA_TYPE_INT32, &iAddr, sizeof(iAddr));

				int iWidth = orUniqueDataPoint.get().getDataPoint().getAddress().m_iWidth;
				add_property_to_set(&prop, "Width", PROPERTY_DATA_TYPE_INT32, &iWidth, sizeof(iWidth));

				string strDataType = orUniqueDataPoint.get().getDataPoint().getAddress().m_sDataType;
				add_property_to_set(&prop, "DataType", PROPERTY_DATA_TYPE_STRING, strDataType.c_str(), strDataType.length());

				eEndPointType endPointType = orUniqueDataPoint.get().getDataPoint().getAddress().m_eType;

				string strType = "";
				switch(endPointType)
				{
				case eEndPointType::eCoil:
					strType = "COIL";
					break;
				case eEndPointType::eDiscrete_Input:
					strType = "DISCRETE_INPUT";
					break;
				case eEndPointType::eHolding_Register:
					strType = "HOLDING_REGISTER";
					break;
				case eEndPointType::eInput_Register:
					strType = "INPUT_REGISTER";
					break;
				default:
					DO_LOG_ERROR("Invalid type of data-point in yml file");
					return false;
				}
				add_property_to_set(&prop, "Type", PROPERTY_DATA_TYPE_STRING, strType.c_str(), strType.length());

				uint32_t iPollingInterval = orUniqueDataPoint.get().getDataPoint().getPollingConfig().m_uiPollFreq;
				add_property_to_set(&prop, "Pollinterval", PROPERTY_DATA_TYPE_UINT32, &iPollingInterval, sizeof(iPollingInterval));

				bool bVal = orUniqueDataPoint.get().getDataPoint().getPollingConfig().m_bIsRealTime;
				add_property_to_set(&prop, "Realtime", PROPERTY_DATA_TYPE_BOOLEAN, &bVal, sizeof(bVal));

				add_propertyset_to_metric(&a_rMetric, &prop);
			}
		}
	}
	catch(exception &ex)
	{
		DO_LOG_FATAL(ex.what());
		return false;
	}
	return true;
}
