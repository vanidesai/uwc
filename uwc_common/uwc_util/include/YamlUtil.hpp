/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

/*** YamlUtil.hpp is  To parse yaml file and save the data*/

#ifndef YAMLUTIL_H_
#define YAMLUTIL_H_
#include "yaml-cpp/yaml.h"  // IWYU pragma: keep

#define BASE_PATH_YAML_FILE "/opt/intel/eii/uwc_data/"

/** CommonUtils is a namespace holding the function for Yaml operations like loading yaml ,parsing yaml files,
 * coverting yaml to string, and reading env variables*/
namespace CommonUtils {

YAML::Node loadYamlFile(const std::string& filename);

bool convertYamlToList(YAML::Node& node, std::vector<std::string>& vlist);

void ConvertIPStringToCharArray(std::string srcString, unsigned char *ptrIpAddr);

bool readEnvVariable(const char *pEnvVarName, std::string &storeVal);

/** This function is used to read common environment variables
 *
 * @return: true/false based on success or error
 */
bool readCommonEnvVariables();

} /* namespace CommonUtils */

#endif /* YAMLUTIL_H_ */
