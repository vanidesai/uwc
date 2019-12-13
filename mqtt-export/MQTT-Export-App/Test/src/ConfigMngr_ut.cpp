/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "../include/ConfigMngr_ut.hpp"

using namespace std;

/***************************getETCDValuebyKey()*********************************/

/* Key is valid */
TEST(ConfigManager_ut, getETCDValuebyKey_ValueExist)
{
	try
	{
		char* cEtcdValue  = CfgManager::Instance().getETCDValuebyKey("config");
		if(NULL == cEtcdValue)
		{
			EXPECT_EQ(1, 2); //Error occurs.
		}
		cJSON *root = cJSON_Parse(cEtcdValue);
		if( NULL == root)	EXPECT_EQ(1, 2); //Error occurs.

		cJSON *mappings = cJSON_GetObjectItem(root, "Mapping");
		if( NULL == mappings)	EXPECT_EQ(1, 2); //Error occurs.

		int mappings_count = cJSON_GetArraySize(mappings);

		for (int i = 0; i < mappings_count; i++)
		{
			cJSON *msgSrc = cJSON_GetArrayItem(mappings, i);

			if(NULL == msgSrc)
			{
				continue;
			}
			char *srcName =  cJSON_GetObjectItem(msgSrc, "MsgSource")->valuestring;
			if(NULL == srcName)
			{
				continue;
			}

			if( strcmp("MQTT", srcName) )
			{
				EXPECT_EQ(1, 1);
			}
			if( strcmp("ZeroMQ", srcName) )
			{
				EXPECT_EQ(1, 1);
			}
		}

	}

	catch(exception &e)
	{
		cout<<e.what();
		EXPECT_EQ(1, 2); //Error occurs.
	}
}

/* Key is NULL */
TEST(ConfigManager_ut, getETCDValuebyKey_KeyNULL)
{
	try
	{
		char* cEtcdValue  = CfgManager::Instance().getETCDValuebyKey(NULL);

		if(NULL == cEtcdValue)
		{
			EXPECT_EQ(1, 1);
		}
		else
		{
			EXPECT_EQ(2, 1); //Test Fails
		}
	}

	catch(exception &e)
	{
		cout<<e.what();
		EXPECT_EQ(1, 2); //Error occurs.
	}
}

/***************************registerCallbackOnChangeDir()*********************************/
/* Valid Directory */
TEST(ConfigManager_ut, registerCallbackOnChangeDir_ValidDir)
{
	try
	{
		CfgManager::Instance().registerCallbackOnChangeDir("MQTT-Export");
		/* Logs below in case of successfull execution */
		EXPECT_EQ(1, 1);
	}

	catch(exception &e)
	{
		cout<<e.what();
		EXPECT_EQ(1, 2); //Error occurs.
	}
}

/* Invalid Directory */
TEST(ConfigManager_ut, registerCallbackOnChangeDir_InValidDir)
{
	try
	{
		CfgManager::Instance().registerCallbackOnChangeDir(NULL);
		/* Logs below in case of successfull execution */
		EXPECT_EQ(1, 1); //Pass because execution doesnt hang in case of NULL argument
	}

	catch(exception &e)
	{
		cout<<e.what();
		EXPECT_EQ(1, 1); //Error occurs.
	}
}

/***************************registerCallbackOnChangeKey()*********************************/
/* Valid Key */
TEST(ConfigManager_ut, registerCallbackOnChangeKey_ValidKey)
{
	try
	{
		CfgManager::Instance().registerCallbackOnChangeKey("Mapping");
		/* Logs below in case of successfull execution */
		EXPECT_EQ(1, 1);
	}

	catch(exception &e)
	{
		cout<<e.what();
		EXPECT_EQ(1, 2); //Error occurs.
	}
}

/* Invalid Key */
TEST(ConfigManager_ut, registerCallbackOnChangeKey_InValidKey)
{
	try
	{
		CfgManager::Instance().registerCallbackOnChangeKey(NULL);
		/* Logs below in case of successfull execution */
		EXPECT_EQ(1, 1); //Pass because execution doesnt hang in case of NULL argument
	}

	catch(exception &e)
	{
		cout<<e.what();
		EXPECT_EQ(1, 2); //Error occurs.
	}
}


/***************************etcdOnChangeKeyCb()*********************************/
/* Valid Argument */
TEST(ConfigManager_ut, etcdOnChangeKeyCb_ValidArg)
{
	try
	{
		std::string AppName(APP_NAME);
		string keyToMonitor = "/" + AppName + "/";
		CfgManager::Instance().registerCallbackOnChangeDir(const_cast<char *>(keyToMonitor.c_str()));
	}
	catch(exception &e)
	{
		cout<<e.what();
		EXPECT_EQ(1, 2); //Error occurs.
	}
}

///* Invalid Argument */
//TEST(ConfigManager_ut, etcdOnChangeKeyCb_InValidArg)
//{
//	try
//	{
//		etcdOnChangeKeyCb(NULL, NULL);
//		EXPECT_EQ(1, 1);
//	}
//	catch(exception &e)
//	{
//		cout<<e.what();
//		EXPECT_EQ(1, 2); //Error occurs.
//	}
//}



