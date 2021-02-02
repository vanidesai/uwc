/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../Inc/Metric_ut.hpp"

void Metric_ut::SetUp()
{
	// Setup code
}

void Metric_ut::TearDown()
{
	// TearDown code
}

/**
 * Test case to check assignToSparkPlug() behaviour when m_uiDataType is METRIC_DATA_TYPE_BOOLEAN,
 * and invalide m_objVal
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Metric_ut, assignToSparkPlug_METRIC_DATA_TYPE_BOOLEAN)
{
	CValObj_main.assignNewDataTypeValue(METRIC_DATA_TYPE_BOOLEAN, CValObj_ins);

	B_Res = CValObj_main.assignToSparkPlug(a_metric);
	EXPECT_EQ( false, B_Res );
}

/**
 * Test case to check assignToSparkPlug() behaviour when m_uiDataType is METRIC_DATA_TYPE_UINT16,
 * and invalide m_objVal
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Metric_ut, assignToSparkPlug_METRIC_DATA_TYPE_UINT16)
{
	CValObj_main.assignNewDataTypeValue(METRIC_DATA_TYPE_UINT16, CValObj_ins);
	EXPECT_EQ( false, CValObj_main.assignToSparkPlug(a_metric) );
}

/**
 * Test case to check assignToSparkPlug() behaviour when m_uiDataType is METRIC_DATA_TYPE_UINT8,
 * and invalide m_objVal
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Metric_ut, assignToSparkPlug_METRIC_DATA_TYPE_UINT8)
{
	CValObj_main.assignNewDataTypeValue(METRIC_DATA_TYPE_UINT8, CValObj_ins);
	EXPECT_EQ( false, CValObj_main.assignToSparkPlug(a_metric) );
}

/**
 * Test case to check assignToSparkPlug() behaviour when m_uiDataType is METRIC_DATA_TYPE_INT8,
 * and invalide m_objVal
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Metric_ut, assignToSparkPlug_METRIC_DATA_TYPE_INT8)
{
	CValObj_main.assignNewDataTypeValue(METRIC_DATA_TYPE_INT8, CValObj_ins);
	EXPECT_EQ( false, CValObj_main.assignToSparkPlug(a_metric) );
}

/**
 * Test case to check assignToSparkPlug() behaviour when m_uiDataType is METRIC_DATA_TYPE_INT16,
 * and invalide m_objVal
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Metric_ut, assignToSparkPlug_METRIC_DATA_TYPE_INT16)
{
	CValObj_main.assignNewDataTypeValue(METRIC_DATA_TYPE_INT16, CValObj_ins);
	EXPECT_EQ( false, CValObj_main.assignToSparkPlug(a_metric) );
}

/**
 * Test case to check assignToSparkPlug() behaviour when m_uiDataType is METRIC_DATA_TYPE_INT32,
 * and invalide m_objVal
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Metric_ut, assignToSparkPlug_METRIC_DATA_TYPE_INT32)
{
	CValObj_main.assignNewDataTypeValue(METRIC_DATA_TYPE_INT32, CValObj_ins);
	EXPECT_EQ( false, CValObj_main.assignToSparkPlug(a_metric) );
}

/**
 * Test case to check assignToSparkPlug() behaviour when m_uiDataType is METRIC_DATA_TYPE_UINT32,
 * and invalide m_objVal
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Metric_ut, assignToSparkPlug_METRIC_DATA_TYPE_UINT32)
{
	CValObj_main.assignNewDataTypeValue(METRIC_DATA_TYPE_UINT32, CValObj_ins);
	EXPECT_EQ( false, CValObj_main.assignToSparkPlug(a_metric) );
}

/**
 * Test case to check assignToSparkPlug() behaviour when m_uiDataType is METRIC_DATA_TYPE_UINT64,
 * and invalide m_objVal
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Metric_ut, assignToSparkPlug_METRIC_DATA_TYPE_UINT64)
{
	CValObj_main.assignNewDataTypeValue(METRIC_DATA_TYPE_UINT64, CValObj_ins);
	EXPECT_EQ( false, CValObj_main.assignToSparkPlug(a_metric) );
}

/**
 * Test case to check assignToSparkPlug() behaviour when m_uiDataType is METRIC_DATA_TYPE_INT64,
 * and invalide m_objVal
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Metric_ut, assignToSparkPlug_METRIC_DATA_TYPE_INT64)
{
	CValObj_main.assignNewDataTypeValue(METRIC_DATA_TYPE_INT64, CValObj_ins);
	EXPECT_EQ( false, CValObj_main.assignToSparkPlug(a_metric) );
}

/**
 * Test case to check assignToSparkPlug() behaviour when m_uiDataType is METRIC_DATA_TYPE_BOOLEAN,
 * and invalide m_objVal
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Metric_ut, assignToSparkPlug_METRIC_DATA_TYPE_FLOAT)
{
	CValObj_main.assignNewDataTypeValue(METRIC_DATA_TYPE_FLOAT, CValObj_ins);
	EXPECT_EQ( false, CValObj_main.assignToSparkPlug(a_metric) );
}

/**
 * Test case to check assignToSparkPlug() behaviour when m_uiDataType is METRIC_DATA_TYPE_BOOLEAN,
 * and invalide m_objVal
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Metric_ut, assignToSparkPlug_METRIC_DATA_TYPE_DOUBLE)
{
	CValObj_main.assignNewDataTypeValue(METRIC_DATA_TYPE_DOUBLE, CValObj_ins);
	EXPECT_EQ( false, CValObj_main.assignToSparkPlug(a_metric) );
}

/**
 * Test case to check assignToSparkPlug() behaviour when m_uiDataType is METRIC_DATA_TYPE_STRING,
 * and invalide m_objVal
 * @param :[in] None
 * @param :[out] None
 * @return None
 */
TEST_F(Metric_ut, assignToSparkPlug_METRIC_DATA_TYPE_STRING)
{
	CValObj_main.assignNewDataTypeValue(METRIC_DATA_TYPE_STRING, CValObj_ins);
	EXPECT_EQ( false, CValObj_main.assignToSparkPlug(a_metric) );
}

/**
 * Test case to check assignToCJSON() behaviour when m_uiDataType is METRIC_DATA_TYPE_BOOLEAN,
 * and valide m_objVal
 * @return :bool
 */
TEST_F(Metric_ut, assignTo_json_METRIC_DATA_TYPE_BOOLEAN)
{
	cJSON *root =  cJSON_CreateObject();
	var_t a_objVal{true};
	CValObj CValObj_obj{METRIC_DATA_TYPE_BOOLEAN, a_objVal};
	bool result = CValObj_obj.assignToCJSON(root);
    EXPECT_EQ(true, result);

}

/**
 * Test case to check assignToCJSON() behaviour when m_uiDataType is METRIC_DATA_TYPE_UINT8,
 * and valide m_objVal
 * @return :bool
 */
TEST_F(Metric_ut, assignTo_json_METRIC_DATA_TYPE_UINT8)
{
	cJSON *root =  cJSON_CreateObject();
	var_t a_objVal{1};
	CValObj CValObj_obj{METRIC_DATA_TYPE_UINT8, a_objVal};
	bool result = CValObj_obj.assignToCJSON(root);
    EXPECT_EQ(false, result);

}

/**
 * Test case to check assignToCJSON() behaviour when m_uiDataType is METRIC_DATA_TYPE_UINT16,
 * and valide m_objVal
 * @return :bool
 */
TEST_F(Metric_ut, assignTo_json_METRIC_DATA_TYPE_UINT16)
{
	cJSON *root =  cJSON_CreateObject();
	var_t a_objVal{123};
	CValObj CValObj_obj{METRIC_DATA_TYPE_UINT16, a_objVal};
	bool result = CValObj_obj.assignToCJSON(root);
    EXPECT_EQ(false, result);

}

/**
 * Test case to check assignToCJSON() behaviour when m_uiDataType is METRIC_DATA_TYPE_INT8,
 * and valide m_objVal
 * @return :bool
 */
TEST_F(Metric_ut, assignTo_json_METRIC_DATA_TYPE_INT8)
{
	cJSON *root =  cJSON_CreateObject();
	var_t a_objVal{8};
	CValObj CValObj_obj{METRIC_DATA_TYPE_INT8, a_objVal};
	bool result = CValObj_obj.assignToCJSON(root);
    EXPECT_EQ(false, result);

}

/**
 * Test case to check assignToCJSON() behaviour when m_uiDataType is METRIC_DATA_TYPE_INT16,
 * and valide m_objVal
 * @return :bool
 */
TEST_F(Metric_ut, assignTo_json_METRIC_DATA_TYPE_INT16)
{
	cJSON *root =  cJSON_CreateObject();
	var_t a_objVal{166};
	CValObj CValObj_obj{METRIC_DATA_TYPE_INT16, a_objVal};
	bool result = CValObj_obj.assignToCJSON(root);
    EXPECT_EQ(false, result);

}

/**
 * Test case to check assignToCJSON() behaviour when m_uiDataType is METRIC_DATA_TYPE_INT32,
 * and valide m_objVal
 * @return :bool
 */
TEST_F(Metric_ut, assignTo_json_METRIC_DATA_TYPE_INT32)
{
	cJSON *root =  cJSON_CreateObject();
	var_t a_objVal{3233};
	CValObj CValObj_obj{METRIC_DATA_TYPE_INT32, a_objVal};
	bool result = CValObj_obj.assignToCJSON(root);
    EXPECT_EQ(true, result);

}

/**
 * Test case to check assignToCJSON() behaviour when m_uiDataType is METRIC_DATA_TYPE_UINT32,
 * and invalide m_objVal
 * @return :bool
 */
TEST_F(Metric_ut, assignTo_json_METRIC_DATA_TYPE_UINT32)
{
	cJSON *root =  cJSON_CreateObject();
	var_t a_objVal{3233};
	CValObj CValObj_obj{METRIC_DATA_TYPE_UINT32, a_objVal};
	bool result = CValObj_obj.assignToCJSON(root);
    EXPECT_EQ(false, result);

}

/**
 * Test case to check assignToCJSON() behaviour when m_uiDataType is METRIC_DATA_TYPE_UINT64,
 * and valide m_objVal
 * @return :bool
 */
TEST_F(Metric_ut, assignTo_json_METRIC_DATA_TYPE_UINT64)
{
	cJSON *root =  cJSON_CreateObject();
	var_t a_objVal{344};
	CValObj CValObj_obj{METRIC_DATA_TYPE_UINT64, a_objVal};
	bool result = CValObj_obj.assignToCJSON(root);
    EXPECT_EQ(false, result);

}

/**
 * Test case to check assignToCJSON() behaviour when m_uiDataType is METRIC_DATA_TYPE_INT64,
 * and valide m_objVal
 * @return :bool
 */
TEST_F(Metric_ut, assignTo_json_METRIC_DATA_TYPE_INT64)
{
	cJSON *root =  cJSON_CreateObject();
	var_t a_objVal{54443};
	CValObj CValObj_obj{METRIC_DATA_TYPE_INT64, a_objVal};
	bool result = CValObj_obj.assignToCJSON(root);
    EXPECT_EQ(false, result);
}

/**
 * Test case to check assignToCJSON() behaviour when m_uiDataType is METRIC_DATA_TYPE_FLOAT,
 * and valide m_objVal
 * @return :bool
 */
TEST_F(Metric_ut, assignTo_json_METRIC_DATA_TYPE_FLOAT)
{
	cJSON *root =  cJSON_CreateObject();
	var_t a_objVal{5.4443};
	CValObj CValObj_obj{METRIC_DATA_TYPE_FLOAT, a_objVal};
	bool result = CValObj_obj.assignToCJSON(root);
    EXPECT_EQ(false, result);
}

/**
 * Test case to check assignToCJSON() behaviour when m_uiDataType is METRIC_DATA_TYPE_DOUBLE,
 * and valide m_objVal
 * @return :bool
 */
TEST_F(Metric_ut, assignTo_json_METRIC_DATA_TYPE_DOUBLE)
{
	cJSON *root =  cJSON_CreateObject();
	var_t a_objVal{54443};
	CValObj CValObj_obj{METRIC_DATA_TYPE_DOUBLE, a_objVal};
	bool result = CValObj_obj.assignToCJSON(root);
    EXPECT_EQ(false, result);
}

/**
 * Test case to check assignToCJSON() behaviour when m_uiDataType is METRIC_DATA_TYPE_STRING,
 * and valide m_objVal
 * @return :bool
 */
TEST_F(Metric_ut, assignTo_json_METRIC_DATA_TYPE_STRING)
{
	cJSON *root =  cJSON_CreateObject();
	var_t a_objVal{"54443"};
	CValObj CValObj_obj{METRIC_DATA_TYPE_STRING, a_objVal};
	bool result = CValObj_obj.assignToCJSON(root);
    EXPECT_EQ(false, result);
}

/**
 * Test case to check addMetricNameValue() behaviour
 * @return :bool
 */
TEST_F(Metric_ut, Add_MEtricNameValue)
{

	bool result = CMetric_obj.addMetricNameValue(a_metric);
	EXPECT_EQ(true, result);
}

/**
 * Test case to check addMetricForBirth() behavior
 * @return :bool
 */
TEST_F(Metric_ut, Add_MetricFor_Birth)
{
	bool result = CMetric_obj.addMetricForBirth(a_metric);
	EXPECT_EQ(true, result);
}



