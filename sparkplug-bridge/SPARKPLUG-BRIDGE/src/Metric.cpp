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
#include <string.h>
#include <limits>
#include "Metric.hpp"
#include "SCADAHandler.hpp"
#include "SparkPlugUDTMgr.hpp"

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
				a_metric.value.string_value = strndup((std::get<std::string>(m_objVal)).c_str(),
											(std::get<std::string>(m_objVal)).length());
				break;

			default:
				DO_LOG_ERROR("Not supported datatype encountered: "
								+ std::to_string(m_uiDataType));
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
 * Assign values to sparkplug parameter data-structure according to the sparkplug specification
 * @param a_param :[out] parameter in which to assign value in sparkplug format
 * @return true/false based on success/failure
 */
bool CValObj::assignToSparkPlug(org_eclipse_tahu_protobuf_Payload_Template_Parameter &a_param) const
{
	do
	{
		try
		{
			switch (m_uiDataType)
			{
			case METRIC_DATA_TYPE_BOOLEAN:
				a_param.type = PARAMETER_DATA_TYPE_BOOLEAN;
				a_param.which_value = org_eclipse_tahu_protobuf_Payload_Template_Parameter_boolean_value_tag;
				a_param.value.boolean_value = std::get<bool>(m_objVal);
				break;

			case METRIC_DATA_TYPE_UINT8:
				a_param.type = PARAMETER_DATA_TYPE_UINT8;
				a_param.which_value = org_eclipse_tahu_protobuf_Payload_Template_Parameter_int_value_tag;
				a_param.value.int_value = std::get<uint8_t>(m_objVal);
				break;
			case METRIC_DATA_TYPE_UINT16:
				a_param.type = PARAMETER_DATA_TYPE_UINT16;
				a_param.which_value = org_eclipse_tahu_protobuf_Payload_Template_Parameter_int_value_tag;
				a_param.value.int_value = std::get<uint16_t>(m_objVal);
				break;
			case METRIC_DATA_TYPE_INT8:
				a_param.type = PARAMETER_DATA_TYPE_INT8;
				a_param.which_value = org_eclipse_tahu_protobuf_Payload_Template_Parameter_int_value_tag;
				a_param.value.int_value = std::get<int8_t>(m_objVal);
				break;
			case METRIC_DATA_TYPE_INT16:
				a_param.type = PARAMETER_DATA_TYPE_INT16;
				a_param.which_value = org_eclipse_tahu_protobuf_Payload_Template_Parameter_int_value_tag;
				a_param.value.int_value = std::get<int16_t>(m_objVal);
				break;
			case METRIC_DATA_TYPE_INT32:
				a_param.type = PARAMETER_DATA_TYPE_INT32;
				a_param.which_value = org_eclipse_tahu_protobuf_Payload_Template_Parameter_int_value_tag;
				a_param.value.int_value = std::get<int32_t>(m_objVal);
				break;

			case METRIC_DATA_TYPE_UINT32:
				a_param.type = PARAMETER_DATA_TYPE_UINT32;
				a_param.which_value = org_eclipse_tahu_protobuf_Payload_Template_Parameter_long_value_tag;
				a_param.value.long_value = std::get<uint32_t>(m_objVal);
				break;
			case METRIC_DATA_TYPE_UINT64:
				a_param.type = PARAMETER_DATA_TYPE_UINT64;
				a_param.which_value = org_eclipse_tahu_protobuf_Payload_Template_Parameter_long_value_tag;
				a_param.value.long_value = std::get<uint64_t>(m_objVal);
				break;
			case METRIC_DATA_TYPE_INT64:
				a_param.type = PARAMETER_DATA_TYPE_INT64;
				a_param.which_value = org_eclipse_tahu_protobuf_Payload_Template_Parameter_long_value_tag;
				a_param.value.long_value = std::get<int64_t>(m_objVal);
				break;

			case METRIC_DATA_TYPE_FLOAT:
				a_param.type = PARAMETER_DATA_TYPE_FLOAT;
				a_param.which_value = org_eclipse_tahu_protobuf_Payload_Template_Parameter_float_value_tag;
				a_param.value.float_value = std::get<float>(m_objVal);
				break;

			case METRIC_DATA_TYPE_DOUBLE:
				a_param.type = PARAMETER_DATA_TYPE_DOUBLE;
				a_param.which_value = org_eclipse_tahu_protobuf_Payload_Template_Parameter_double_value_tag;
				a_param.value.double_value = std::get<double>(m_objVal);
				break;

			case METRIC_DATA_TYPE_STRING:
				a_param.type = PARAMETER_DATA_TYPE_STRING;
				a_param.which_value = org_eclipse_tahu_protobuf_Payload_Template_Parameter_string_value_tag;
				a_param.value.string_value = strndup((std::get<std::string>(m_objVal)).c_str(), 
											(std::get<std::string>(m_objVal)).length());
				break;

			default:
				DO_LOG_ERROR("Not supported datatype encountered: "
								+ std::to_string(m_uiDataType));
				return false;
				break;
			}
		} catch (std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
			if(METRIC_DATA_TYPE_STRING == m_uiDataType)
			{
				if(NULL != a_param.value.string_value)
				{
					free(a_param.value.string_value);
					a_param.value.string_value = NULL;
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
 * @param a_sKeyname : [in] For real devices keyname is 'scaledValue' and for apps it is only 'value' 
 * @return true/false based on success/failure
 */
bool CValObj::assignToCJSON(cJSON *a_cjMetric, const std::string &a_sKeyName) const
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
					cJSON_AddItemToObject(a_cjMetric, a_sKeyName.c_str(), cJSON_CreateTrue());
				}
				else
				{
					cJSON_AddItemToObject(a_cjMetric, a_sKeyName.c_str(), cJSON_CreateFalse());
				}
			}
			break;

		case METRIC_DATA_TYPE_UINT8:
		{
			cJSON_AddItemToObject(a_cjMetric, "dataType", cJSON_CreateString("UInt8"));
			uint8_t val = std::get<uint8_t>(m_objVal);
			cJSON_AddItemToObject(a_cjMetric, a_sKeyName.c_str(), cJSON_CreateNumber(val));
		}
			break;
		case METRIC_DATA_TYPE_UINT16:
			{
				cJSON_AddItemToObject(a_cjMetric, "dataType", cJSON_CreateString("UInt16"));
				uint16_t val = std::get<uint16_t>(m_objVal);
				cJSON_AddItemToObject(a_cjMetric, a_sKeyName.c_str(), cJSON_CreateNumber(val));
			}
			break;
		case METRIC_DATA_TYPE_INT8:
			{
				cJSON_AddItemToObject(a_cjMetric, "dataType", cJSON_CreateString("Int8"));
				int8_t val = std::get<int8_t>(m_objVal);
				cJSON_AddItemToObject(a_cjMetric, a_sKeyName.c_str(), cJSON_CreateNumber(val));
			}
			break;
		case METRIC_DATA_TYPE_INT16:
			{
				cJSON_AddItemToObject(a_cjMetric, "dataType", cJSON_CreateString("Int16"));
				int16_t val = std::get<int16_t>(m_objVal);
				cJSON_AddItemToObject(a_cjMetric, a_sKeyName.c_str(), cJSON_CreateNumber(val));
			}
			break;
		case METRIC_DATA_TYPE_INT32:
			{
				cJSON_AddItemToObject(a_cjMetric, "dataType", cJSON_CreateString("Int32"));
				int32_t val = std::get<int32_t>(m_objVal);
				cJSON_AddItemToObject(a_cjMetric, a_sKeyName.c_str(), cJSON_CreateNumber(val));
			}
			break;

		case METRIC_DATA_TYPE_UINT32:
			{
				cJSON_AddItemToObject(a_cjMetric, "dataType", cJSON_CreateString("UInt32"));
				uint32_t val = std::get<uint32_t>(m_objVal);
				cJSON_AddItemToObject(a_cjMetric, a_sKeyName.c_str(), cJSON_CreateNumber(val));
			}
			break;
		case METRIC_DATA_TYPE_UINT64:
			{
				cJSON_AddItemToObject(a_cjMetric, "dataType", cJSON_CreateString("UInt64"));
				uint64_t val = std::get<uint64_t>(m_objVal);
				cJSON_AddItemToObject(a_cjMetric, a_sKeyName.c_str(), cJSON_CreateNumber(val));
			}
			break;
		case METRIC_DATA_TYPE_INT64:
			{
				cJSON_AddItemToObject(a_cjMetric, "dataType", cJSON_CreateString("Int64"));
				int64_t val = std::get<int64_t>(m_objVal);
				cJSON_AddItemToObject(a_cjMetric, a_sKeyName.c_str(), cJSON_CreateNumber(val));
			}
			break;

		case METRIC_DATA_TYPE_FLOAT:
			{
				cJSON_AddItemToObject(a_cjMetric, "dataType", cJSON_CreateString("Float"));
				float val = std::get<float>(m_objVal);
				cJSON_AddItemToObject(a_cjMetric, a_sKeyName.c_str(), cJSON_CreateNumber(val));
			}
			break;

		case METRIC_DATA_TYPE_DOUBLE:
			{
				cJSON_AddItemToObject(a_cjMetric, "dataType", cJSON_CreateString("Double"));
				double val = std::get<double>(m_objVal);
				cJSON_AddItemToObject(a_cjMetric, a_sKeyName.c_str(), cJSON_CreateNumber(val));
			}
			break;

		case METRIC_DATA_TYPE_STRING:
			{
				cJSON_AddItemToObject(a_cjMetric, "dataType", cJSON_CreateString("String"));
				string val = std::get<std::string>(m_objVal);
				cJSON_AddItemToObject(a_cjMetric, a_sKeyName.c_str(), cJSON_CreateString(val.c_str()));
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
				// Check if negative value is received for unsigned datatype
				if(a_cjValue->valuedouble < 0.0)
				{
					DO_LOG_ERROR("Negative value is received for an unsigned datatype. Ignored.");
					return false;
				}
				m_uiDataType = METRIC_DATA_TYPE_UINT8;
				m_objVal = (uint8_t) a_cjValue->valueint;
			}
			else if (("uint16" == a_sDatatype)
					&& (1 == cJSON_IsNumber(a_cjValue)))
			{
				// Check if negative value is received for unsigned datatype
				if(a_cjValue->valuedouble < 0.0)
				{
					DO_LOG_ERROR("Negative value is received for an unsigned datatype. Ignored.");
					return false;
				}
				m_uiDataType = METRIC_DATA_TYPE_UINT16;
				m_objVal = (uint16_t) a_cjValue->valuedouble;
			}
			else if (("uint32" == a_sDatatype)
					&& (1 == cJSON_IsNumber(a_cjValue)))
			{
				// Check if negative value is received for unsigned datatype
				if(a_cjValue->valuedouble < 0.0)
				{
					DO_LOG_ERROR("Negative value is received for an unsigned datatype. Ignored.");
					return false;
				}
				m_uiDataType = METRIC_DATA_TYPE_UINT32;
				m_objVal = (uint32_t) a_cjValue->valuedouble;
			}
			else if (("uint64" == a_sDatatype)
					&& (1 == cJSON_IsNumber(a_cjValue)))
			{
				// Check if negative value is received for unsigned datatype
				if(a_cjValue->valuedouble < 0.0)
				{
					DO_LOG_ERROR("Negative value is received for an unsigned datatype. Ignored.");
					return false;
				}
				m_uiDataType = METRIC_DATA_TYPE_UINT64;
				m_objVal = (uint64_t) a_cjValue->valuedouble;
			}
			else if (("int8" == a_sDatatype)
					&& (1 == cJSON_IsNumber(a_cjValue)))
			{
				m_uiDataType = METRIC_DATA_TYPE_INT8;
				m_objVal = (int8_t) a_cjValue->valueint;
			}
			else if (("int16" == a_sDatatype)
					&& (1 == cJSON_IsNumber(a_cjValue)))
			{
				m_uiDataType = METRIC_DATA_TYPE_INT16;
				m_objVal = (int16_t) a_cjValue->valueint;
			}
			else if (("int32" == a_sDatatype)
					&& (1 == cJSON_IsNumber(a_cjValue)))
			{
				m_uiDataType = METRIC_DATA_TYPE_INT32;
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
				m_objVal = (float) a_cjValue->valuedouble;
			}
			else if (("double" == a_sDatatype)
					&& (1 == cJSON_IsNumber(a_cjValue)))
			{
				m_uiDataType = METRIC_DATA_TYPE_DOUBLE;
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
 * @param a_metric :[in] Sparkplug metric object
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
			break;

		case METRIC_DATA_TYPE_FLOAT:
			m_objVal = (float)a_metric.value.float_value;
			break;

		case METRIC_DATA_TYPE_DOUBLE:
			m_objVal = (double)a_metric.value.double_value;
			break;

		case METRIC_DATA_TYPE_STRING:
			if(NULL == a_metric.value.string_value)
			{
				return false;
			}
			if (((string)a_metric.value.string_value).empty())
			{
				return false;
			}
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
 * @param a_rMetric :[out] reference of sparkplug object in which to store data
 * @return true/false depending on the success/failure
 */
bool CMetric::addMetricNameValue(org_eclipse_tahu_protobuf_Payload_Metric& a_rMetric)
{
	try
	{
		{
			a_rMetric.name = strndup(m_sSparkPlugName.c_str(), m_sSparkPlugName.length());
			if(a_rMetric.name == NULL)
			{
				DO_LOG_ERROR("Failed to allocate new memory");
				return false;
			};

			m_objVal.assignToSparkPlug(a_rMetric);
			a_rMetric.is_null = false;
		}
	}
	catch(std::exception &ex)
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
 * Add name value to a parameter to be published on SCADA system
 * @param a_rMetric :[out] reference of sparkplug object in which to store data
 * @return true/false depending on the success/failure
 */
bool CMetric::addParameterNameValue(org_eclipse_tahu_protobuf_Payload_Template_Parameter& a_rParam)
{
	try
	{
		{
			a_rParam.name = strndup(m_sSparkPlugName.c_str(), m_sSparkPlugName.length());
			if(a_rParam.name == NULL)
			{
				DO_LOG_ERROR("Failed to allocate new memory");
				return false;
			};

			m_objVal.assignToSparkPlug(a_rParam);
			a_rParam.has_type = true;
		}
	}
	catch(std::exception &ex)
	{
		DO_LOG_FATAL(ex.what());
		if(a_rParam.name != NULL)
		{
			free(a_rParam.name);
			a_rParam.name = NULL;
		}
		return false;
	}
	return true;
}

/**
 * Prepare device birth messages to be published on SCADA system
 * @param a_rMetric :[out] reference of sparkplug metric to store data
 * @return true/false depending on the success/failure
 */
bool CMetric::addMetricForBirth(org_eclipse_tahu_protobuf_Payload_Metric& a_rMetric)
{
	using namespace network_info;
	try
	{
		{
			if (true == std::holds_alternative<std::reference_wrapper<const network_info::CUniqueDataPoint>>(m_rDirectProp))
			{
				auto &orUniqueDataPoint = std::get<std::reference_wrapper<const network_info::CUniqueDataPoint>>(m_rDirectProp);
				
				CSCADAHandler::instance().addModbusMetric(a_rMetric, m_sSparkPlugName, m_objVal,
					true, orUniqueDataPoint.get().getDataPoint().getPollingConfig().m_uiPollFreq,
					orUniqueDataPoint.get().getDataPoint().getPollingConfig().m_bIsRealTime,
					orUniqueDataPoint.get().getDataPoint().getAddress().m_dScaleFactor);
			}
			else
			{
				if(addMetricNameValue(a_rMetric) == false)
				{
					DO_LOG_ERROR(m_sSparkPlugName + ":Failed to add metric name and value");
					return false;
				}
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

/**
 * Prepare sparkplug formatted metric for modbus device
 * @param a_rMetric :[out] reference of sparkplug metric to store data
 * @param a_bIsBirth :[In] Indicates whether it is a birth message
 * @return true/false depending on the success/failure
 */
bool CMetric::addModbusMetric(org_eclipse_tahu_protobuf_Payload_Metric& a_rMetric, bool a_bIsBirth)
{
	using namespace network_info;
	try
	{
		if (true == std::holds_alternative<std::reference_wrapper<const network_info::CUniqueDataPoint>>(m_rDirectProp))
		{
			auto &orUniqueDataPoint = std::get<std::reference_wrapper<const network_info::CUniqueDataPoint>>(m_rDirectProp);
			
			CSCADAHandler::instance().addModbusMetric(a_rMetric, m_sSparkPlugName, m_objVal,
				a_bIsBirth, orUniqueDataPoint.get().getDataPoint().getPollingConfig().m_uiPollFreq,
				orUniqueDataPoint.get().getDataPoint().getPollingConfig().m_bIsRealTime, orUniqueDataPoint.get().getDataPoint().getAddress().m_dScaleFactor);
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
 * Processes metric to parse its data-type and value; sets in CValueObj corresponding to the metric
 * @param a_cjArrayElemMetric :[in] cJSON array element containing details about the metric
 * @return true/false based on success/failure
 */
bool CMetric::processMetric(cJSON *a_cjArrayElemMetric)
{
	do
	{
		try
		{
			if (a_cjArrayElemMetric == NULL)
			{
				DO_LOG_ERROR("Invalid payload: Name or datatype or value is missing.");
				return false;
			}

			//timestamp is optional parameter
			uint64_t current_time = 0;
			cJSON *cjTimestamp = cJSON_GetObjectItem(a_cjArrayElemMetric,"timestamp");
			if(cjTimestamp == NULL)
			{
				current_time = get_current_timestamp();
			}
			else
			{
				current_time = cjTimestamp->valuedouble;
			}

			setTimestamp(current_time);

			cJSON *cjDataType = cJSON_GetObjectItem(a_cjArrayElemMetric, "dataType");
			if (cjDataType == NULL)
			{
				DO_LOG_ERROR("Invalid payload: dataType is not found.");
				return false;
			}

			if (cjDataType->valuestring == NULL)
			{
				DO_LOG_ERROR("Datatype is NULL. Cannot parse payload.");
				return false;
			}

			std::string sDataType{ cjDataType->valuestring };

			cJSON *cjValue = cJSON_GetObjectItem(a_cjArrayElemMetric, "value");
			if (cjValue == NULL)
			{
				DO_LOG_ERROR("Invalid payload: value is not found.");
				return false;
			}

			//map data-type with Sparkplug data-type
			if (false == m_objVal.setValObj(sDataType, cjValue))
			{
				DO_LOG_ERROR(std::string("Cannot parse value. Ignored the metric: ") + m_sName);
				return false;
			}
		}
		catch (std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
			return false;
		}
	} while (0);
	return true;
}

/**
 * Validates UDT instance data against UDT definition reference data
 * @return true/false based on success/failure
 */
bool CUDT::validate()
{
	do
	{
		try
		{
			if(m_bIsDefinition)
			{
				// This UDT instance itself is a definition 
				return false;
			}

			if(nullptr == m_pCUDTDefRef)
			{
				DO_LOG_ERROR("UDT reference with given name and version not found. Ignored the metric. UDT Name: " + m_sUDTDefName
						+ ", Version: " + m_sUDTDefVer);
				return false;
			}

			if(false == compareMetrics(m_pCUDTDefRef))
			{
				DO_LOG_ERROR("Metric does not match with UDT definition. Ignored");
				return false;
			}
		}
		catch (std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
			return false;
		}
	} while (0);
	return true;
}

/**
 * Processes metric to parse UDT reference data
 * @param a_cjArrayElemMetric :[in] cJSON array element containing details about the metric
 * @return true/false based on success/failure
 */
bool CUDT::readUDTRefData(cJSON *a_cjArrayElemMetric)
{
	if(NULL == a_cjArrayElemMetric)
		return false;
	do
	{
		try
		{
			cJSON* cjUDTRef = cJSON_GetObjectItem(a_cjArrayElemMetric, "udt_ref");
			if (cjUDTRef == NULL)
			{
				DO_LOG_ERROR("Invalid payload: value is not found.");
				return false;
			}

			auto readParam = [&cjUDTRef](std::string &a_sOutput, const char*a_psKey) 
			{
				if(NULL == a_psKey)
				{ return false; }
				cJSON *cjParam = cJSON_GetObjectItem(cjUDTRef, a_psKey);
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

			if(false == readParam(m_sUDTDefName, "name"))
			{
				DO_LOG_DEBUG("Invalid payload: Name is not found.");
				return false;
			}
			if(false == readParam(m_sUDTDefVer, "version"))
			{
				DO_LOG_DEBUG("Invalid payload: Version is not found.");
				return false;
			}

			m_pCUDTDefRef = CSparkPlugUDTManager::getInstance().isPresent(m_sUDTDefName, m_sUDTDefVer);
		}
		catch (std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
			return false;
		}
	} while (0);
	return true;
}

/**
 * Compares metric maps with given input metric map
 * @param a_map1 :[in] input metric map 1
 * @param a_map2 :[in] input metric map 2
 * @return true/false based on success/failure
 */
bool CUDT::compareMetricMaps(metricMapIf_t &a_map1, metricMapIf_t &a_map2)
{
	do
	{
		try
		{
			if(a_map1.size() != a_map2.size())
			{
				DO_LOG_ERROR("Map size does not match");
				return false;
			}
			for(auto &itr1: a_map1)
			{
				auto itr2 = a_map2.find(itr1.first);
				if(a_map2.end() == itr2)
				{
					DO_LOG_ERROR(itr1.first + ": Key not found in 2nd map");
					return false;
				}
				auto &obj1 = itr1.second;
				auto &obj2 = itr2->second;
				if((nullptr == obj1) && (nullptr == obj2))
				{
					continue;
				}
				else if(nullptr == obj1)
				{
					DO_LOG_DEBUG("map1 element is null");
					return false;
				}
				else if(nullptr == obj2)
				{
					DO_LOG_DEBUG("map2 element is null");
					return false;
				}
				// Compare the metric
				if(false == obj1->compareMetrics(obj2))
				{
					return false;
				}
			}
		}
		catch (std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
			return false;
		}
	} while (0);
	return true;
}

/**
 * Compares metric maps with given input metric map
 * @param a_myMap :[in] input metric map 1
 * @param a_newMap :[in] input metric map 2
 * @return list of metrics having change of value
 */
metricMapIf_t CUDT::compareMetricMapValues(const metricMapIf_t &a_myMap, metricMapIf_t &a_newMap) const
{
	metricMapIf_t mapChangedMetrics;
	do
	{
		try
		{
			for(auto &itrNew: a_newMap)
			{
				auto itrMy = a_myMap.find(itrNew.first);
				if(a_myMap.end() == itrMy)
				{
					DO_LOG_ERROR(itrNew.first + ": Key not found in 2nd map");
					break;
				}
				auto &objMy = itrMy->second;
				auto &objNew = itrNew.second;
				if((nullptr == objMy) && (nullptr == objNew))
				{
					continue;
				}
				else if(nullptr == objMy)
				{
					DO_LOG_DEBUG("map1 element is null");
					break;
				}
				else if(nullptr == objNew)
				{
					DO_LOG_DEBUG("map1 element is null");
					break;
				}
				// Compare the metric
				if(VALUES_DIFFERENT == objMy->compareValue(*objNew))
				{
					mapChangedMetrics.emplace(itrNew.first, objNew);
				}
			}
		}
		catch (std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
		}
	} while (0);
	return mapChangedMetrics;
}

/**
 * Compared UDT metric with given input metric
 * @param a_pUDT :[in] input metric
 * @return true/false based on success/failure
 */
bool CUDT::compareMetrics(const std::shared_ptr<CIfMetric> &a_pUDT)
{
	do
	{
		try
		{
			if(nullptr == a_pUDT)
			{
				return false;
			}
			CUDT *pOtherMetric = dynamic_cast<CUDT*>(a_pUDT.get());
			if(nullptr == pOtherMetric)
			{
				return false;
			}
			// Compare UDT reference
			if((m_sUDTDefName != pOtherMetric->m_sUDTDefName) 
				&& (m_sUDTDefVer != pOtherMetric->m_sUDTDefVer))
			{
				return false;
			}

			// Compare metric name if metric being used for comparison 
			// is NOT UDT defintion 
			if(false == pOtherMetric->m_bIsDefinition)
			{
				if(false == CIfMetric::compareMetrics(a_pUDT))
				{
					DO_LOG_DEBUG("Metric name or type does not match");
					return false;
				}
			}

			// Compare metrics
			if(false == compareMetricMaps(m_mapMetrics, pOtherMetric->m_mapMetrics))
			{
				DO_LOG_DEBUG("map metrics do not match");
				return false;
			}

			// Compare parameters
			if(false == compareMetricMaps(m_mapParams, pOtherMetric->m_mapParams))
			{
				DO_LOG_DEBUG("map Params do not match");
				return false;
			}

		}
		catch (std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
			return false;
		}
	} while (0);
	return true;
}

/**
 * Processes metric to parse its data-type and value; sets in CValueObj corresponding to the metric
 * @param a_cjArrayElemMetric :[in] cJSON array element containing details about the metric
 * @return true/false based on success/failure
 */
bool CUDT::processMetric(cJSON *a_cjArrayElemMetric)
{
	do
	{
		try
		{
			if (a_cjArrayElemMetric == NULL)
			{
				DO_LOG_ERROR("Invalid payload: Name or datatype or value is missing.");
				return false;
			}

			//timestamp is optional parameter
			uint64_t current_time = 0;
			cJSON *cjTimestamp = cJSON_GetObjectItem(a_cjArrayElemMetric, "timestamp");
			if(cjTimestamp == NULL)
			{
				current_time = get_current_timestamp();
			}
			else
			{
				current_time = cjTimestamp->valuedouble;
			}
			setTimestamp(current_time);

			cJSON *cjValue = NULL;
			if(m_bIsDefinition)
			{
				cjValue = a_cjArrayElemMetric;
			}
			else
			{
				cjValue = cJSON_GetObjectItem(a_cjArrayElemMetric, "value");
			}
			if (cjValue == NULL)
			{
				DO_LOG_ERROR("Invalid payload: value is not found.");
				return false;
			}

			// Read metrics from value data
			CSparkPlugDevManager::getInstance().parseVendorAppMericData(m_mapMetrics, cjValue, "metrics");

			// Read parameter from value data
			CSparkPlugDevManager::getInstance().parseVendorAppMericData(m_mapParams, cjValue, "parameters");

			if(false == m_bIsDefinition)
			{
			// Read udt reference data
				return readUDTRefData(cjValue);
			}
		}
		catch (std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
			return false;
		}
	} while (0);
	return true;
}

/**
 * Prepare device birth messages to be published on SCADA system
 * @param a_rMetric :[out] reference of spark plug metric in which to store birth messages
 * @param a_bIsNBirth: [in] indicates whether it is NBIRTH process
 * @return true/false depending on the success/failure
 */
bool CUDT::addMetricForBirth(org_eclipse_tahu_protobuf_Payload_Metric& a_rMetric, bool a_bIsNBirth)
{
	try
	{
		org_eclipse_tahu_protobuf_Payload_Template udt_template = org_eclipse_tahu_protobuf_Payload_Template_init_default;
		udt_template.version = strndup(m_sUDTDefVer.c_str(), m_sUDTDefVer.length());
		udt_template.metrics_count = m_mapMetrics.size();
		udt_template.metrics = (org_eclipse_tahu_protobuf_Payload_Metric *) calloc(m_mapMetrics.size(), sizeof(org_eclipse_tahu_protobuf_Payload_Metric));
		udt_template.has_is_definition = true;
		
		// If this is NBIRTH, then it is UDT defintion 
		if(a_bIsNBirth)
		{
			udt_template.is_definition = true;
			udt_template.template_ref = NULL;
		}
		else
		{
			udt_template.is_definition = false;
			udt_template.template_ref = strndup(m_sUDTDefName.c_str(), m_sUDTDefName.length());
		}
		int iLoop = 0;

		if(udt_template.metrics != NULL)
		{
			for(auto &itr: m_mapMetrics)
			{
				if(itr.second)
				{
					udt_template.metrics[iLoop] = org_eclipse_tahu_protobuf_Payload_Metric_init_default;

					init_metric(&udt_template.metrics[iLoop], (itr.second)->getSparkPlugName().c_str(), false, 0, 
						(itr.second)->getDataType(), false, false, 0, {0});

					if(true != (itr.second)->addMetricForBirth(udt_template.metrics[iLoop]))
					{
						DO_LOG_ERROR((itr.second)->getSparkPlugName() + ":Could not add metric. Trying to add other metrics.");
					}
					udt_template.metrics[iLoop].timestamp = (itr.second)->getTimestamp();
					udt_template.metrics[iLoop].has_timestamp = true;
				}
				++iLoop;
			}
		}

		if(m_mapParams.size() > 0)
		{
			udt_template.parameters_count = m_mapParams.size();
			udt_template.parameters = (org_eclipse_tahu_protobuf_Payload_Template_Parameter *) calloc(m_mapParams.size(), sizeof(org_eclipse_tahu_protobuf_Payload_Template_Parameter));
			if(NULL != udt_template.parameters)
			{
				iLoop = 0;
				for(auto &itr: m_mapParams)
				{
					if(itr.second)
					{
						itr.second->addParameterNameValue(udt_template.parameters[iLoop]);
					}
					++iLoop;
				}
			}
			else
			{
				udt_template.parameters_count = 0;
			}
		}

		// Create the root UDT definition and add the UDT definition value which includes the UDT members and parameters
		a_rMetric = org_eclipse_tahu_protobuf_Payload_Metric_init_default;
		init_metric(&a_rMetric, getSparkPlugName().c_str(), false, 0, METRIC_DATA_TYPE_TEMPLATE, false, false, &udt_template, sizeof(udt_template));
		a_rMetric.timestamp = get_current_timestamp();
		a_rMetric.has_timestamp = true;
	}
	catch(std::exception &ex)
	{
		DO_LOG_FATAL(ex.what());
		return false;
	}
	return true;
}

/**
 * Prepare birth messages to be published on SCADA system
 * @param a_rMetric :[out] reference of spark plug metric in which to store birth messages
 * @return true/false depending on the success/failure
 */
bool CUDT::addMetricForBirth(org_eclipse_tahu_protobuf_Payload_Metric& a_rMetric)
{
	return addMetricForBirth(a_rMetric, false);
}

/**
 * Assigns new value from given metric
 * @param a_obj :[in] input metric
 * @return status success/failure
 */
uint8_t CUDT::assignNewValue(CIfMetric &a_obj)
{
	do
	{
		try
		{
			CUDT *pOtherMetric = dynamic_cast<CUDT*>(&a_obj);
			if(nullptr == pOtherMetric)
			{
				return DATATYPE_DIFFERENT;
			}

			// Compare metrics
			for(auto &itrNew: pOtherMetric->m_mapMetrics)
			{
				if(nullptr == itrNew.second)
				{
					continue;
				}
				auto itrMy = m_mapMetrics.find(itrNew.first);
				if(itrMy != m_mapMetrics.end())
				{
					if(itrMy->second)
					{
						itrMy->second.get()->assignNewValue(*(itrNew.second));
					}
				}
			}

			// Normally for DATA kind of messages, 
			// UDT def and version might not be available in incoming data.
			// Set those fields in incoming data
			
			// Check if version for incoming UDT is not set
			if(true == pOtherMetric->m_sUDTDefVer.empty())
			{
				pOtherMetric->m_sUDTDefVer.assign(m_sUDTDefVer);
			}
			// Check if version for incoming UDT is not set
			if((true == pOtherMetric->m_sUDTDefName.empty()) && 
					(false == pOtherMetric->m_bIsDefinition))
			{
				pOtherMetric->m_sUDTDefName.assign(m_sUDTDefName);
			}
		}
		catch (std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
		}
	} while (0);
	return 0;
}

/**
 * Compared value from given metric with this metric
 * @param a_obj :[in] input metric
 * @return status comparison result
 */
uint8_t CUDT::compareValue(CIfMetric &a_obj) const
{
	do
	{
		try
		{
			CUDT *pOtherMetric = dynamic_cast<CUDT*>(&a_obj);
			if(nullptr == pOtherMetric)
			{
				return DATATYPE_DIFFERENT;
			}

			// Compare metrics
			metricMapIf_t mapChanges = compareMetricMapValues(m_mapMetrics, pOtherMetric->m_mapMetrics);
			if(0 == mapChanges.size())
			{
				return SAMEVALUE_OR_DTATYPE;
			}

			pOtherMetric->m_mapMetrics.clear();
			pOtherMetric->m_mapMetrics.insert(mapChanges.begin(), mapChanges.end());

			return VALUES_DIFFERENT;
		}
		catch (std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
		}
	} while (0);
	return SAMEVALUE_OR_DTATYPE;
}

/**
 * Add name value to a metric to be published on SCADA system
 * @param a_rMetric :[out] reference of spark plug message payload 
 * @return true/false depending on the success/failure
 */
bool CUDT::addMetricNameValue(org_eclipse_tahu_protobuf_Payload_Metric& a_rMetric)
{
	try
	{
		org_eclipse_tahu_protobuf_Payload_Template udt_template = org_eclipse_tahu_protobuf_Payload_Template_init_default;
		udt_template.version = strndup(m_sUDTDefVer.c_str(), m_sUDTDefVer.length());
		udt_template.metrics_count = m_mapMetrics.size();
		udt_template.metrics = (org_eclipse_tahu_protobuf_Payload_Metric *) calloc(m_mapMetrics.size(), sizeof(org_eclipse_tahu_protobuf_Payload_Metric));
		udt_template.has_is_definition = true;
		
		udt_template.is_definition = false;
		udt_template.template_ref = strndup(m_sUDTDefName.c_str(), m_sUDTDefName.length());
		int iLoop = 0;

		if(udt_template.metrics != NULL)
		{
			for(auto &itr: m_mapMetrics)
			{
				if(itr.second)
				{
					udt_template.metrics[iLoop] = org_eclipse_tahu_protobuf_Payload_Metric_init_default;

					init_metric(&udt_template.metrics[iLoop], (itr.second)->getSparkPlugName().c_str(), false, 0, 
						(itr.second)->getDataType(), false, false, 0, {0});

					if(true != (itr.second)->addMetricNameValue(udt_template.metrics[iLoop]))
					{
						DO_LOG_ERROR((itr.second)->getSparkPlugName() + ":Could not add metric. Trying to add other metrics.");
					}
					udt_template.metrics[iLoop].timestamp = (itr.second)->getTimestamp();
					udt_template.metrics[iLoop].has_timestamp = true;
				}
				++iLoop;
			}
		}

		// Create the root UDT definition and add the UDT definition value which includes the UDT members and parameters
		a_rMetric = org_eclipse_tahu_protobuf_Payload_Metric_init_default;
		init_metric(&a_rMetric, getSparkPlugName().c_str(), false, 0, METRIC_DATA_TYPE_TEMPLATE, false, false, &udt_template, sizeof(udt_template));
		a_rMetric.timestamp = get_current_timestamp();
		a_rMetric.has_timestamp = true;
	}
	catch(std::exception &ex)
	{
		DO_LOG_FATAL(ex.what());
		return false;
	}
	return true;
}

/**
 * Sets metric value based on Sparkplug data object
 * @param a_sparkplugMetric :[in] Sparkplug metric type
 * @return true/false depending on the success/failure
 */
bool CUDT::setValObj(org_eclipse_tahu_protobuf_Payload_Metric& a_sparkplugMetric)
{
	do
	{
		try
		{
			CIfMetric::setValObj(a_sparkplugMetric);
			org_eclipse_tahu_protobuf_Payload_Template &udt_template = a_sparkplugMetric.value.template_value;
			for(uint iLoop = 0; iLoop < udt_template.metrics_count; iLoop++)
			{
				if(NULL == udt_template.metrics[iLoop].name)
				{
					DO_LOG_DEBUG("Metric name is not present in DCMD message. Ignored.");
					return false;
				}
				std::shared_ptr<CIfMetric> ptrCIfMetric = 
							CSparkPlugDevManager::getInstance().metricFactoryMethod(
								udt_template.metrics[iLoop].name, udt_template.metrics[iLoop].datatype);
				if(nullptr == ptrCIfMetric)
				{
					DO_LOG_ERROR("Unable to build metric. Not processing this message.");
					return false;
				}

				if(false == ptrCIfMetric->setValObj(udt_template.metrics[iLoop]))
				{
					DO_LOG_ERROR(ptrCIfMetric->getName() + " : Could not process metric data");
					return false;
				}

				m_mapMetrics.emplace(ptrCIfMetric->getName(), std::move(ptrCIfMetric));
				ptrCIfMetric = nullptr;
			}
		}
		catch (std::exception &e)
		{
			DO_LOG_ERROR(std::string("Error:") + e.what());
		}
	} while (0);
	return true;
}

/**
 * Sets CJSON object based on data in this metric
 * @param a_sparkplugMetric :[in] Sparkplug metric type
 * @return true/false depending on the success/failure
 */
bool CUDT::assignToCJSON(cJSON *a_cjMetric, bool a_bIsRealDevice)
{
	if(NULL != a_cjMetric)
	{
		cJSON *pJSONArray = NULL;
		try
		{
			//cJSON_AddItemToObject(a_cjMetric, "name", cJSON_CreateString(m_sName.c_str()));
			cJSON_AddItemToObject(a_cjMetric, "dataType", cJSON_CreateString("UDT"));
			pJSONArray = cJSON_CreateArray();
			if (pJSONArray == NULL)
			{
				DO_LOG_ERROR("Creation of CJSON array failed");
				return false;
			}
			for(auto &itr: m_mapMetrics)
			{
				if(nullptr == itr.second)
					continue;
				
				cJSON *cjNestedMetric = cJSON_CreateObject();
				if(cjNestedMetric == NULL)
				{
					cJSON_Delete(pJSONArray);
					DO_LOG_ERROR("Error while creating CJSON message");
					return false;
				}
				// pass udt flag is false pass the same value as argument b_isRealDevice
				if(false == itr.second->assignToCJSON(cjNestedMetric, a_bIsRealDevice))
				{
					cJSON_Delete(cjNestedMetric);
					cJSON_Delete(pJSONArray);
					DO_LOG_ERROR(itr.first + ": Unable to form write request.");
					return false;
				}
				cJSON_AddItemToObject(cjNestedMetric, "name", cJSON_CreateString((itr.second)->getName().c_str()));
				cJSON_AddItemToObject(cjNestedMetric, "timestamp", cJSON_CreateNumber((itr.second)->getTimestamp()));
				cJSON_AddItemToArray(pJSONArray, cjNestedMetric);
			}
			cJSON_AddItemToObject(a_cjMetric, "metrics", pJSONArray);
		}
		catch (std::exception &e)
		{
			if(NULL != pJSONArray)
			{
				cJSON_Delete(pJSONArray);
			}
			DO_LOG_ERROR(std::string("Error:") + e.what());
		}
	} while (0);
	return true;
}
