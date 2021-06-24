/********************************************************************************
* Copyright (c) 2021 Intel Corporation.

* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:

* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*********************************************************************************/

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
