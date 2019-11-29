/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#ifndef INCLUDE_ZMQTOMQTT_HPP_
#define INCLUDE_ZMQTOMQTT_HPP_

#include <string>
/*#include <iostream>
#include <map>
#include <vector>
#include <iterator>
#include <fstream>
#include "mqtt/async_client.h"
#include "eis/utils/json_config.h"*/
#include "eis/msgbus/msgbus.h"
/*#include "eis/msgbus/protocol.h"
#include "eis/msgbus/zmq.h"
#include "MQTTCallback.hpp"
#include "TopicMapper.h"

using namespace std;*/

struct stZmqContext
{
	void *m_pContext;
};
struct stZmqSubContext
{
	recv_ctx_t *m_pContext;
};

/*
 * CZmqToMqtt to receive topic-msg from ZMQ and send to MQTT
 */
class CZmqToMqtt {
public:
	CZmqToMqtt();
	virtual ~CZmqToMqtt();

	bool destroy();

	bool prepareCommonContext(std::string topicType);

	/// function to get message bus context based on topic
	stZmqContext& getCTX(std::string str_Topic);

	/// function to insert new entry in map
	void insertCTX(std::string, stZmqContext );

	/// function to remove entry from the map once reply is sent
	void removeCTX(std::string);

	/// function to get message bus publish context based on topic
	stZmqSubContext getSubCTX(std::string str_Topic);

	/// function to insert new entry in map
	bool insertSubCTX(std::string, stZmqSubContext );

	/// function to remove entry from the map
	void removeSubCTX(std::string);
};

#endif
