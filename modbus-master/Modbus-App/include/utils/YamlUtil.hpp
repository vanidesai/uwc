/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#ifndef YAMLUTIL_H_
#define YAMLUTIL_H_
#include "yaml-cpp/yaml.h"  // IWYU pragma: keep
#include "NetworkInfo.hpp"

using namespace std;

namespace CommonUtils {

YAML::Node loadYamlFile(const std::string& filename);
YAML::Node loadFromETCD(const std::string& a_sYamlFileData);
void convertYamlToList(YAML::Node&, std::vector<std::string>&);
void ConvertIPStringToCharArray(string strIPaddr, unsigned char *ptrIpAddr);

class YamlUtil {
public:
	YamlUtil();
	virtual ~YamlUtil();
};

} /* namespace CommonUtils */

#endif /* YAMLUTIL_H_ */
