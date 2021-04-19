/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#ifndef TEST_INC_METRIC_UT_HPP_
#define TEST_INC_METRIC_UT_HPP_

#include <string.h>
#include "Metric.hpp"
#ifdef UNIT_TEST
#include <gtest/gtest.h>
#endif

class Metric_ut : public::testing::Test
{
protected:
	virtual void SetUp();
	virtual void TearDown();


	bool _assignToCJSON()
	{
		uint32_t dtype = METRIC_DATA_TYPE_STRING;
		std::string objVal= "xyz";
		std::string a_version = "1.0";
		bool a_bIsDefinition = false;
		CValObj obj;
		obj.initTestData(dtype, objVal);
		std::string a_sName = "Properties/Version";
		const uint64_t a_timestamp = 1486144502122;
		std::string a_sUDTDefName ="UDT";
		std::shared_ptr<CIfMetric> pCMetric = std::make_shared<CMetric>(a_sName, obj, a_timestamp);
		metricMapIf_t mapMetrics;
		mapMetrics.emplace(a_sName, pCMetric);
		CUDT oUDT(a_sName, dtype, false, a_version);
		oUDT.initTestData(mapMetrics, mapMetrics, a_sUDTDefName, a_version, a_bIsDefinition);
		cJSON *cjRoot = NULL;
		try
			{
			   cjRoot = cJSON_CreateObject();
				if (NULL == cjRoot)
				{
					DO_LOG_ERROR(
								"Message received from MQTT could not be parsed in json format");
				}

			 }
		 catch (const std::exception &e)
			{
					DO_LOG_ERROR(std::string("Error:") + e.what());
			}

		bool result =  oUDT.assignToCJSON(cjRoot);
		EXPECT_EQ(true, result);
	}


	bool _setValObjTestDataType(org_eclipse_tahu_protobuf_Payload_Metric& a_metric)
	{
		CValObj obj;
		uint32_t dtype = METRIC_DATA_TYPE_STRING;
		std::string objVal= "xyz";
		obj.initTestData(dtype, objVal);
		a_metric.datatype = dtype;
		obj.callsetValObj(a_metric);

		// Bool
		dtype = METRIC_DATA_TYPE_BOOLEAN;
		bool testBool = true;
		a_metric.datatype = dtype;
		obj.initTestData(dtype, testBool);
		obj.callsetValObj(a_metric);

		//UINT8
         dtype = METRIC_DATA_TYPE_UINT8;
         uint8_t testuint8_t = 1;
         a_metric.datatype = dtype;
         obj.initTestData(dtype, testuint8_t);
         obj.callsetValObj(a_metric);

		//UNINT16
		 dtype = METRIC_DATA_TYPE_UINT16;
		 uint16_t testuint16_t = 1;
		 a_metric.datatype = dtype;
		 obj.initTestData(dtype, testuint16_t);
		 obj.callsetValObj(a_metric);

		 //INT8
		 dtype = METRIC_DATA_TYPE_INT8;
		 int8_t testint8_t = -1;
		 a_metric.datatype = dtype;
		 obj.initTestData(dtype, testint8_t);
		 obj.callsetValObj(a_metric);

		 //INT16
		 dtype = METRIC_DATA_TYPE_INT16;
		 int16_t testint16_t = -123;
		 a_metric.datatype = dtype;
		 obj.initTestData(dtype, testint16_t);
		 obj.callsetValObj(a_metric);

		 //INT32
		 dtype = METRIC_DATA_TYPE_INT32;
		 int32_t testint32_t = -123;
		 a_metric.datatype = dtype;
		 obj.initTestData(dtype, testint32_t);
		 obj.callsetValObj(a_metric);

		 //UINT32
		 dtype = METRIC_DATA_TYPE_UINT32;
		 uint32_t testuint32_t = 123;
		 a_metric.datatype = dtype;
		 obj.initTestData(dtype, testuint32_t);
		 obj.callsetValObj(a_metric);

		 //UINT64
		 dtype = METRIC_DATA_TYPE_UINT64;
		 uint64_t testuint64_t = 12345;
		 a_metric.datatype = dtype;
		 obj.initTestData(dtype, testuint64_t);
		 obj.callsetValObj(a_metric);

		 //INT64
		 dtype = METRIC_DATA_TYPE_INT64;
		 int64_t testint64_t = 12345;
		 a_metric.datatype = dtype;
		 obj.initTestData(dtype, testint64_t);
		 obj.callsetValObj(a_metric);

		 //Float
		 dtype = METRIC_DATA_TYPE_FLOAT;
		 float testfloat = 123.344;
		 a_metric.datatype = dtype;
		 obj.initTestData(dtype, testfloat);
		 obj.callsetValObj(a_metric);

		 //Double
		 dtype = METRIC_DATA_TYPE_DOUBLE;
		 double testDouble = 12345.344;
		 a_metric.datatype = dtype;
		 obj.initTestData(dtype, testDouble);
		 obj.callsetValObj(a_metric);
	}



	void _assignToSparkPlug(org_eclipse_tahu_protobuf_Payload_Template_Parameter &a_param)
	{
		uint32_t dtype = METRIC_DATA_TYPE_STRING;
		std::string objVal1= "xyz";
		std::string a_version = "1.0";
		bool a_bIsDefinition = false;
		CValObj obj;
		obj.initTestData(dtype, objVal1);
		obj.assignToSparkPlug(a_param);

		// Float
		dtype = METRIC_DATA_TYPE_BOOLEAN;
		bool testBool = true;
		obj.initTestData(dtype, testBool);
		obj.assignToSparkPlug(a_param);

		//UINT8
		dtype = METRIC_DATA_TYPE_UINT8;
		uint8_t testuint8_t = 1;
		obj.initTestData(dtype, testuint8_t);
		obj.assignToSparkPlug(a_param);

		//UNINT16
		dtype = METRIC_DATA_TYPE_UINT16;
		uint16_t testuint16_t = 1;
		obj.initTestData(dtype, testuint16_t);
		obj.assignToSparkPlug(a_param);

		//INT8
		dtype = METRIC_DATA_TYPE_INT8;
		int8_t testint8_t = -1;
		obj.initTestData(dtype, testint8_t);
		obj.assignToSparkPlug(a_param);

		//INT16
		dtype = METRIC_DATA_TYPE_INT16;
		int16_t testint16_t = -123;
		obj.initTestData(dtype, testint16_t);
		obj.assignToSparkPlug(a_param);

		//INT32
		dtype = METRIC_DATA_TYPE_INT32;
		int32_t testint32_t = -123;
		obj.initTestData(dtype, testint32_t);
		obj.assignToSparkPlug(a_param);

		//UINT32
		dtype = METRIC_DATA_TYPE_UINT32;
		uint32_t testuint32_t = 123;
		obj.initTestData(dtype, testuint32_t);
		obj.assignToSparkPlug(a_param);

		//UINT64
		dtype = METRIC_DATA_TYPE_UINT64;
		uint64_t testuint64_t = 12345;
		obj.initTestData(dtype, testuint64_t);
		obj.assignToSparkPlug(a_param);

		//INT64
		dtype = METRIC_DATA_TYPE_INT64;
		int64_t testint64_t = 12345;
		obj.initTestData(dtype, testint64_t);
		obj.assignToSparkPlug(a_param);

		//Float
		dtype = METRIC_DATA_TYPE_FLOAT;
		float testfloat = 123.344;
		obj.initTestData(dtype, testfloat);
		obj.assignToSparkPlug(a_param);

		//Double
		dtype = METRIC_DATA_TYPE_DOUBLE;
		double testDouble = 12345.344;
		obj.initTestData(dtype, testDouble);
		obj.assignToSparkPlug(a_param);

	}

	void _readUDTRefData(cJSON *cjUDT)
	{
		uint32_t dtype = METRIC_DATA_TYPE_STRING;
		std::string objVal= "xyz";
		std::string a_version = "1.0";
		bool a_bIsDefinition = false;
		CValObj obj;
		obj.initTestData(dtype, objVal);
		std::string a_sName = "Properties/Version";
		const uint64_t a_timestamp = 1486144502122;
		std::string a_sUDTDefName ="UDT";
		std::shared_ptr<CIfMetric> pCMetric = std::make_shared<CMetric>(a_sName, obj, a_timestamp);
		metricMapIf_t mapMetrics;
		mapMetrics.emplace(a_sName, pCMetric);
		CUDT oUDT(a_sName, dtype, false, a_version);
		oUDT.initTestData(mapMetrics, mapMetrics, a_sUDTDefName, a_version, a_bIsDefinition);
		oUDT.readUDTRefData(cjUDT);
	}


public:

	bool B_Res = false;
	CValObj CValObj_ins;
	CValObj CValObj_main;
	org_eclipse_tahu_protobuf_Payload_Metric a_metric = { NULL, false, 0, true, get_current_timestamp(), true,
			METRIC_DATA_TYPE_UNKNOWN, false, 0, false, 0, false,
			true, false,
org_eclipse_tahu_protobuf_Payload_MetaData_init_default,
			false,
					org_eclipse_tahu_protobuf_Payload_PropertySet_init_default,
			0,
			{ 0 } };
	std::string SpargPlugName = "A";
	CMetric CMetric_obj{SpargPlugName};


};


#endif /* TEST_INC_METRIC_UT_HPP_ */
