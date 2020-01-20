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

#define BASE_PATH_YAML_FILE "/opt/intel/eis/uwc_data/"

namespace CommonUtils {

/** This function is used to load YAML file from local files
 *
 * @filename : This variable is use to pass actual filename to load
 * @return: YAML node
 */
YAML::Node loadYamlFile(const std::string& filename);


/** This function is used to load YAML file from ETCD
 *
 * @filename : This variable is use to pass actual filename to load
 * @return: YAML node
 */
YAML::Node loadFromETCD(const std::string& a_sYamlFileData);

/** This function is used to store YAML to list
 *
 * @filename : This variable is use to pass actual filename to load
 * @vlist : variable to store values from YAML
 * @return: true/false based on success or error
 */
bool convertYamlToList(YAML::Node& node, std::vector<std::string>& vlist);


/** This function is convert string to char array
 *
 * @srcString : string to convert
 * @ptrIpAddr : char array to store actual key
 * @return: Nothing
 */
void ConvertIPStringToCharArray(string srcString, unsigned char *ptrIpAddr);

/** This function is used to read environment variable
 *
 * @sEnvVarName : environment variable to be read
 * @storeVal : variable to store env variable value
 * @return: true/false based on success or error
 */
bool readEnvVariable(const char *pEnvVarName, string &storeVal);

/** This function is used to read common environment variables
 *
 * @return: true/false based on success or error
 */
bool readCommonEnvVariables();

} /* namespace CommonUtils */

#endif /* YAMLUTIL_H_ */
