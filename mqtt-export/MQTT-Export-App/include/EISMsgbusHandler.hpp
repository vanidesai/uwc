/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#ifndef INCLUDE_EISMSGBUSHANDLER_HPP_
#define INCLUDE_EISMSGBUSHANDLER_HPP_

#include <string>
#include "eis/msgbus/msgbus.h"

struct stZmqContext
{
	void *m_pContext;
};
struct stZmqSubContext
{
	recv_ctx_t *m_pContext;
};

struct stZmqPubContext
{
	void *m_pContext;
};

/*
 * CEISMsgbusHandler to receive topic-msg from ZMQ and send to MQTT
 */
class CEISMsgbusHandler {

	CEISMsgbusHandler();
	CEISMsgbusHandler(const CEISMsgbusHandler &obj);
	CEISMsgbusHandler operator=(const CEISMsgbusHandler &obj);

public:
	static CEISMsgbusHandler& Instance() {
		static CEISMsgbusHandler _self;
			return _self;
	}

	//CEISMsgbusHandler();
	virtual ~CEISMsgbusHandler();

	void cleanup();

	bool prepareCommonContext(std::string topicType);

	/// function to get message bus context based on topic
	bool getCTX(std::string a_sTopic, stZmqContext& msgbusContext);

	/// function to insert new entry in map
	bool insertCTX(std::string, stZmqContext );

	/// function to remove entry from the map once reply is sent
	void removeCTX(std::string);

	/// function to get message bus publish context based on topic
	bool getSubCTX(std::string a_sTopic, stZmqSubContext& subContext);

	/// function to insert new entry in map
	bool insertSubCTX(std::string, stZmqSubContext );

	/// function to remove entry from the map
	void removeSubCTX(std::string, stZmqContext& zmqCtx);

	///////////////Sub topics/////////////////////////
	bool getPubCTX(std::string a_sTopic, stZmqPubContext& pubContext);

	/// function to insert new entry in map
	bool insertPubCTX(std::string, stZmqPubContext);

	/// function to remove entry from the map
	void removePubCTX(std::string, stZmqContext& zmqCtx);
};

#endif
