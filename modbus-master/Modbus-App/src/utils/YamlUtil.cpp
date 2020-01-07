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

using namespace std;

namespace CommonUtils {

std::vector<std::string> g_sWellSiteFileList1;

YAML::Node loadYamlFile(const std::string& filename)
{
	std::string sBasePath{""};
	const char* pcConfigPath = std::getenv("CONFIG_FILE_PATH");
	if(NULL != pcConfigPath)
	{
		sBasePath = pcConfigPath;
	}
	YAML::Node baseNode = YAML::LoadFile( sBasePath + filename);
	return baseNode;
}

YAML::Node loadFromETCD(const std::string& a_sYamlFileData)
{
	YAML::Node baseNode = YAML::Load(a_sYamlFileData);
	return baseNode;
}

void convertYamlToList(YAML::Node &node, std::vector<std::string>& a_slist)
{
	try
	{
		for (auto it : node)
		{
			if(it.second.IsSequence() && it.first.as<std::string>() == "wellsitelist")
			{
				//std::cout << "key is::" << it.first.as<std::string>() << std::endl;
				const YAML::Node& list =  it.second;

				cout << "Number of sites in yaml file are ::" << list.size() << std::endl;

				for (auto element : list)
				{
					YAML::Node temp = element;
					cout << "Wellsites are ::" << temp.as<std::string>() <<endl;
					a_slist.push_back(temp.as<std::string>());
				}
			}
		}
	}
	catch (YAML::Exception &e)
	{
			std::cerr << e.what() << "\n";
	}
}

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

YamlUtil::YamlUtil() {
	// TODO Auto-generated constructor stub

}

YamlUtil::~YamlUtil() {
	// TODO Auto-generated destructor stub
}

} /* namespace CommonUtils */
