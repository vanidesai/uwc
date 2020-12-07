/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/
#include "CommonDataShare.hpp"
#include <chrono>

/** Constructor
 */
CcommonEnvManager::CcommonEnvManager()
{
}

/** Returns the single instance of this class
 *
 * @param  : nothing
 * @return : object of this class
 */
CcommonEnvManager& CcommonEnvManager::Instance()
{
	static CcommonEnvManager _self;
	return _self;;
}

/**
 * split string with given delimeter
 * @param str :[in] string to split
 * @param delim :[in] delimeter by which string needs to spit
 * @return
 */
void CcommonEnvManager::splitString(const std::string &str, char delim)
{
	std::stringstream ss(str);
	std::string item;
	while (std::getline(ss, item, delim))
	{
		std::size_t pos = item.find('/');
		item = (item.substr(pos + 1));
		std::cout <<" Topic:: " << std::string(item) << std::endl;
		addTopicToList(std::string(item));
	}
}

// std::vector<std::string> CcommonEnvManager::getTopicList(std::string topicType) const {
// 	std::vector<std::string> topics;
// 	if(topicType == "sub") {
// 		for(size_t it =0;it<(CfgManager::Instance().getEiiCfgMgr()->getNumSubscribers());++it) {
// 			SubscriberCfg* sub_ctx = getSubscriberByIndex(it);
// 			topics = sub_ctx->getTopics();
// 		}
// 	} else if(topicType == "pub") {
// 		for(size_t it =0;it<(CfgManager::Instance().getEiiCfgMgr()->getNumPublishers());++it) {
// 			PublisherCfg* pub_ctx = getPublisherByIndex(it);
// 			topics = pub_ctx->getTopics();
// 		}
// 	}
// 	return topics;
// }


/**
 * Get time parameters
 * @param a_sTimeStamp	:[in] reference to store time stamp
 * @param a_sUsec		:[in] reference to store time in usec
 */
void CcommonEnvManager::getTimeParams(std::string &a_sTimeStamp, std::string &a_sUsec)
{
	a_sTimeStamp.clear();
	a_sUsec.clear();

	const auto p1 = std::chrono::system_clock::now();

	std::time_t rawtime = std::chrono::system_clock::to_time_t(p1);
	std::tm* timeinfo = std::gmtime(&rawtime);
	if(NULL == timeinfo)
	{
		return;
	}
	char buffer [80];

	std::strftime(buffer,80,"%Y-%m-%d %H:%M:%S",timeinfo);
	a_sTimeStamp.insert(0, buffer);

	{
		std::stringstream ss;
		ss << std::chrono::duration_cast<std::chrono::microseconds>(p1.time_since_epoch()).count();
		a_sUsec.insert(0, ss.str());
	}
}
