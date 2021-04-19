/************************************************************************************
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation. Title to the
 * Material remains with Intel Corporation.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery of
 * the Materials, either expressly, by implication, inducement, estoppel or otherwise.
 ************************************************************************************/

#include "QueueMgr.hpp"

using namespace QMgr;

/**
 * Get reference of a queue for storing messages from internal mqtt
 * @param None
 * @return reference of CQueueHandler
 */
CQueueHandler& QMgr::getDatapointsQ()
{
	// This queue holds all internal MQTT messages
	static CQueueHandler ng_qDatapoints;
	return ng_qDatapoints;
}

/**
 * Get reference of a queue for storing messages from external mqtt
 * @param None
 * @return reference of CQueueHandler
 */
CQueueHandler& QMgr::getScadaSubQ()
{
	// This queue holds CMD messages coming from SCADA system
	static CQueueHandler ng_qScadaSub;
	return ng_qScadaSub;
}
