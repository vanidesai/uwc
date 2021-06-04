/*************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 *************************************************************************************/

/***  CommonDataShare.hpp To share module related common data to uwc-common library */

#ifndef UWC_COMMONDATASHARE_HPP_
#define UWC_COMMONDATASHARE_HPP_

#include <iostream>
#include "Logger.hpp"
//eii configmgr
#include "eii/config_manager/config_mgr.hpp"
// uwc configmgr
#include "ConfigManager.hpp"

/** Common data recieved from calling app*/
struct stUWCComnDataVal_t
{
	std::string	m_sAppName; /** App name */
	bool m_devMode; /** DEV mode(true or false)*/
	bool m_isCommonDataInitialised = false; /** tells whether common data init is done*/
};

/** Class to manage the common environment*/
class CcommonEnvManager{

private:
	stUWCComnDataVal_t CommonDataFromApp; /** object to hold common data */
	std::vector<std::string> m_vect_TopicList; /** vector of topic list */

	/** Constructor */
	CcommonEnvManager();

	/** copy constructor is private */
	CcommonEnvManager(CcommonEnvManager const&);

	/** assignment operator is private */
	CcommonEnvManager& operator=(CcommonEnvManager const&);

public:

	/** Returns the single instance of this class
	 *
	 * @param  : nothing
	 * @return : object of this class
	 */
	static CcommonEnvManager& Instance();

	/** Get app name */
	std::string getAppName(){
		return CommonDataFromApp.m_sAppName;
	}

	/** Get Dev mode */
	bool getDevMode(){
		return CommonDataFromApp.m_devMode;
	}

	/** Function to set stUWCComnDataVal_t structure.*/
	void ShareToLibUwcCmnData(stUWCComnDataVal_t &ImportFromApp_Locobj) {
		CommonDataFromApp = ImportFromApp_Locobj;
	}

	/** Function to split string with given delimeter */
	void splitString(const std::string &str, char delim);

	/**
	 * Insert topic to topiclist vector
	 * @param a_sTopic :[in] subscribe topic to add to topiclist vector
	 * @return
	 */
	void addTopicToList(const std::string a_sTopic) {
		m_vect_TopicList.push_back(a_sTopic);
	}

	/** function to read current time and usec */
	void getTimeParams(std::string &a_sTimeStamp, std::string &a_sUsec);
};


#endif
