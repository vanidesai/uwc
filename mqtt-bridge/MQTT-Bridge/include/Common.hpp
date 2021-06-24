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
/**
 * File contains the class CCommon that manages common environment variables
 */

#ifndef INCLUDE_COMMON_HPP_
#define INCLUDE_COMMON_HPP_

#include <string>
#include <vector>

/**
 * class CCommon that manages common environment variables & timestamp for msg payload
 */
class CCommon
{
private:
	// Private constructor so that no objects can be created.
	CCommon();
	CCommon(const CCommon & obj){}
	CCommon& operator=(CCommon const&);

	std::vector<std::string> m_vecEnv{"ReadRequest", "WriteRequest",
		"AppName", "MQTT_URL_FOR_EXPORT", "DEV_MODE", "ReadRequest_RT", "WriteRequest_RT"}; /*!< a vector of strings of common environment variables */

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
