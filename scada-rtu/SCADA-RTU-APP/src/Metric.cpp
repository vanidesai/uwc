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
#include "Metric.hpp"

/**
 * Assign values to sparkplug metric data-structure according to the sparkplug specification
 * @param a_metric :[out] metric in which to assign value in sparkplug format
 * @return true/false based on success/failure
 */
bool CValObj::assignToSparkPlug(
		org_eclipse_tahu_protobuf_Payload_Metric &a_metric)
{
	do
	{
		try
		{
			switch (m_uiDataType)
			{
			case METRIC_DATA_TYPE_BOOLEAN:
				cout << "** Found METRIC_DATA_TYPE_BOOLEAN" << endl;
				a_metric.which_value = org_eclipse_tahu_protobuf_Payload_Metric_boolean_value_tag;
				a_metric.value.boolean_value = std::get<bool>(m_objVal);
				break;

			case METRIC_DATA_TYPE_UINT8:
				cout << "** Found METRIC_DATA_TYPE_UINT8" << endl;
				a_metric.which_value = org_eclipse_tahu_protobuf_Payload_Metric_int_value_tag;
				a_metric.value.int_value = std::get<uint8_t>(m_objVal);
				break;
			case METRIC_DATA_TYPE_UINT16:
				cout << "** Found METRIC_DATA_TYPE_UINT16" << endl;
				a_metric.which_value = org_eclipse_tahu_protobuf_Payload_Metric_int_value_tag;
				a_metric.value.int_value = std::get<uint16_t>(m_objVal);
				break;
			case METRIC_DATA_TYPE_INT8:
				cout << "** Found METRIC_DATA_TYPE_INT8" << endl;
				a_metric.which_value = org_eclipse_tahu_protobuf_Payload_Metric_int_value_tag;
				a_metric.value.int_value = std::get<int8_t>(m_objVal);
				break;
			case METRIC_DATA_TYPE_INT16:
				cout << "** Found METRIC_DATA_TYPE_INT16" << endl;
				a_metric.which_value = org_eclipse_tahu_protobuf_Payload_Metric_int_value_tag;
				a_metric.value.int_value = std::get<int16_t>(m_objVal);
				break;
			case METRIC_DATA_TYPE_INT32:
				cout << "** Found METRIC_DATA_TYPE_INT32" << endl;
				a_metric.which_value = org_eclipse_tahu_protobuf_Payload_Metric_int_value_tag;
				a_metric.value.int_value = std::get<int32_t>(m_objVal);
				break;

			case METRIC_DATA_TYPE_UINT32:
				cout << "** Found METRIC_DATA_TYPE_UINT32" << endl;
				a_metric.which_value = org_eclipse_tahu_protobuf_Payload_Metric_long_value_tag;
				a_metric.value.int_value = std::get<uint32_t>(m_objVal);
				break;
			case METRIC_DATA_TYPE_UINT64:
				cout << "** Found METRIC_DATA_TYPE_UINT64" << endl;
				a_metric.which_value = org_eclipse_tahu_protobuf_Payload_Metric_long_value_tag;
				a_metric.value.long_value = std::get<uint64_t>(m_objVal);
				break;
			case METRIC_DATA_TYPE_INT64:
				cout << "** Found METRIC_DATA_TYPE_INT64" << endl;
				a_metric.which_value = org_eclipse_tahu_protobuf_Payload_Metric_long_value_tag;
				a_metric.value.long_value = std::get<int64_t>(m_objVal);
				break;

			case METRIC_DATA_TYPE_FLOAT:
				cout << "** Found METRIC_DATA_TYPE_FLOAT" << endl;
				a_metric.which_value = org_eclipse_tahu_protobuf_Payload_Metric_float_value_tag;
				a_metric.value.float_value = std::get<float>(m_objVal);
				break;

			case METRIC_DATA_TYPE_DOUBLE:
				cout << "** Found METRIC_DATA_TYPE_DOUBLE" << endl;
				a_metric.which_value = org_eclipse_tahu_protobuf_Payload_Metric_double_value_tag;
				a_metric.value.double_value = std::get<double>(m_objVal);
				break;

			case METRIC_DATA_TYPE_STRING:
				cout << "** Found METRIC_DATA_TYPE_STRING" << endl;
				a_metric.which_value = org_eclipse_tahu_protobuf_Payload_Metric_string_value_tag;
				a_metric.value.string_value = strdup((std::get<std::string>(m_objVal)).c_str());
				break;

			default:
				cout << "** Found Invalid data type" << endl;
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
			return false;
		}
	} while (0);
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
				m_objVal = (uint32_t) a_cjValue->valueint;
			}
			else if (("uint64" == a_sDatatype)
					&& (1 == cJSON_IsNumber(a_cjValue)))
			{
				m_uiDataType = METRIC_DATA_TYPE_UINT64;
				//m_objVal = (uint64_t)cJSON_GetNumberValue(a_cjValue);
				m_objVal = (uint64_t) a_cjValue->valueint;
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
				m_uiDataType = METRIC_DATA_TYPE_INT64;
				//m_objVal = (int64_t)cJSON_GetNumberValue(a_cjValue);
				m_objVal = (int64_t) a_cjValue->valueint;
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
