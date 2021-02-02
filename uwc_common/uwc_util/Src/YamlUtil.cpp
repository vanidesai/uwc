/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#include "YamlUtil.hpp"
#include<iostream>
#include "Logger.hpp"

namespace CommonUtils {

/** This function is used to load YAML file from local files
 *
 * @param filename : This variable is use to pass actual filename to load
 * @return : YAML node
 */
YAML::Node loadYamlFile(const std::string& filename)
{
	std::string sBasePath{BASE_PATH_YAML_FILE};
	std::string sfileToRead = sBasePath + filename;

	std::cout << "YAML file to be read is :: " + sfileToRead << std::endl;
	DO_LOG_DEBUG("YAML file to be read is :: " + sfileToRead);

	YAML::Node baseNode = YAML::LoadFile(sfileToRead);

	return baseNode;
}

/** This function is used to store YAML data from a node to a string list
 *
 * @param node : [in] YML node
 * @param a_slist : [out] string list
 * @return : true/false based on success or error
 */
bool convertYamlToList(YAML::Node &node, std::vector<std::string>& a_slist)
{
	// locals
	bool bRetVal = false;
	try
	{
		for (auto it : node)
		{
			if(it.second.IsSequence() && it.first.as<std::string>() == "devicegrouplist")
			{
				const YAML::Node& list =  it.second;

				std::cout << "Number of sites in yaml file are ::" << list.size() << std::endl;
				DO_LOG_INFO("Number of sites in yaml file are ::" + list.size());

				for (auto element : list)
				{
					YAML::Node temp = element;
					DO_LOG_INFO("Wellsite is :: " + temp.as<std::string>());
					std::cout << "Wellsite is ::" << temp.as<std::string>() <<std::endl;
					a_slist.push_back(temp.as<std::string>());
				}
				bRetVal = true;
			}
		}
	}
	catch (YAML::Exception &e)
	{
		DO_LOG_ERROR("Exception is :: " + std::string(e.what()));
		std::cerr << e.what() << "\n";
	}

	return bRetVal;
}

/** This function converts IP string to char array
 *
 * @param strIPaddr : IP string to convert
 * @param ptrIpAddr : char array to store tokenize key
 * @return : Nothing
 */
void ConvertIPStringToCharArray(std::string strIPaddr, unsigned char *ptrIpAddr)
{
	std::string delimiter = ".";
	size_t pos = 0;
	int8_t i8Index = 0;

	while((pos = strIPaddr.find(delimiter))!= std::string::npos)
	{
		ptrIpAddr[i8Index] = (uint8_t)std::stoi(strIPaddr.substr(0,pos).c_str());
		strIPaddr.erase(0,pos+delimiter.length());
		++i8Index;
	}
	ptrIpAddr[i8Index] = (uint8_t)std::stoi(strIPaddr.substr(0,pos).c_str());
}

/**
 * Reads environment variable and stores its value in givenn parameter
 * @param pEnvVarName:[in] Environment variable name
 * @param storeVal:[out] String to store value of environment variable
 * @return true on successfule reading, false otherwise
 */
bool readEnvVariable(const char *pEnvVarName, std::string &storeVal)
{
	if(NULL == pEnvVarName)
	{
		return false;
	}
	bool bRetVal = false;
	char *cEvar = getenv(pEnvVarName);
	if (NULL != cEvar)
	{
		bRetVal = true;
		std::string tmp (cEvar);
		storeVal = tmp;
		DO_LOG_INFO(std::string(pEnvVarName) + " environment variable is set to ::" + storeVal);
		std::cout << std::string(pEnvVarName) + " environment variable is set to ::" + storeVal << std::endl;
	}
	else
	{
		DO_LOG_ERROR(std::string(pEnvVarName) + " environment variable is not found");
		std::cout << std::string(pEnvVarName) + " environment variable is not found" <<std::endl;
	}
	return bRetVal;
}

} /* namespace CommonUtils */
