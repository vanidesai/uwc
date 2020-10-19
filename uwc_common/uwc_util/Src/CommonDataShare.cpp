/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/
#include "CommonDataShare.hpp"

/** Constructor
 */
CcommonEnvManager::CcommonEnvManager()
{
	if(false == CommonDataFromApp.m_isCommonDataInitialised)
	{
		DO_LOG_ERROR("Critical data has not been initialized");
	}
}

/** Returns the single instance of this class
 *
 * @param  : nothing
 * @return : object of this class
 */
CcommonEnvManager& CcommonEnvManager::Instance()
{
	static CcommonEnvManager _self;
	return _self;;
}

/**
 * split string with given delimeter
 * @param str :[in] string to split
 * @param delim :[in] delimeter by which string needs to spit
 * @return
 */
void CcommonEnvManager::splitString(const std::string &str, char delim)
{
	std::stringstream ss(str);
	std::string item;
	while (std::getline(ss, item, delim))
	{
		std::size_t pos = item.find('/');
		item = (item.substr(pos + 1));
		std::cout <<" Topic:: " << std::string(item) << std::endl;
		addTopicToList(std::string(item));
	}
}
