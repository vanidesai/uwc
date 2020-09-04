/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef METRIC_HPP_
#define METRIC_HPP_

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <variant>
#include <typeinfo>
#include <map>
#include <functional>
#include "Logger.hpp"

extern "C"
{
#include "cjson/cJSON.h"
}

#include <tahu.pb.h>
#include <pb_decode.h>
#include <pb_encode.h>
#include <inttypes.h>

extern "C"
{
	#include <tahu.h>
}

using namespace std;

#define SAMEVALUE_OR_DTATYPE  (0)
#define DATATYPE_DIFFERENT (1)
#define VALUES_DIFFERENT   (2)
#define NO_CHANGE_IN_VALUE (3)
#define VALUE_ASSINED      (4)
#define SUBDEV_SEPARATOR_CHAR ("-")

using var_t = std::variant<bool, uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t, float, double, std::string>;

class CValObj
{
	friend class CMetric;
	uint32_t m_uiDataType;
	var_t m_objVal;

	bool setValObj(std::string a_sDatatype, cJSON *a_cjValue);

public:
	CValObj() :
			m_uiDataType
			{ 0 }
	{
		;
	}
	;
	CValObj(uint32_t a_uiDataType, var_t a_objVal) :
			m_uiDataType
			{ a_uiDataType }, m_objVal
			{ a_objVal }
	{
		;
	}
	;
	CValObj(const CValObj &a_obj) :
			m_uiDataType
			{ a_obj.m_uiDataType }, m_objVal
			{ a_obj.m_objVal }
	{
	}
	;

	uint8_t compareDataType(const CValObj &a_obj)
	{
		if (a_obj.m_uiDataType == m_uiDataType)
		{
			// Datatypes are equal
			return SAMEVALUE_OR_DTATYPE;
		}
		// Datatypes are different
		return DATATYPE_DIFFERENT;
	}
	;

	uint8_t compareValue(const CValObj &a_obj)
	{
		if (SAMEVALUE_OR_DTATYPE == compareDataType(a_obj))
		{
			// Datatypes are equal
			if (a_obj.m_objVal == m_objVal)
			{
				// Values are same
				return SAMEVALUE_OR_DTATYPE;
			}
			// Values are different
			return VALUES_DIFFERENT;
		}
		// Datatypes are different
		return DATATYPE_DIFFERENT;
	}
	;

	uint8_t assignValue(const CValObj &a_obj)
	{
		uint8_t uiRetVal = compareValue(a_obj);
		if (SAMEVALUE_OR_DTATYPE == uiRetVal)
		{
			return NO_CHANGE_IN_VALUE;
		}
		else if (VALUES_DIFFERENT == uiRetVal)
		{
			// assign the value
			m_objVal = a_obj.m_objVal;
			return VALUE_ASSINED;
		}
		// Datatypes are different
		return DATATYPE_DIFFERENT;
	}
	;

	void assignNewDataTypeValue(uint32_t a_uiDataType, const CValObj &a_obj)
	{
		m_uiDataType = a_uiDataType;
		m_objVal = a_obj.m_objVal;
	}
	;

	uint32_t getDataType()
	{
		return m_uiDataType;
	}
	;
	var_t& getValue()
	{
		return m_objVal;
	}
	;

	bool assignToSparkPlug(org_eclipse_tahu_protobuf_Payload_Metric &a_metric);

	void print()
	{
		std::cout << "\t DataType: " << m_uiDataType << "\t Value: ";
		std::visit([](auto &&arg)
		{	std::cout << typeid(arg).name() << ":" << arg << std::endl;},
				m_objVal);
	}
	;
};

class CMetric
{
	std::string m_sName;
	std::string m_sSparkPlugName;
	CValObj m_objVal;

	friend class CSparkPlugDevManager;
	CMetric()
	{
	}
	;
	void setName(std::string a_sName)
	{
		m_sName = a_sName;
	}
	;
	bool setValObj(std::string a_sDatatype, cJSON *a_cjValue)
	{
		return m_objVal.setValObj(a_sDatatype, a_cjValue);
	}
	;

public:
	CMetric(std::string a_sName) :
			m_sName
			{ a_sName }, m_sSparkPlugName
			{ a_sName }, m_objVal
			{ }
	{
		;
	}
	;
	CMetric(std::string a_sName, const CValObj &a_objVal) :
			m_sName
			{ a_sName }, m_sSparkPlugName
			{ a_sName }, m_objVal
			{ a_objVal }
	{
		;
	}
	;

	std::string getName()
	{
		return m_sName;
	}
	;
	std::string getSparkPlugName()
	{
		return m_sSparkPlugName;
	}
	;
	CValObj& getValue()
	{
		return m_objVal;
	}
	;

	void setSparkPlugName(std::string a_sVal)
	{
		m_sSparkPlugName = a_sVal;
	}
	;
	void setValue(const CValObj &a_objVal)
	{
		m_objVal = a_objVal;
	}
	;

	void print()
	{
		std::cout << "Metric Name: " << m_sName;
		m_objVal.print();
	}
	;
};

#endif
