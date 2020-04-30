/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#include "utils/YamlUtil.hpp"
#include<iostream>
#include "Logger.hpp"

using namespace std;

namespace CommonUtils {

/** This function is use to load YAML file from local files
 *
 * @filename : This variable is use to pass actual filename to load
 * @return: YAML node
 */
YAML::Node loadYamlFile(const std::string& filename)
{
	std::string sBasePath{BASE_PATH_YAML_FILE};
	string sfileToRead = sBasePath + filename;

	cout << "YAML file to be read is :: " + sfileToRead << endl;
	DO_LOG_DEBUG("YAML file to be read is :: " + sfileToRead);

	YAML::Node baseNode = YAML::LoadFile(sfileToRead);

	return baseNode;
}

/** This function is use to store YAML to list
 *
 * @filename : This variable is use to pass actual filename to load
 * @vlist : variable to store values from YAML
 * @return: true/false based on success or error
 */
bool convertYamlToList(YAML::Node &node, std::vector<std::string>& a_slist)
{
	// locals
	bool bRetVal = false;
	try
	{
		for (auto it : node)
		{
			if(it.second.IsSequence() && it.first.as<std::string>() == "wellsitelist")
			{
				const YAML::Node& list =  it.second;

				cout << "Number of sites in yaml file are ::" << list.size() << std::endl;
				DO_LOG_INFO("Number of sites in yaml file are ::" + list.size());

				for (auto element : list)
				{
					YAML::Node temp = element;
					DO_LOG_INFO("Wellsite is :: " + temp.as<std::string>());
					cout << "Wellsite is ::" << temp.as<std::string>() <<endl;
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

/** This function is convert string to char array
 *
 * @srcString : string to convert
 * @ptrIpAddr : char array to store tokenize key
 * @return: Nothing
 */
void ConvertIPStringToCharArray(string strIPaddr, unsigned char *ptrIpAddr)
{
	std::string delimiter = ".";
	size_t pos = 0;
	int8_t i8Index = 0;

	while((pos = strIPaddr.find(delimiter))!= std::string::npos)
	{
		ptrIpAddr[i8Index] = (uint8_t)stoi(strIPaddr.substr(0,pos).c_str());
		strIPaddr.erase(0,pos+delimiter.length());
		++i8Index;
	}
	ptrIpAddr[i8Index] = (uint8_t)stoi(strIPaddr.substr(0,pos).c_str());
}

} /* namespace CommonUtils */
