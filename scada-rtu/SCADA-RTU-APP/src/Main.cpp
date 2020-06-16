/*************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
*************************************************************************************/

#include "Common.hpp"
#include "ConfigManager.hpp"
#include <iterator>
#include <vector>
#include <future>

#include "MQTTHandler.hpp"
#include "SCADAHandler.hpp"
#include "Publisher.hpp"

#ifdef UNIT_TEST
#include <gtest/gtest.h>
#endif

struct stFuture
{
	std::future<bool> m_fut;
	std::future_status m_bStatus;
};
vector<std::thread> g_vThreads;

std::atomic<bool> g_shouldStop(false);

#define APP_VERSION "0.0.0.1"

bool processMsgToSendOnMqtt(QMgr::stMqttMsg a_mqttMsgToPublish)
{
	string message = "";
	cJSON *root = NULL;

	try
	{
		//decode spark plug msg
		// Decode the payload
		org_eclipse_tahu_protobuf_Payload inbound_payload = org_eclipse_tahu_protobuf_Payload_init_zero;
		int msgLen = a_mqttMsgToPublish.m_mqttMsg->get_payload().length();

		if(decode_payload(&inbound_payload, a_mqttMsgToPublish.m_mqttMsg->get_payload().c_str(), msgLen))
		{
			root  = cJSON_CreateObject();

			for (pb_size_t i=0; i<inbound_payload.metrics_count; i++)
			{
				DO_LOG_DEBUG("Metric name : " + (std::string)inbound_payload.metrics[i].name);
				DO_LOG_DEBUG("Metric value : " + (std::string)inbound_payload.metrics[i].value.string_value);

	            cJSON_AddItemToObject(root, inbound_payload.metrics[i].name, cJSON_CreateString(inbound_payload.metrics[i].value.string_value));
			}
		}
		else
		{
			DO_LOG_ERROR("Failed to decode the SCADA payload");
		}

		a_mqttMsgToPublish.m_mqttTopic = CCommon::getInstance().getMqttPublishTopic();

	    message = cJSON_Print(root);

	    if(root != NULL)
	    {
	    	cJSON_Delete(root);
	    }

		CPublisher::instance().publishMqttExportMsg(message, a_mqttMsgToPublish.m_mqttTopic);
		return true;
	}
	catch(exception &ex)
	{
		DO_LOG_FATAL(ex.what());
	}

	return true;
}

/**
 * Process message received from MQTT and send it on EIS
 * @param a_mqttMsgToPublish :[in] structure containing information about request to send on EIS
 * @param a_context :[in] EIS msgbus context on which to send request
 * @param a_pubContext :[in] EIS pub context on which to send request
 * @return none
 */
bool processMsgToSendOnSCADA(stDataPointRepo a_dataPointValue)
{
	try
	{
		DO_LOG_DEBUG("Request received from data point " + a_dataPointValue.m_dataPoint);

		// Create the DDATA payload
		org_eclipse_tahu_protobuf_Payload ddata_payload;
		get_next_payload(&ddata_payload);

		cout << a_dataPointValue.m_dataPoint << endl;

		//fill up spark plug payload
		//get the topic on which to send
		//publish message on SCADA

		free_payload(&ddata_payload);

		return true;
	}
	catch(exception &ex)
	{
		DO_LOG_FATAL(ex.what());
	}

	return true;
}

/**
 * Post messages for MQTT-Export
 * @param a_qMgr :[in] reference of queue manager having SCADA messages
 * @return none
 */
void postMsgToMqtt(QMgr::CQueueMgr& a_qMgr)
{
	string eisTopic = "";

	try
	{
		QMgr::stMqttMsg recvdMsg;

		while (false == g_shouldStop.load())
		{
			if(true == a_qMgr.isMsgArrived(recvdMsg))
			{
				DO_LOG_DEBUG("Retrieved msg from queue");

				processMsgToSendOnMqtt(recvdMsg);
			}
		}
	}
	catch (const std::exception &e)
	{
		DO_LOG_FATAL(e.what());
	}
}

/**
 * Update data points repository with previous and current values
 * @param a_qMgr 	:[in] reference of queue manager having MQTT-Export messages
 * @return None
 */
void updateDataPoints(QMgr::CQueueMgr& a_qMgr)
{
	string eisTopic = "";

	try
	{
		QMgr::stMqttMsg recvdMsg;

		while (false == g_shouldStop.load())
		{
			if(true == a_qMgr.isMsgArrived(recvdMsg))
			{
				DO_LOG_DEBUG("Retrieved msg from queue");

				//should we check complete message or we can skip timer values
				//and compare other values
				//we can keep only latest value with a flag
				//that will tell if the message has been swapped
				//with latest value or false if both the values are the same
			}
		}
	}
	catch (const std::exception &e)
	{
		DO_LOG_FATAL(e.what());
	}
}

/**
 * Post messages for SCADA, publish only changed values from data_points
 * @param a_qMgr 	:[in] reference of queue manager having MQTT-Export messages
 * @return None
 */
bool postMsgsToSCADA(QMgr::CQueueMgr& a_qMgr)
{
	bool bRetVal = true;

	try
	{
		std::map<string, std::map<string, stDataPointRepo>> v_modifiedDataPoints;

		for(auto it = v_modifiedDataPoints.begin(); it != v_modifiedDataPoints.end(); ++it)
		{
			//fill up message with data point information which has changed
		}

	}
	catch (const std::exception &e)
	{
		DO_LOG_FATAL(e.what());
		bRetVal = false;
	}

	return bRetVal;
}

bool isTaskComplete(stFuture &a_futures)
{
		//check the status of task
		//The function returned because the shared state was ready.
		if(a_futures.m_bStatus == future_status::ready)
		{
			cout << "** status is ready" << endl;
			if(a_futures.m_fut.valid())
			{
				bool bres = a_futures.m_fut.get();
				if(bres == true)
				{
					cout << "Task returned value true" << endl;
				}
				else
				{
					cout << "Task returned value false" << endl;
				}
			}
			return true;//update new task here
		}
		else
		{
			cout << "** status is timeout" << endl;
			return false;
		}
}

/**
 * Function to track timer for reporting data to SCADA system
 * @param : [in] interval in milliseconds: Minimum timer tick value
 * @return : none
 */
void timerThread(uint32_t interval)
{
	uint32_t uiCurCounter = 0;

	stFuture stTask;
	stTask.m_bStatus = future_status::deferred;

	struct timespec ts;
	int rc = clock_getres(CLOCK_MONOTONIC, &ts);
	if(0 != rc)
	{
		std::cout << "Error: clock_getres failed: " << errno << std::endl;
		std::cout << "Continuing further\n";
	}
	else
	{
		std::cout << "Clock resolution: " << (long)ts.tv_sec << " seconds, " << (long)ts.tv_nsec << " nanoseconds \n";
	}

	uint32_t uiMsecInterval = interval;

	// for following calculation, convert interval to nanoseconds
	interval = interval*1000*1000;
	rc = clock_gettime(CLOCK_MONOTONIC, &ts);
	if(0 != rc)
	{
		std::cout << "Fatal error: polling timer: clock_gettime failed: " << errno << std::endl;
		return;
	}
	while(!g_shouldStop)
	{
		unsigned long next_tick = (ts.tv_sec * 1000000000L + ts.tv_nsec) + interval;
		ts.tv_sec = next_tick / 1000000000L;
		ts.tv_nsec = next_tick % 1000000000L;

		do
		{
			rc = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL);
		} while(EINTR == rc);

		if(0 == rc)
		{
			struct timespec tsPoll = {0};
			rc = clock_gettime(CLOCK_REALTIME, &tsPoll);
			if(0 != rc)
			{
				std::cout << "Fatal error: polling timer: clock_gettime failed in polling: " << errno << std::endl;
			}

			//Check modified values
			bool bRes = isTaskComplete(stTask);

			//check to replace
			if(bRes == false)
			{
				cout << "Previous reporting task timed out" << endl;
			}
			else
			{
				cout << "Previous reporting task has completed" << endl;
			}

			stTask.m_fut = std::move(std::async(std::launch::async, postMsgsToSCADA, std::ref(QMgr::getMqtt())));
			stTask.m_bStatus = stTask.m_fut.wait_for(std::chrono::seconds(10));

			//continue with timer
			uiCurCounter = uiCurCounter + uiMsecInterval;
			// reset the counter
			if(uiCurCounter == interval)
			{
				uiCurCounter = 0;
			}
		}
		else
		{
			DO_LOG_FATAL("Polling timer error:" + to_string(rc));
		}
	}
}

/**
 * Main function of application
 * @param argc :[in] number of input parameters
 * @param argv :[in] input parameters
 * @return 	0/-1 based on success/failure
 */
int main(int argc, char *argv[])
{
	DO_LOG_DEBUG("Starting SCADA RTU ...");
	std::cout << __func__ << ":" << __LINE__ << " ------------- Starting SCADA RTU Container -------------" << std::endl;

	try
	{
		//initialize CCommon class to get common variables
		string AppName = CCommon::getInstance().getStrAppName();
		if(AppName.empty())
		{
			DO_LOG_ERROR("AppName Environment Variable is not set");
			std::cout << __func__ << ":" << __LINE__ << " Error : AppName Environment Variable is not set" <<  std::endl;
			return -1;
		}

		DO_LOG_INFO("SCADA RTU container app version is set to :: "+  std::string(APP_VERSION));
		cout << "SCADA RTU container app version is set to :: "+  std::string(APP_VERSION) << endl;

		cout << "MQTT URL : " << CCommon::getInstance().getStrMqttURL() << endl;

		CPublisher::instance();

		if( false == CPublisher::instance().isPublisherConnected())
		{
			std::cout << "Publisher failed to connect with MQTt broker" << endl;
			exit(-1);
		}
		CSCADAHandler::instance();
		CMQTTHandler::instance();

#ifdef UNIT_TEST
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
#endif

		//send messages for SCADA
		g_vThreads.push_back(std::thread(updateDataPoints, std::ref(QMgr::getMqtt())));
		//send messages for MQTT-Export
		g_vThreads.push_back(std::thread(postMsgToMqtt, std::ref(QMgr::getScada())));

		for (auto &th : g_vThreads)
		{
			if (th.joinable())
			{
				th.join();
			}
		}

	}
	catch (std::exception &e)
	{
		DO_LOG_FATAL(e.what());
		std::cout << __func__ << ":" << __LINE__ << " Exception : " << e.what() << std::endl;
		return -1;
	}
	catch (...)
	{
		DO_LOG_FATAL("Unknown Exception Occurred. Exiting");
		std::cout << __func__ << ":" << __LINE__ << "Exception : Unknown Exception Occurred. Exiting" << std::endl;
		return -1;
	}

	std::cout << __func__ << ":" << __LINE__ << " ------------- Exiting SCADA RTU Container -------------" << std::endl;
	return 0;
}
