/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#ifndef INC_ENVIRONMENTVARHANDLER_HPP_
#define INC_ENVIRONMENTVARHANDLER_HPP_

#include <unordered_map>
#include <vector>
#include <mutex>
#include <string>

using namespace std;

class EnvironmentInfo
{
	unordered_map<string, string> m_umapEnv;

	/// Default Constructor
	EnvironmentInfo(){}

	/// Copy Constructor
	EnvironmentInfo(const EnvironmentInfo&) = delete;

	/// Assignment Operator
	EnvironmentInfo& operator=(const EnvironmentInfo&) = delete;
public:
	/// Default Destructor
	~EnvironmentInfo(){}

	/// Function to read common environment variables
	bool readCommonEnvVariables(std::vector<string>);

	/// Function to add environment variable data to Map
	bool addDataToEnvMap(string, string);

	/// Function to get environment variable data from Map based on key
	string getDataFromEnvMap(string);

	/** Returns instance of EnvironmentInfo class
	 *
	 * @param : Nothing
	 * @return: object of EnvironmentInfo class
	 */
	static EnvironmentInfo& getInstance()
	{
		static EnvironmentInfo _self;
		return _self;
	}
};

#endif /* INC_ENVIRONMENTVARHANDLER_HPP_ */
