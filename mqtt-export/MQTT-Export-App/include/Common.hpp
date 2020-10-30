/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#ifndef INCLUDE_COMMON_HPP_
#define INCLUDE_COMMON_HPP_

#include <string>
#include <vector>

class CCommon
{
private:
	// Private constructor so that no objects can be created.
	CCommon();
	CCommon(const CCommon & obj){}
	CCommon& operator=(CCommon const&);

	std::vector<std::string> m_vecEnv{"ReadRequest", "WriteRequest",
		"AppName", "MQTT_URL_FOR_EXPORT", "DEV_MODE", "ReadRequest_RT", "WriteRequest_RT"};

public:
	virtual ~CCommon();

	void initializeCommonData(std::string strDevMode, std::string strAppName);

	bool addTimestampsToMsg(std::string &a_sMsg, std::string tsKey, std::string strTimestamp);
	void getCurrentTimestampsInString(std::string &a_sMsg);

	/**
	 * Get single instance of this class
	 * @param None
	 * @return this instance of CCommon class
	 */
	static CCommon& getInstance() {
		static CCommon _self;
			return _self;
	}

	/**
	 * Get current time in nano seconds
	 * @param ts :[in] timestamp to get microsecond value
	 * @return time in micro seconds
	 */
	unsigned long get_micros(struct timespec ts)
	{
		return (unsigned long)ts.tv_sec * 1000000L + ts.tv_nsec/1000;
	}
};


#endif
