/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#ifndef INCLUDE_TOPICMAPPER_HPP_
#define INCLUDE_TOPICMAPPER_HPP_

#include <string>
#include <map>
#include <vector>
/*
 * TopicMapper Class to map MQTT topics with ZMQ topics
 */

class CTopicMapper {
private:
	// Private constructor so that no objects can be created.
	CTopicMapper();
	CTopicMapper(const CTopicMapper & obj){}
	CTopicMapper& operator=(CTopicMapper const&);

	void ParseJson();
	std::map<std::string, std::string> m_MQTTopics;
	std::map<std::string, std::string> m_ZMQTopics;

public:
	virtual ~CTopicMapper();

	std::string GetZMQTopic(std::string topic);
	std::string GetMQTTopic(std::string topic);
	std::vector<std::string> GetMqttTopics();

	static CTopicMapper& getInstance() {
		static CTopicMapper _self;
			return _self;
	}
};


#endif /* INCLUDE_TOPICMAPPER_HPP_ */
