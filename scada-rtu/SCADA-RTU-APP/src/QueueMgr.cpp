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

#include "Common.hpp"
#include "Logger.hpp"

using namespace QMgr;

//initialize queue manager instances
CQueueHandler ng_qDatapoints; //this queue holds all internal MQTT messages, this is majorly used for data-points
CQueueHandler ng_qScadaSub; //this queue holds CMD messages coming from SCADA system
CQueueHandler ng_qScadaPub; //this queue holds messages to be published on SCADA master
CQueueHandler ng_qMEPub;	//this queue holds messages to be published on MQTT-Export

/**
 * Get reference of
 * @param None
 * @return
 */
CQueueHandler& QMgr::getDatapointsQ()
{
	return ng_qDatapoints;
}

/**
 * Get reference of on-demand real-time write operation instance
 * @param None
 * @return reference of on-demand real-time write operation instance
 */
CQueueHandler& QMgr::getScadaSubQ()
{
	return ng_qScadaSub;
}

/**
 * Get reference of on-demand real-time write operation instance
 * @param None
 * @return reference of on-demand real-time write operation instance
 */
CQueueHandler& QMgr::getScadaPubQ()
{
	return ng_qScadaPub;
}

/**
 * Get reference of on-demand real-time write operation instance
 * @param None
 * @return reference of on-demand real-time write operation instance
 */
CQueueHandler& QMgr::getPubMqttExportQ()
{
	return ng_qMEPub;
}
