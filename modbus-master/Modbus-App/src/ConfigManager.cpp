/************************************************************************************
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Intel Corporation. Title to the
* Material remains with Intel Corporation.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or delivery of
* the Materials, either expressly, by implication, inducement, estoppel or otherwise.
************************************************************************************/

#include "ConfigManager.hpp"
#include "Logger.hpp"

/** Constructor
*/
CfgManager:: CfgManager()
{
	// this client is used to read environment variables mentioned in SubTopics/PubTopics given in
	// docker-compose.yml file
	env_config_client = env_config_new();

	// based on DEV_MODE env variable this will create config_mgr_client instance
	if(PublishJsonHandler::instance().isDevMode())
	{
		/// create client without certificates
		config_mgr_client = config_mgr_new((char *)"etcd", (char *)"", (char *)"", (char *)"");
	}
	else
	{
		/// create client with certificates
		string sCert = "/run/secrets/etcd_" + PublishJsonHandler::instance().getAppName() + "_cert";
		string sKey = "/run/secrets/etcd_" + PublishJsonHandler::instance().getAppName() + "_key";
		config_mgr_client = config_mgr_new((char *)"etcd", (char *)sCert.c_str(),
				(char *)sKey.c_str(),
				(char *)"/run/secrets/ca_etcd");
	}

	isClientCreated = false;

}

/** Returns the single instance of this class
 *
 * @param  : nothing
 * @return : Object of this class
 */
CfgManager& CfgManager::Instance()
{
	static CfgManager _self;
	return _self;
}

/** Returns the client status of creation
 *
 * @param : Nothing
 * @return: true/false based on status
 */
bool CfgManager::IsClientCreated()
{
	//config_mgr_t* cfgMangr = env_config.get_config_mgr_client();
	bool isClientCreated = (getConfigClient() !=NULL && getEnvClient() !=NULL)?true:false;
	return isClientCreated;
}

/** Returns the single instance of this class
 *
 * @param  : nothing
 * @return : Object of this class
 */
globalConfig::CGlobalConfig& globalConfig::CGlobalConfig::getInstance()
{
	static globalConfig::CGlobalConfig  _self;
	return _self;
}

// default constructor to initialize default values
globalConfig::COperation::COperation()
{
	m_isRT = false;
	m_operationPriority = 0;
	m_qos = 0;
	m_retries = 0;
}

// default constructor to initialize default values
globalConfig::COperationInfo::COperationInfo()
{
	m_opType = UNKNOWN_OPERATION;
	m_defaultIsRT = false;
}

/**
 * Display scheduler attributes
 * @param policy : [in] scheduler policy
 * @param param : [in] param to display thread priority
 */
void globalConfig::display_sched_attr(int policy, struct sched_param& param)
{
	string sPolicy = "";
	if(policy == SCHED_FIFO)
	{
		sPolicy = "FIFO";
	}
	else if(policy == SCHED_RR)
	{
		sPolicy = "RR";
	}
	else if(policy == SCHED_OTHER)
	{
		sPolicy = "SCHED_OTHER";
	}
	else
	{
		sPolicy = "UNKNOWN";
	}
	CLogger::getInstance().log(INFO, LOGDETAILS("policy = " + sPolicy +
			" priority = "+ to_string(param.sched_priority)));
}

/**
 * Display thread attributes
 * @param a_sMsg : [in] string msg
 */
void globalConfig::display_thread_sched_attr(const std::string a_sMsg)
{
	int policy, s;
	static int count = 1;
	struct sched_param param;

	s = pthread_getschedparam(pthread_self(), &policy, &param);
	if (s != 0)
	{
		handle_error_en(s, "pthread_getschedparam");
	}
	CLogger::getInstance().log(INFO, LOGDETAILS("Thread number :: " + to_string(count++) +" Thread Name :: " + a_sMsg));

	display_sched_attr(policy, param);
}

/**
 * Constructor - fill up values in map with priority values
 */
globalConfig::CPriorityMgr::CPriorityMgr()
{
	//initialize all the default parameters
	m_opPriorityThreadInfo.insert(std::make_pair(6, std::make_pair(60, RR)));
	m_opPriorityThreadInfo.insert(std::make_pair(5, std::make_pair(55, RR)));
	m_opPriorityThreadInfo.insert(std::make_pair(4, std::make_pair(50, RR)));
	m_opPriorityThreadInfo.insert(std::make_pair(3, std::make_pair(15, RR)));
	m_opPriorityThreadInfo.insert(std::make_pair(2, std::make_pair(10, RR)));
	m_opPriorityThreadInfo.insert(std::make_pair(1, std::make_pair(5, RR)));
}

/** Function to set thread parameters
 ** @param : a_OpsInfo [in]: Operation class reference,
 ** @param : a_iPriority [in]: optional parameter for priority other than defined operation priority
 ** @param : a_eSched [in]: optional parameter for scheduler other than defined operation priority
 ** param : a_bIsOperation [in]: optional value. (if this flag is set then only a_iPriority & a_eSched will be used)
 * @return: Nothing
 */
void globalConfig::set_thread_sched_param(const COperation a_OpsInfo,
		const int a_iPriority,
		const eThreadScheduler a_eSched,
		const bool a_bIsOperation)
{
	int iThreadPriority = 0;
	int operationPriority = 0;
	globalConfig::eThreadScheduler threadPolicy;

	if(!a_bIsOperation)
	{
		operationPriority = a_OpsInfo.getOperationPriority();
		iThreadPriority = CPriorityMgr::getInstance().getThreadPriority(operationPriority);
		threadPolicy = CPriorityMgr::getInstance().getThreadScheduler(operationPriority);
	}
	else
	{
		iThreadPriority = a_iPriority;
		threadPolicy = a_eSched;
	}

	//set priority
	sched_param param;

	if(iThreadPriority == -1 || threadPolicy == UNKNOWN)
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS("Failed to set thread priority for this thread"));
		CLogger::getInstance().log(ERROR, LOGDETAILS("Invalid operation priority received for thread set priority"));
		return;
	}

	param.sched_priority = iThreadPriority;

	int result;
	result = pthread_setschedparam(pthread_self(), threadPolicy, &param);
	if(0 != result)
	{
		handle_error_en(result, "pthread_setschedparam");
		CLogger::getInstance().log(ERROR, LOGDETAILS("Cannot set thread priority to : " + std::to_string(iThreadPriority)));
		std::cout << __func__ << ":" << __LINE__ << " Cannot set thread priority, result : " << result << std::endl;
	}
	else
	{
		CLogger::getInstance().log(INFO, LOGDETAILS("thread priority to set to: " + std::to_string(iThreadPriority)));
	}
	//end of set priority for current thread
}

/** Function to set default value based on operation type
 ** @param : a_eOpType [in]: 	operation type (POLLING,ON_DEMAND_READ,	ON_DEMAND_WRITE)
 * @return: Nothing
 */
void globalConfig::setDefaultConfig(const enum eOperationType a_eOpType)
{
	/// set to default value based on operation type
	YAML::Node defaultNode;
	COperationInfo::buildOperationInfo(defaultNode,
			globalConfig::CGlobalConfig::getInstance().getOpPollingOpConfig(),
			a_eOpType);
}

/** Function to validate key in YAML file
 *
 * @param : baseNode [in] : YAML node to read from
 * @param : a_sKey [in] : key to validate
 * @param : a_eDataType [in] : data type
 * @return: [int] : 0- success & -1 - if key is empty or absent & -2 if incorrect data type
 */
int globalConfig::validateParam(const YAML::Node& a_BaseNode,
		const std::string& a_sKey,
		const enum eDataType a_eDataType)
{
	int iRet = 0;
	try
	{
		if(!(a_BaseNode[a_sKey])  || a_BaseNode[a_sKey].as<std::string>() == "")
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS(a_sKey + " is not present or empty !!"));
			CLogger::getInstance().log(ERROR, LOGDETAILS("setting it to default !!"));
			iRet = -1;
		}
		else
		{
			switch (a_eDataType)
			{
				case DT_BOOL:
				{
					a_BaseNode[a_sKey].as<bool>();
				}
				break;
				case DT_INTEGER:
				{
					a_BaseNode[a_sKey].as<int>();
				}
				break;
				case DT_UNSIGNED_INT:
				{
					a_BaseNode[a_sKey].as<unsigned>();
				}
				break;
				case DT_STRING:
				{
					a_BaseNode[a_sKey].as<std::string>();
				}
				break;
				case DT_MAP:
				{
					a_BaseNode[a_sKey].as<std::map<string,string>>();
				}
				break;
				default:
				{
					CLogger::getInstance().log(ERROR, LOGDETAILS("Invalid data type::" + to_string(a_eDataType)));
					iRet = -3;
				}
				break;
			}
		}
	}
	catch (YAML::Exception &e)
	{
		CLogger::getInstance().log(ERROR, LOGDETAILS( a_sKey +
				" field data type is invalid, setting it to default, exception :: " +	std::string(e.what())));
		iRet = -2;
	}
	return iRet;
}
/** Populate COperation data structures
 *
 * @param : a_baseNode [in] : YAML node to read from
 * @param : a_refOpration [in] : data structure to be fill
 * @param : m_isRT [in] : RT/Noon-RT
 * @return: Nothing
 */
void globalConfig::COperation::build(const YAML::Node& a_baseNode,
		COperation& a_refOpration,
		const bool a_isRT)
{

	string ops = "";
	if(!a_isRT)
	{
		cout << "non-realtime parameters: " << endl;
		CLogger::getInstance().log(INFO, LOGDETAILS("non-realtime parameters: "));
		ops = "non-realtime";
	}
	else
	{
		cout << "realtime parameters: " << endl;
		CLogger::getInstance().log(INFO, LOGDETAILS("realtime parameters: "));
		ops = "realtime";
	}

	/// validate priority
	if (validateParam(a_baseNode[ops], "operation_priority", DT_INTEGER) != 0)
	{
		// default value
		a_refOpration.m_operationPriority = DEFAULT_OPERATION_PRIORITY;
	}
	else
	{
		if(a_baseNode[ops]["operation_priority"].as<int>() <= 0 ||
				a_baseNode[ops]["operation_priority"].as<int>() > 6)
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS(
					"operation_priority parameter is out of range (i.e. expected value must be between 1-6 inclusive ) setting it to default (i.e. 1)"));
			a_refOpration.m_operationPriority = DEFAULT_OPERATION_PRIORITY;
		}
		else
		{
			a_refOpration.m_operationPriority = a_baseNode[ops]["operation_priority"].as<int>();
		}
	}

	// validate retries
	if (validateParam(a_baseNode[ops], "retries", DT_INTEGER) != 0)
	{
		// default value
		a_refOpration.m_retries = DEFAULT_RETRIES;
	}
	else
	{
		if(a_baseNode[ops]["retries"].as<int>() < 0 ||
				a_baseNode[ops]["retries"].as<int>() > 4)
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS(
					"retries parameter is out of range (i.e. expected value must be between 0-4 inclusive) setting it to default (i.e. 0)"));
			a_refOpration.m_retries = DEFAULT_RETRIES;
		}
		else
		{
			a_refOpration.m_retries = a_baseNode[ops]["retries"].as<int>();
		}
	}

	// validate qos
	if (validateParam(a_baseNode[ops], "qos", DT_INTEGER) != 0)
	{
		// default value
		a_refOpration.m_qos = DEFAULT_QOS;
	}
	else
	{
		if(a_baseNode[ops]["qos"].as<int>() < 0 ||
				a_baseNode[ops]["qos"].as<int>() > 2)
		{
			CLogger::getInstance().log(ERROR, LOGDETAILS(
					"qos parameter is out of range (i.e. expected value must be between 0-2 inclusive) setting it to default (i.e. 0)"));
			a_refOpration.m_qos = DEFAULT_QOS;
		}
		else
		{
			a_refOpration.m_qos = a_baseNode[ops]["qos"].as<int>();
		}
	}

	cout << "	operation priority :: " <<a_refOpration.getOperationPriority()<< endl;
	cout << "	retries :: " <<a_refOpration.getRetries()<< endl;
	CLogger::getInstance().log(INFO, LOGDETAILS("operation priority :: " + to_string(a_refOpration.getOperationPriority())));
	CLogger::getInstance().log(INFO, LOGDETAILS("retries :: " + to_string(a_refOpration.getRetries())));
}

/** Populate COperation data structures
 *
 * @param : a_baseNode [in] : YAML node to read from
 * @param : a_refOpInfo [in] : data structure to be fill
 * @param : a_eOpType [in]: 	operation type (POLLING,ON_DEMAND_READ,	ON_DEMAND_WRITE)
 * @return: Nothing
 */
void globalConfig::COperationInfo::buildOperationInfo(const YAML::Node& a_baseNode,
		COperationInfo& a_refOpInfo,
		const eOperationType a_eOpType)
{
	/// fill out operationType and default real-time
	a_refOpInfo.m_opType = a_eOpType;
	if (validateParam(a_baseNode, "default_realtime", DT_BOOL) != 0)
	{
		a_refOpInfo.m_defaultIsRT = DEFAULT_REALTIME;
	}
	else
	{
		a_refOpInfo.m_defaultIsRT = a_baseNode["default_realtime"].as<bool>();
	}
	cout << " default RT :: " << a_refOpInfo.m_defaultIsRT << endl;
	CLogger::getInstance().log(INFO, LOGDETAILS(" default RT :: " + to_string(a_refOpInfo.m_defaultIsRT)));

	COperation Obj;
	/// realtime
	Obj.build(a_baseNode, a_refOpInfo.getRTConfig(), true);

	/// non-realtime
	Obj.build(a_baseNode, a_refOpInfo.getNonRTConfig(), false);
}


/** Read global configurations from YAML file (Global_Config.yml)
 *  global configuration is available in common_config dir from docker volume
 *
 *  if any configuration is missing/invalid then following configuration will be used as a default one,
	#	default_realtime: false
	#	operation_priority: 1
	#	retries: 0
	#	qos: 0
 *
 * @return: true/false - based on success/failure
 */
bool globalConfig::loadGlobalConfigurations()
{
	bool bRetVal = false;
	bool isPollingExist = false;
	bool isOdReadExist = false;
	bool isOdWriteExist = false;

	try
	{
		YAML::Node config = YAML::LoadFile(GLOBAL_CONFIG_FILE_PATH);
		for (auto it : config)
		{
			if(it.first.as<std::string>() == "Global")
			{
				YAML::Node ops = it.second;
				YAML::Node listOps = ops["Operations"];
				for (auto key : listOps)
				{
					YAML::Node node;
					if(key["Polling"])
					{
						node = key["Polling"];

						cout << "********************************************************************";
						CLogger::getInstance().log(INFO, LOGDETAILS("\nFollowing Global configurations is available inside container"));
						cout << "\nFollowing Global configurations is available inside container \n";
						cout << "********************************************************************\n";
						CLogger::getInstance().log(INFO, LOGDETAILS("For Polling >>>"));
						cout << "For Polling >>>\n";
						COperationInfo::buildOperationInfo(node,
								globalConfig::CGlobalConfig::getInstance().getOpPollingOpConfig(),
								POLLING);
						isPollingExist = true;
						continue;
					}
					else if (key["on-demand-read"])
					{
						node = key["on-demand-read"];
						cout << "For On-demand read >>>\n";
						CLogger::getInstance().log(INFO, LOGDETAILS("For On-demand read >>>"));
						COperationInfo::buildOperationInfo(node,
								globalConfig::CGlobalConfig::getInstance().getOpOnDemandReadConfig(),
								ON_DEMAND_READ);
						isOdReadExist= true;
						continue;
					}
					else if (key["on-demand-write"])
					{
						node = key["on-demand-write"];
						cout << "For On-demand write >>>\n";
						CLogger::getInstance().log(INFO, LOGDETAILS("For On-demand write >>>"));
						COperationInfo::buildOperationInfo(node,
								globalConfig::CGlobalConfig::getInstance().getOpOnDemandWriteConfig(),
								ON_DEMAND_WRITE);
						isOdWriteExist= true;
						continue;
					}
				}
			}
		}
	}
	catch (YAML::Exception& e)
	{
		cout << "Error while loading global configurations :: " << e.what()<< "\n";
		CLogger::getInstance().log(ERROR, LOGDETAILS("Error while loading global configurations::" + std::string(e.what())));
		bRetVal = false;
	}

	if(isPollingExist && isOdReadExist && isOdWriteExist)
	{
		// success case
		bRetVal = true;
	}

	// check for errors
	if(!isPollingExist)
	{
		CLogger::getInstance().log(ERROR,LOGDETAILS("Polling key is missing"));
		CLogger::getInstance().log(INFO, LOGDETAILS("Setting default config for Polling >>>"));
		cout << "Setting default config for Polling >>>\n";
		bRetVal = false;
		setDefaultConfig(POLLING);
	}
	if (!isOdReadExist)
	{
		CLogger::getInstance().log(ERROR,LOGDETAILS("on-demand-read key is missing"));
		CLogger::getInstance().log(INFO, LOGDETAILS("Setting default config for On-Demand-Read >>>"));
		cout << "Setting default config for On-Demand-Read >>>\n";
		bRetVal = false;
		setDefaultConfig(ON_DEMAND_READ);
	}
	if (!isOdWriteExist)
	{
		CLogger::getInstance().log(ERROR,LOGDETAILS("on-demand-write key is missing"));
		CLogger::getInstance().log(INFO, LOGDETAILS("Setting default config for On-Demand-Write >>>"));
		cout << "Setting default config for On-Demand-Write >>>\n";
		bRetVal = false;
		setDefaultConfig(ON_DEMAND_WRITE);
	}
	return bRetVal;
}
