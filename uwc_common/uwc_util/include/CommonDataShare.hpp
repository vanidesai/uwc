/*************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 *************************************************************************************/

#ifndef UWC_COMMONDATASHARE_HPP_
#define UWC_COMMONDATASHARE_HPP_

#include <iostream>
#include "Logger.hpp"

/*
 * Common data recieved from calling app
 */
struct stUWCComnDataVal_t
{
	std::string								m_sAppName;
	bool									m_devMode;
	bool 			m_isCommonDataInitialised = false;
};

class CcommonEnvManager{

private:
	stUWCComnDataVal_t CommonDataFromApp;
	std::vector<std::string> m_vect_TopicList;

	/// Constructor
	CcommonEnvManager();

	/// copy constructor is private
	CcommonEnvManager(CcommonEnvManager const&);

	/// assignment operator is private
	CcommonEnvManager& operator=(CcommonEnvManager const&);

public:

	/** Returns the single instance of this class
	 *
	 * @param  : nothing
	 * @return : object of this class
	 */
	static CcommonEnvManager& Instance();

	/// Get app name
	std::string gEtAppName(){
		return CommonDataFromApp.m_sAppName;
	}

	/// Get Dev mode
	bool gEtDevMode(){
		return CommonDataFromApp.m_devMode;
	}

	/// Function to set stUWCComnDataVal_t structure.
	void ShareToLibUwcCmnData(stUWCComnDataVal_t &ImportFromApp_Locobj) {
		CommonDataFromApp = ImportFromApp_Locobj;
	}

	/// Function to split string with given delimeter
	void splitString(const std::string &str, char delim);

	/**
	 * Get subscribe topic list
	 * @return vector containing topic list
	 */
	std::vector<std::string> getTopicList() const {
		return m_vect_TopicList;
	}

	/**
	 * Insert topic to topiclist vector
	 * @param a_sTopic :[in] subscribe topic to add to topiclist vector
	 * @return
	 */
	void addTopicToList(const std::string a_sTopic) {
		m_vect_TopicList.push_back(a_sTopic);
	}

	/// function to read current time and usec
	void getTimeParams(std::string &a_sTimeStamp, std::string &a_sUsec);
};


#endif
