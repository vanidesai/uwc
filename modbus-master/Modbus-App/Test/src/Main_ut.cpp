/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#include "../include/Main_ut.hpp"

extern void signal_handler(int signo);
// extern bool isElementExistInJson(cJSON *root, std::string a_sKeyName)
//extern void ConvertIPStringToCharArray(string strIPaddr, unsigned char *ptrIpAddr);
extern void populatePollingRefData();


void Main_ut::SetUp()
{
	// Setup code
}

void Main_ut::TearDown()
{
	// TearDown code
}



TEST_F(Main_ut, read_env_variables)
{
	try
	{
	setenv("DEV_MODE", "false", 1);
	CommonUtils::readCommonEnvVariables();
	//setenv("DEV_MODE", "true", 1);
	setenv("DEV_MODE", "", 1);
	CommonUtils::readCommonEnvVariables();
	setenv("DEV_MODE", "true", 1);
	EXPECT_EQ(1,1);
	}
	catch(std::exception &e)
	{
		EXPECT_EQ("", e.what());
	}

}











